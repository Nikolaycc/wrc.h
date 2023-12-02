#ifndef WRC_H
#define WRC_H

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <netdb.h>
#include <linux/if_link.h>

#define panicf(_msg_, ...) { \
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stderr, "\x1B[31mPANIC\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t" _msg_ "\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, __VA_ARGS__); \
    exit(1); \
}

#define logf(_msg_, ...) { \
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stdout, "\x1B[34mLOG\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t" _msg_ "\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, __VA_ARGS__); \
}

#define panic(_msg_) { \
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stderr, "\x1B[31mPANIC\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t%s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, (_msg_)); \
    exit(1); \
}

#define log(_msg_) {\
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stdout, "\x1B[34mLOG\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t%s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, (_msg_)); \
}

#define ARRAY_LEN(x) sizeof((x))/sizeof((x)[0])

#define MAX_TEXTFORMAT_BUFFERS 2048
#define MAX_TEXT_BUFFER_LENGTH 2048
#define MAX_IFNAME 32
#define MAX_IFACE 16

typedef struct {
    char name[MAX_IFNAME];
    size_t mtu;
    uint8_t flag;
} wrc_ifc; 

typedef struct {
    wrc_ifc ifc[MAX_IFACE];
    size_t len;
} wrc_ifc_list; 

void todo(FILE *, const char *, int);
char *wrc_format(const char *, ...);

void wrc_get_ifcs();

#endif // WRC_H

#ifdef WRC_IMPL

void wrc_get_ifcs() {
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];
    logf("NI_MAXHOST %d", NI_MAXHOST);
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
    can free list later. */

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
        continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
        form of the latter for the common families). */

        logf("%-8s %s (%d)", ifa->ifa_name,
        (family == AF_PACKET) ? "AF_PACKET" :
        (family == AF_INET) ? "AF_INET" :
        (family == AF_INET6) ? "AF_INET6" : "???",
        family);

        /* For an AF_INET* interface address, display the address. */

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                logf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            
            logf("address: <%s>", host);
            
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats *stats = ifa->ifa_data;

            logf("tx_packets = %10u; rx_packets = %10u\n"
            "\ttx_bytes   = %10u; rx_bytes   = %10u",
            stats->tx_packets, stats->rx_packets,
            stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs(ifaddr);
}

void todo(FILE *f, const char *text, int code) {
    fprintf((f != NULL) ? f : stderr, text, 0);
    exit(code);
}

char *wrc_format(const char *text, ...) {
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = {0};
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);

    va_list args;
    va_start(args, text);
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    index += 1;
    if (index >= MAX_TEXTFORMAT_BUFFERS)
        index = 0;

    return currentBuffer;
}

#endif // WRC_IMPL
