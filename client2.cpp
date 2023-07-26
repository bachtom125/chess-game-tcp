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
#include <vector>
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

struct Player
{
    // Working ....
    // Need to all User field to everywhere containing Player, and then proceed with handleChanllengeRequest()
    // struct User *user;
    int round;
    int fd;
    int free;
};

enum class RequestType
{
    Login,
    Logout,
    MatchMaking,
    Challenge,
    Move,
    GetOnlinePlayersList,
    SomeOtherRequest,
    // Add more request types as needed
};
enum class RespondType
{
    Login,
    Logout,
    MatchMaking,
    Move, // Working ... needs implementing
    Challenge,
    GameResult, // Working ... needs implementing
    OnlinePlayersList,
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
    if (send(sd, serializedRequest.c_str(), serializedRequest.size(), 0) == -1)
    {
        cerr << "Error occurred while sending the request to the server." << endl;
        close(sd);
        return 0;
    }
    return 1;
}

json receive_respond(int sd) // get respond and return corresponding json object
{
    array<char, 1024> buffer{};
    ssize_t bytesRead = recv(sd, buffer.data(), buffer.size(), 0);
    if (bytesRead <= 0)
    {
        cerr << "Error receiving data" << endl;
        close(sd);
        return NULL;
    }

    string respond_data(buffer.data(), bytesRead);
    cout << respond_data << endl;

    size_t bracePos = respond_data.find_first_of('{');
    if (bracePos != string::npos)
    {
        // Extract the substring from the brace position until the end of the string
        string jsonSubstring = respond_data.substr(bracePos);

        // Parse the extracted JSON substring
        json json_data = json::parse(jsonSubstring);

        // Determine the type of request and dispatch to the appropriate handler
        return json_data;
    }

    return NULL;
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
    cout << "0. Log in" << endl;
    cout << "1. Matchmaking" << endl;
    cout << "2. Challenge Another Player" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice: ";
    fflush(stdin);
    cin >> option;
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
    char msg[BUFF_SIZE];
    while (in_game)
    {
        array<char, 1024> buffer{};
        ssize_t bytesRead = recv(sd, buffer.data(), buffer.size(), 0);
        if (bytesRead <= 0)
        {
            cerr << "Error receiving data" << endl;
            in_game = 0;
            continue;
        }

        // Parse the received data into JSON
        string responseData(buffer.data(), bytesRead);
        // Find the position of the first opening brace '{'
        size_t bracePos = responseData.find_first_of('{');
        if (bracePos != string::npos)
        {
            // Extract the substring from the brace position until the end of the string
            string jsonSubstring = responseData.substr(bracePos);

            // Parse the extracted JSON substring
            json jsonData = json::parse(jsonSubstring);
            // cout << "server said: " << jsonData << endl;

            // Determine the type of request and dispatch to the appropriate handler
            int responseType = jsonData["type"];
            json received_data = jsonData["data"];

            string message = received_data["message"];

            if (responseType == static_cast<int>(RespondType::Move))
            {
                cout << "server said: " << message << endl;
                string board = "";
                if (received_data.contains("board"))
                    board = received_data["board"];
                // working ...
                if (strcmp(msg, "Invalid move") == 0)
                {
                    cout << "Invalid move!" << endl;
                }

                if (board != "") // receive a board, need to change this to check message type
                {
                    board.resize(100);
                    convert(a, board.c_str());
                    cout << "Received new board" << endl;
                    // print_board(a);
                    // cout << endl;

                    printf("[client]Enter your desired move: ");
                    fflush(stdout);
                }
            }
            else if (responseType == static_cast<int>(RespondType::GameResult))
            {
                if (message == "winner")
                {
                    cout << "You won!!!" << endl;
                    in_game = 0;
                }
                if (message == "loser")
                {
                    cout << "You lost!!!" << endl;
                    in_game = 0;
                }
                string moves_played = received_data["log"];
                cout << "Moves played" << endl;
                cout << moves_played << endl;
                break;
            }
            else if (responseType == static_cast<int>(RespondType::MatchMaking))
            {
                cout << message << received_data << endl;
            }
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
    // temp variables
    int option, a[9][9];
    string s;
    string username, password;
    int board_int[8][8] =
        {-1, -2, -3, -4, -5, -3, -2, -1,
         -6, -6, -6, -6, -6, -6, -6, -6,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         6, 6, 6, 6, 6, 6, 6, 6,
         1, 2, 3, 4, 5, 3, 2, 1};

    string board = reverse_convert(board_int);

    while (1)
    {
        fflush(stdin);
        option = menu();

        json move_request, get_online_players_request, received_data, challenge_request, login_request;

        string chosen_player;
        switch (option)
        {
        case 0:
            cout << "Enter username\npassword" << endl;
            cin >> username;
            cin >> password;
            login_request["username"] = username;
            login_request["password"] = password;
            if (send_request(RequestType::Login, login_request, sd) == 0)
            {
                cout << "Failed to send log in request" << endl;
                return 1;
            }

            received_data = receive_respond(sd);

            if (received_data["type"] == RespondType::Login)
            {
                cout << received_data["message"] << endl;
            }
            else
            {
                cout << "Wrong respond types" << endl;
            }
            break;
        case 1:
            move_request["sd"] = sd;
            if (send_request(RequestType::MatchMaking, move_request, sd) == 0)
            {
                cout << "Failed to send matchmaking request" << endl;
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
            // first get the online players list
            if (send_request(RequestType::GetOnlinePlayersList, get_online_players_request, sd) == 0)
            {
                cout << "Failed to send request" << endl;
                return 1;
            }
            received_data = receive_respond(sd); // get player list
            if (!received_data.empty())
            {
                if (received_data["type"] == RespondType::OnlinePlayersList)
                {
                    json online_players = received_data["data"];
                    cout << received_data["data"] << endl;
                    // cout << "Online Players: ";
                    // for (json player : online_players)
                    // {
                    //     cout << player["username"] << ':' << player["elo"] << endl;
                    // }
                    // cout << endl;
                }
                else
                {
                    cout << "Wrong respond types" << endl;
                }
            }

            // now choose 1 player
            chosen_player = received_data["data"][0]["username"];
            // now send match request and picked user
            challenge_request["challenger"] = username;
            challenge_request["opponent"] = chosen_player;
            challenge_request["board"] = board;

            if (send_request(RequestType::Challenge, challenge_request, sd) == 0)
            {
                cout << "Failed to send request" << endl;
                return 1;
            }

            break;

        case 3:
            if (send_request(RequestType::Logout, nullptr, sd) == 0)
            {
                cout << "Failed to send request" << endl;
                return 0;
            }
            return 0;
            // Working ... maybe needs log out success respond from server
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    close(sd);
}
