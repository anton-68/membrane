/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#ifndef SM_PQUEUE_H
#define SM_PQUEUE_H

#include "sm_event.h"
#include "sm_logger.h"

/* sm_pqueue */

typedef struct sm_pqueue {
	size_t capacity;
	size_t size;
	bool synchronized;
    pthread_mutex_t lock;
	pthread_cond_t empty;
	sm_event **heap;
} sm_pqueue;

// Public methods

sm_pqueue *sm_pqueue_create(size_t capacity, bool synchronized);
void sm_pqueue_free(sm_pqueue *q);
size_t sm_pqueue_size(sm_pqueue *q);

sm_event *sm_pqueue_top(const sm_pqueue * q);
int sm_pqueue_enqueue(sm_event *e, sm_pqueue *q);
sm_event *sm_pqueue_dequeue(sm_pqueue *q);

#endif //SM_PQUEUE_H