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
#include "internal/buffer.h"
#include "internal/state.h"

#ifdef HAVE_PRAGMA_WEAK
#pragma weak MPI_Buffer_detach = PMPI_Buffer_detach
#endif

int PMPI_Buffer_detach(void *buffer, int *size)
{
    char **tmpPtr;
    int rc = MPI_SUCCESS;
    int wantToDetach = 1;

    /* lock control structure */
    ATOMIC_LOCK_THREAD(lampiState.bsendData->lock);

    /* check to make sure pool is in use */
    if (!(lampiState.bsendData->poolInUse)) {

	/*
	 * pool not in use
	 */

	/* set error code */
	rc = MPI_ERR_BUFFER;

	/* unlock control structure */
        ATOMIC_UNLOCK_THREAD(lampiState.bsendData->lock);

	_mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
	return rc;
    }

    /* spin until bytes in use is 0  - this is a blocking call */
    while (lampiState.bsendData->bytesInUse > 0) {
        ATOMIC_UNLOCK_THREAD(lampiState.bsendData->lock);
        rc = ulm_make_progress();
        if (rc != ULM_SUCCESS) {
            rc = _mpi_error(rc);
            _mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
            return rc;
        }
        ATOMIC_LOCK_THREAD(lampiState.bsendData->lock);
        ulm_bsend_clean_alloc(wantToDetach);
    }

    /* set size - return value */
    *size = lampiState.bsendData->bufferLength;

    /* set  buffer - return value */
    tmpPtr = (char **) buffer;
    *tmpPtr = (char *) lampiState.bsendData->buffer;

    /* reset in use flag */
    lampiState.bsendData->poolInUse = 0;

    /* set internal buffer pointer to 0 */
    lampiState.bsendData->buffer = (void *) 0;

    /* set internal buffer length to 0 */
    lampiState.bsendData->bufferLength = 0;

    /* all allocations should be gone; otherwise there is a problem */
    if (lampiState.bsendData->allocations != NULL) {
        ATOMIC_UNLOCK_THREAD(lampiState.bsendData->lock);
        rc = MPI_ERR_INTERN;
        _mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
        return rc;
    }

    /* unlock control structure */
    ATOMIC_UNLOCK_THREAD(lampiState.bsendData->lock);

    return rc;
}
