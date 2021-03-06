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

#include <stdio.h>

#include "internal/log.h"
#include "internal/profiler.h"
#include "path/common/path.h"
#include "path/common/pathContainer.h"
#include "queue/globals.h"
#include "ulm/ulm.h"
#include "util/dclock.h"

extern "C" void ulm_finalize(void)
{
    BasePath_t *pathArray[MAX_PATHS];
    bool stickAround;
    int pathCount;
    int rc;

#ifdef USE_ELAN_COLL
    Broadcaster * bcaster;
    int bcaster_id = 0; 
    rc=ULM_SUCCESS;

    /* To free all the sub communicator traffic,
     * Deadlock can occur if the ordering of different broadcasters
     * are not consistent.
     *
     * However, a correct MPI program should always free its additional
     * communicators before it reaches MPI_Finalize.
     */

    /* To complete all the broadcast traffic */
    bcaster = communicators[ULM_COMM_WORLD]->bcaster;
    if ( bcaster && communicators[ULM_COMM_WORLD]->hw_bcast_enabled)
      while ( bcaster->desc_avail < bcaster->total_channels 
	  && rc == ULM_SUCCESS)
	rc = bcaster->make_progress_bcast();

    /* In case of error, use point-to-point */
    if (rc != ULM_SUCCESS)
    {
      bcast_request_t * last = communicators[ULM_COMM_WORLD]->bcast_queue.last;
      if (last)
          communicators[ULM_COMM_WORLD]->fail_over(last);
    }

#endif

    rc = ulm_barrier(ULM_COMM_WORLD);
    if (rc != ULM_SUCCESS) {
        ulm_err(("ulm_finalize: barrier failed with return code %d\n", rc));
        return;
    }

    pathCount = pathContainer()->allPaths(pathArray, MAX_PATHS);
    do {
        double now = dclock();
        int error;
        stickAround = false;

        // check all of the paths to see if there is still data that
        // needs to be pushed...
        for (int i = 0; i < pathCount; i++) {
            if (pathArray[i]->needsPush() && pathArray[i]->push(now, &error)) {
                stickAround = true;
            }
        }

        // send any unsent acks, and loop if there are more unsent acks to send
        SendUnsentAcks();
        if (UnprocessedAcks.size() != 0) {
            stickAround = true;
        }

        mb();

    } while (stickAround);

    // path specific cleanup
    for (int i = 0; i < pathCount; i++) {
        pathArray[i]->finalize();
    }

    if (0) { // ulm_cleanup() nees some debugging
        ulm_cleanup();
    }
#ifdef USE_ELAN_COLL
    rc = ulm_barrier(ULM_COMM_WORLD); /* Barrier can help */
#endif
}
