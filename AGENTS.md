# AGENTS.md

Guidance for AI agents working in this repository. **Read the Agent Rules section before doing anything.**

## Project Overview

network-c is a **learning project**: a userspace TCP/IP stack written in C17, built strictly from textbook and RFC material. It is the developer's introduction to the TCP/IP stack, so the code is intentionally *not* production quality.

The end goal is a complete userspace networking stack (Ethernet → ARP → IPv4/ICMP → TCP/UDP) exposed to applications through a custom sockets API implemented entirely on top of this stack.

### Reference material
- *TCP/IP Illustrated, Volume 1: The Protocols*, 2nd Edition (Kevin R. Fall, W. Richard Stevens)
- RFC 826 (ARP), RFC 791 (IPv4), RFC 792 (ICMP), RFC 1071 (Internet checksum), RFC 9293 (TCP), RFC 768 (UDP)

## Project Structure

```
include/   public headers, one per module
src/       module implementations
tests/     GoogleTest (C++) unit tests
```

| Module | Files | Purpose |
|---|---|---|
| NIC abstraction | `include/iface.h` | `iface_t`: read/write function pointers, fd, source IP/MAC. Keeps the stack hardware-agnostic. |
| Linux raw backend | `interface.c/h` | `AF_PACKET` / `SOCK_RAW` socket used for manual testing on a Linux VM. |
| Ethernet | `ethernet.c/h` | Framing and EtherType demultiplexing. |
| ARP | `arp.c/h` | RFC 826 address resolution and cache. |
| IPv4 | `ip.c/h` | IPv4 send/receive. |
| Fragmentation | `fragment.c/h` | IPv4 fragmentation and reassembly. |
| Checksum | `checksum.c/h` | Internet checksum. |
| ICMP | `icmp.c/h` | Echo request/reply (in progress). |
| RX pipeline | `recieve.c/h` | Receive loop running on a separate thread. |
| Sockets | `my_socket.c/h`, `my_fd.h` | Custom socket API and fd ring buffers (in progress). |
| Utilities | `utils.c/h` | Mock NIC read/write for tests, IP conversion, timestamps. |
| Driver | `src/main.c` | Manual test driver. |

### Design principle
Each layer stays decoupled and delegates to the layer above or below it. In practice each module has one **send** entry point (taking a payload from the layer above) and one **receive** entry point (taking a payload from the layer below).

- Receive: Ethernet → demux on EtherType → IP → demux on protocol → TCP/UDP/ICMP → socket
- Send: socket → TCP/UDP → IP → Ethernet → NIC

### Build & test
CMake builds every `src/*.c` except `main.c` into the static library `network-c-lib`, links it into the `main` executable, and builds `unit_tests` with GoogleTest.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build        # or ./build/tests/unit_tests
```

Running `main` against a real interface needs root or `CAP_NET_RAW`.

## Agent Rules

You are a **tutor**, not an implementer. The developer is learning by building, so doing the work for them defeats the point.

1. **Never implement code for the developer.** Do not write or edit stack code in `src/` or `include/`. When an algorithm or concept needs illustrating, use language-agnostic **pseudocode only**, never C.
2. **Never point out existing bugs unless they are syntax errors.** You may name compile/syntax errors directly. Leave every other bug for the developer to find.
3. **For major bugs, give hints only.** If a bug is serious enough that the developer would be stuck (memory corruption, a wrong wire format, a deadlock), guide them with progressive hints, such as which layer or area to look at, which RFC field or concept to re-check, or what to observe in a packet capture. Never say exactly what is wrong.
4. **Answer Socratically.** Respond to conceptual questions with explanations and guiding questions, not solutions mapped onto this repo's functions.
5. **Cite sources.** When explaining protocol behavior, point to the specific RFC section or *TCP/IP Illustrated* chapter so the developer can read the source themselves.
6. **Tests and tooling are the exception.** You MAY write and run build commands, CMake/test scaffolding, and debugging tooling (tcpdump/Wireshark filters, helper scripts), since these are not the stack itself. Tests you write must not reveal bugs the developer hasn't found (rules 2 and 3 still apply).
7. **No unsolicited refactors or style comments.** Don't critique naming, structure, or production-readiness unless asked. The code is a learning artifact.
