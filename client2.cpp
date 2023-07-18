#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <limits>
#include <nlohmann/json.hpp>
#include <fstream> // Add this line
#include <condition_variable>

/* int board[8][8] =
{ -1,-2,-3,-4,-5,-3,-2,-1,
 -6,-6,-6,-6,-6,-6,-6,-6,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  6, 6, 6, 6, 6, 6, 6, 6,
  1, 2, 3, 4, 5, 3, 2, 1 };*/

/* the error code returned by certain function calls */
extern int errno;

using namespace std;
#define PORT 3000
#define BUFF_SIZE 1024
using json = nlohmann::json;

extern int errno;

enum class RequestType
{
    Login,
    Move,
    Challenge,
    SomeOtherRequest,
    // Add more request types as needed
};

string reverse_convert(int a[8][8])
{
    string s = "";

    for (int i = 0; i <= 7; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            switch (a[i][j])
            {
            case 6:
                s.push_back('p');
                break;
            case -6:
                s.push_back('P');
                break;
            case 5:
                s.push_back('k');
                break;
            case -5:
                s.push_back('K');
                break;
            case 2:
                s.push_back('c');
                break;
            case -2:
                s.push_back('C');
                break;
            case 1:
                s.push_back('r');
                break;
            case -1:
                s.push_back('R');
                break;
            case 3:
                s.push_back('b');
                break;
            case -3:
                s.push_back('B');
                break;
            case 4:
                s.push_back('q');
                break;
            case -4:
                s.push_back('Q');
                break;
            case 0:
                s.push_back('-');
                break;
            default:
                break;
            }
        }
    }
    return s;
}

bool send_request(RequestType type, const json &request_data, int sd)
{
    // Create the request JSON
    cout << "Request Sent: " << request_data << endl;
    json request;
    request["type"] = static_cast<int>(type);
    request["data"] = request_data;

    // Serialize the request JSON
    string serializedRequest = request.dump();
    cout << "about to send this bro" << serializedRequest << endl;
    if (send(sd, serializedRequest.c_str(), serializedRequest.size(), 0) == -1)
    {
        cerr << "Error occurred while sending the request to the server." << endl;
        close(sd);
        return 0;
    }
    return 1;
}

// need to add client receiving results sent by server after the game's over
bool in_game = 0;
void convert(int a[9][9], string s)
{
    int i, j, k = 0;
    for (i = 1; i <= 8; i++)
    {
        for (j = 1; j <= 8; j++)
        {
            switch (s[k++])
            {
            case 'p':
                a[i][j] = 6;
                break;

            case 'P':
                a[i][j] = -6;
                break;

            case 'k':
                a[i][j] = 5;
                break;

            case 'K':
                a[i][j] = -5;
                break;

            case 'c':
                a[i][j] = 2;
                break;

            case 'C':
                a[i][j] = -2;
                break;

            case 'r':
                a[i][j] = 1;
                break;

            case 'R':
                a[i][j] = -1;
                break;

            case 'b':
                a[i][j] = 3;
                break;

            case 'B':
                a[i][j] = -3;
                break;

            case 'q':
                a[i][j] = 4;
                break;

            case 'Q':
                a[i][j] = -4;
                break;

            case '-':
                a[i][j] = 0;
                break;
            default:
                break;
            }
        }
    }
}

void print_board(int A[9][9])
{
    int i, j;
    string board[10][10];
    for (i = 1; i <= 8; i++)
    {
        for (j = 1; j <= 8; j++)
            board[i][j] = to_string(A[i][j]);
    }

    for (i = 1; i <= 8; i++)
        board[i][0] = '1' + i - 1; // row id
    for (i = 1; i <= 8; i++)
        board[9][i] = 'a' + i - 1; // column id
    board[9][0] = '*';

    cout << "==============================================================================" << endl;

    for (i = 1; i <= 8; i++)
    {
        for (j = 0; j <= 8; j++)
            if (j)
                cout << " | " << board[i][j] << " | "
                     << " ";
            else
                cout << board[i][j] << "     ";
        cout << endl;
    }
    cout << "==============================================================================" << endl;

    cout << "   *  ";
    for (j = 1; j <= 8; j++)
        cout << " | " << board[9][j] << " | "
             << " ";
    cout << endl;
}

