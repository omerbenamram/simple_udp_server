#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <netdb.h>

#define DEBUG

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

    char in[BUFSIZE];
    char out[BUFSIZE];

    fprintf(stdout, "INFO: server starting\n");
    fflush(stdout);

    // Create a socket object.
    // Three parameters are:
    // Socket Domain - AF_INET for IPv4
    // Socket Type - SOCK_DGRAM for UDP
    // Socket Protocol - 0 for default.
    // Protocol can also be many other types:
    // ex. IPPROTO_UDPLITE (UDP Lite) to support variable-length checksums.
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        panic("ERROR opening socket");
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // INADDR_ANY - bind to "0.0.0.0" (all interfaces)
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        panic("ERROR binding");
    }

    while (TRUE) {
        /*
         * Notice how we have no other state outside this while loop!
         * we can possibly serve a huge number of client without opening any new sockets!
         */
        struct sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);
        in_addr_t *client_addr_str;

        // Clean buffers
        memset(in, 0, BUFSIZE);
        memset(out, 0, BUFSIZE);
        memset(&clientaddr, 0, sizeof(clientaddr));

        /*
         * Packets are sent individually and are checked for integrity only if they arrive.
         * Packets have definite boundaries which are honored upon receipt,
         * meaning a read operation at the receiver socket will yield an entire message as it was originally sent!
         */
        ssize_t message_len = recvfrom(sockfd, in, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientaddr_len);
        DEBUG_PRINT("DEBUG: got message of len %zi\n", message_len);
        DEBUG_PRINT("DEBUG: message content: %s\n", in);

        if (message_len < 0) {
            panic("ERROR recvmsg failed");
        } else {
            // Datagram is good! send echo replay.
            client_addr_str = &clientaddr.sin_addr.s_addr;

            struct hostent *hostp = gethostbyaddr((const char *) client_addr_str, sizeof(client_addr_str), AF_INET);
            if (hostp == NULL) {
                panic("ERROR gethostbyaddr failed");
            }

            DEBUG_PRINT("DEBUG: Answering: \"Hello %s, you've sent: %s\"\n", hostp->h_name, in);
            sprintf(out, "Hello %s, you've sent: %s", hostp->h_name, in);


            if (sendto(sockfd, out, BUFSIZE, 0, (const struct sockaddr *) &clientaddr, clientaddr_len) < 0) {
                panic("ERROR send_message failed");
            }
        }

    }
}

#pragma clang diagnostic pop