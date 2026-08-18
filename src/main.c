#include "recieve.h"
#include "interface.h"
#include "iface.h"
#include "utils.h"

/**
 * Need to run with sudo
 */
int main()
{
    iface_t interface = {
        .fd = create_interface(),
        .write = write_interface,
        .read = read_interface,
        .src_mac = {0x2, 0x0, 0x0, 0x0, 0x6, 0x7},
        .src_ip = convert_ip("192.168.1.104")
    };

    start_rx(interface);
    for (;;) {

    }
}