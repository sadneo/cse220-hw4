#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <time.h>

#define PORT1 2201
#define PORT2 2202
#define BUFFER_SIZE 1024

const int BOARD_MAX_SIZE = 100;
const int P_QUERY = 2;
const int P_VALID = 10;
const int P_OFF_BOARD = 2;
const int P_HISTORY = 2;

const int P_SUM = 16;
const int P_CHOICES = 4;

int print_board(int board[], int width, int height) {
    for (int row = 0; row < height; row++) {
        printf("\t");
        for (int col = 0; col < width; col++) {
            int tile = board[row * width + col];
            if (tile == -1) {
                printf("▓");
            } else if (tile == 0) {
                printf(".");
            } else {
                printf("%d", tile);
            }
        }
        printf("\n");
    }
}

int piece_offsets(int piece_type, int rot, int blocks[3][2]) {
    int rotation = (rot - 1) % 4;
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
                blocks[1][0] = -1; blocks[1][1] = 1;
                blocks[2][0] = -1; blocks[2][1] = 2;
            } else { // 2 and 4
                blocks[0][0] = 1; blocks[0][1] = 0;
                blocks[1][0] = 1; blocks[1][1] = 1;
                blocks[2][0] = 2; blocks[2][1] = 1;
            }
            break;

        case 4: // Orange L
            switch (rotation) {
                case 0:
                    blocks[0][0] = 1; blocks[0][1] = 0;
                    blocks[1][0] = 2; blocks[1][1] = 0;
                    blocks[2][0] = 2; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = 1; blocks[0][1] = 0;
                    blocks[1][0] = 0; blocks[1][1] = 1;
                    blocks[2][0] = 0; blocks[2][1] = 2;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = 2; blocks[2][1] = 1;
                    break;
                case 3:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = -1; blocks[2][1] = 2;
                    break;
            }
            break;

        case 5: // Green Squiggle
            if (rotation % 2 == 0) { // Rotations 1, 3
                blocks[0][0] = 0; blocks[0][1] = 1;
                blocks[1][0] = 1; blocks[1][1] = 1;
                blocks[2][0] = 1; blocks[2][1] = 2;
                break;
            } else { // Rotations 2, 4
                blocks[0][0] = 1; blocks[0][1] = 0;
                blocks[1][0] = 0; blocks[1][1] = 1;
                blocks[2][0] = -1; blocks[2][1] = 1;
                break;
            }
            break;

        case 6: // Pink L
            switch (rotation) {
                case 0:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = -2; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = 1; blocks[0][1] = 0;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = 1; blocks[2][1] = 2;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 1; blocks[1][1] = 0;
                    blocks[2][0] = 2; blocks[2][1] = 0;
                    break;
                case 3:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = 1; blocks[2][1] = 2;
                    break;
            }
            break;

        case 7: // T
            switch (rotation) {
                case 0:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = 0; blocks[1][1] = 2;
                    blocks[2][0] = 1; blocks[2][1] = 1;
                    break;
                case 1:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = 1; blocks[2][1] = 1;
                    break;
                case 2:
                    blocks[0][0] = 0; blocks[0][1] = 1;
                    blocks[1][0] = -1; blocks[1][1] = 1;
                    blocks[2][0] = 0; blocks[2][1] = 2;
                    break;
                case 3:
                    blocks[0][0] = 1; blocks[0][1] = 0;
                    blocks[1][0] = 1; blocks[1][1] = 1;
                    blocks[2][0] = 2; blocks[2][1] = 0;
                    break;
            }
            break;

        default:
            return 0; // Invalid piece type
    }

    return 1; 
}

int weighted_random_choice() {
    const int weights[] = {P_QUERY, P_VALID, P_OFF_BOARD, P_HISTORY};
    const int n_choices = sizeof(weights) / sizeof(weights[0]);

    // Calculate total weight
    int total_weight = 0;
    for (int i = 0; i < n_choices; i++) {
        total_weight += weights[i];
    }

    // Generate a random number between 0 and total_weight - 1
    int random_number = rand() % total_weight;

    // Determine which choice is selected
    int cumulative_weight = 0;
    for (int i = 0; i < n_choices; i++) {
        cumulative_weight += weights[i];
        if (random_number < cumulative_weight) {
            return i; // Return the index of the selected choice
        }
    }

    return -1; // Should not reach here
}

void generate_initialize(FILE *file, int *board, int width, int height) {
    int ship_data[20] = {0};
    int retry = 1;
    while (retry) {
        memset(board, 0, sizeof(int) * width * height);
        retry = 0;
        for (int ship_no = 0; ship_no < 5; ship_no++) {
            int type = rand() % 7 + 1;
            int rot = rand() % 4 + 1;
            int col = rand() % width;
            int row = rand() % height;

            ship_data[ship_no*4+0] = type;
            ship_data[ship_no*4+1] = rot;
            ship_data[ship_no*4+2] = col;
            ship_data[ship_no*4+3] = row;

            int offsets[3][2] = {0};
            piece_offsets(type, rot, offsets);
            int locations[4] = {0};
            locations[0] = row * width + col;
            for (int i = 0; i < 3; i++) {
                int offset_row = row+offsets[i][0];
                int offset_column = col+offsets[i][1];
                locations[i+1] = offset_row * width + offset_column;
                if (offset_row < 0 || offset_row >= height || offset_column < 0 || offset_column >= width) {
                    retry = 1;
                    break;
                }
            }

            for (int i = 0; i < 4; i++) {
                int location = locations[i];
                if (board[location] != 0) {
                    retry = 1;
                    break;
                }
                board[location] = ship_no + 1;
            }
        }
    }
    fprintf(file, "I %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", ship_data[0], ship_data[1], ship_data[2], ship_data[3], ship_data[4], ship_data[5], ship_data[6], ship_data[7], ship_data[8], ship_data[9], ship_data[10], ship_data[11], ship_data[12], ship_data[13], ship_data[14], ship_data[15], ship_data[16], ship_data[17], ship_data[18], ship_data[19]);
}

