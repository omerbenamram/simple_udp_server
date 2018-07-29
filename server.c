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

    // Initialize server address struct
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // Same as 0.0.0.0
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        panic("ERROR binding");
    }

    // https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/apis/recvms.htm
    while (TRUE) {
        struct sockaddr_in clientaddr;
        // Clean buffer
        memset(in, 0, BUFSIZE);
        memset(out, 0, BUFSIZE);
        memset(&clientaddr, 0, sizeof(clientaddr));

        struct msghdr incoming_message;
        struct iovec iov_in[1];

        in_addr_t *client_addr_str;
        ssize_t message_len;

        memset(&incoming_message, 0, sizeof(incoming_message));
        iov_in[0].iov_base = &in[0];
        iov_in[0].iov_len = BUFSIZE;

        // Client addr will be stored here
        incoming_message.msg_name = &clientaddr;
        incoming_message.msg_namelen = sizeof(clientaddr);

        incoming_message.msg_iov = iov_in;
        incoming_message.msg_iovlen = 1;
        incoming_message.msg_control = NULL;
        incoming_message.msg_controllen = 0;

        message_len = recvmsg(sockfd, &incoming_message, 0);
        DEBUG_PRINT("DEBUG: got message of len %zi\n", message_len);
        DEBUG_PRINT("DEBUG: message content: %s\n", in);

        if (message_len < 0) {
            panic("ERROR recvmsg failed");
        } else if (incoming_message.msg_flags & MSG_TRUNC) {
            // Of couse we could warn here.. but panic for example.
            panic("ERROR datagram too large for buffer message truncated");
        } else {
            // Datagram is good! send echo replay.
            client_addr_str = &clientaddr.sin_addr.s_addr;

            struct hostent *hostp = gethostbyaddr((const char *) client_addr_str, sizeof(client_addr_str), AF_INET);
            if (hostp == NULL) {
                panic("ERROR gethostbyaddr failed");
            }

            DEBUG_PRINT("DEBUG: Answering: \"Hello %s, you've sent: %s\"\n", hostp->h_name, in);
            sprintf(out, "Hello %s, you've sent: %s", hostp->h_name, in);

            struct msghdr outgoing_message;
            struct iovec iov_out[1];
            iov_out[0].iov_base = out;
            iov_out[0].iov_len = BUFSIZE;

            memset(&outgoing_message, 0, sizeof(outgoing_message));
            outgoing_message.msg_name = &clientaddr;
            outgoing_message.msg_namelen = sizeof(clientaddr);
            outgoing_message.msg_iov = iov_out;
            outgoing_message.msg_iovlen = 1;
            outgoing_message.msg_control = NULL;
            outgoing_message.msg_controllen = 0;

            if (sendmsg(sockfd, &outgoing_message, 0) < 0) {
                panic("ERROR send_message failed");
            }

        }

    }
}

#pragma clang diagnostic pop