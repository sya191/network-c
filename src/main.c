#include "recieve.h"
#include "interface.h"
#include "iface.h"
#include "ethernet.h"
#include "ip.h"
#include "string.h"
#include "utils.h"
#include <linux/if_ether.h>
#include <arpa/inet.h>

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

    start_rx(&interface);

    char ip_frame[sizeof(ip_t) + 13] = {0};
    ip_t *hdr = (ip_t *)ip_frame;
    uint32_t macbook_ip = convert_ip("192.168.1.102");
    hdr->dest_addr = ntohl(macbook_ip);
    hdr->src_addr = ntohl(interface.src_ip);

    // copy payload
    char *msg = "Hello World!";
    memcpy(ip_frame + sizeof(ip_t), msg, 13);


    for (;;) {
        send_eth_to_ip(
            ip_frame,
            ETH_P_IP,
            macbook_ip,
            sizeof(ip_t) + 13,
            interface
        );
    }
}