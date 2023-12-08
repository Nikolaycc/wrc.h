#include <stdio.h>

#define WRC_IMPL
#include <wrc.h>

int main(void) {
    if (geteuid())
    panic("Please run program with sudo");
    
    wrc_ifc_list ifalist;
    wrc_ifc_init(&ifalist);

    wrc_get_ifcs(&ifalist);
    logf("ifalist.len = %zu", ifalist.len);

    wrc_print_ifalist(ifalist);
    
    // logf("geteuid() return %d", geteuid());
    // logf("getuid() return %d", getuid());

    // char addr[NI_MAXHOST], netmask[NI_MAXHOST];
    // printf("char addr[NI_MAXHOST], netmask[NI_MAXHOST] => %zu, %zu\n", sizeof addr, sizeof netmask);

    // struct rtnl_link_stats *stats;
    // printf("struct rtnl_link_stats *stats => %zu\n", sizeof stats);
    
    return 0;
}
