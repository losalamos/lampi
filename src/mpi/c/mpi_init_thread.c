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



#include "internal/mpi.h"
#include "internal/mpif.h"

#include "internal/cLock.h"

#pragma weak MPI_Init_thread = PMPI_Init_thread
int PMPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    int rc;

    if (_mpi.finalized) {
	rc = MPI_ERR_INTERN;
	_mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
	return rc;
    }

    if( required == MPI_THREAD_SINGLE || required == MPI_THREAD_FUNNELED ||
		    required == MPI_THREAD_SERIALIZED ) {
	    /* no concurent threads - don't need to lock in order to avoid
	     *   thread race conditions */
	    rc = ulm_init_threads(0);
	    if (rc != ULM_SUCCESS) {
		    return MPI_ERR_INTERN;
	    }
	    *provided=required;
    } else if( required == MPI_THREAD_MULTIPLE ) {
	    /* concurent threads */
	    rc = ulm_init_threads(1);
	    if (rc != ULM_SUCCESS) {
		    return MPI_ERR_INTERN;
	    }
	    *provided=required;
    } else {
	rc = MPI_ERR_ARG;
	_mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
	return rc;
    }

    /* set MPI thread usage flag - MPI defines more parameters than
     *   ulm uses internally */
    _mpi.thread_usage=required;


    /* barrier to make sure initialization is complete */
    ulm_barrier(ULM_COMM_WORLD);

    rc = _mpi_init();
    if (rc) {
	return MPI_ERR_INTERN;
    }

    if (_MPI_FORTRAN) {
        rc = _mpif_init();
        if (rc) {
            return MPI_ERR_INTERN;
        }
    }

    return MPI_SUCCESS;
}
