/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <stdlib.h>         // malloc(), free(), NULL, size_t, 
#include <string.h>         // memset()

#include "sm_logger.h"
#include "sm_event.h"
#include "sm_queue.h"
//#include "sm_memory.h"

/* sm_event */

sm_event *sm_event_create(uint32_t payload_size) {
    sm_event *e;
    if((e = malloc(sizeof(sm_event))) == NULL) {
        REPORT(ERROR, "malloc()");
        return e;
    }
/*    if(SM_MEMORY_MANAGER)
        e->data_size = sm_memory_size_align(payload_size, sizeof(sm_chunk));
    else */
        e->data_size = payload_size;
    if(e->data_size > 0) {
        if ((e->data = malloc(e->data_size)) == NULL) {
            REPORT(ERROR, "malloc()");
            free(e);
            return NULL;
        }
        memset(e->data, 0, e->data_size);
    }
    else {
        e->data = NULL;
    }
    e->next = NULL;
    e->id = 0;
    for (int stage = 0; stage < SM_NUM_OF_PRIORITY_STAGES; stage++)
        e->priority[stage] = 0;
    e->home = NULL;
    e->key = NULL;
    e->key_hash = 0;
    e->key_length = 0;
    e->disposable = true;
    return e;    
}

int sm_event_set_key(sm_event *e, const void *key, size_t key_length) {
    memset(e->key, '\0', SM_STATE_HASH_KEYLEN);
    e->key_length = MIN(key_length, SM_STATE_HASH_KEYLEN);
    memcpy(e->key, key, MIN(key_length, e->key_length));
    return EXIT_SUCCESS;
}
    
void sm_event_free(sm_event *e) {
    free(e->data);
    free(e);
    e = NULL;
}

void sm_event_park(sm_event *e) {
    if(e->home == NULL)
        sm_event_free(e);
    else {
        if(e->key != NULL) 
            free(e->key);
        e->key_length = 0;
        e->key_hash = 0;
        memset(e->data, 0, e->data_size);
        sm_queue_enqueue(e, e->home);
    }
}

