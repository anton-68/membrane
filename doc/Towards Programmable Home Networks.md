# Towards Programmable Home Networks

## Abstract

In the proposed research we investigate how the key components of home networks (Wi-Fi routers, IoT Gateways, Smart Home Controllers etc.) can be transformed into the remotely programmable agents and the network configuration and maintenance can be automated.

The complexity of home networks will become comparable to the complexity of enterprise ones soon. Growing markets of Smart Home and Internet of Things and the increasing rate the new services are introduced at are contributing extensively to this change. 

Network  devices used in home networks are transforming into fully-featured 'headless' servers using Linux derivatives as an operating system. Another important trend is the wide adoption of IP (either IPv4 or IPv6) as the protocol for endpoint connectivity. 

All these changes make the network configuring more complex and make home networks more prone to failures and various malicious activities. 

To overcome all these problems the various middleware and API concepts are introduced and actively developed. In this project we will propose the network architecture based on SDN concept widely adopted now in service providers networks and we will introduce the specialized application execution environment for adding programmability into home network elements. 

With such middleware implemented the adding new applications, changing service parameters or adding new devices will become simple and convenient task. Any network extensions and upgrades also will be simplified. 

Please see the Additional Information section for the list of design priorities taken into account for the developed middleware architecture.

## Differentiation

Users of Smart Home and IoT applications want to be allowed to choose equipment, services and applications freely. Any service offer including multiple or complicated mutual dependencies and vendor constraints raises strong frustration. 

To meet this requirement the network and application features must be decoupled both from each other and from the underlying platform by some layer of software which supports asynchronous interactions and is able to work on the most common platforms.   

For user such transformation will allow:
- Simplified adding of new services and devices in the plug-n-play manner.
- Secure and scalable home network.
- Wider choice of devices, networking technologies and services.

For service  providers and for their customer introduction of such middleware will mean:
- Faster time-to-market for partners solution.
- Simplified certification of new hardware and applications.
- The complete Home-Network-as-a-Service offer.
- Decoupling the home network technology evolution from the applications evolution.

## Status of Research Proposal

The working version of the target architecture is ready and the software prototype of execution environment is under development. 

The current architecture includes the following components:
- Event-based asynchronous execution environment.
- Application stateless functions in precompiled executable native libraries.
- (Re)configurable interpreted service finite state machines (FSM).
- Repository of FSM instances keeping states and contexts of active service sessions. 
- Protocol adaptation layer consisting of parsing and composing procedures for messages of various protocols.

## Additional Information

The initial list of design priorities for the developed architecture includes the following goals:
- To decouple network logic, such routing and filtering, from the networking hardware and OS API. 
- To let service designers to add new applications easily without equipment change or complicated provisioning.
- To open home networks for active implementation of AI and ML methods both in the interest of the applications and for network optimization and protection. 
- To allow dynamical allocation of computational task on different network elements to support load balancing and overload protection of home network processors.
- To replace manual configuration with automated role- and policy-based provisioning.
- To protect home networks with smart instantly updated malicious action prevention logic sourced from the centralized repository maintained by service provider.
- To make the complete Home-Network-as-a-Service offer possible.
- To allow easily introduce 3rd-party solution based on vendor's proprietary hardware. 