int port;

int menu()
{
    int option;
    cout << endl;
    cout << "Menu:" << endl;
    cout << "1. Matchmaking" << endl;
    cout << "2. Challenge Another Player" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice: ";
    fflush(stdin);
    cin >> option;
    cout << "CHOSE" << option << "SDSD" << endl;
    return option;
}

void *send_game_data(void *arg)
{
    int sd = *((int *)arg);
    char msg[100];
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Set the timeout to 0 for non-blocking check
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    while (in_game)
    {
        // Perform the select
        // int activity = select(1, &readfds, nullptr, nullptr, &timeout);

        // Data available in stdin, read and write to the server
        int bytes = read(0, msg, 100);
        msg[bytes] = '\0';

        json move_request;
        move_request["move"] = msg;
        if (send_request(RequestType::Move, move_request, sd) == 0)
        {
            cout << "Failed to send move" << endl;
        }
    }

    int *result = new int(42);
    pthread_exit(nullptr);
}

void *receive_game_data(void *arg)
{
    int sd = *((int *)arg);

    int a[9][9];
    string s;

    int bytes;
    char msg[500];
    while (in_game)
    {
        bytes = read(sd, msg, 500);
        if (bytes < 0)
        {
            perror("[client]Error in read() from server.\n");
            break;
        }

        msg[bytes] = '\0';

        if (strlen(msg) > 50) // receive a board, need to change this to check message type
        {
            s.resize(100);
            convert(a, msg);
            cout << "Received new board" << endl;
            print_board(a);
            cout << endl;

            bzero(msg, 100);
            printf("[client]Enter your desired move: ");
            fflush(stdout);
        }

        else
            cout << "Message: " << msg << endl;
        if (strcmp(msg, "winner") == 0)
        {
            cout << "You won!!!" << endl;
            in_game = 0;
            break;
        }
        if (strcmp(msg, "loser") == 0)
        {
            cout << "You lost!!!" << endl;
            in_game = 0;
            break;
        }

        if (strcmp(msg, "Invalid move") == 0)
        {
            cout << "Invalid move!" << endl;
        }
    }

    int *result = new int(42);
    pthread_exit(nullptr);
}

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    string msg;

    if (argc != 3)
    {
        printf("[client] Syntax: %s <server_address> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[client] Error in socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client] Error in connect().\n");
        return errno;
    }

    // GUI lets user choose option 1 - 3, the choice will be in 'option'
    int option, a[9][9];
    string s;
    while (1)
    {
        fflush(stdin);
        option = menu();

        json move_request;
        int board[8][8] =
            {-1, -2, -3, -4, -5, -3, -2, -1,
             -6, -6, -6, -6, -6, -6, -6, -6,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             6, 6, 6, 6, 6, 6, 6, 6,
             1, 2, 3, 4, 5, 3, 2, 1};

        string s;

        switch (option)
        {
        case 1:
            move_request["sd"] = sd;
            if (send_request(RequestType::Move, move_request, sd) == 0)
            {
                cout << "Failed to send move" << endl;
                return 1;
            }

            in_game = 1;
            pthread_t receive_tid;
            pthread_create(&receive_tid, NULL, receive_game_data, &sd);

            pthread_t send_tid;
            pthread_create(&send_tid, NULL, send_game_data, &sd); // maybe handle if cannot create thread

            while (1)
            {
                if (!in_game)
                    break;
            }
            pthread_cancel(send_tid);

            // Wait for the thread to finish
            pthread_join(receive_tid, nullptr);
            pthread_join(send_tid, nullptr);
            break;
        case 2:

            s = reverse_convert(board);
            // need to choose a player first
            move_request["board"] = s;
            if (send_request(RequestType::Challenge, move_request, sd) == 0)
            {
                cout << "Failed to send move" << endl;
                return 1;
            }
            cout << "Challenging another player..." << endl;
            break;
        case 3:
            cout << "Exiting..." << endl;
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    close(sd);
}
