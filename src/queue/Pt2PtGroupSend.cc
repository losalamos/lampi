/*
 * Copyright 2002-2003. The Regents of the University of California. This material 
 * was produced under U.S. Government contract W-7405-ENG-36 for Los Alamos 
 * National Laboratory, which is operated by the University of California for 
 * the U.S. Department of Energy. The Government is granted for itself and 
 * others acting on its behalf a paid-up, nonexclusive, irrevocable worldwide 
 * license in this material to reproduce, prepare derivative works, and 
 * perform publicly and display publicly. Beginning five (5) years after 
 * October 10,2002 subject to additional five-year worldwide renewals, the 
 * Government is granted for itself and others acting on its behalf a paid-up, 
 * nonexclusive, irrevocable worldwide license in this material to reproduce, 
 * prepare derivative works, distribute copies to the public, perform publicly 
 * and display publicly, and to permit others to do so. NEITHER THE UNITED 
 * STATES NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR THE UNIVERSITY OF 
 * CALIFORNIA, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY, EXPRESS OR 
 * IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, 
 * COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR 
 * PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY 
 * OWNED RIGHTS.

 * Additionally, this program is free software; you can distribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 2 of the License, 
 * or any later version.  Accordingly, this program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Lesser General Public License for more details.
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/



#include <stdio.h>

#include "queue/Communicator.h"
#include "ulm/ulm.h"
#include "mpi/mpi.h"

#include "queue/globals.h"

#include "internal/buffer.h"
#include "internal/constants.h"
#include "internal/profiler.h"
#include "internal/types.h"
#include "internal/log.h"
#include "client/ULMClient.h"
#include "queue/globals.h"
#include "path/common/InitSendDescriptors.h"
#include "path/common/path.h"

//
// Initial attempt to send data.  This is the when the send descriptor is
//  first put in the libraries internal messaging queues.
//
int Communicator::isend_start(ULMRequestHandle_t *request)
{
    // create pointer to ULM request object
    RequestDesc_t *tmpRequest = (RequestDesc_t *) (*request);
    BaseSendDesc_t *SendDescriptor ;
    int errorCode = ULM_SUCCESS;

    // make sure that he request object is in the inactive state
    if (tmpRequest->status == ULM_STATUS_INVALID) {
        ulm_err(("Error: ULM request structure has invalid status\n"
                 "  Status :: %d\n", tmpRequest->status));
        return ULM_ERR_REQUEST;
    }

    /* for buffered send copy data from user space into
     *   the "buffer"
     */
    if ((tmpRequest->sendType == ULM_SEND_BUFFERED) && 
        (tmpRequest->persistent)) {
        int offset = 0;
        errorCode = PMPI_Pack(tmpRequest->appBufferPointer,
                       tmpRequest->bsendDtypeCount, 
		       (MPI_Datatype)tmpRequest->bsendDtypeType,
                       tmpRequest->pointerToData,
                       tmpRequest->bsendBufferSize, &offset, contextID);
        if (errorCode != MPI_SUCCESS) {
            return errorCode;
        }
    }
    // set sent completion to false
    tmpRequest->messageDone = false;

    // bind send descriptor to a given path....this can fail...
    errorCode = (*pt2ptPathSelectionFunction) (tmpRequest, (void **) 
		    (&SendDescriptor) );
    if (errorCode != ULM_SUCCESS) {
        SendDescriptor->path_m->ReturnDesc(SendDescriptor);
        return errorCode;
    }

    // set the clear to send flag - depending on send mode
    if (tmpRequest->sendType != ULM_SEND_SYNCHRONOUS) {
        SendDescriptor->clearToSend_m = true;
    } else {
        SendDescriptor->clearToSend_m = false;
    }

    // set the pointer to the request object and the send type
    // needed for ReturnDesc() to do the right thing for bsends...
    SendDescriptor->requestDesc = tmpRequest;
    SendDescriptor->sendType = tmpRequest->sendType;

    // for buffered send, send is already complete, since the data resides
    // in the buffered region by this stage; also we need to find our buffer
    // allocation and increment the reference count for this send descriptor
    if (tmpRequest->sendType == ULM_SEND_BUFFERED) {
        SendDescriptor->bsendOffset = (ssize_t)((long)tmpRequest->pointerToData - (long)lampiState.bsendData->buffer);
        SendDescriptor->PostedLength = tmpRequest->posted_m.length_m;
        if ((SendDescriptor->PostedLength > 0) && !ulm_bsend_increment_refcount(*request, 
                SendDescriptor->bsendOffset)) {
            SendDescriptor->path_m->ReturnDesc(SendDescriptor);
            return ULM_ERR_FATAL;
        }
        tmpRequest->messageDone = true;
    }

    // set the destination process ID, communicator ID, user tag, and length of the message
    // and other fields not specific to a given path...
    SendDescriptor->dstProcID_m =
        tmpRequest->posted_m.proc.destination_m;
    SendDescriptor->tag_m = tmpRequest->posted_m.UserTag_m;
    SendDescriptor->PostedLength = tmpRequest->posted_m.length_m;
    SendDescriptor->ctx_m = tmpRequest->ctx_m;
    SendDescriptor->datatype = tmpRequest->datatype;
    SendDescriptor->NumAcked = 0;
    SendDescriptor->NumSent = 0;
    SendDescriptor->NumFragDescAllocated = 0;
    SendDescriptor->numFragsCopiedIntoLibBufs_m = 0;
    SendDescriptor->sendDone =
        (SendDescriptor->sendType == ULM_SEND_BUFFERED) ? 1 : 0;
