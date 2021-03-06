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



#ifndef _BARRIER
#define _BARRIER

//! structure defining data needed for fetch and add barrier with
//!   hardware supporting atomic variable access

#include "util/Lock.h"

typedef struct {
    int commSize;                    //! number of processes participating in the barrier
    //!  process private memory
    long long releaseCnt;            //! value that fetchOpVar must have to "realese" processes
    //!  process private memory
    Locks *lock;                     //! lock for element - process shared memory

    volatile long long *Counter;     //! pointer to atmoic variable - process shared memory

} swBarrierData;


typedef struct {
    int inUse;                       //! element in use
    int commRoot;                    //! commroot and groupID form a unique idendifier for
    //!   process group using this data
    int contextID;

    int nAllocated;                  //! number of times this element has been allocated
    int nFreed;                      //! number of free's for this element

    swBarrierData *barrierData; //! pointer to barrier data

} swBarrierCtlData;

//! management stucture for barrierFetchOpData pool
typedef struct {

    int nPools ;   // number of pools, 1 per hub
    swBarrierCtlData **pool;
    int *nElementsInPool;
    int *lastPoolUsed; // in shared memory
    Locks *Lock;

} SWBarrierPool;

// function prototypes
//!
//!  Simple on host barrier
//!
void SMPSWBarrier(volatile void *barrierData);

//!
//! This routine is used to allocate a pool of O2k atomic fetch-
//!    and-op variables.
//!
int allocSWSMPBarrierPools();

#endif /* !_BARRIER */
