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

char[5] SEND_ERROR_BUFFER = "";
void send_error(int conn_fd, int message) {
    // error is always 5 characters long
    sprintf(&SEND_ERROR_BUFFER, "E %d", message);
    send(conn_fd, SEND_ERROR_BUFFER, 5, 0);
}

void send_a(int conn_fd, char* message, int len) {
    send(conn_fd, message, len, 0);
}

int get_piece_blocks(int piece_type, int rotation, int blocks[3][2]) {
    switch (piece_type) {
        case 1: // Square 
            blocks[0][0] = 0; blocks[0][1] = 1;
            blocks[1][0] = 1; blocks[1][1] = 0;
            blocks[2][0] = 1; blocks[2][1] = 1;
            break;

        case 2: // Line
            if (rotation % 2 == 0) { // Vertical (rotations 0, 2)
                blocks[0][0] = 1; blocks[0][1] = 0;
                blocks[1][0] = 2; blocks[1][1] = 0;
                blocks[2][0] = 3; blocks[2][1] = 0;
            } else { // Horizontal (rotations 1, 3)
                blocks[0][0] = 0; blocks[0][1] = 1;
                blocks[1][0] = 0; blocks[1][1] = 2;
                blocks[2][0] = 0; blocks[2][1] = 3;
            }
            break;

        case 3: // Red Squiggle
            if (rotation % 2 == 0) { // 1 and 3
                blocks[0][0] = 0; blocks[0][1] = 1;
                blocks[1][0] = 1; blocks[1][1] = 1;
                blocks[2][0] = 1; blocks[2][1] = 2;
            } else { // 2 and 4
                blocks[0][0] = -1; blocks[0][1] = 0;
                blocks[1][0] = -1; blocks[1][1] = 1;
                blocks[2][0] = -2; blocks[2][1] = 1;
            }
            break;

        case 4: // Orange L
            switch (rotation) {
                case 0:
                    blocks[0][0] = -1; blocks[0][1] = 0;
                    blocks[1][0] = -1; blocks[1][1] = 0;
                    blocks[2][0] = -2; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = -1; blocks[0][1] = 0;
                    blocks[1][0] = 0; blocks[1][1] = 1;
                    blocks[2][0] = 0; blocks[2][1] = 2;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = -2; blocks[2][1] = 1;
                    break;
                case 3:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = 1; blocks[2][1] = 2;
                    break;
            }
            break;

        case 5: // Green Squiggle
            if (rotation % 2 == 0) { // Rotations 1, 3
                blocks[0][0] = 0; blocks[0][1] = 1;
                blocks[1][0] = -1; blocks[1][1] = 1;
                blocks[2][0] = -1; blocks[2][1] = 2;
                break;
            } else { // Rotations 2, 4
                blocks[0][0] = -1; blocks[0][1] = 0;
                blocks[1][0] = -1; blocks[1][1] = 1;
                blocks[2][0] = -2; blocks[2][1] = 1;
                break;
            }
            break;

        case 6: // Pink L
            switch (rotation) {
                case 0:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = 2; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = -1; blocks[0][1] = 0;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = -1; blocks[2][1] = 2;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = -1; blocks[1][1] = 0;
                    blocks[2][0] = -2; blocks[2][1] = 0;
                    break;
                case 3:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = -1; blocks[2][1] = 2;
                    break;
            }
            break;

        case 7: // T
            switch (rotation) {
                case 0:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = -1; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = -1; blocks[2][1] = 1;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = 0; blocks[2][1] = 2;
                    break;
                case 3:
                    blocks[0][0] = -1; blocks[0][1] = 0;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = -2; blocks[2][1] = 0;
                    break;
            }
            break;

        default:
            return 0; // Invalid piece type
    }
    return 1; 
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

    // 0 for blank tiles
    // 12345 for ships, positive if player1, negative if player2
    int width, int height;
    int *board = NULL;
    char buffer[BUFFER_SIZE] = {0};

    // begin
    while (1) {
        read_to_buffer(buffer, conn_fd1);
        if (strcmp(buffer, "B") != 0) {
            send_error(conn_fd1, 100)); // Invalid packet type (Expected Begin packet)
            continue;
        }
        int scanned = sscanf(buffer, "%c %d %d", &packet_type, &width, &height);
        if (sscanf(buffer, "B %d %d", &width, &height) != 2) {
            send_error(conn_fd1, 200)); // Invalid Begin packet (invalid number of parameters)
            continue;
        }
        if (width < 10 || height < 10) {
            // 200: Invalid Begin packet (invalid number of parameters)
            // width or height less than 10, this is best matching error
            send_error(conn_fd1, 200));
            continue;
        }
        board = malloc(sizeof(int) * width * height);
        send_a(conn_fd1, "A", 1);
        break;
    }

    while (1) {
        read_to_buffer(buffer, conn_fd2);
        if (strcmp(buffer, "B") != 0) {
            send_error(conn_fd1, 100)); // Invalid packet type (Expected Begin packet)
            continue;
        }
        break;
    }

    while (1) {
        read_to_buffer(buffer, conn_fd1);
        if (strcmp(buffer, "I") != 0) {
            send_error(conn_fd1, 101)); // Invalid packet type (Expected Init packet)
            continue;
        }

        int error = 0;
        char *buffer_r = buffer + 2; // skip "I "
        for (int i = 0; i < 5; i++) {
            int type, rot, col, row;

            if (sscanf(buffer, "%d %d %d %d", &type, &rot, &col, &row) != 4) {
                error = 201;
                send_error(conn_fd1, 201)); // Invalid Init packet (invalid number of parameters)
                continue;
            }
            if (row < 0 || col < 0 || col >= width || row >= height) {
                error = 300;
                send_error(conn_fd1, 300); // 300: Invalid Initialize packet (shape out of range)
                continue;
            }

            int location = board + row * width + col;
            int rotation = (rot - 1) % 4;
            int[3][2] offsets = piece_offsets(type, rotation, blocks, width);
            int[4] locations = {0};
            location[3] = location;

            int yes_continue = 0; // extra locations
            for (int i = 0; i < 3; i++) {
                int this_row = row+offsets[i][0];
                int this_col = col+offsets[i][0];
                extra_locations[i] = board + this_row*width + this_col;
                if (this_row < 0 || this_row >= height || this_col < 0 || this_col > width) {
                    error = 301
                    send_error(conn_fd1, 301); // 301: Invalid Initialize packet (rotation out of range)
                    break;
                }
            }
            if (yes_continue) { continue; }

            for (int i = 0; i < 4; i++) {
                int location = locations[i];
                if (board[location] != 0) {
                    error = 303;
                    send_error(conn_fd, 303);
                    break;
                }
            }

            // attempt to place pieces
            // 301: Invalid Initialize packet (rotation out of range)
            // 302: Invalid Initialize packet (ship doesn’t fit in game board)
            // 303: Invalid Initialize packet (ships overlap)
        }

        send_a(conn_fd1, "A", 1);
        break;
    }


    //      The server should attempt to bind/listen/accept on both ports. (2201 is player 1 and 2202 is player 2)
    //      The server should expect a Begin packet from player 1 that contains a board size of at least 10 x 10.
    // Acknowledge or send errors until a Begin is received from both player 1
    //
    // The server should expect a Begin packet from player 2 that contains no parameters
    //
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

    close(conn_fd1);
    close(conn_fd2);
    close(listen_fd1);
    close(listen_fd2);
    return EXIT_SUCCESS;
}