#ifdef RELIABILITY_ON
    SendDescriptor->earliestTimeToResend = -1.0;
#endif

    if (SendDescriptor->PostedLength)
        SendDescriptor->AppAddr = tmpRequest->pointerToData;
    else
        SendDescriptor->AppAddr = 0;

    // path-specific initialization of descriptor for fields like numfrags...
    SendDescriptor->path_m->init(SendDescriptor);

    // fill in request
    tmpRequest->requestDesc = (void *) SendDescriptor;

    // change status
    tmpRequest->status = ULM_STATUS_INCOMPLETE;

    // lock descriptor, so if data gets sent and is acked before we have a chance to
    //   post the descriptor on either the IncompletePostedSends or UnackedPostedSends list,
    //   we don't try to move it between lists.
    //
    // get sequence number...now that we can't fail for ordinary reasons...
    //
    unsigned long seq;
    if(SendDescriptor->path_m->pathType_m != SHAREDMEM) {
	    if (usethreads())
		    next_isendSeqsLock[tmpRequest->posted_m.proc.destination_m].
			    lock();
	    seq = next_isendSeqs[tmpRequest->posted_m.proc.destination_m]++;
	    if (usethreads())
		    next_isendSeqsLock[tmpRequest->posted_m.proc.destination_m].
			    unlock();
	    // set sequence number
	    tmpRequest->sequenceNumber_m = seq;
	    SendDescriptor->isendSeq_m = seq;
    }

    if (usethreads())
        SendDescriptor->Lock.lock();

    // start sending data
    int sendReturn;
    bool incomplete;
    while (!SendDescriptor->path_m->
           send(SendDescriptor, &incomplete, &sendReturn)) {
        if (sendReturn == ULM_ERR_BAD_PATH) {
// revisit            // unbind from the current path
// revisit            SendDescriptor->path_m->unbind(SendDescriptor, (int *) 0, 0);
// revisit            // select a new path, if at all possible
// revisit            sendReturn =
// revisit                (*pt2ptPathSelectionFunction) ((void *) SendDescriptor);
// revisit            if (sendReturn != ULM_SUCCESS) {
// revisit                if (usethreads())
// revisit                    SendDescriptor->Lock.unlock();
// revisit                SendDescriptor->path_m->ReturnDesc(SendDescriptor);
// revisit                return sendReturn;
// revisit            }
// revisit            // initialize the descriptor for this path
// revisit            SendDescriptor->path_m->init(SendDescriptor);
		return ULM_ERROR;
        } else {
            // unbind should empty SendDescriptor of frag descriptors...
            SendDescriptor->path_m->unbind(SendDescriptor, (int *) 0, 0);
            if (usethreads())
                SendDescriptor->Lock.unlock();
            SendDescriptor->path_m->ReturnDesc(SendDescriptor);
            return sendReturn;
        }
    }

    // push record onto the appropriate list
    if (incomplete) {
        // We're going to have to push this along later, so save it.
        SendDescriptor->WhichQueue = INCOMPLETEISENDQUEUE;
        IncompletePostedSends.Append(SendDescriptor);
    } else {
        SendDescriptor->WhichQueue = UNACKEDISENDQUEUE;
        UnackedPostedSends.Append(SendDescriptor);
    }
    // unlock descriptor
    if (usethreads())
        SendDescriptor->Lock.unlock();

    return ULM_SUCCESS;
}
