Self define networking macros (e.g. ETH_P_IP) and remove linux header dependencies

./tests/test_arp.cpp:#include <linux/if_ether.h>
./tests/test_eth.cpp:#include <linux/if_ether.h>
./tests/test_fragment.cpp:#include <linux/if_ether.h>
./src/ethernet.c:#include <linux/if_ether.h>
./src/interface.c:#include <linux/if_packet.h>
./src/interface.c:#include <linux/if_arp.h>
./src/arp.c:#include <linux/if_arp.h>
./src/arp.c:#include <linux/if_ether.h>
./src/ip.c:#include <linux/if_ether.h>
./src/main.c:#include <linux/if_ether.h>
