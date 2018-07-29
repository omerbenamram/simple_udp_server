#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <netdb.h>

void panic(char *msg) {
    perror(msg);
    exit(-1);
}

#define SERVER_PORT 8080
#define BUFSIZE 4096
#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( FALSE )
#else
#define DEBUG_PRINT(...) do{ } while ( FALSE )
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"


int main() {
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_sock < 0) {
        panic("ERROR opening socket");
    }

    char out[BUFSIZE];
    char in[BUFSIZE];
    memset(out, 0, BUFSIZE);
    memset(in, 0, BUFSIZE);

    struct msghdr message;
    struct iovec iov[1];

    in_addr_t *client_addr_str;
    ssize_t message_len;

    sprintf(out, "Hello world!");

    iov[0].iov_base = out;
    iov[0].iov_len = BUFSIZE;

    memset(&message, 0, sizeof(message));
    message.msg_name = &serveraddr;
    message.msg_namelen = sizeof(serveraddr);
    message.msg_iov = iov;
    message.msg_iovlen = 1;
    message.msg_control = NULL;
    message.msg_controllen = 0;

    if (sendmsg(client_sock, &message, 0) < 0) {
        panic("ERROR send_message failed");
    } else {
        ssize_t resplen = recvfrom(client_sock, in, BUFSIZE, 0, NULL, 0);

        if (resplen < 0) {
            panic("ERROR: failed to recv response from server");
        }

        fprintf(stdout, "Server sent: %s\n", in);
        fflush(stdout);
    }

}

#pragma clang diagnostic pop