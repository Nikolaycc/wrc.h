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
#include <sys/types.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <linux/if_link.h>
#include <ifaddrs.h>
#include <netinet/in.h>

#define panicf(_fmt_, ...) { \
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stderr, "\x1B[31mPANIC\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t" _fmt_ "\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, __VA_ARGS__); \
    exit(1); \
}

#define logf(_fmt_, ...) { \
    time_t t = time(NULL); \
    struct tm tm = *localtime(&t); \
    fprintf(stdout, "\x1B[34mLOG\x1B[0m [%02d:%02d:%02d] (%s:%d)\n  ->\t" _fmt_ "\n", tm.tm_hour, tm.tm_min, tm.tm_sec, __FILE__, __LINE__, __VA_ARGS__); \
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

#define INET4 0b100
#define INET6 0b010
#define ISTATS 0b001

typedef struct {
    char name[MAX_IFNAME];
    int family;
    char addr[NI_MAXHOST], netmask[NI_MAXHOST];
    char addr6[NI_MAXHOST], netmask6[NI_MAXHOST];
    struct rtnl_link_stats *stats;

    unsigned int is_inet4:1;
    unsigned int is_inet6:1;
    unsigned int is_stats:1;
} wrc_ifc;

typedef struct {
    wrc_ifc ifc[MAX_IFACE];
    size_t len;
} wrc_ifc_list;

void todo(FILE *, const char *, int);
char *wrc_format(const char *, ...);

void wrc_ifc_init(wrc_ifc_list *);
void wrc_get_ifcs(wrc_ifc_list *);
void wrc_print_ifalist(const wrc_ifc_list);
void wrc_print_ifa(const wrc_ifc_list, size_t);

#endif // WRC_H

#ifdef WRC_IMPL

void wrc_ifc_init(wrc_ifc_list *ifc) {
    ifc->len = 0;
    memset(ifc->ifc, 0, MAX_IFACE);
}

void wrc_get_ifcs(wrc_ifc_list *ifc) {
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST], nhost[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
        continue;

        family = ifa->ifa_addr->sa_family;
        wrc_ifc tmp = {0};
        wrc_ifc* lifc = &tmp;
        int inclen = 1;
        
        for (int v = 0; v < ifc->len; v++) {
            if (strcmp(ifa->ifa_name, ifc->ifc[v].name) == 0) {
                lifc = &ifc->ifc[v];
                inclen = 0;
            } 
        }
        
        lifc->family = family;
        strcpy(lifc->name, ifa->ifa_name);

        if (family == AF_INET) {            
            lifc->is_inet4 = 1;            
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), lifc->addr, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                logf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            int n = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), lifc->netmask, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);
            if (n != 0) {
                logf("getnameinfo() failed: %s\n", gai_strerror(n));
                exit(EXIT_FAILURE);
            }
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            lifc->is_stats = 1;
            lifc->stats = ifa->ifa_data;
        } else if (family == AF_INET6) {
            lifc->is_inet6 = 1;
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), lifc->addr6, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                logf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            int n = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in6), lifc->netmask6, NI_MAXHOST,
            NULL, 0, NI_NUMERICHOST);
            if (n != 0) {
                logf("getnameinfo() failed: %s\n", gai_strerror(n));
                exit(EXIT_FAILURE);
            }
        }
        ifc->ifc[ifc->len] = *(wrc_ifc*)lifc;
        if (inclen)
        ifc->len++;
    }

    freeifaddrs(ifaddr);
}

void wrc_print_ifalist(const wrc_ifc_list ifalist) {
    for (int i = 0; i < ifalist.len; i++) {
        printf("\nifalist.ifc[%d] = (wrc_ifc) {\n", i);
        printf("\t.name = %s\n", ifalist.ifc[i].name);
        printf("\t.family = %d\n", ifalist.ifc[i].family);
        if (ifalist.ifc[i].is_inet4) {
            printf("\t.addr = %s\n", ifalist.ifc[i].addr);
            printf("\t.netmask = %s\n", ifalist.ifc[i].netmask);
        }
        if (ifalist.ifc[i].is_inet6) {
            printf("\t.addr6 = %s\n", ifalist.ifc[i].addr6);
            printf("\t.netmask6 = %s\n", ifalist.ifc[i].netmask6);
        }
        if (ifalist.ifc[i].is_stats) {
            printf("\t.stats->tx_packets = %10u\n\t.stats->rx_packets = %10u\n"
            "\t.stats->ttx_bytes = %10u\n\t.stats->rx_bytes = %10u\n",
            ifalist.ifc[i].stats->tx_packets, ifalist.ifc[i].stats->rx_packets,
            ifalist.ifc[i].stats->tx_bytes, ifalist.ifc[i].stats->rx_bytes);
        }
        printf("}\n");
    }
}

void wrc_print_ifa(const wrc_ifc_list ifalist, size_t idx) {
    int i = idx;
    if (!(ifalist.len < idx)) {
        printf("\nifalist.ifc[%d] = (wrc_ifc) {\n", i);
        printf("\t.name = %s\n", ifalist.ifc[i].name);
        printf("\t.family = %d\n", ifalist.ifc[i].family);
        if (ifalist.ifc[i].is_inet4) {
            printf("\t.addr = %s\n", ifalist.ifc[i].addr);
            printf("\t.netmask = %s\n", ifalist.ifc[i].netmask);
        }
        if (ifalist.ifc[i].is_inet6) {
            printf("\t.addr6 = %s\n", ifalist.ifc[i].addr6);
            printf("\t.netmask6 = %s\n", ifalist.ifc[i].netmask6);
        }
        if (ifalist.ifc[i].is_stats) {
            printf("\t.stats->tx_packets = %10u\n\t.stats->rx_packets = %10u\n"
            "\t.stats->ttx_bytes = %10u\n\t.stats->rx_bytes = %10u\n",
            ifalist.ifc[i].stats->tx_packets, ifalist.ifc[i].stats->rx_packets,
            ifalist.ifc[i].stats->tx_bytes, ifalist.ifc[i].stats->rx_bytes);
        }
        printf("}\n");
    }
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
