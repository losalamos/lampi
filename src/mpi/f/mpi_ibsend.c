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


#include "internal/mpif.h"

void mpi_ibsend_f(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    MPI_Datatype c_type = MPI_Type_f2c(*type);
    MPI_Request c_req;

    *rc = MPI_Ibsend(buf, *count, c_type, *dest, *tag, *comm, &c_req);

    if (*rc == MPI_SUCCESS) {
        *req = MPI_Request_c2f(c_req);
    }
}

#if defined(HAVE_PRAGMA_WEAK)

#pragma weak PMPI_IBSEND = mpi_ibsend_f
#pragma weak pmpi_ibsend = mpi_ibsend_f
#pragma weak pmpi_ibsend_ = mpi_ibsend_f
#pragma weak pmpi_ibsend__ = mpi_ibsend_f

#pragma weak MPI_IBSEND = mpi_ibsend_f
#pragma weak mpi_ibsend = mpi_ibsend_f
#pragma weak mpi_ibsend_ = mpi_ibsend_f
#pragma weak mpi_ibsend__ = mpi_ibsend_f

#else

void PMPI_IBSEND(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void pmpi_ibsend(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void pmpi_ibsend_(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void pmpi_ibsend__(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void MPI_IBSEND(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void mpi_ibsend(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void mpi_ibsend_(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

void mpi_ibsend__(void *buf, MPI_Fint *count, MPI_Fint *type,
                  MPI_Fint *dest, MPI_Fint *tag, MPI_Comm *comm,
                  MPI_Fint *req, MPI_Fint *rc)
{
    mpi_ibsend_f(buf, count, type, dest, tag, comm,
                 req, rc);
}

#endif
