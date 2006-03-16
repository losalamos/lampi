/*
 * Copyright 2002-2004. The Regents of the University of
 * California. This material was produced under U.S. Government
 * contract W-7405-ENG-36 for Los Alamos National Laboratory, which is
 * operated by the University of California for the U.S. Department of
 * Energy. The Government is granted for itself and others acting on
 * its behalf a paid-up, nonexclusive, irrevocable worldwide license
 * in this material to reproduce, prepare derivative works, and
 * perform publicly and display publicly. Beginning five (5) years
 * after October 10,2002 subject to additional five-year worldwide
 * renewals, the Government is granted for itself and others acting on
 * its behalf a paid-up, nonexclusive, irrevocable worldwide license
 * in this material to reproduce, prepare derivative works, distribute
 * copies to the public, perform publicly and display publicly, and to
 * permit others to do so. NEITHER THE UNITED STATES NOR THE UNITED
 * STATES DEPARTMENT OF ENERGY, NOR THE UNIVERSITY OF CALIFORNIA, NOR
 * ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
 * ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE ACCURACY,
 * COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT,
 * OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
 * PRIVATELY OWNED RIGHTS.

 * Additionally, this program is free software; you can distribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or any later version.  Accordingly, this
 * program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "internal/mpi.h"

#ifdef HAVE_PRAGMA_WEAK
#pragma weak MPI_Wait = PMPI_Wait
#endif

int PMPI_Wait(MPI_Request *request, MPI_Status *status)
{
    ULMStatus_t stat;
    int rc;

    if (_mpi.check_args) {
        rc = MPI_SUCCESS;
        if (_mpi.finalized) {
            rc = MPI_ERR_INTERN;
        } else if (ulm_invalid_request(request)) {
            rc = MPI_ERR_REQUEST;
        }
        if (rc != MPI_SUCCESS) {
            goto ERRHANDLER;
        }
    }

    if (request == NULL || *request == MPI_REQUEST_NULL) {
        if (status != MPI_STATUS_IGNORE) {
            memset(status, 0, sizeof(MPI_Status));
        }
        return MPI_SUCCESS;
    }

    if (*request == _mpi.proc_null_request ||
        *request == _mpi.proc_null_request_persistent) {
        if (status != MPI_STATUS_IGNORE) {
	    status->MPI_ERROR = MPI_SUCCESS;
	    status->MPI_SOURCE = MPI_PROC_NULL;
	    status->MPI_TAG = MPI_ANY_TAG;
	    status->_count = 0;
	    status->_persistent = 0;
	}
        if (*request == _mpi.proc_null_request) {
            *request = MPI_REQUEST_NULL;
        }
	return MPI_SUCCESS;
    }

    rc = ulm_wait((ULMRequest_t *) request, &stat);
    if (rc != ULM_SUCCESS) {
        if (status != MPI_STATUS_IGNORE) {
	    status->MPI_ERROR = _mpi_error(stat.error_m);
	    status->MPI_SOURCE = stat.peer_m;
	    status->MPI_TAG = stat.tag_m;
	    status->_count = stat.length_m;
	    status->_persistent = stat.persistent_m;
	    rc = status->MPI_ERROR;
	} else {
	    rc = _mpi_error(rc);
	}
        goto ERRHANDLER;
    }
    if (status != MPI_STATUS_IGNORE) {
	status->MPI_ERROR = _mpi_error(stat.error_m);
	status->MPI_SOURCE = stat.peer_m;
	status->MPI_TAG = stat.tag_m;
	status->_count = stat.length_m;
	status->_persistent = stat.persistent_m;
    }

ERRHANDLER:
    if (rc != MPI_SUCCESS) {
        _mpi_errhandler(MPI_COMM_WORLD, rc, __FILE__, __LINE__);
        return rc;
    }

    return MPI_SUCCESS;
}
