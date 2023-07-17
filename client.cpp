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

/* the error code returned by certain function calls */
extern int errno;

using namespace std;

/* portul de conectare la server*/
// need to add client receiving results sent by server after the game's over

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
    // for (i = 1; i <= 8; i++)
    // {
    //     for (j = 1; j <= 8; j++)
    //     {
    //         cout << a[i][j] << ' ';
    //     }
    //     cout << endl;
    // }
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
    cin >> option;

    return option;
}

void *communication_thread(void *arg) // for now, for handling surrenders request only, can be updated for communication overall
{
    while (true)
    {

        if (surrenderRequested)
        {
            lock_guard<mutex> lock(mtx);
            // Handle surrender request in the main game logic
            cout << "Surrender requested by the other player!" << endl;
        }

        // Send messages to the other player
        // ...

        // Sleep for a while to avoid excessive CPU usage
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    int *result = new int(42);
    pthread_exit(result);
}

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server;
    char msg[100];

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
        int signal;
        option = menu();
        if (send(sd, &option, sizeof(option), 0) == -1)
        {
            cerr << "Error occurred while sending the option to the server." << endl;
            close(sd);
            return 1;
        }

        switch (option)
        {
        case 1:
            pthread_t threadId;

            int result = pthread_create(&threadId, nullptr, communication_thread, nullptr);
            if (result != 0)
            {
                cerr << "Failed to create thread." << endl;
                return 1;
            }
            s.resize(100);
            if (read(sd, &s[0], s.size()) < 0)
            {
                perror("[client]Error at read() from the server.\n");
                return errno;
            }
            convert(a, s);
            print_board(a);
            cout << endl;

            // Working ...
            // Need to make client get the existing board right after making a move, so when win can receive the win message
            while (1)
            {
                bzero(msg, 100);
                printf("[client]Enter your desired move: ");
                fflush(stdout);
                read(0, msg, 100);

                if (write(sd, msg, 100) <= 0)
                {
                    perror("[client]Error in write() to server.\n");
                    return errno;
                }

                // if (strcmp(msg, "surrender\n") == 0)
                //     break;

                int bytes;
                bytes = read(sd, msg, 100);
                if (bytes < 0)
                {
                    perror("[client]Error in read() from server.\n");
                    return errno;
                }

                msg[bytes] = '\0';
                cout << "MESSAGE: " << msg << endl;
                if (strcmp(msg, "winner") == 0)
                {
                    cout << "You won!!!" << endl;
                    break;
                }
                if (strcmp(msg, "loser") == 0)
                {
                    cout << "You lost!!!" << endl;
                    break;
                }
                if (!strstr(msg, "Invalid"))
                {
                    s.resize(100);
                    if (read(sd, &s[0], s.size()) < 0)
                    {
                        perror("[client]Error in read() from server.\n");
                        return errno;
                    }
                    convert(a, s);
                    print_board(a);
                    cout << endl;
                }
            }
            break;
        case 2:
            cout << "Challenging another player..." << endl;
            // Add  challenge logic here
            break;
        case 3:
            cout << "Exiting..." << endl;
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    // The commented code below is for a chess game
    // int a[9][9];

    // int x = 0;
    // string s;
    // s.resize(100);
    // if (read(sd, &s[0], s.size()) < 0)
    // {
    //     perror("[client]Error at read() from the server.\n");
    //     return errno;
    // }
    // convert(a, s);
    // print_board(a);
    // cout << endl;

    // while (1)
    // {
    //     bzero(msg, 100);
    //     printf("[client]`your desired move: ");
    //     fflush(stdout);
    //     read(0, msg, 100);

    //     if (write(sd, msg, 100) <= 0)
    //     {
    //         perror("[client]Error in write() to server.\n");
    //         return errno;
    //     }

    //     if (strcmp(msg, "surrender\n") == 0)
    //         break;

    //     if (read(sd, msg, 100) < 0)
    //     {
    //         perror("[client]Error in read() from server.\n");
    //         return errno;
    //     }
    //     cout << msg << endl;

    //     if (!strstr(msg, "Invalid"))
    //     {
    //         s.resize(100);
    //         if (read(sd, &s[0], s.size()) < 0)
    //         {
    //             perror("[client]Error in read() from server.\n");
    //             return errno;
    //         }
    //         convert(a, s);
    //         print_board(a);
    //         cout << endl;
    //     }
    // }
    close(sd);
}