// int generate_random(int *shoot_board, int hit, int width, int height) {
//     int *rand_board = malloc(sizeof(int) * width * height);
//     int rand_count = 0;
//
//     if (hit == -1) {
//         for (int row = 0; row < height; row++) {
//             for (int col = 0; col < width; col++) {
//                 if (shoot_board[row * width + col] == hit) {
//                     rand_board[rand_count] = row * width + col;
//                     rand_count++;
//                 }
//             }
//         }
//         if (rand_count == 0) {
//             generate_random(shoot_board, 0, width, height);
//         }
//     } else if (hit == 0) {
//         for (int col = 0; col < width; col++) {
//             if (hit == -1 && shoot_board[row * width + col] == hit) {
//                 rand_board[rand_count] = row * width + col;
//                 rand_count++;
//             } else if (hit == 0 && shoot_board[row * width + col] >= 0) {
//                 rand_board[rand_count] = row * width + col;
//                 rand_count++;
//             }
//         }
//         if (rand_count == 0) {
//             generate_random(shoot_board, 1, width, height);
//         }
//     }
//
//     int rand_index = rand() % rand_count;
//     int rand_location = rand_board[rand_index];
//     free(rand_board);
//     return rand_location;
// }

int generate_shoot(FILE *file, int *shoot_board, int width, int height) {
    int choice = weighted_random_choice();
    if (choice == 0) { //query
        fprintf(file, "Q\n");
        return 1;
    } else if (choice == 1 || choice == 3) { //valid
        int shoot_row = rand() % height;
        int shoot_col = rand() % width;

        shoot_board[shoot_row * width + shoot_col] = -1;
        fprintf(file, "S %d %d\n", shoot_row, shoot_col);

        int encountered_ships[5] = {0};
        int ships_remaining = 0;
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                int ship = shoot_board[row*width+col];
                if (ship > 0) {
                    encountered_ships[ship - 1] = 1; // offset from 1-5 to 0-4
                }
            }
        }
        for (int i = 0; i < 5; i++) {
            ships_remaining += encountered_ships[i];
        }
        if (ships_remaining == 0) {
            return 2;
        }
        return 0;
    } else if (choice == 2) { //off_board
        int shoot_row = rand() % 100 + height;
        int shoot_col = rand() % 100 + width;
        fprintf(file, "S %d %d\n", shoot_row, shoot_col);
        return 1;
    }
    // } else if (choice == 3) { //history
    //     int rand_location = generate_random(shoot_board, -1, width, height);
    //     int shoot_row = rand_location / width;
    //     int shoot_col = rand_location % width;
    //     fprintf(file, "S %d %d\n", shoot_row, shoot_col);
    //     return 1;
    // }
}

int main() {
    srand(time(NULL));
    remove("player1.out");
    remove("player2.out");
    FILE *player1 = fopen("player1.out", "a");
    FILE *player2 = fopen("player2.out", "a");

    int width = rand() % (BOARD_MAX_SIZE-10) + 10;
    int height = rand() % (BOARD_MAX_SIZE-10) + 10;
    fprintf(player1, "B %d %d\n", width, height);
    fprintf(player2, "B\n");

    int *board1 = malloc(sizeof(int) * width * height);
    int *board2 = malloc(sizeof(int) * width * height);

    generate_initialize(player1, board1, width, height);
    generate_initialize(player2, board2, width, height);
    printf("Board 1\n");
    print_board(board1, width, height);
    printf("Board 2\n");
    print_board(board2, width, height);

    int not_done = 1;
    while (not_done) {
        printf("Starting\n");
        while (1) {
            int x = generate_shoot(player1, board2, width, height);
            if (x == 1) {
                printf("retrying\n");
                continue;
            }
            if (x == 2) {
                printf("done fr fr\n");
                not_done = 0;
            }
            break;
        }
        printf("Starting\n");
        while (1) {
            int x = generate_shoot(player2, board1, width, height);
            if (x == 1) {
                printf("retrying\n");
                continue;
            }
            if (x == 2) {
                printf("done fr fr\n");
                not_done = 0;
            }
            break;
        }
    }
    printf("Board 1\n");
    print_board(board1, width, height);
    printf("Board 2\n");
    print_board(board2, width, height);

    fprintf(player1, "Q\n");
    fprintf(player2, "Q\n");
    fprintf(player1, "Q\n");
    fprintf(player2, "Q\n");
    free(board1);
    free(board2);
    return EXIT_SUCCESS;
}
