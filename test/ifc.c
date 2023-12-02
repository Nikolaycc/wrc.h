#include <stdio.h>

#define WRC_IMPL
#include <wrc.h>

int main(void) {
    if (geteuid())
        panic("Please run program with sudo");

    wrc_get_ifcs();

    logf("geteuid() return %d", geteuid());
    logf("getuid() return %d", getuid());

    return 0;
}
