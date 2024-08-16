/* SM.EXEC 
   SM application library for Telekom Challenge PoC
   anton.bondarenko@gmail.com */

#ifndef DTAG_TC_APPS_H
#define DTAG_TC_APPS_H

#include "../src/sm.h"

// Console
#define DT_COLLECTOR "CollectorQueue"
#define DT_POOL "pool"

// Router
#define DT_ROUTER "Router"
#define DT_ROUTER_QUEUE "RouterQueue"
#define DT_ROUTER_SM_NAME "RouterSM"
#define DT_FLOW_PROCESSOR_SM_NAME "FlowProcessor"
#define DT_FLOW_PROCESSOR_ARRAY "FlowProcessorArray"

// User Equipment
#define DT_CLIENT "SmartPhone_1"
#define DT_CLIENT_QUEUE "SmartPhone_1_q"
#define DT_CLIENT_SM_NAME "ClientSM"

// 42-Server
#define DT_SERVER "Server"
#define DT_SERVER_QUEUE "ServerQueue"
#define DT_SERVER_SM_NAME "ServerSM"

// SIP Server
#define DT_SIP_SERVER "SIPServer"
#define DT_SIP_SERVER_QUEUE "SIPServerQueue"
#define DT_SIP_SERVER_SM_NAME "SIPServerSM"
#define DT_SIP_SESSION_SM_NAME "SIPSessionSM"
#define DT_SIP_SESSION_ARRAY "SIPSessionArray"

// Home Network Controller
#define DT_CONTROLLER "Controller"
#define DT_CONTROLLER_QUEUE "ControllerQueue"
#define DT_CONTROLLER_SM_NAME "ControllerSM"
#define DT_RULE_ENGINE_SM_NAME "RuleEngineSM"
#define DT_RULE_ENGINE_ARRAY "RuleEngineArray"

// Executors
#define DT_TX0 "tx0"
#define DT_TX1 "tx1"
#define DT_TX2 "tx2"

// Defaults
#define DT_EVENT_SIZE 1024
#define DT_STATE_SIZE 4096
#define DT_QUEUE_SIZE 4096
#define DT_ARRAY_SIZE 12 // this is power of 2 hense for 5 it is 32 elements
#define DT_DEFAULT_REPORTING_FLAG 1;

// Vocabularies
#define DT_PROTO_SPACE 1024
#define DT_COMMAND_SPACE 1024
#define DT_STATUS_SPACE 1024
#define DT_SIP_SPACE 1024
#define DT_NUM_OF_PROTOCOLS 6
#define DT_DTMP_NUM_OF_COMMANDS 11
#define DT_DTMP_NUM_OF_STATUS_VALUES 5
#define DT_SIP_NUM_OF_METHODS 7
#define DT_NUM_OF_END_NODES 15

// Apps
int dt_send(sm_event *e, sm_state *s);
int dt_rout(sm_event *e, sm_state *s);
int dt_recv(sm_event *e, sm_state *s);
int dt_orig(sm_event *e, sm_state *s); 
int dt_ssel(sm_event *e, sm_state *s);
int dt_dtmp(sm_event *e, sm_state *s);
int dt_appl(sm_event *e, sm_state *s); // DEPRECATED
int dt_drop(sm_event *e, sm_state *s); 
int dt_wait(sm_event *e, sm_state *s); // DEPRECATED
int dt_noap(sm_event *e, sm_state *s);
int dt_serv(sm_event *e, sm_state *s);
int dt_fltr(sm_event *e, sm_state *s); // DEPRECATED
int dt_setk(sm_event *e, sm_state *s);
int dt_setf(sm_event *e, sm_state *s);
int dt_reqf(sm_event *e, sm_state *s);
int dt_reqr(sm_event *e, sm_state *s);
int dt_chck(sm_event *e, sm_state *s);
int dt_getr(sm_event *e, sm_state *s);
int dt_addr(sm_event *e, sm_state *s);
int dt_xtrr(sm_event *e, sm_state *s);
int dt_askf(sm_event *e, sm_state *s);
int dt_askr(sm_event *e, sm_state *s);
int dt_sips(sm_event *e, sm_state *s);
int dt_sinv(sm_event *e, sm_state *s);
int dt_s100(sm_event *e, sm_state *s);
int dt_wrrt(sm_event *e, sm_state *s);

#endif //DTAG_TC_APPS_H
