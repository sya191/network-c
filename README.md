# Network-C (A userspace TCP/IP stack)
The goal of this project is to complete a vertical slice of TCP functionality. That is, to be able to connect to a remote server (that uses UNIX sockets) and send/recv data.

## Resources Used
- TCP/IP Illustrated Volume 1: The Protocols, 2nd Edition (Kevin R. Fall, W. Richard Stevens)
- RFC 826

## Modules 
- [ ] TCP
- [ ] IPv4
- [ ] ICMP
- [x] ARP (arp.h)
- [x] Ethernet (ethernet.h)

### Design
Each module should aim to be decoupled from other modules to the best of their abilities. Practically, this means that each module should delegate the functionality needed to the module below or above it.

<img width="588" height="448" alt="image" src="https://github.com/user-attachments/assets/1c8cacde-d762-410c-b251-0eb6c245d77c" />

*Diagram from Chapter 1 of TCP/IP Illustrated Volume 1: The Protocols, 2nd Edition (Kevin R. Fall, W. Richard Stevens)*


All in all, this results in each module having one function to **send** payloads from the module above it, and one to **receive** payloads from the one below it.

**Example recv:** ethernet -> demux on type field (hand to IP module) -> hand to TCP module -> ... etc. (stripping each header along the way)

**Example send:** TCP -> IP -> Ethernet

## Hardware dependencies & Support for different NICs
Because this stack is meant to be hardware agnostic (up to the network card level), all functions that write to/recv from a NIC accept a generic function pointer that may be defined by the user. For the purposes of manual testing, interface.h and interface.c defines the set of functions used to test on a VM running Ubuntu Linux via an ethernet card.

A SOCK_RAW/AF_PACKET Unix socket interface is defined in interface.c/h.
