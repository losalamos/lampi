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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "internal/mpi.h"
#include "internal/collective.h"

#ifdef HAVE_PRAGMA_WEAK
#pragma weak MPI_Gatherv = PMPI_Gatherv
#endif

int PMPI_Gatherv(void *sendbuf, int sendcount, MPI_Datatype senddatatype,
		 void *recvbuf, int *recvcounts, int *displs,
		 MPI_Datatype recvdatatype, int root, MPI_Comm comm)
{
    int i;
    int size;
    int rc;
    ULMType_t *sendtype;
    ULMType_t *recvtype;
    ulm_gatherv_t *gatherv;

    if (_mpi.check_args) {
        rc = MPI_SUCCESS;
        if (_mpi.finalized) {
            rc = MPI_ERR_INTERN;
        } else if (senddatatype == MPI_DATATYPE_NULL) {
            rc = MPI_ERR_TYPE;
        } else if (sendcount < 0) {
            rc = MPI_ERR_COUNT;
        } else if (ulm_invalid_comm(comm)) {
            rc = MPI_ERR_COMM;
        } else if (ulm_invalid_source(comm, root)) {
            rc = MPI_ERR_RANK;
        } else if ((sendbuf == NULL) && (sendcount > 0)) {
            rc = MPI_ERR_BUFFER;
        } else if (ulm_am_i(comm, root)) {
            if (recvdatatype == MPI_DATATYPE_NULL) {
                rc = MPI_ERR_TYPE;
            } else if (recvcounts == NULL) {
                rc = MPI_ERR_COUNT;
            } else if (displs == NULL) {
                rc = MPI_ERR_DISP;
            } else if (recvbuf == NULL) {
                rc = PMPI_Comm_size(comm, &size);
                if (MPI_SUCCESS == rc) {
                    for (i = 0; i < size; i++) {
                        if (recvcounts[i] != 0) {
                            rc = MPI_ERR_BUFFER;
                        }
                    }
                }
            }
        }
        if (rc != MPI_SUCCESS) {
            goto ERRHANDLER;
        }
    }

    sendtype = (ULMType_t *) senddatatype;
    recvtype = (ULMType_t *) recvdatatype;

    rc = ulm_comm_get_collective(comm, ULM_COLLECTIVE_GATHERV,
                                 (void **) &gatherv);
    if (ULM_SUCCESS == rc) {
        rc = gatherv(sendbuf, sendcount, sendtype,
                     recvbuf, recvcounts, displs, recvtype, root, comm);
    }
    rc = (rc == ULM_SUCCESS) ? MPI_SUCCESS : _mpi_error(rc);

  ERRHANDLER:
    if (rc != MPI_SUCCESS) {
        _mpi_errhandler(comm, rc, __FILE__, __LINE__);
    }

    return rc;
}
