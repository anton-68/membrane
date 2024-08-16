/* SM.EXEC 
   SM applications library for Telekom Challenge PoC
   anton.bondarenko@gmail.com */

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "dtag_tc_apps.h"
#include "../lib/parson/parson.h"

char *dt_protocols[] = {
    "default",
    "ip",
    "dtmp",
    "sip",
    "rtsp",
    "serv"
};

char *dt_dtmp_commands[] = {
    "reserved",
    "originate",         // 1025
    "report",            // 1026
    "register",          // 1027
    "set_flow",          // 1028
    "request_flow",      // 1029
    "set_rule",          // 1030
    "get_rule",          // 1031
    "deploy_rule",       // 1032
    "receipt",           // 1033
    "pending"            // 1034
};

char *dt_dtmp_status_values[] = {
    "ok",                // 2048
    "blocked",           // 2049
    "allowed",           // 2050
    "unknown",           // 2051
    "skip"               // 2052
};

#define DT_BLOCKED 2049
#define DT_ALLOWED 2050
#define DT_UNKNOWN 2051

char *dt_sip_methods[] = {
    "reserved",
    "100 Trying",        // 3073
    "180 Ringing",       // 3074
    "200 OK",            // 3075
    "INVITE",            // 3076
    "CANCEL",            // 3077
    "BYE"                // 3078
};

char *dt_end_nodes[] = {
    "Camera_1",
    "Camera_2",
    "TV_Set_1",
    "TV_Set_2",
    "GrannyWristyBand",
    "SmartVoice_1",
    "IPPhone_1",
    "IPPhone_2",
    "LightSensor_1",
    "TempSensor_1",
    "TempSensor_2",
    "Laptop_1",
    "Laptop_2",
    "SmartPhone_1",
    "SmartPhone_2"
};

char *dt_roles[] = {
    "CCTVCam",
    "Smart_TV_Set",
    "IoT_Wearable",
    "SmartHome",
    "IPTelephony",
    "Client"
};

size_t dt_assigned_role[] = {0, 0, 1, 1, 2, 3, 4, 4, 3, 3, 3, 5, 5, 5, 5};


extern char *sm_node_type_to_string[];
extern char *sm_transition_type_to_string[];

int dt_recv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SEND cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "received packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    e->disposable = true;
    return 0;   
}

int dt_send(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp();
    JSON_Value *mv = json_parse_string((char *)((sm_event *)s->trace)->data); 
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SEND cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SEND cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet sent");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet to send", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    s->trace->disposable = false;
    sm_lock_enqueue2((sm_event *)s->trace, *sm_directory_get_ref(s->tx->exec->dir, DT_ROUTER_QUEUE));
    //s->trace = NULL;
    return 0;   
}

int dt_rout(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    char *routing_status;

    void **t;
    sm_timestamp ts = sm_get_timestamp();
    JSON_Value *mv = json_parse_string((char *)e->data); 
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ROUT cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ROUT cannot read the message reporting flag", 
                                       "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(json_object_has_value(m, "to") && json_object_get_string_len(m, "to") > 0) {
        const char *to = json_object_get_string(m, "to");
        t = sm_directory_get_ref(s->tx->exec->dir, to);
        if(t == NULL) {
            SM_SYSLOG(SM_CORE, SM_LOG_ERR, "ROUT cannot find destination in the system directory", 
                                           "sm_directory_get_ref() returned NULL");
            routing_status = "failure";
            e->id = 2051;
        }
        else { // Routing assuming all 'to' fields point to tx-s
            e->disposable = false;
            sm_lock_enqueue2(e, *(*(sm_tx **)t)->input_queue);
            routing_status = "success";
            e->id = 1;
        }
    }
    else {
        SM_SYSLOG(SM_CORE, SM_LOG_ERR, "ROUT cannot read destination address", 
                                       "json_object... returned NULL or zero length string");
        routing_status = "failure";
        e->id = 2051;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet routed");
        json_object_set_string(dtmp_o, "status", routing_status);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;   
}

