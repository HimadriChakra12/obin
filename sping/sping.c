// avgping.c
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>

#define TIMEOUT_MS 1000

static const char *hosts[] = {
    "1.1.1.1",
    "9.9.9.9",
    "discord.com",
    "instagram.com",
    "google.com",
    "208.67.222.222"
};

enum {
    HOSTS = (int)(sizeof(hosts) / sizeof(hosts[0]))
};

static unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (unsigned short)~sum;
}

static double diff_ms(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) * 1000.0 +
           (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(void)
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    pid_t pid = getpid() & 0xffff;
    unsigned short seq = 0;

    struct sockaddr_in addr[HOSTS];
    struct timespec sent[HOSTS];

    for (int i = 0; i < HOSTS; i++) {
        struct addrinfo hints = {0}, *res;

        memset(&addr[i], 0, sizeof(addr[i]));
        hints.ai_family = AF_INET;

        if (getaddrinfo(hosts[i], NULL, &hints, &res) != 0) {
            fprintf(stderr, "Cannot resolve %s\n", hosts[i]);
            continue;
        }

        memcpy(&addr[i], res->ai_addr, sizeof(struct sockaddr_in));
        freeaddrinfo(res);
    }

    while (1) {

        seq++;

        for (int i = 0; i < HOSTS; i++) {

            struct icmphdr pkt;
            memset(&pkt, 0, sizeof(pkt));

            pkt.type = ICMP_ECHO;
            pkt.code = 0;
            pkt.un.echo.id = htons(pid);
            pkt.un.echo.sequence = htons(seq * HOSTS + i);
            pkt.checksum = checksum(&pkt, sizeof(pkt));

            clock_gettime(CLOCK_MONOTONIC, &sent[i]);

            sendto(sock,
                   &pkt,
                   sizeof(pkt),
                   0,
                   (struct sockaddr *)&addr[i],
                   sizeof(addr[i]));
        }

        double sum = 0;
        int count = 0;

        struct timeval tv = {
            .tv_sec = 1,
            .tv_usec = 0
        };

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);

        while (select(sock + 1, &rfds, NULL, NULL, &tv) > 0) {

            unsigned char buf[1500];
            struct sockaddr_in from;
            socklen_t len = sizeof(from);

            ssize_t n = recvfrom(sock,
                                 buf,
                                 sizeof(buf),
                                 0,
                                 (struct sockaddr *)&from,
                                 &len);

            if (n <= 0)
                continue;

            struct iphdr *ip = (struct iphdr *)buf;
            struct icmphdr *icmp =
                (struct icmphdr *)(buf + ip->ihl * 4);

            if (icmp->type != ICMP_ECHOREPLY)
                continue;

            if (ntohs(icmp->un.echo.id) != pid)
                continue;

            int s = ntohs(icmp->un.echo.sequence);

            if (s < seq * HOSTS || s >= seq * HOSTS + HOSTS)
                continue;

            int idx = s % HOSTS;

            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);

            sum += diff_ms(sent[idx], now);
            count++;

            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
        }

        if (count)
            printf("%.0f ms\n", sum / count);
        else
            puts("N/A");

        fflush(stdout);
        sleep(1);
    }
}
