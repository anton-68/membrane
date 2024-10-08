/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_ARRAY_H
#define SM_ARRAY_H

#include <pthread.h>
#include "sm_sys.h"
#include "sm_state.h"
#include "sm_logger.h"
#include "../lib/bj_hash/bj_hash.h"

typedef uint32_t HASH_TYPE;

typedef struct sm_array {
    size_t stack_size;  // initial num of allocated items
    sm_state **stack; 
    int32_t next_free;
    
    size_t hash_size;   // num of items allowed, logarithmic: used as a power over 2
    sm_state **table;
    HASH_TYPE hash_mask;
    HASH_TYPE (*hash_function)(const void *key, size_t key_length, HASH_TYPE initval);
    HASH_TYPE last_hash_value;
    
    bool synchronized;
//    pthread_mutex_t table_lock;
//    pthread_mutex_t stack_lock;
    pthread_mutex_t lock;  
    pthread_cond_t empty;
} sm_array;

// Public methods
sm_array *sm_array_create(size_t stack_size, size_t state_size, sm_fsm **fsm, bool synchronized);
void sm_array_free(sm_array *a);
sm_state *sm_array_find_state(sm_array *, const void *, size_t);
sm_state *sm_array_get_state(sm_array *, const void *, size_t);
void sm_array_release_state(sm_array *, sm_state *);  
void sm_array_depot_state(sm_array *, sm_state *);  

#endif //SM_ARRAY_H


