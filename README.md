# Network-C (A userspace TCP/IP stack)
The goal of this project is to complete a vertical slice of TCP functionality. That is, to be able to connect to a remote server (that uses UNIX sockets) and send/recv data.

## Modules 
- [ ] TCP
- [ ] IPv4
- [ ] ICMP
- [ ] ARP (arp.h)
- [x] Ethernet (ethernet.h)

### Design
Each module should aim to be decoupled from other modules to the best of their abilities. Practically, this means that each module should delegate the functionality needed to the module below or above it.

All in all, this results in each module having one function to **recieve** payloads from the module above it, and one to **send** payloads from the one below it.

**Example recv:** ethernet -> demux on type field (hand to IP module) -> hand to TCP module -> ... etc. (stripping each header along the way)

**Example send:** TCP -> IP -> Ethernet