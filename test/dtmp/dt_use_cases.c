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

#define DT_ROUTER_SM "../../app/dt_router.json"
#define DT_CLIENT_SM "../../app/dt_client.json"
#define DT_SERVER_SM "../../app/dt_server.json"
#define DT_FLOW_PROCESSOR_SM "../../app/dt_flow_processor.json"
#define DT_CONTROLLER_SM "../../app/dt_controller.json"
#define DT_RULE_ENGINE_SM "../../app/dt_rule_engine.json"
#define DT_SIP_SERVER_SM "../../app/dt_sip_server.json"
#define DT_SIP_SESSION_SM "../../app/dt_sip_session.json"

extern char *dt_protocols[];
extern char *dt_dtmp_commands[];

//#define DT_SM_PRINTOUT 1

#define DT_USE_CASE1 1
//#define DT_USE_CASE2 1
//#define DT_USE_CASE3 1
//#define DT_USE_CASE4 1


int main() {

    json_set_escape_slashes(0);

    // Create the system directory
    sm_directory *dir = sm_directory_create();

    // Create and register the executor object 
    sm_exec *exec = sm_exec_create(65536, dir);
    dir = sm_directory_set(dir, "exec", exec);
    // !! Rework this to remove this work-around!!
    exec->dir = dir;
    
    // Create and register the reporting priority queue
    sm_pqueue *pq = sm_pqueue_create(DT_QUEUE_SIZE, true);
    dir = sm_directory_set(dir, DT_COLLECTOR, pq);     
    
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
    
    // Report event application to SM: DEPRECATED
    sm_app appl = dlsym(handle, "dt_appl");
    dir = sm_directory_set(dir, "APPL", appl);
    
    // Report event application to SM:
    sm_app drop = dlsym(handle, "dt_drop");
    dir = sm_directory_set(dir, "DROP", drop);
    
    // Pause SM processing: DEPRECATED
    sm_app wait = dlsym(handle, "dt_wait"); 
    dir = sm_directory_set(dir, "WAIT", wait);
    
    // No application
    sm_app noap = dlsym(handle, "dt_send");
    dir = sm_directory_set(dir, "NOAP", noap);
    
    // Server dummy application
    sm_app serv = dlsym(handle, "dt_serv");
    dir = sm_directory_set(dir, "SERV", serv);
    
    // Setting hash key: DEPRECATED 
    sm_app fltr = dlsym(handle, "dt_fltr");
    dir = sm_directory_set(dir, "FLTR", fltr);
    
    // Setting hash key
    sm_app setk = dlsym(handle, "dt_setk");
    dir = sm_directory_set(dir, "SETK", setk);
    
    // Set flow in forwarder
    sm_app setf = dlsym(handle, "dt_setf");
    dir = sm_directory_set(dir, "SETF", setf);
    
    // Request flow from controller
    sm_app reqf = dlsym(handle, "dt_reqf");
    dir = sm_directory_set(dir, "REQF", reqf);
    
    // Request rule confirmation from user
    sm_app reqr = dlsym(handle, "dt_reqr");
    dir = sm_directory_set(dir, "REQR", reqr);
    
    // Check the message against flow
    sm_app chck = dlsym(handle, "dt_chck");
    dir = sm_directory_set(dir, "CHCK", chck);
    
    // Get rule from controller
    sm_app getr = dlsym(handle, "dt_getr");
    dir = sm_directory_set(dir, "GETR", getr);
    
    // Add rule to the controller
    sm_app addr = dlsym(handle, "dt_addr");
    dir = sm_directory_set(dir, "ADDR", addr);
    
    // Translate rule to flow
    sm_app xtrr = dlsym(handle, "dt_xtrr");
    dir = sm_directory_set(dir, "XTRR", xtrr);
    
    // Ask user to confirm flow (on UE)
    sm_app askf = dlsym(handle, "dt_askf");
    dir = sm_directory_set(dir, "ASKF", askf);
    
    // Ask user to confirm rule
    sm_app askr = dlsym(handle, "dt_askr");
    dir = sm_directory_set(dir, "ASKR", askr);
    
    // SIP server dummy
    sm_app sips = dlsym(handle, "dt_sips");
    dir = sm_directory_set(dir, "SIPS", sips);
    
    // SIP INVITE forward 
    sm_app sinv = dlsym(handle, "dt_sinv");
    dir = sm_directory_set(dir, "SINV", sinv);
    
    // SIP 100
    sm_app s100 = dlsym(handle, "dt_s100");
    dir = sm_directory_set(dir, "S100", s100);
    
    // Write rule
    sm_app wrrt = dlsym(handle, "dt_wrrt");
    dir = sm_directory_set(dir, "WRRT", wrrt);
    
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
#ifdef DT_SM_PRINTOUT    
    printf("\nClient SM:\n%s\n", sm_fsm_to_string(client_sm, dir));
#endif

    // Compile and register the rule engine SM
    file = fopen(DT_RULE_ENGINE_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Rule Engine SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *rule_engine_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_RULE_ENGINE_SM_NAME, rule_engine_sm);
#ifdef DT_SM_PRINTOUT    
    printf("\nRule Engine SM:\n%s\n", sm_fsm_to_string(rule_engine_sm, dir));
#endif 

    // RuleEngineArray
    sm_array *rule_engine_array = sm_array_create(DT_ARRAY_SIZE, DT_STATE_SIZE, 
                                  (sm_fsm **)sm_directory_get_ref(dir, DT_RULE_ENGINE_SM_NAME), false);
    sm_directory_set(dir, DT_RULE_ENGINE_ARRAY, rule_engine_array);

    // Compile and register the flow processor SM
    file = fopen(DT_FLOW_PROCESSOR_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Flow Processor SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *flow_processor_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_FLOW_PROCESSOR_SM_NAME, flow_processor_sm);
#ifdef DT_SM_PRINTOUT    
    printf("\nFlow Processor SM:\n%s\n", sm_fsm_to_string(flow_processor_sm, dir));
#endif 

    // FlowProcessorArray
    sm_array *flow_processor_array = sm_array_create(DT_ARRAY_SIZE, DT_STATE_SIZE, 
                                  (sm_fsm **)sm_directory_get_ref(dir, DT_FLOW_PROCESSOR_SM_NAME), false);
    sm_directory_set(dir, DT_FLOW_PROCESSOR_ARRAY, flow_processor_array);

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
#ifdef DT_SM_PRINTOUT    
    printf("\nRouter SM:\n%s\n", sm_fsm_to_string(router_sm, dir));
#endif 
    
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
#ifdef DT_SM_PRINTOUT    
    printf("\nServer SM:\n%s\n", sm_fsm_to_string(server_sm, dir));
#endif 

    // Compile and register the controller SM
    file = fopen(DT_CONTROLLER_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input Controller SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *controller_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_CONTROLLER_SM_NAME, controller_sm);
#ifdef DT_SM_PRINTOUT    
    printf("\nController SM:\n%s\n", sm_fsm_to_string(controller_sm, dir));
#endif 

    // Compile and register the SIP session SM
    file = fopen(DT_SIP_SESSION_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input SIP session SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *sip_session_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_SIP_SESSION_SM_NAME, sip_session_sm);
#ifdef DT_SM_PRINTOUT    
    printf("\nSIP Session SM:\n%s\n", sm_fsm_to_string(sip_session_sm, dir));
#endif 

    // SIPSessionArray
    sm_array *sip_session_array = sm_array_create(DT_ARRAY_SIZE, DT_STATE_SIZE, 
                                  (sm_fsm **)sm_directory_get_ref(dir, DT_SIP_SESSION_SM_NAME), false);
    sm_directory_set(dir, DT_SIP_SESSION_ARRAY, sip_session_array);

    // Compile and register the SIP server SM
    file = fopen(DT_SIP_SERVER_SM, "r");
    if (!file) {
        REPORT(ERROR, "Input SIP server SM file is missing or incorrect\n");
        exit(0);
    }
    i=0;
    while(!feof(file))
        jstr[i++] = fgetc(file);
    jstr[--i]='\0';
    fclose(file);
    sm_fsm *sip_server_sm = sm_fsm_create(jstr, dir);
    dir = sm_directory_set(dir, DT_SIP_SERVER_SM_NAME, sip_server_sm);
#ifdef DT_SM_PRINTOUT    
    printf("\nSIP Server SM:\n%s\n", sm_fsm_to_string(sip_server_sm, dir));
#endif 


#ifdef DT_USE_CASE1
    
    // Create and register the bi-priority queues for all instances:
    sm_queue2 *q0 = sm_queue2_create();
    dir = sm_directory_set(dir, "TV_Set_1_q", q0); 
    sm_queue2 *q1 = sm_queue2_create();
    dir = sm_directory_set(dir, "Camera_1_q", q1); 
    sm_queue2 *q2 = sm_queue2_create();
    dir = sm_directory_set(dir, DT_ROUTER_QUEUE, q2); 
    sm_queue2 *q3 = sm_queue2_create();
    dir = sm_directory_set(dir, DT_CONTROLLER_QUEUE, q3); 
    sm_queue2 *q4 = sm_queue2_create();
    dir = sm_directory_set(dir, "SmartPhone_1_q", q4); 
    sm_queue2 *q5 = sm_queue2_create();
    dir = sm_directory_set(dir, "TV_Set_2_q", q5); 
    sm_queue2 *q6 = sm_queue2_create();
    dir = sm_directory_set(dir, "Camera_2_q", q6); 

    // Create and register the thread-workers 
    sm_tx *tx0 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_CLIENT_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, "TV_Set_1_q"), true);
    dir = sm_directory_set(dir, "TV_Set_1", tx0);  
    sm_tx *tx1 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_SERVER_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, "Camera_1_q"), true);
    dir = sm_directory_set(dir, "Camera_1", tx1);  
    sm_tx *tx2 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_ROUTER_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, DT_ROUTER_QUEUE), true);
    dir = sm_directory_set(dir, DT_ROUTER, tx2);  
    sm_tx *tx3 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_CONTROLLER_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, DT_CONTROLLER_QUEUE), true);
    dir = sm_directory_set(dir, DT_CONTROLLER, tx3);  
    sm_tx *tx4 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_CLIENT_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, "SmartPhone_1_q"), true);
    dir = sm_directory_set(dir, "SmartPhone_1", tx4);  
    sm_tx *tx5 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_CLIENT_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, "TV_Set_2_q"), true);
    dir = sm_directory_set(dir, "TV_Set_2", tx5);  
    sm_tx *tx6 = sm_tx_create(exec, (sm_fsm **)sm_directory_get_ref(dir, DT_SERVER_SM_NAME), 4096, 4096, 
                                 (sm_queue2 **)sm_directory_get_ref(dir, "Camera_2_q"), true);
    dir = sm_directory_set(dir, "Camera_2", tx6);  

    // Create and register the event pool
    sm_queue *p0 = sm_queue_create(4096, DT_QUEUE_SIZE, true);
    dir = sm_directory_set(dir, DT_POOL, p0);

    // Run threads:
    pthread_t t0;
    pthread_create(&t0, NULL, &sm_tx_runner, (void *)tx0);
    pthread_t t1;
    pthread_create(&t1, NULL, &sm_tx_runner, (void *)tx1);
    pthread_t t2;
    pthread_create(&t2, NULL, &sm_tx_runner, (void *)tx2);
    pthread_t t3;
    pthread_create(&t3, NULL, &sm_tx_runner, (void *)tx3);
    pthread_t t4;
    pthread_create(&t4, NULL, &sm_tx_runner, (void *)tx4);
    pthread_t t5;
    pthread_create(&t5, NULL, &sm_tx_runner, (void *)tx5);
    pthread_t t6;
    pthread_create(&t6, NULL, &sm_tx_runner, (void *)tx6);

    // 1. Dequeue event from the pool
    sm_event *e = sm_queue_dequeue(p0);
        
    // 2. Compose the RTSP request tx0->tx2  wrapped into the ORIG DTMP command
    JSON_Value *rtsp_v = json_value_init_object();
    JSON_Object *rtsp_o = json_value_get_object(rtsp_v);
    json_object_set_string(rtsp_o, "proto", "rtsp");
    json_object_dotset_string(rtsp_o, "data.Request-Line.Method", "GET");
    json_object_dotset_string(rtsp_o, "data.Request-Line.Request-URI", "/twister.sdp HTTP/1.1");
    json_object_dotset_string(rtsp_o, "data.Host", "Camera_1 addr");
    json_object_dotset_string(rtsp_o, "data.Accept", "application/sdp");
    json_object_dotset_string(rtsp_o, "data.Retry", "no");

    // 3. Wrap RTSP into DTMP
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "originate");
    json_object_set_string(dtmp_o, "to", "Camera_1");
    json_object_set_value(dtmp_o, "data", rtsp_v);
    
    // 4. Wrap DTMP into IP
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_string(ip_o, "from", "Console");
    json_object_set_string(ip_o, "to", "TV_Set_1");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", dtmp_v);

    // 5. Serialize JSON object and copy to the event data block
    char *json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(e->data, json_string);
    e->id = 1;

    // 6. Send the request to tx0
    sm_lock_enqueue2(e, *sm_directory_get_ref(dir, DT_ROUTER_QUEUE));

    // 7. Wait
    sleep(1);

    // 8. Repeat firing packet
    e = sm_queue_dequeue(p0);
    json_object_dotset_string(ip_o, "data.data.data.Retry", "yes");
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(e->data, json_string);
    e->id = 1;
    sm_lock_enqueue2(e, *sm_directory_get_ref(dir, DT_ROUTER_QUEUE));
    
    // 9. Wait
    sleep(1);

    // 10. Fire similar packet (same role and proto but different devices)
    e = sm_queue_dequeue(p0);
    json_object_dotset_string(ip_o, "data.data.data.Retry", "no");
    json_object_set_string(ip_o, "to", "TV_Set_2");
    json_object_dotset_string(ip_o, "data.data.data.Host", "Camera_2 addr");
    json_object_dotset_string(ip_o, "data.to", "Camera_2");
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(e->data, json_string);
    printf("\n%s\n", (char *)e->data);
    e->id = 1;
    sm_lock_enqueue2(e, *sm_directory_get_ref(dir, DT_ROUTER_QUEUE));
 
    // 9. Wait
    sleep(1);

    // 10. Repeat
    e = sm_queue_dequeue(p0);
    json_object_dotset_string(ip_o, "data.data.data.Retry", "yes");
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(e->data, json_string);
    e->id = 1;
    sm_lock_enqueue2(e, *sm_directory_get_ref(dir, DT_ROUTER_QUEUE));
    
    // 11. Wait
    sleep(1);

#endif
#ifdef DT_USE_CASE2
#endif    
#ifdef DT_USE_CASE3
#endif    
#ifdef DT_USE_CASE4
#endif 

    // __. Print out the reporting queue
    sm_event *r; i = 1;
    while(sm_pqueue_top(pq) != NULL) {
        r = sm_pqueue_dequeue(pq); 
        printf("\n\n[%d]\n%s", i++, (char *)r->data);
    }
    printf("\n\nDone.\n\n");

/*  pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL); */

    return 0;
}
