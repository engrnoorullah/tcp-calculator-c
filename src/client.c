/* One request per connection, matching the original coursework task. */
#include "net.h"

int main(int argc, char **argv) {
    if (argc!=4) { fprintf(stderr,"Usage: tcp_client IPV4 PORT 'EXPRESSION'\n"); return 1; }
    int port=port_number(argv[2],0);
    size_t length=strlen(argv[3]);
    if (port<0 || length==0 || length>126 || strchr(argv[3],'\n') || strchr(argv[3],'\r')) {
        fprintf(stderr,"invalid port or request\n"); return 1;
    }
    struct sockaddr_in address={0};
    address.sin_family=AF_INET; address.sin_port=htons((uint16_t)port);
    if (inet_pton(AF_INET,argv[1],&address.sin_addr)!=1) { fprintf(stderr,"invalid IPv4 address\n"); return 1; }
    (void)signal(SIGPIPE,SIG_IGN);
    (void)alarm(6);
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if (fd<0) { perror("socket"); return 1; }
    if (connect(fd,(struct sockaddr *)&address,sizeof(address))<0) { perror("connect"); close(fd); return 1; }
    if (send_all(fd,argv[3],length)<0 || send_all(fd,"\n",1)<0) { perror("send"); close(fd); return 1; }
    char response[160];
    int n=read_line(fd,response,sizeof(response));
    close(fd);
    if (n<=0) { fprintf(stderr,"missing or invalid response\n"); return 1; }
    puts(response);
    return strncmp(response,"OK ",3)==0 ? 0 : 2;
}
