#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT1 2201
#define PORT2 2202
#define BUFFER_SIZE 1024

int create_socket() {
    int opt = 1;
    int fd = 0;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int bind_socket(int listen_fd, uint16_t port) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT1);
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[Server] bind() failed.");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 3) < 0) {
        perror("[Server] listen() failed.");
        exit(EXIT_FAILURE);
    }

    int conn_fd = 0;
    if ((conn_fd = accept(listen_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("[Server] accept() failed.");
        exit(EXIT_FAILURE);
    }
    return conn_fd;
}

void read_to_buffer(char[] buffer, int conn_fd) {
    memset(buffer, 0, BUFFER_SIZE);
    int nbytes = read(conn_fd, buffer, BUFFER_SIZE);
    if (nbytes <= 0) {
        printf("read_to_buffer failed")
        exit(EXIT_FAILURE);
    }
}

// EXPECTED:
// begin: B 11 11
// shoot: S 1 1
// query: Q
// forfeit: F
//
// RESPONSE:
// error: E 101
// halt: H 1
// acknowledgement: A
// S response R <ships_remaining> <{M for miss, H for hit}>
// Q response: G <ships_remaining> for_each_guess:<{‘M’ for miss, ‘H’ for hit} column# row#>
int main() {
    int listen_fd1 = create_socket();
    int listen_fd2 = create_socket();
    int conn_fd1 = bind_socket(listen_fd1, PORT1);
    int conn_fd2 = bind_socket(listen_fd2, PORT2);
    printf("sockets create/bind/listen/accept done\n");

    char buffer[BUFFER_SIZE] = {0};
    read_to_buffer(buffer, conn_fd1);

    // 0 for blank tiles
    // 12345 for ships, positive if player1, negative if player2
    int width, int height;
    int *board = NULL;
    if (strcmp(buffer, "B") != 0) {
        error();
        goto cleanup;
    }

    sscanf(buffer, "B %d %d", &width, &height);
    if (width < 10 || height < 10) {
        printf("width or height less than 10");
    }
    board = malloc(sizeof(int) * width * height);

    //     The server should attempt to bind/listen/accept on both ports. (2201 is player 1 and 2202 is player 2)
    // The server should expect a Begin packet from player 1 that contains a board size of at least 10 x 10.
    // Acknowledge or send errors until a Begin is received from both player 1
    //  The server should expect a Begin packet from player 2 that contains no parameters
    // The server should expect a Initialize packet on each port (first player 1, then player 2)
    // Acknowledge or send errors until a valid Initialization is received from each player
    // The server should expect either a Shoot, Query, or Forfeit packet from player 1.
    // If the packet was Query, reply and expect another packet
    // If the packet was Shoot, reply and expect a packet from the other player
    // If the packet was Forfeit, reply with Halts to both players
    // Otherwise send errors until a valid packet arrives
    // The server then should expect a Shoot, Query, or Forfeit packet from player 2.
    // Same behavior as above
    // If a shoot packet sinks the last ship, send a Halt to both players



    /*{
        printf("[Server] Enter message: ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0';
        if (strcmp(buffer, "quit") == 0) {
            printf("[Server] Quitting...\n");
            send(conn_fd, buffer, strlen(buffer), 0);
            break;
        }
        send(conn_fd, buffer, strlen(buffer), 0);
    }*/

cleanup:
    close(conn_fd1);
    close(conn_fd2);
    close(listen_fd1);
    close(listen_fd2);
    return EXIT_SUCCESS;
}