int dt_orig(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ORIG cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SEND cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    JSON_Value *serv_v = json_value_deep_copy(json_object_dotget_value(m, "data.data"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "to", json_object_dotget_string(m, "data.to"));
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", serv_v);
    json_string = json_serialize_to_string_pretty(ip_v);
    sm_event *eo = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    strcpy(eo->data, json_string);
    eo->id = 1;
    s->trace = eo;
    if(result) { // to_report
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "packet originated");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "incoming packet", mv);
        json_object_set_value(dtmp_o, "originated packet", ip_v);
        json_string = json_serialize_to_string_pretty(dtmp_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        strcpy(ep->data, json_string);
        ep->id = 1;
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;   
}

int dt_ssel(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SSEL cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv); 
    const char *proto;
    if(json_object_dothas_value(m, "data.proto")) {
        proto = json_object_dotget_string(m, "data.proto");
    }
    else {
        proto = json_object_dotget_string(m, "data.proto");
    }
    size_t i;
    for (i = 0; i < DT_NUM_OF_PROTOCOLS; i++) {
        if(strcmp(dt_protocols[i], proto) == 0) {
            e->id = i;
            break;
        }
    }
    const char *to = json_object_get_string(m, "to");
    if(to == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SSEL cannot read the message to field", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
/*    if(i == 2 && ((strcmp(to, sm_directory_get_name(s->tx->exec->dir, s->tx)) == 0) || 
                  (strcmp(to, sm_directory_get_name(s->tx->exec->dir, *s->tx->input_queue)) == 0))) {
        i = 1;
    }*/
    if(i == DT_NUM_OF_PROTOCOLS){
        e->id = 0;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SSEL cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "service selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "detected_proto", dt_protocols[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(/*ip_v*/dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_dtmp(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "DTMP cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    const char *command = json_object_dotget_string(m, "data.command");
    size_t i;
    for (i = 0; i < DT_DTMP_NUM_OF_COMMANDS; i++) {
        if(strcmp(dt_dtmp_commands[i], command) == 0) {
            e->id = i + DT_PROTO_SPACE;
            break;
        }
    }
    if(i == DT_DTMP_NUM_OF_COMMANDS) {
        e->id = DT_PROTO_SPACE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "DTMP cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "dtmp command selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "detected_command", dt_dtmp_commands[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(/*ip_v*/dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_appl(sm_event *e, sm_state *s) {
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "report");
    json_object_set_string(dtmp_o, "event", "event applied to SM");
    json_object_set_string(dtmp_o, "timestamp", ts.timestring);
    json_object_set_number(dtmp_o, "event_id", e->id);
    json_object_set_number(dtmp_o, "state_id", s->id);
    sm_fsm_node *n = sm_state_get_node(s);
    json_object_set_string(dtmp_o, "node_type", sm_node_type_to_string[n->type]);    
    sm_fsm_transition *t = sm_state_get_transition(e, s);
    json_object_set_string(dtmp_o, "transition_type", sm_transition_type_to_string[t->type]);    
    json_object_set_string(dtmp_o, "transition_ref", sm_directory_get_name(s->tx->exec->dir, *(void **)t->transitionRef));
    json_object_set_number(dtmp_o, "target_node", t->targetNode);
    json_object_set_string(dtmp_o, "fsm_name", sm_directory_get_name(s->tx->exec->dir, *(s->fsm)));
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "APPLY cannot parse message id", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    json_object_dotset_value(dtmp_o, "packet", mv);
    sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    char *json_string = json_serialize_to_string_pretty(dtmp_v);
    strcpy(ep->data, json_string);
    ep->priority[0] = -ts.seconds;
    ep->priority[1] = -ts.nanoseconds;
    sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    return 0;
}

int dt_drop(sm_event *e, sm_state *s) {
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *dtmp_v = json_value_init_object();
    JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
    json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(dtmp_o, "proto", "dtmp");
    json_object_set_string(dtmp_o, "command", "report");
    json_object_set_string(dtmp_o, "event", "closing event processing");
    json_object_set_string(dtmp_o, "timestamp", ts.timestring);
    json_object_set_number(dtmp_o, "event_id", e->id);
    json_object_set_number(dtmp_o, "state_id", s->id);
    sm_fsm_node *n = sm_state_get_node(s);
    json_object_set_string(dtmp_o, "node_type", sm_node_type_to_string[n->type]);    
    sm_fsm_transition *t = sm_state_get_transition(e, s);
    json_object_set_string(dtmp_o, "transition_type", sm_transition_type_to_string[t->type]);    
    json_object_set_string(dtmp_o, "transition_ref", sm_directory_get_name(s->tx->exec->dir, *(void **)t->transitionRef));
    json_object_set_number(dtmp_o, "target_node", t->targetNode);
    json_object_set_string(dtmp_o, "fsm_name", sm_directory_get_name(s->tx->exec->dir, *(s->fsm)));
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "DROP cannot parse message id", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    json_object_dotset_value(dtmp_o, "packet", mv);
    sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    char *json_string = json_serialize_to_string_pretty(dtmp_v);
    strcpy(ep->data, json_string);
    ep->priority[0] = -ts.seconds;
    ep->priority[1] = -ts.nanoseconds;
    sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    return 0;
}

int dt_wait(sm_event *e, sm_state *s) { // DEPRECATED
    struct timespec ts = {0};
    ts.tv_sec = e->priority[0];
    ts.tv_nsec = e->priority[1];
    nanosleep(&ts, NULL);
    return 0;
}

int dt_noap(sm_event *e, sm_state *s) {
    return 0;
}

int dt_serv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SERV cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SERV cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *serv_v = json_value_init_object();
    JSON_Object *serv_o = json_value_get_object(serv_v);
    json_object_set_string(serv_o, "proto", "serv");
    json_object_set_number(serv_o, "answer", 42);
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", serv_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "SERV request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}    

int dt_fltr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "FLTR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "FLTR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    //size_t k_length = 0;
    char *k_position = (char *)e->key;
    strcpy((char *)k_position, json_object_get_string(m, "from"));
    k_position += json_object_get_string_len(m, "from");
    strcpy((char *)k_position++, "#");
    //((char *)k_position++)[0] = "#";
    strcpy((char *)k_position, json_object_get_string(m, "to"));
    k_position += json_object_get_string_len(m, "to");
    strcpy((char *)k_position++, "#");
    strcpy((char *)k_position, json_object_get_string(m, "proto"));
    k_position += json_object_get_string_len(m, "proto");
    strcpy((char *)k_position, "\0");
    e->key_length = strlen(e->key);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow filter key prepared");
        json_object_set_string(dtmp_o, "key", e->key);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_setk(sm_event *e, sm_state *s) {
    //char error[80];
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SETK cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SETK cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    size_t k_length = 0;
    char *k_position = NULL;
    switch(e->id) {
        case 1: // IP request_flow - from router to user
        case 4: // RTSP
            k_length += json_object_get_string_len(m, "from");
            k_length += json_object_get_string_len(m, "to");
            k_length += json_object_dotget_string_len(m, "data.proto");
            k_length += 3;
            e->key = malloc(k_length);
            k_position = (char *)e->key;
            strcpy(k_position, json_object_get_string(m, "from"));
            k_position += json_object_get_string_len(m, "from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_get_string(m, "to"));
            k_position += json_object_get_string_len(m, "to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.proto"));
            k_position += json_object_dotget_string_len(m, "data.proto");
            strcpy(k_position, "\0");
            break;
        case 3: // SIP Call-ID
        case 1033: // DTMP reseipt indexed by SIP Call-ID
            e->key = malloc(json_object_dotget_string_len(m, "data.Call-ID"));
            k_position = (char *)e->key;
            strcpy((char *)k_position, json_object_dotget_string(m, "data.Call-ID"));
            break;
        case 1028: // set_flow - from user to router
            k_length += json_object_dotget_string_len(m, "data.data.from");
            k_length += json_object_dotget_string_len(m, "data.data.to");
            k_length += json_object_dotget_string_len(m, "data.data.proto");
            k_length += 3;
            e->key = malloc(k_length);
            k_position = (char *)e->key;
            strcpy(k_position, json_object_dotget_string(m, "data.data.from"));
            k_position += json_object_dotget_string_len(m, "data.data.from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.to"));
            k_position += json_object_dotget_string_len(m, "data.data.to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.proto"));
            k_position += json_object_dotget_string_len(m, "data.data.proto");
            strcpy(k_position, "\0");
            break;
        case 1030: // set_rule - from user to controller
        case 1031: // get_rule - from router to controller
        case 1032: // deploy_rule - from controller to router
            k_length += json_object_dotget_string_len(m, "data.data.from_role");
            k_length += json_object_dotget_string_len(m, "data.data.to_role");
            k_length += json_object_dotget_string_len(m, "data.data.proto");
            k_length += 2;
            e->key = malloc(k_length);
            k_position = (char *)e->key;
            strcpy(k_position, json_object_dotget_string(m, "data.data.from_role"));
            k_position += json_object_dotget_string_len(m, "data.data.from_role");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.to_role"));
            k_position += json_object_dotget_string_len(m, "data.data.to_role");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.data.proto"));
            k_position += json_object_dotget_string_len(m, "data.data.proto");
            strcpy(k_position, "\0");
            break;
        default: // Undefined
            e->key = malloc(strlen("undefined"));
            strcpy(e->key, "undefined"); /*
            k_length += json_object_get_string_len(m, "from");
            k_length += json_object_get_string_len(m, "to");
            k_length += json_object_dotget_string_len(m, "data.proto");
            k_length += 3;
            e->key = malloc(k_length);
            k_position = (char *)e->key;
            strcpy(k_position, json_object_get_string(m, "from"));
            k_position += json_object_get_string_len(m, "from");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_get_string(m, "to"));
            k_position += json_object_get_string_len(m, "to");
            strcpy(k_position++, "#");
            strcpy(k_position, json_object_dotget_string(m, "data.proto"));
            k_position += json_object_dotget_string_len(m, "data.proto");
            strcpy(k_position, "\0"); */
            break;
            //sprintf(error, "SETK cannot conpose key for event-Id %ld in state with state-Id %ld", e->id, s->id);
            //SM_SYSLOG(SM_JSON, SM_LOG_ERR, error, "Malformed message JSON");
            //usleep(100);
            //return EXIT_FAILURE;
            //break;
    }
    e->key_length = strlen(e->key);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow filter key prepared");
        json_object_set_string(dtmp_o, "key", e->key);
        json_object_set_number(dtmp_o, "eventId", e->id);
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_setf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SETF cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SETF cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    strcpy(s->data, json_object_dotget_string(m, "data.data.decision"));
    if(json_object_has_value(m, "data.data.from_role")) {
        char* from = malloc(json_object_dotget_string_len(m, "data.data.from") + 1);
        char* from_role = malloc(json_object_dotget_string_len(m, "data.data.from_role"));
        from[0] = '$';
        strcpy(from + 1, json_object_dotget_string(m, "data.data.from"));
        strcpy(from_role, json_object_dotget_string(m, "data.data.from_role"));
        //const char* from = json_object_dotget_string(m, "data.data.from");
        sm_directory_set(s->tx->exec->dir, (const char *)from, from_role);
    }
    if(json_object_has_value(m, "data.data.to_role")) {
        char* to = malloc(json_object_dotget_string_len(m, "data.data.to") + 1);
        char* to_role = malloc(json_object_dotget_string_len(m, "data.data.to_role"));
        to[0] = '$';
        strcpy(to + 1, json_object_dotget_string(m, "data.data.to"));
        strcpy(to_role, json_object_dotget_string(m, "data.data.to_role"));
        //const char* to = json_object_dotget_string(m, "data.data.to");
        sm_directory_set(s->tx->exec->dir, (const char *) to, to_role);
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_reqf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "REQF cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "REQF cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *req_v = json_value_init_object();
    JSON_Object *req_o = json_value_get_object(req_v);
    json_object_set_string(req_o, "proto", "dtmp");
    json_object_set_string(req_o, "command", "request_flow");
    json_object_dotset_string(req_o, "data.from", json_object_get_string(m, "from"));
    json_object_dotset_string(req_o, "data.to", json_object_get_string(m, "to"));
    json_object_dotset_string(req_o, "data.proto", json_object_dotget_string(m, "data.proto"));
    json_object_dotset_string(req_o, "data.key", e->key);
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", DT_CLIENT);
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", req_v);
    json_string = json_serialize_to_string_pretty(ip_v);
    sm_event *er = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    strcpy(er->data, json_string);
    er->id = 1; 
    s->trace = er;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow requested from user");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "processed packet", mv);
        json_object_set_value(dtmp_o, "request packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}    

int dt_reqr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "REQR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "REQR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *req_v = json_value_init_object();
    JSON_Object *req_o = json_value_get_object(req_v);

    char *key = malloc(json_object_get_string_len(m, "from") + 1);
    key[0] = '$';
    strcpy(key + 1, json_object_get_string(m, "from"));
    void **tmp = sm_directory_get_ref(s->tx->exec->dir, key);
    char *from_role = tmp?(char *)(*tmp):NULL;
    key = malloc(json_object_get_string_len(m, "to") + 1);
    key[0] = '$';
    strcpy(key + 1, json_object_get_string(m, "to"));
    tmp = sm_directory_get_ref(s->tx->exec->dir, key);
    char *to_role = tmp?(char *)(*tmp):NULL;
    free(key);

    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    if(from_role  == NULL || to_role == NULL ) {
        e->id = 2051;
    }
    else {
        json_object_set_string(req_o, "proto", "dtmp");
        json_object_set_string(req_o, "command", "get_rule");
        json_object_dotset_string(req_o, "data.key", (char *)e->key);
        json_object_dotset_string(req_o, "data.from_role", from_role);
        json_object_dotset_string(req_o, "data.to_role", to_role);
        json_object_dotset_string(req_o, "data.key", e->key);
        json_object_dotset_string(req_o, "data.proto", json_object_get_string(m, "proto"));
        json_object_set_string(ip_o, "to", DT_CONTROLLER);
        json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(ip_o, "proto", "ip");
        json_object_set_boolean(ip_o, "to_report", 1);
        json_object_set_value(ip_o, "data", req_v);
        json_string = json_serialize_to_string_pretty(ip_v);
        sm_event *er = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        strcpy(er->data, json_string);
        er->id = 1; 
        s->trace = er;
        e->id = 1031;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule requested by router");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "processed packet", mv);
        json_object_set_value(dtmp_o, "request packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}    

int dt_chck(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "CHCK cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv); 
    if(strcmp(s->data, "allowed") == 0) {
        e->id = 2050;
    }
    else if(strcmp(s->data, "blocked") == 0) {
        e->id = 2049;
    }
    else {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "CHCK cannot parse decision", "Malformed decision value in s->data");
        return EXIT_FAILURE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "CHCK cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "flow checked");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "flow decision", s->data);
        json_object_set_string(dtmp_o, "key", e->key);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_getr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "GETR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "GETR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    json_object_dotset_string(m, "data.data.decision", (char *)s->data);
    json_string = json_serialize_to_string_pretty(mv);
    strcpy(e->data, json_string);
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "GETR request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}    

int dt_wrrt(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "WRRT cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "WRRT cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "deploy_rule");
    json_object_dotset_string(getr_o, "data.to_role", json_object_dotget_string(m, "data.data.to_role"));
    json_object_dotset_string(getr_o, "data.from_role", json_object_dotget_string(m, "data.data.from_role"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.decision", json_object_dotget_string(m, "data.data.decision"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "GETR response prepared received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_addr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ADDR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ADDR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    strcpy(s->data, json_object_dotget_string(m, "data.data.decision"));
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_xtrr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ADDR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ADDR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    json_object_dotset_string(m, "data.direction", json_object_dotget_string(m, "data.rule.direction"));
    //strcpy(s->data, json_serialize_to-string_pretty(json_object_dotget_value(m, "data.rule")));
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule was set up");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_askf(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ASKR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ASKR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "set_flow");
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    const char *to = json_object_dotget_string(m, "data.data.to");
    const char *from = json_object_dotget_string(m, "data.data.from");
    size_t i;
    char *to_role = NULL;
    char *from_role = NULL;
    for(i = 0; i < DT_NUM_OF_END_NODES; i++) {
        if(strcmp(dt_end_nodes[i], to) == 0) {
            to_role = dt_roles[dt_assigned_role[i]];
        }
        if(strcmp(dt_end_nodes[i], from) == 0) {
            from_role = dt_roles[dt_assigned_role[i]];
        }
    }
    if(to_role != NULL) {
        json_object_dotset_string(getr_o, "data.to_role", to_role);
        json_object_dotset_string(m, "data.data.to_role", to_role);
    }
    if(from_role != NULL) {
        json_object_dotset_string(getr_o, "data.from_role", from_role);
        json_object_dotset_string(m, "data.data.from_role", from_role);
    }
    json_string = json_serialize_to_string_pretty(mv);
    strcpy(e->data, json_string);
    json_object_dotset_string(getr_o, "data.from", json_object_dotget_string(m, "data.data.from"));
    json_object_dotset_string(getr_o, "data.to", json_object_dotget_string(m, "data.data.to"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "request_flow command received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_askr(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ASKR cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "ASKR cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *getr_v = json_value_init_object();
    JSON_Object *getr_o = json_value_get_object(getr_v);
    json_object_set_string(getr_o, "proto", "dtmp");
    json_object_set_string(getr_o, "command", "set_rule");
    json_object_dotset_string(getr_o, "data.decision", "allowed");
    json_object_dotset_string(getr_o, "data.to_role", json_object_dotget_string(m, "data.data.to_role"));
    json_object_dotset_string(getr_o, "data.from_role", json_object_dotget_string(m, "data.data.from_role"));
    json_object_dotset_string(getr_o, "data.key", json_object_dotget_string(m, "data.data.key"));
    json_object_dotset_string(getr_o, "data.proto", json_object_dotget_string(m, "data.data.proto"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", DT_CONTROLLER);
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", getr_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "rule sent after request flow command processing");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}    

int dt_s100(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "s100 cannot parse message", "malformed message json");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "s100 cannot read the message reporting flag", "malformed message json");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *s100_v = json_value_init_object();
    JSON_Object *s100_o = json_value_get_object(s100_v);
    json_object_set_string(s100_o, "proto", "sip");
    //json_object_set_string(s100_o, "", json_object_dotget_string(""));
    json_object_dotset_string(s100_o, "data.request-line.method", "100 Trying");
    json_object_dotset_string(s100_o, "data.request-line.version", "sip/2.0");
    json_object_dotset_string(s100_o, "data.via", json_object_dotget_string(m, "data.via"));
    json_object_dotset_string(s100_o, "data.from", json_object_dotget_string(m, "data.from"));
    json_object_dotset_string(s100_o, "data.to", json_object_dotget_string(m, "data.to"));
    json_object_dotset_string(s100_o, "data.call-id", json_object_dotget_string(m, "data.call-id"));
    json_object_dotset_string(s100_o, "data.cseq", json_object_dotget_string(m, "data.cseq"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", s100_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "s100 request received");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_sinv(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "S100 cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "S100 cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    JSON_Value *sinv_v = json_value_init_object();
    JSON_Object *sinv_o = json_value_get_object(sinv_v);
    json_object_set_string(sinv_o, "proto", "sip");
    //json_object_set_string(sinv_o, "", json_object_dotget_string(""));
    json_object_dotset_string(sinv_o, "data.Request-Line.Method", json_object_dotget_string(m, "data.Request-Line.Method"));
    json_object_dotset_string(sinv_o, "data.Request-Line.Request-URI", json_object_dotget_string(m, "data.Request-Line.Request-URI"));
    json_object_dotset_string(sinv_o, "data.Request-Line.Version", json_object_dotget_string(m, "data.Request-Line.Version"));
    json_object_dotset_string(sinv_o, "data.Via", json_object_dotget_string(m, "data.Via"));
    json_object_dotset_string(sinv_o, "data.From", json_object_dotget_string(m, "data.From"));
    json_object_dotset_string(sinv_o, "data.To", json_object_dotget_string(m, "data.To"));
    json_object_dotset_string(sinv_o, "data.Call-ID", json_object_dotget_string(m, "data.Call-ID"));
    json_object_dotset_number(sinv_o, "data.Max-Forwards", json_object_dotget_number(m, "data.Max-Forwards") - 1);
    json_object_dotset_string(sinv_o, "data.CSeq", json_object_dotget_string(m, "data.CSeq"));
    JSON_Value *ip_v = json_value_init_object();
    JSON_Object *ip_o = json_value_get_object(ip_v);
    json_object_set_string(ip_o, "to", json_object_get_string(m, "from"));
    json_object_set_string(ip_o, "from", sm_directory_get_name(s->tx->exec->dir, s->tx));
    json_object_set_string(ip_o, "proto", "ip");
    json_object_set_boolean(ip_o, "to_report", 1);
    json_object_set_value(ip_o, "data", sinv_v);
    sm_event *ea = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
    json_string = json_serialize_to_string_pretty(ip_v);
    strcpy(ea->data, json_string);
    ea->id = 1;
    s->trace = ea;
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "INVITE is forwarded");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_value(dtmp_o, "request packet", mv);
        json_object_set_value(dtmp_o, "response packet", ip_v);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

int dt_sips(sm_event *e, sm_state *s) {
    int result;
    char *json_string;
    sm_timestamp ts = sm_get_timestamp(); 
    JSON_Value *mv = json_parse_string((char *)e->data);
    if(mv == NULL) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SIPS cannot parse message", "Malformed message JSON");
        return EXIT_FAILURE;
    }
    JSON_Object *m = json_object(mv);  
    const char *command = json_object_dotget_string(m, "data.Request-Line.Method");
    size_t i;
    for (i = 0; i < DT_SIP_NUM_OF_METHODS; i++) {
        if(strcmp(dt_sip_methods[i], command) == 0) {
            e->id = i + DT_PROTO_SPACE;
            break;
        }
    }
    if(i == DT_SIP_NUM_OF_METHODS) {
        e->id = DT_PROTO_SPACE + DT_COMMAND_SPACE + DT_STATUS_SPACE;
    }
    result = json_object_get_boolean(m, "to_report");
    if(result < 0) {
        SM_SYSLOG(SM_JSON, SM_LOG_ERR, "SIPS cannot read the message reporting flag", "Malformed message JSON");
        result = DT_DEFAULT_REPORTING_FLAG;
    }
    if(result) { // to_report
        JSON_Value *dtmp_v = json_value_init_object();
        JSON_Object *dtmp_o = json_value_get_object(dtmp_v);
        json_object_set_string(dtmp_o, "reporter", sm_directory_get_name(s->tx->exec->dir, s->tx));
        json_object_set_string(dtmp_o, "proto", "dtmp");
        json_object_set_string(dtmp_o, "command", "report");
        json_object_set_string(dtmp_o, "event", "sip method selected");
        json_object_set_string(dtmp_o, "timestamp", ts.timestring);
        json_object_set_string(dtmp_o, "command", dt_dtmp_commands[i]);
        json_object_set_number(dtmp_o, "assigned_event_id", e->id);
        json_object_set_value(dtmp_o, "packet", mv);
        sm_event *ep = sm_queue_dequeue(*sm_directory_get_ref(s->tx->exec->dir, DT_POOL));
        json_string = json_serialize_to_string_pretty(/*ip_v*/dtmp_v);
        strcpy(ep->data, json_string);
        ep->priority[0] = -ts.seconds;
        ep->priority[1] = -ts.nanoseconds;
        sm_pqueue_enqueue(ep, *sm_directory_get_ref(s->tx->exec->dir, DT_COLLECTOR));
    }
    return 0;
}

