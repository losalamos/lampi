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

#include "internal/mpif.h"

void mpi_type_ub_f(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    MPI_Datatype c_type;
    MPI_Aint c_ub;

    c_type = MPI_Type_f2c(*f_type);

    *rc = MPI_Type_ub(c_type, &c_ub);
    *f_ub = (MPI_Fint) c_ub & 0xffffffff;
}

#ifdef HAVE_PRAGMA_WEAK

#pragma weak PMPI_TYPE_UB = mpi_type_ub_f
#pragma weak pmpi_type_ub = mpi_type_ub_f
#pragma weak pmpi_type_ub_ = mpi_type_ub_f
#pragma weak pmpi_type_ub__ = mpi_type_ub_f

#pragma weak MPI_TYPE_UB = mpi_type_ub_f
#pragma weak mpi_type_ub = mpi_type_ub_f
#pragma weak mpi_type_ub_ = mpi_type_ub_f
#pragma weak mpi_type_ub__ = mpi_type_ub_f

#else

void PMPI_TYPE_UB(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void pmpi_type_ub(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void pmpi_type_ub_(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void pmpi_type_ub__(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void MPI_TYPE_UB(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void mpi_type_ub(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void mpi_type_ub_(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

void mpi_type_ub__(MPI_Fint *f_type, MPI_Fint *f_ub, MPI_Fint *rc)
{
    mpi_type_ub_f(f_type, f_ub, rc);
}

#endif
