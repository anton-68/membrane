/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_STATE_H
#define SM_STATE_H

#include <stdlib.h>
#include "sm_sys.h"
#include "sm_event.h"
#include "sm_app.h"
#include "sm_fsm.h"
#include "sm_logger.h"

// DEPRECATED:
//#define SM_STATE_FSM(S) (*(S)->fsm)
//#define SM_STATE_EVENT_ID(s, e) (e)->id >= SM_STATE_FSM(s)->num_of_nodes ? SM_STATE_FSM(s)->omega : (e)->id
 
// state
struct sm_tx;
typedef struct sm_state {
    SM_STATE_ID id;
    struct sm_state *next;
    sm_fsm **fsm;
    void *key;
    sm_event *trace;
    size_t key_length;
    uint32_t key_hash;
    struct sm_array *home;
    struct sm_tx *tx;
    size_t data_size;
    void *data;
} sm_state;

// Public methods
bool sm_state_key_match(sm_state *c, const void *key, size_t key_length);
int sm_state_set_key(sm_state *c, const void *key, size_t key_length);
sm_state *sm_state_create(sm_fsm **f, size_t payload_size);
void sm_state_purge(sm_state *c);
void sm_state_clear(sm_state *s); // for use in sm_array 
void sm_state_free(sm_state *c);
//void sm_apply_event(sm_state *s, sm_event *e); // DEPRECATED: backward compatibility
sm_fsm_node *sm_state_get_node (sm_state *s);
sm_fsm_transition *sm_state_get_transition (sm_event *e, sm_state *s);

#endif //SM_STATE_H 
