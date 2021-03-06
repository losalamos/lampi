/*
 * Copyright 2002-2003. The Regents of the University of
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

/*!
 * MPI_Probe - wait for a receivable message
 *
 * \param source	Source proc or MPI_ANY_SOURCE
 * \param tag		Tag to match of MPI_ANY_TAG
 * \param comm		Communicator handle
 * \param status	MPI status object to be filled in
 * \return		MPI return code
 */

#ifdef HAVE_PRAGMA_WEAK
#pragma weak MPI_Probe = PMPI_Probe
#endif

int PMPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    ULMStatus_t stat;
    int rc = ULM_SUCCESS;
    int flag = 0;
    int first = 1;

    if (_mpi.check_args) {
        rc = MPI_SUCCESS;
        if (_mpi.finalized) {
            rc = MPI_ERR_INTERN;
        } else if (ulm_invalid_comm(comm)) {
            rc = MPI_ERR_COMM;
        } else if (tag < MPI_ANY_TAG || tag > MPI_TAG_UB_VALUE) {
            rc = MPI_ERR_TAG;
        } else if (ulm_invalid_source(comm, source)) {
            rc = MPI_ERR_RANK;
        }
        if (rc != MPI_SUCCESS) {
            goto ERRHANDLER;
        }
    }

    while (flag == 0) {
	rc = ulm_iprobe(source, comm, tag, &flag, &stat);
        if (0) {
            if (first && (flag == 0)) {
                ulm_err(("MPI_Probe: ulm_iprobe(peer=%d, tag=%d) spinning...\n", source, tag));
                first = 0;
            } else if (flag) {
                ulm_err(("MPI_Probe: ulm_iprobe(peer=%d, tag=%d) succeeded. "
                         "status.(peer=%d, tag=%d, length=%d)\n",
                         source, tag, stat.peer_m, stat.tag_m, stat.length_m));
            }
        }
    }

    if (status != MPI_STATUS_IGNORE) {
        status->MPI_SOURCE = stat.peer_m;
        status->MPI_TAG = stat.tag_m;
        status->MPI_ERROR = _mpi_error(stat.error_m);
        status->_count = stat.length_m;
        status->_persistent = stat.persistent_m;
        status->_state = stat.state_m;
    }

    rc =  (rc == ULM_SUCCESS) ? MPI_SUCCESS : _mpi_error(rc);

ERRHANDLER:
    if (rc != MPI_SUCCESS) {
        _mpi_errhandler(comm, rc, __FILE__, __LINE__);
    }

    return rc;
}
