#ifndef NET_H
#define NET_H
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static inline int port_number(const char *text, int allow_zero) {
    char *end;
    errno=0;
    long port=strtol(text,&end,10);
    if (errno || !*text || *end || port<(allow_zero ? 0 : 1) || port>65535) return -1;
    return (int)port;
}

static inline int64_t now_ms(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC,&t)<0) return -1;
    return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000;
}

static inline int send_all(int fd, const char *data, size_t length) {
    while (length) {
        ssize_t n=send(fd,data,length,0);
        if (n<0 && errno==EINTR) continue;
        if (n<=0) return -1;
        data+=n; length-=(size_t)n;
    }
    return 0;
}

/* TCP is a byte stream: a recv call need not contain a complete line. */
static inline int read_line(int fd, char *buffer, size_t capacity) {
    size_t used=0;
    int64_t start=now_ms();
    if (start<0) return -1;
    const int64_t deadline=start+5000;
    while (used+1<capacity) {
        int64_t current=now_ms();
        if (current<0) return -1;
        if (current>=deadline) return -3;
        struct pollfd p={fd,POLLIN,0};
        int ready=poll(&p,1,(int)(deadline-current));
        if (ready<0 && errno==EINTR) continue;
        if (ready<0) return -1;
        if (ready==0) return -3;
        char c;
        ssize_t n=recv(fd,&c,1,0);
        if (n<0 && errno==EINTR) continue;
        if (n<0) return -1;
        if (n==0) return used ? -4 : 0;
        if (c=='\n') { buffer[used]='\0'; return (int)used+1; }
        if (c=='\0') return -2;
        buffer[used++]=c;
    }
    return -2;
}
#endif
