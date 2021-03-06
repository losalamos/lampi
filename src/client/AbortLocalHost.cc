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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <signal.h>

#include "internal/constants.h"
#include "internal/log.h"
#include "internal/profiler.h"
#include "internal/state.h"
#include "internal/types.h"
#include "client/daemon.h"
#include "client/SocketGeneric.h"

/*
 * Abort all local children, notify the server, and abort.
 */
void ClientAbort(lampiState_t *s, unsigned int MessageType, int Notify)
{
    int *ProcessCount = s->map_host_to_local_size;
    int hostIndex = s->hostid;
    int ServerSocketFD = s->client->socketToServer_m;
    pid_t *ChildPIDs = (pid_t *) s->local_pids;

    /* kill all children */
    for (int i = 0; i < ProcessCount[hostIndex]; i++) {
        if (ChildPIDs[i] != -1) {
            kill(ChildPIDs[i], SIGKILL);
            ChildPIDs[i] = -1;
        }
    }

    /* notify server of termination */
    if (Notify) {
        ulm_iovec_t iovec;
        iovec.iov_base = (char *) &MessageType;
        iovec.iov_len = (ssize_t) (sizeof(unsigned int));
        ssize_t rc = SendSocket(ServerSocketFD, 1, &iovec);
        if (rc < 0) {
            ulm_exit(("Error: reading Tag in ClientAbort. "
                      "RetVal: %ld\n", rc));
        }
    }

    /* kill process group */
    /* kill(0, SIGKILL); */

    exit(EXIT_FAILURE);
}
