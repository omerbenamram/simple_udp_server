#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <netdb.h>
#include <zconf.h>

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

int main(int argc, char **argv) {
    char *message;

    if (argc < 2) // no arguments were passed
    {
        message = "Hello world";
    } else {
        message = argv[1];
    }

    struct sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_sock < 0) {
        panic("ERROR opening socket");
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        panic("ERROR: setsockopt");
    }

    char out[BUFSIZE];
    char in[BUFSIZE];

    memset(out, 0, BUFSIZE);
    memset(in, 0, BUFSIZE);

    in_addr_t *client_addr_str;
    ssize_t message_len;

    sprintf(out, "%s", message);

    while (TRUE) {
        if (sendto(client_sock, out, BUFSIZE, 0, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
            panic("ERROR: sendto");
        } else {
            ssize_t resplen = recvfrom(client_sock, in, BUFSIZE, 0, NULL, 0);

            if (resplen < 0) {
                panic("ERROR: failed to recv response from server");
            }

            fprintf(stdout, "Server sent: %s\n", in);
            fflush(stdout);
        }
        sleep(1);
    }


}

#pragma clang diagnostic pop