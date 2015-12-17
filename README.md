# ACN-Project
Implementation of OLSR Routing Protocol

Brief about the project:
The Optimized Link State Routing Protocol (OLSR)[1] is an IP routing protocol optimized for mobile ad hoc networks, which can also be used on other wireless ad hoc networks.
OLSR is a proactive link-state routing protocol, which uses hello and topology control (TC) messages to discover and then disseminate link state information throughout the mobile ad hoc network. Individual nodes use this topology information to compute next hop destinations for all nodes in the network using shortest hop forwarding paths.

This project requires topology.txt file as input which will be read by the controller to determine which nodes are in one hop vicinity and if they are unidirectional or bidirectional.
Depending on the number of nodes in the wireless network, node can be executed multiple numbe of times varying the node id.
Usage: node 1 2 "message from 1" 25 
which means node 1 wants to send message to node 2 at time 25.

compilation  instruction: 
g++ node.cpp -o node
g++ controller.cpp -o controller
