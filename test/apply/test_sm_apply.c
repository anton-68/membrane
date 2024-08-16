/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../lib/parson/parson.h"
#include "../../src/sm.h"
#include "../../app/dtag_tc_apps.h"

#define DT_APPS "../../app/dtag_tc_apps.so"
#define DT_CLIENT_SM "../../app/client.json"
#define DT_ROUTER_SM "../../app/router.json"
#define DT_SERVER_SM "../../app/server.json"

extern char *dt_protocols[];
extern char *dt_dtmp_commands[];
int main() {

    // Create the system directory
    sm_directory *dir = sm_directory_create();

    // Create and register the executor object 
    sm_exec *exec = sm_exec_create(65536, dir);
    dir = sm_directory_set(dir, "exec", exec);
    // !! Rework this to remove this work-around!!
    exec->dir = dir;
    
    // Create and register the reporting priority queue
    sm_pqueue *pq = sm_pqueue_create(64, true);
    dir = sm_directory_set(dir, "col_pqueue", pq);     
    
    // Load and register the applications
    void *handle = dlopen(DT_APPS, RTLD_LAZY);
    
    // Report sending packet 
    sm_app send = dlsym(handle, "dt_send");
    dir = sm_directory_set(dir, "SEND", send);
    
    // Report routing packet
    sm_app rout = dlsym(handle, "dt_rout");
    dir = sm_directory_set(dir, "ROUT", rout);
    
    // Report receiving packet
    sm_app recv = dlsym(handle, "dt_recv");
    dir = sm_directory_set(dir, "RECV", recv);
    
    // Command node to originate packet
    sm_app orig = dlsym(handle, "dt_orig");
    dir = sm_directory_set(dir, "ORIG", orig);
    
    // Service selector
    sm_app ssel = dlsym(handle, "dt_ssel");
    dir = sm_directory_set(dir, "SSEL", ssel);
    
    // DTMP command parser
    sm_app dtmp = dlsym(handle, "dt_dtmp");
    dir = sm_directory_set(dir, "DTMP", dtmp);
    
    // Report event application to SM
    sm_app appl = dlsym(handle, "dt_appl");
    dir = sm_directory_set(dir, "APPL", appl);
    
    // Pause SM processing
    sm_app wait = dlsym(handle, "dt_wait");
    dir = sm_directory_set(dir, "WAIT", wait);
    
    // No application
    sm_app noap = dlsym(handle, "dt_send");
    dir = sm_directory_set(dir, "NOAP", noap);
    
    // Server dummy application
    sm_app serv = dlsym(handle, "dt_serv");
    dir = sm_directory_set(dir, "SERV", serv);
    
    // Compile and register the client SM
    FILE * file = fopen(DT_CLIENT_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Client SM file is missing or incorrect\n");
        exit(0);
    }
    char jstr[32 * 1024];
    int i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *client_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_CLIENT_SM_NAME, client_sm);
    // printf("\nClient SM:\n%s\n", sm_fsm_to_string(client_sm, dir));
 
    // Compile and register the router SM
    file = fopen(DT_ROUTER_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Router SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *router_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_ROUTER_SM_NAME, router_sm);
    // printf("\nRouter SM:\n%s\n", sm_fsm_to_string(router_sm, dir));
    
    // Compile and register the server SM
    file = fopen(DT_SERVER_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Server SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *server_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_SERVER_SM_NAME, server_sm);
    
    // Create and register the worker bipriority queues q0 & q1
    sm_queue2 *q0 = sm_queue2_create();
    dir = sm_directory_set(dir, DT_CLIENT_QUEUE, q0); 
    sm_queue2 *q1 = sm_queue2_create();
    dir = sm_directory_set(dir, DT_SERVER_QUEUE, q1); 
    
    // Create and register the router bipriority queue q2
    sm_queue2 *q2 = sm_queue2_create();
    dir = sm_directory_set(dir, DT_ROUTER_QUEUE, q2); 
    
    // Create and register the thread-worker tx0(exec, worker SM, q0, sync = true)
    sm_tx *tx0 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_CLIENT_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, DT_CLIENT_QUEUE), true);
    dir = sm_directory_set(dir, DT_CLIENT, tx0);  
    
    // Create and register the thread-worker tx1(exec, worker SM, q1, sync = true)
    sm_tx *tx1 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_SERVER_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, DT_SERVER_QUEUE), true);
    dir = sm_directory_set(dir, DT_SERVER, tx1);  
    
    // Create and register the thread-worker tx2(exec, router SM, q2, sync = true)i
    sm_tx *tx2 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_ROUTER_SM_NAME), 4096, 4096, 
                              (sm_queue2 **)sm_directory_get_ref(dir, DT_ROUTER_QUEUE), true);
    dir = sm_directory_set(dir, DT_ROUTER, tx2);  
    
    // Create and register the event pool(16, 4k)
    sm_queue *p0 = sm_queue_create(4096, 32, true);
    dir = sm_directory_set(dir, DT_POOL, p0);

    // Run threads:
    pthread_t t0;
    pthread_create(&t0, NULL, &sm_tx_runner, (void *)tx0);
    pthread_t t1;
    pthread_create(&t1, NULL, &sm_tx_runner, (void *)tx1);
    pthread_t t2;
    pthread_create(&t2, NULL, &sm_tx_runner, (void *)tx2);

    // Use-Case 1: request-responce with SERV protocol
    // ---
    // 1. Dequeue event from the pool
    sm_event *e = sm_queue_dequeue(p0);
        
    // 2. Compose the SERV request tx0->tx2  wrapped into the ORIG DTMP command
    JSON_Value *serv_v = json_value_init_object();
    JSON_Object *serv_o = json_value_get_object(serv_v);
    json_object_set_string(serv_o, "proto", "serv");
    json_object_set_string(serv_o, "command", "request");

    // 3. Wrap SERV into DTMP
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "originate_packet");
    json_object_set_string(dtmp_o, "to", DT_SERVER_QUEUE);
    json_object_set_value(dtmp_o, "data", serv_v);
    
    // 4. Wrap DTMP into IP
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_string(ip_o, "from", DT_CONTROLLER);
    json_object_set_string(ip_o, "to", DT_CLIENT_QUEUE);
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", dtmp_v);

    // 5. Serialize JSON object and copy to the event data block
    char *json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(e->data, json_string);
    e->id = 1;

    // 6. Send the request to tx0
    sm_lock_enqueue2(e, *sm_directory_get_ref(dir, DT_ROUTER_QUEUE));

    // 7. Wait 1 sec.
    sleep(1);

    // 8. Print out the reporting queue
    sm_event *r;
    while(sm_pqueue_top(pq) != NULL) { 
        r = sm_pqueue_dequeue(pq); 
        printf("\n%s", (char *)r->data);
    }

    // send kill event
    /*
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    */
    return 0;
}
