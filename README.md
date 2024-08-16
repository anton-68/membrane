# The Membrane Technology for Protecting Home Networks 

(The proof of concept project)

## Preface

Imagine the network evolved to the level where its elements are able to cooperate for understanding whether the messages passing through the network are consistent with each other and with the existing policies. Imagine the network elements able to extract cooperatively new security rules. 

## Differentiation

With all the changes occurring in home networks with the rise of Internet of Things and Smart Home technologies the protection of home networks becomes a complicated task. Multiple access technologies, mesh connectivity, multiple devices with direct access to the Internet and other changes make traditional perimeter-based models insufficient. 

To avoid the limitations of the traditional approaches, in this project we're focusing on the distributed traffic analysis performed by key network elements such as Wi-Fi routers, IoT Gateways and Smart Home Controllers in cooperation with each other and with service provider network. These elements form a dense layer between home network elements and the internet providing active filtering of the messages passing in and out - hence the Membrane name of the project.

Each Membrane 'cell' has some builtin rule distribution and application logic similar to the routing protocols. And some 'cells' have builtin rule extraction AI/ML capabilities.

## Description of the Idea

The approach adopted in this project is based on checking the consistency of the values and parameters of each message with other messages and with other significant external events. Such events can be either intentionally triggered messages, such as a receipt from the sender sent over an alternate channel, or regular events in the service logic. 

The consistency assurance may include the following checks:
- Scenarios integrity and roles consistency.
- Consistency of message parameters between protocols in the stack.
- Spatial consistency - compliance of the subscriber's location with the requested service. 
- Temporal consistency - compliance of the service request time with the requested service.
- Anomalies detection using artificial intelligence and machine learning methods.

The main sources of rules for traffic analysis are: 
- Statically provisioned role-based default rule sets by service provider.
- Customized static rules provisioned by customer.
- Dynamic rules learned by AI/ML agent.

## Some Use-Cases to illustrate the possible usage related directly to security

### Malicious attempt to access CCTV camera through compromised Smart TV set

*Detection:* The Smart TV set is trying to establish video session with the CCTV camera.

*System Reaction:* Traffic is blocked, user and service provider service centre are notified.  

*Explanation:* The provisioned policy allows CCTV cameras send traffic only towards the registered customer's smartphones and laptops.

### Unauthorized attempt of VoIP call

*Detection:* The inbound VoIP call is coming without the receipt from the approved VoIP server.

*System Reaction:* The call is blocked, user and service provider service centre are notified.  

*Explanation:* All communications are mediated by the known servers deployed within operator's infrastructure.

### New medical wearable IoT device attempted connection

_Detection:_ New device attempted 6LoWPAN connection. 

_System Reaction:_ User approval is requested.

_Explanation:_ Unknown device is trying to attache to home 6LoWPAN network. User informed and asked to confirm attachment. In case of positive result new rule is provisioned. 

### Unexpected voice command received by Smart Audio system 

_Detection:_ The traffic associated with an unexpected voice command is detected.

_System reaction:_ Traffic is blocked, user is notified.

_Explanation:_ The voice command is detected by Smart Audio system or traffic associated with voice command is detected while room was empty.

## The main goals of the project

The main goals of this project from the functional / business application perspective are:
1. The choice of the relevant model and implementation of mechanisms for security application blocks deployment, integration and running.
2. The choice or development of the relevant method for describing and applying traffic processing rules (the rule engine).
3. The development of the core algorithms for consistency rules extraction< installing and application.
 
## Technological Description

To implement and maintain efficiently the necessary logic within the Home Network will require some extended remote application management capabilities which in turn assumes that some kind of application execution environment is installed on the key elements of the Home Network.

Bringing the extended application management capabilities will create the opportunity to use home network resources from intelligent traffic analysis inside the home network without exposing it to the external system and to protect sensible personal information.

Observing current trends on the market of the embedded solutions offered for use in Home Networks and Smart Home solutions we can make the following two assumptions:
1. With no regards to the radio technology in use communications between all network elements will employ IP - IPv4 or IPv6.
2. With no regards to the particular architecture of the of Home Network elements used for traffic processing (routers,switches, MQTT brokers, various gateways etc.) we can assume that some version of Linux can run on it.

The initial set of application blocks to be run in the proposed execution environment architecture includes:
1. Protocol parsers.
2. Packet Filters.
3. Rule Engine.
4. ML model loader/connector.
5. Operation and management tools.
   
The main goal of this project from the implementation perspective is the choice and proving the architecture and capabilities of such execution environment.

## Some more possible Use-Cases

We're providing here additional possible use-cases to illustrate the solution concept. These use-cases are not directly connected to the information security but still important from the safety and to the protection of personal information:

### Broken sensor node or depleted sensor node battery

_Detection:_ The sensor node doesn't send data for more than configured threshold.

_System Reaction:_ User is notified.

_Explanation:_ The learned behavior of the node to send data on equal time intervals. 

### Parental control policy applied

_Detection:_ Child's attempted to access internet from one of his devices.

_System reaction:_ The Internet access interrupted for child's smartphone, tablet and computer.

_Explanation:_ Screen time limits provisioned by parents is triggered.

### Smart Home energy conservation

_Detection:_ ML Edge System detected inefficiency in floor heater schedule.

_System reaction:_ Floor heater schedule is updated, user is notified.

_Explanation:_ Edge ML engine running on Smart Home controller node spotted the repeating interval of wasting energy (heater is on for a while when the room is empty).

### Crowdsourcing traffic patterns

_Detection:_ Traffic filter on Wi-Fi router spots configured pattern.

_System reaction:_ The collected portion of traffic is processed by edge AI/ML solution and the extracted depersonalized rules uploaded to operator's repository for operator's subscribers community benefit.

_Explanation:_ The traffic patterns of specific interest is collected and analyzed. Examples are detected malicious activities, equipment failures etc. The traffic and the associated personal information do not leave the home network.

### Unusual behavior of elderly person is detected

_Detection:_  ML Edge System detected unusual pattern in motion, presence, home equipment usage etc. by elderly person.

_System reaction:_ Notify person's relatives and optionally care or emergency service.

_Explanation:_ Sensors and computational resources in the Smart Home system can be configured to solve other tasks.

## See also:
- The SM.EXEC library [https://github.com/anton-68/sm.exec] - is the core framework of the Membrane project
- The SM.EXEC whitepaper [https://www.researchgate.net/publication/379564545_SMEXEC_A_Network_Service_Logic_Execution_Environment_Architecture]
- The doc/ directory
- 
## NOTE! This code is using outdated version of SM.EXEC!
