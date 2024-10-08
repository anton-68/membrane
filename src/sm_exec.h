/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_EXEC_H
#define SM_EXEC_H

#include <stdlib.h>
#include "sm_sys.h" 
#include "sm_queue2.h"
#include "sm_state.h"
#include "sm_directory.h"
#include "sm_logger.h"
      
#define SM_TX_STACK_POINTER(tx) (sm_state **)((char *)(tx)->data + (tx)->data_size /*- sizeof(sm_state **) */) 

// Executor
struct sm_tx;
typedef struct sm_exec {
    struct sm_tx *master_tx;
    sm_directory *dir;
    size_t data_size;
    void *data;
} sm_exec;

// Public methods
sm_exec *sm_exec_create(size_t s, sm_directory *dir/*, struct sm_tx *tx*/);
void sm_exec_free(sm_exec *exec);

#endif //SM_EXEC_H 
