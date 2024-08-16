/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_exec.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>           // nanosleep ()
#include "sm_fsm.h"
#include "sm_event.h"
#include "sm_apply.h"

// Public methods

sm_exec *sm_exec_create(size_t size, sm_directory *dir/*, struct sm_tx *tx*/) {
    sm_exec *exec;
    if((exec = malloc(sizeof(sm_exec))) == NULL) {
        REPORT(ERROR, "malloc()");
        return NULL;
    }
/*    if(SM_MEMORY_MANAGER)
        exec->data_size = sm_memory_size_align(size, sizeof(sm_chunk));
    else */
        exec->data_size = size;
    if((exec->data = malloc(exec->data_size)) == NULL) {
        free(exec);
        REPORT(ERROR, "malloc()");
        return NULL;
    }
    //exec->master_tx = tx;
    exec->dir = dir;
    return exec;
}

void sm_exec_free(sm_exec *exec) {
    /*if(exec->master_tx != NULL)
        sm_tx_free(exec->master_tx);*/
    sm_directory_free(exec->dir);             
    free(exec->data);
    free(exec);
}

