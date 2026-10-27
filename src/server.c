/* Cleaned edition of Noor Ullah's MSc network-programming math-server task. */
#include "net.h"
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>

static const char *calculate(const char *request, int64_t *answer) {
    char *end;
    errno=0;
    long a=strtol(request,&end,10);
    if (errno || end==request || a<INT32_MIN || a>INT32_MAX) return "invalid operand";
    while (isspace((unsigned char)*end)) ++end;
    const char op=*end;
    if (op!='+' && op!='-' && op!='*' && op!='/') return "invalid operator";
    const char *right=end+1;
    errno=0;
    long b=strtol(right,&end,10);
    if (errno || end==right || b<INT32_MIN || b>INT32_MAX) return "invalid operand";
    while (isspace((unsigned char)*end)) ++end;
    if (*end) return "trailing input";
    switch (op) {
        case '+': *answer=(int64_t)a+b; break;
        case '-': *answer=(int64_t)a-b; break;
        case '*': *answer=(int64_t)a*b; break;
        case '/':
            if (b==0) return "division by zero";
            *answer=(int64_t)a/b; break;
        default: return "invalid operator";
    }
    return NULL;
}

static void serve(int fd) {
    char request[128], response[160];
    int n=read_line(fd,request,sizeof(request));
    if (n==0) return;
    if (n<0) {
        const char *error=n==-2 ? "ERR invalid or oversized request\n" :
                          n==-3 ? "ERR request timeout\n" : "ERR incomplete request\n";
        (void)send_all(fd,error,strlen(error));
        return;
    }
    int64_t answer=0;
    const char *error=calculate(request,&answer);
    if (error) (void)snprintf(response,sizeof(response),"ERR %s\n",error);
    else (void)snprintf(response,sizeof(response),"OK %" PRId64 "\n",answer);
    (void)send_all(fd,response,strlen(response));
}

int main(int argc, char **argv) {
    if (argc<2 || argc>3) { fprintf(stderr,"Usage: tcp_server PORT [BIND_IPV4]\n"); return 1; }
    int port=port_number(argv[1],1);
    if (port<0) { fprintf(stderr,"invalid port\n"); return 1; }
    struct sockaddr_in address={0};
    address.sin_family=AF_INET; address.sin_port=htons((uint16_t)port);
    if (inet_pton(AF_INET,argc==3 ? argv[2] : "127.0.0.1",&address.sin_addr)!=1) {
        fprintf(stderr,"invalid IPv4 address\n"); return 1;
    }
    (void)signal(SIGPIPE,SIG_IGN);
    (void)signal(SIGCHLD,SIG_IGN);
    int listener=socket(AF_INET,SOCK_STREAM,0);
    if (listener<0) { perror("socket"); return 1; }
    int reuse=1;
    (void)setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    if (bind(listener,(struct sockaddr *)&address,sizeof(address))<0 || listen(listener,16)<0) {
        perror("listen/bind"); close(listener); return 1;
    }
    socklen_t length=sizeof(address);
    if (getsockname(listener,(struct sockaddr *)&address,&length)<0) {
        perror("getsockname"); close(listener); return 1;
    }
    printf("READY %u\n",(unsigned)ntohs(address.sin_port)); fflush(stdout);
    for (;;) {
        int client=accept(listener,NULL,NULL);
        if (client<0) {
            if (errno==EINTR) continue;
            perror("accept"); close(listener); return 1;
        }
        pid_t child=fork();
        if (child<0) { perror("fork"); close(client); continue; }
        if (child==0) {
            close(listener);
            (void)alarm(6); // Bound each demonstration connection, including a blocked send.
            serve(client); close(client); _exit(0);
        }
        close(client);
    }
}
