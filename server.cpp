#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <pthread.h>
#include <vector>
#include <queue>
#include <mutex>
#include <nlohmann/json.hpp>
#include <fstream> // Add this line
#include <condition_variable>

#define PORT 3000
#define BUFF_SIZE 1024

using namespace std;
using json = nlohmann::json;

extern int errno;

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
    Move, // Working ... needs implementing
    Challenge,
    GameResult, // Working ... needs implementing
    OnlinePlayersList,
};

struct User
{
    string username;
    string password;
    int elo;
};

struct Player
{
    // Working ....
    // Need to all User field to everywhere containing Player, and then proceed with handleChanllengeRequest()
    string username = "";
    int elo = -1;
    int round;
    int fd;
    int free;
    int logged_in = 0;
};

fd_set readfds;
fd_set actfds;
int v[250] = {0};

vector<Player *> online_players;
queue<Player *> match_making_players;

mutex queue_mutex, vector_mutex;
condition_variable queue_condition;

struct PlayGameThreadData
{
    Player *player_a;
    Player *player_b;
    string initial_board = "";

    // char initial_board[9][9];
};

void *client_operation(void *);
void *play_game(void *);
void *match_making_system(void *);
std::vector<User> readAccountsFile();
bool disconnect_player(int);

constexpr const char *ACCOUNTS_FILE = "accounts.txt";

Player *find_online_player(int client_fd)
{
    for (Player *player : online_players)
    {
        if (player->fd == client_fd)
        {
            cout << "He is " << &player << endl;
            return player;
        }
    }
    return NULL;
}

vector<User> readAccountsFile()
{
    vector<User> users;
    ifstream accountsFile("accounts.txt");
    if (!accountsFile)
    {
        cerr << "Failed to open accounts file" << endl;
        return users;
    }

    string line;
    while (getline(accountsFile, line))
    {
        istringstream iss(line);
        User user;
        if (iss >> user.username >> user.password >> user.elo)
        {
            users.push_back(user);
            cout << '.' << user.username << '.' << user.password << '.' << user.elo << '.' << endl;
        }
    }

    return users;
}

User findUserByUsername(const string &username)
{
    vector<User> users = readAccountsFile();
    for (const User &user : users)
    {
        if (user.username == username)
        {
            return user;
        }
    }

    // Return a default-constructed User object if the user is not found
    return User();
}

bool isUserValid(const string &username, const string &password)
{
    ifstream accounts(ACCOUNTS_FILE);
    if (!accounts)
    {
        cerr << "Failed to open accounts file" << endl;
        return false;
    }

    string line;
    while (getline(accounts, line))
    {
        string storedUsername, storedPassword;
        istringstream iss(line);
        if (iss >> storedUsername >> storedPassword)
        {
            if (storedUsername == username && storedPassword == password)
            {
                return true;
            }
        }
    }

    return false;
}

int find_player_fd(const string username)
{
    for (Player *player : online_players)
    {
        if (player->username == username)
            return player->fd;
    }
    return -1;
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
        cerr << "Error occurred while sending the request to the client." << endl;
        close(sd);
        return 0;
    }
    return 1;
}
bool send_respond(RespondType type, const json &respond_data, int sd)
{
    // Create the respond JSON
    cout << "Respond Sent: " << respond_data << endl;
    json respond;
    respond["type"] = static_cast<int>(type);
    respond["data"] = respond_data;

    // Serialize the respond JSON
    string serializedRespond = respond.dump();
    cout << "about to send this bro" << serializedRespond << endl;
    if (send(sd, serializedRespond.c_str(), serializedRespond.size(), 0) == -1)
    {
        cerr << "Error occurred while sending the respond to the server." << endl;
        close(sd);
        return 0;
    }
    return 1;
}

void handleMatchMakingRequest(const json &requestData, int client_fd)
{
    // need to check if logged in
    Player *this_player = find_online_player(client_fd);
    cout << "and he is " << client_fd << ':' << this_player << endl;
    if (!this_player)
    {
        printf("Player not online");
        // hanlde this case
        return;
    }

    // if reaches here, then player is both logged in and online
    printf("This player %d\n", this_player->fd);
    match_making_players.push(this_player);
    this_player->free = 0;
    while (1)
    {
        if (this_player->free == 1)
            break;
    }
}

void handleChallengeRequest(const json &requestData, int client_fd)
{
    // get the board
    string board = requestData["board"];
    string opponent_username = requestData["opponent"];
    string challenger_username = requestData["challenger"];

    // send notification to the other player
    cout << challenger_username << " challenged " << opponent_username << " with this board " << board << endl;

    int opponent_fd = find_player_fd(opponent_username);
    json respond_type;
    respond_type["challenger"] = challenger_username;
    respond_type["board"] = board;

    if (send_request(RequestType::Challenge, respond_type, opponent_fd) == 0)
    {
        cout << "Failed to send out challenge to " << opponent_fd << endl;
    }
    // make a game with the recieved board
}

bool handleLogoutRequest(const json &requestData, int client_fd)
{
    return (disconnect_player(client_fd));
}

void handleGetOnlinePlayersListRequest(const json &requestData, int client_fd)
{
    json respond_type;

    for (const auto *player : online_players)
    {
        if (player->fd == client_fd)
            continue;
        json playerJson;
        playerJson["username"] = player->username;
        playerJson["elo"] = player->elo;
        playerJson["free"] = player->free;
        respond_type.push_back(playerJson);
    }
    cout << "GOT HERE THO" << endl;

    if (send_respond(RespondType::OnlinePlayersList, respond_type, client_fd) == 0)
    {
        cout << "Failed to send online players list to " << client_fd << endl;
    }
}

void handleLoginRequest(const json &requestData, int client_fd)
{
    string username = requestData["username"];
    string password = requestData["password"];

    cout << username << endl;
    cout << password << endl;

    // Perform login validation/authentication logic
    User user = findUserByUsername(username);
    bool isCorrectInfo = (user.username == username && user.password == password);
    int isOnline = find_player_fd(username);
    bool isValid = isCorrectInfo && (isOnline == -1);

    // Craft the response JSON
    json response;
    response["type"] = static_cast<int>(RequestType::Login);
    response["success"] = isValid;

    if (!isCorrectInfo)
        response["message"] = "Invalid username or password";
    else if (isOnline != -1)
        response["message"] = "User already online";
    else
        response["message"] = "Login successful";

    if (isValid)
    {
        Player *this_player = find_online_player(client_fd);
        this_player->username = username;
        this_player->logged_in = 1;
        this_player->elo = user.elo;

        cout << "Player " << username << " logged in with fd " << client_fd << endl;
    }
    else
        cout << "Client " << client_fd << " failed to log in" << endl;

    // Serialize the response JSON
    string responseStr = response.dump();

    // Send the response back to the client
    if (send(client_fd, responseStr.c_str(), responseStr.size(), 0) == -1)
    {
        cerr << "Failed to send response to client" << endl;
    }
}

void remove_player_from_matchmaking()
{
    // Working
}

bool disconnect_player(int fd)
{
    auto it = online_players.begin();
    while (it != online_players.end())
    {
        if ((*it)->fd == fd)
            break;

        it++;
    }

    if (it != online_players.end())
    {
        online_players.erase(it);
        cout << "Player " << fd << " disconnected!" << endl;
    }
    else
    {
        cout << "Player " << fd << " not found in online player vector list" << endl;
        return 0;
    }
    FD_CLR(fd, &actfds);
    FD_CLR(fd, &readfds);
    close(fd);
    v[fd] = 0;
    return 1;
}

json convert_to_json(string buffer)
{
    string request_data = buffer;
    cout << request_data << endl;
    // Find the position of the first opening brace '{'
    size_t bracePos = request_data.find_first_of('{');

    // Extract the substring from the brace position until the end of the string
    string jsonSubstring = request_data.substr(bracePos);

    // Parse the extracted JSON substring
    json json_data = json::parse(jsonSubstring);

    // Determine the type of request and dispatch to the appropriate handler
    int requestType = json_data["type"];

    // if (requestType == static_cast<int>(RequestType::Login))
    // {
    //     cout << "Received login request" << endl;
    //     handleLoginRequest(json_data["data"], client_fd);
    // }
    // else if (requestType == static_cast<int>(RequestType::SomeOtherRequest))
    // {
    //     cout << "Received some other request" << endl;
    //     handleSomeOtherRequest(json_data["data"]);
    // }
    // else
    // {
    //     // Handle unknown or unsupported request types
    //     cerr << "Unknown request type: " << requestType << endl;
    // }
    return json_data;
}

void create_table(char A[9][9])
{
    int i, j;
    for (i = 1; i <= 8; i++)
        for (j = 1; j <= 8; j++)
            A[i][j] = '-';

    for (i = 1; i <= 8; i++)
        A[i][0] = '1' + i - 1; // row number
    for (i = 1; i <= 8; i++)
        A[9][i] = 'a' + i - 1; // column id
    A[9][0] = '*';

    for (j = 1; j <= 8; j++)
        A[2][j] = 'P';
    for (j = 1; j <= 8; j++)
        A[7][j] = 'p';

    // rooks
    A[1][1] = A[1][8] = 'R';
    A[8][1] = A[8][8] = 'r';

    // knights
    A[1][2] = A[1][7] = 'C';
    A[8][2] = A[8][7] = 'c';

    // bishops
    A[1][3] = A[1][6] = 'B';
    A[8][3] = A[8][6] = 'b';

    // queen
    A[1][4] = 'Q';
    A[8][4] = 'q';

    // king
    A[1][5] = 'K';
    A[8][5] = 'k';

    // for (i = 0; i <= 8; i++)
    // {
    //     for (j = 1; j <= 8; j++)
    //         cout << A[i][j] << ' ';
    //     cout << endl;
    // }
}

string convert(char A[9][9])
{
    int i, j;
    string s = "";
    for (i = 1; i <= 8; i++)
    {
        for (j = 1; j <= 8; j++)
        {
            s += A[i][j];
        }
    }
    return s;
}

int move_straight_line(char A[9][9], int sr, int sc, int dr, int dc)
{ // move along either row or column (1 only)
    int i, j;
    if (dr > sr && dc == sc)
    {
        i = sr + 1, j = sc;
        while (A[i][j] == '-' && i < dr)
            i++;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dr < sr && dc == sc)
    {
        i = sr - 1, j = sc;
        while (A[i][j] == '-' && i > dr)
            i--;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dc > sc && dr == sr)
    {
        i = sr, j = sc + 1;
        while (A[i][j] == '-' && j < dc)
            j++;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dc < sc && dr == sr)
    {
        i = sr, j = sc - 1;
        while (A[i][j] == '-' && j > dc)
            j--;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

int move_diagonal_line(char A[9][9], int sr, int sc, int dr, int dc)
{ // moving diagonally (long both rows and columns)
    int i, j;
    if (dr > sr && dc > sc)
    {
        i = sr + 1, j = sc + 1;
        while (A[i][j] == '-' && i < dr && j < dc)
            i++, j++;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dr > sr && dc < sc)
    {
        i = sr + 1, j = sc - 1;
        while (A[i][j] == '-' && i < dr && j > dc)
            i++, j--;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dr < sr && dc > sc)
    {
        i = sr - 1, j = sc + 1;
        while (A[i][j] == '-' && i > dr && j < dc)
            i--, j++;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else if (dr < sr && dc < sc)
    {
        i = sr - 1, j = sc - 1;
        while (A[i][j] == '-' && i > dr && j > dc)
            i--, j--;
        if (i == dr && j == dc)
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

int is_white_piece(char letter)
{
    if (letter >= 'A' && letter <= 'Z')
        return 1;
    return 0;
}

int is_black_piece(char letter)
{
    if (letter >= 'a' && letter <= 'z')
        return 1;
    return 0;
}

int pawn(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // Outside of matrix
        else if (dr <= sr)
            return 0; // The pawn is not allowed to move backwards.
        else if (dr - sr == 2 && sr == 2 && dc == sc && A[dr][dc] == '-')
        {
            if (is_black_piece(A[dr][dc]))
                return 0;
            else
                return 1;
        }
        else if (dr - sr == 1 && abs(dc - sc) == 1 && (is_black_piece(A[dr][dc])))
            return 1;
        else if (dr - sr == 1 && dc == sc && !is_black_piece(A[dr][dc]))
            return 1;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if (dr >= sr)
            return 0; // nu are voie pawnul sa fie dat inapoi
        else if (sr - dr == 2 && sr == 7 && dc == sc && A[dr][dc] == '-')
        {
            if (is_white_piece(A[dr][dc]))
                return 0;
            else
                return 1;
        }
        else if (sr - dr == 1 && abs(dc - sc) == 1 && (is_white_piece(A[dr][dc])))
            return 1;
        else if (sr - dr == 1 && dc == sc && !is_white_piece(A[dr][dc]))
            return 1;
    }
}

int king(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if ((abs(dr - sr) == 1 || abs(dr - sr) == 0) && (abs(dc - sc) == 1 || abs(dc - sc) == 0))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if ((abs(dr - sr) == 1 || abs(dr - sr) == 0) && (abs(dc - sc) == 1 || abs(dc - sc) == 0))
            return 1;
        else
            return 0;
    }
}

int knight(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        if ((abs(dr - sr) == 1 || abs(dr - sr) == 2) && (abs(dc - sc) == 1 || abs(dc - sc) == 2))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        if ((abs(dr - sr) == 1 || abs(dr - sr) == 2) && (abs(dc - sc) == 1 || abs(dc - sc) == 2))
            return 1;
        else
            return 0;
    }
}

int rook(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if (move_straight_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if (move_straight_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
}

int bishop(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0;
        else if (move_diagonal_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0;
        else if (move_diagonal_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
}

int queen(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    int i, j;
    if (is_white_piece(type))
    {
        if (is_white_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if (move_diagonal_line(A, sr, sc, dr, dc) || move_straight_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[dr][dc]))
            return 0;
        else if (dr > 8 || dr < 1 || dc > 8 || dc < 1)
            return 0; // outside of the board
        else if (move_diagonal_line(A, sr, sc, dr, dc) || move_straight_line(A, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    }
}

int is_valid_move(char A[9][9], char type, int sr, int sc, int dr, int dc)
{
    switch (type)
    {
    case 'p':
        if (pawn(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'P':
        if (pawn(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'k':
        if (king(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'K':
        if (king(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'c':
        if (knight(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'C':
        if (knight(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'r':
        if (rook(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'R':
        if (rook(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'b':
        if (bishop(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'B':
        if (bishop(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'q':
        if (queen(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    case 'Q':
        if (queen(A, type, sr, sc, dr, dc))
            return 1;
        else
            return 0;
    default:
        return 0;
    }
}

void find_my_king(char A[9][9], char type, int &dr, int &dc)
{
    int i, j;
    for (i = 1; i <= 8; i++)
        for (j = 1; j <= 8; j++)
            if (A[i][j] == type)
            {
                dr = i;
                dc = j;
                break;
            }
}

int is_checked(char A[9][9], char type, int dr, int dc)
{
    int i, j;
    if (dr < 1 || dr > 8 || dc < 1 || dc > 8)
        return 0;
    else if (is_white_piece(type))
    {
        for (i = 1; i <= 8; i++)
            for (j = 1; j <= 8; j++)
                if (is_black_piece(A[i][j]))
                    if (is_valid_move(A, A[i][j], i, j, dr, dc))
                        return 1;

        return 0;
    }
    else if (is_black_piece(type))
    {
        for (i = 1; i <= 8; i++)
            for (j = 1; j <= 8; j++)
                if (is_white_piece(A[i][j]))
                    if (is_valid_move(A, A[i][j], i, j, dr, dc))
                        return 1;

        return 0;
    }
}

int check(char A[9][9], char type)
{
    int dr, dc;
    find_my_king(A, type, dr, dc);
    if (is_checked(A, type, dr, dc))
        return 1;
    return 0;
}

void copy(char B[9][9], char A[9][9])
{
    int i, j;
    for (i = 1; i <= 8; i++)
        for (j = 1; j <= 8; j++)
            B[i][j] = A[i][j];
}

int check_mate(char A[9][9], char type)
{
    const int dir[8] = {0};
    char B[9][9], C[9][9];
    copy(B, A);
    copy(C, A);
    int i, j, dr, dc;
    find_my_king(A, type, dr, dc);
    if (is_black_piece(type))
    {
        if (C[dr - 1][dc] == '-')
            A[dr - 1][dc] = 'K';
        if (C[dr + 1][dc] == '-')
            A[dr + 1][dc] = 'K';
        if (C[dr - 1][dc - 1] == '-')
            A[dr - 1][dc - 1] = 'K';
        if (C[dr - 1][dc + 1] == '-')
            A[dr - 1][dc + 1] = 'K';
        if (C[dr + 1][dc + 1] == '-')
            A[dr + 1][dc + 1] = 'K';
        if (C[dr + 1][dc - 1] == '-')
            A[dr + 1][dc - 1] = 'K';
        if (C[dr][dc - 1] == '-')
            A[dr][dc - 1] = 'K';
        if (C[dr][dc + 1] == '-')
            A[dr][dc + 1] = 'K';
    }
    else if (is_white_piece(type))
    {
        if (C[dr - 1][dc] == '-')
            A[dr - 1][dc] = 'k';
        if (C[dr + 1][dc] == '-')
            A[dr + 1][dc] = 'k';
        if (C[dr - 1][dc - 1] == '-')
            A[dr - 1][dc - 1] = 'k';
        if (C[dr - 1][dc + 1] == '-')
            A[dr - 1][dc + 1] = 'k';
        if (C[dr + 1][dc + 1] == '-')
            A[dr + 1][dc + 1] = 'k';
        if (C[dr + 1][dc - 1] == '-')
            A[dr + 1][dc - 1] = 'k';
        if (C[dr][dc - 1] == '-')
            A[dr][dc - 1] = 'k';
        if (C[dr][dc + 1] == '-')
            A[dr][dc + 1] = 'k';
    }

    if (check(A, type))
    {
        int nr = 1;
        if (is_checked(A, type, dr - 1, dc) && B[dr - 1][dc] == '-')
            B[dr - 1][dc] = 'x', nr++;
        if (is_checked(A, type, dr + 1, dc) && B[dr + 1][dc] == '-')
            B[dr + 1][dc] = 'x', nr++;
        if (is_checked(A, type, dr - 1, dc - 1) && B[dr - 1][dc - 1] == '-')
            B[dr - 1][dc - 1] = 'x', nr++;
        if (is_checked(A, type, dr - 1, dc + 1) && B[dr - 1][dc + 1] == '-')
            B[dr - 1][dc + 1] = 'x', nr++;
        if (is_checked(A, type, dr + 1, dc + 1) && B[dr + 1][dc + 1] == '-')
            B[dr + 1][dc + 1] = 'x', nr++;
        if (is_checked(A, type, dr + 1, dc - 1) && B[dr + 1][dc - 1] == '-')
            B[dr + 1][dc - 1] = 'x', nr++;
        if (is_checked(A, type, dr, dc - 1) && B[dr][dc - 1] == '-')
            B[dr][dc - 1] = 'x', nr++;
        if (is_checked(A, type, dr, dc + 1) && B[dr][dc + 1] == '-')
            B[dr][dc + 1] = 'x', nr++;
        copy(A, C);
        if (B[dr][dc + 1] == '-' || B[dr][dc - 1] == '-' || B[dr + 1][dc - 1] == '-' || B[dr + 1][dc + 1] == '-' || B[dr - 1][dc + 1] == '-' || B[dr - 1][dc - 1] == '-' || B[dr + 1][dc] == '-' ||
            B[dr - 1][dc] == '-')
            return 0;
        if (nr > 1)
            return 1;
    }
    return 0;
}

void send_result(int loser_fd, int winner_fd, string moves_played)
{
    char msg[BUFF_SIZE];
    strcpy(msg, "winner");
    cout << "ALL MOVES " << moves_played << endl;
    // send result
    if (write(winner_fd, msg, BUFF_SIZE) < 0)
    {
        cerr << "Error occurred while sending message to the Winner." << endl;
    }

    // send moves played
    if (write(winner_fd, moves_played.c_str(), moves_played.size()) < 0)
    {
        cerr << "Error occurred while sending moves to the Winner." << endl;
    }

    // send match result
    strcpy(msg, "loser");
    if (write(loser_fd, msg, BUFF_SIZE) < 0)
    {
        cerr << "Error occurred while sending message to the Loser" << endl;
    }

    // send moves played
    if (write(loser_fd, moves_played.c_str(), moves_played.size()) < 0)
    {
        cerr << "Error occurred while sending moves to the Loser." << endl;
    }

    cout << "The winner is " << winner_fd << endl;
}

int get_move(char A[9][9], int vizA[4], int vizB[4], int fd, int opponent_fd, int &verify, char move, string &moves_played)
{
    string s;
    char buffer[BUFF_SIZE];
    int bytes;
    char save;
    char msg[BUFF_SIZE];
    char msgrasp[BUFF_SIZE] = " ";

    bytes = read(fd, buffer, sizeof(buffer));
    if (bytes < 0)
    {
        perror("Error in read() from the client.\n");
        return 0;
    }
    // // testing begin
    // cout << "buffer: " << buffer << endl;
    // return 1;
    // // testing en

    json json_data = convert_to_json(buffer);
    const json &request_data = json_data["data"];
    string msg_received = request_data["move"];

    // Copy string to char array
    strncpy(msg, msg_received.c_str(), sizeof(msg) - 1);
    msg[sizeof(msg) - 1] = '\0'; // Ensure null-termination

    // const char *msg = msg_received.c_str();
    cout << fd << " said " << msg << endl;

    int sr, sc, dr, dc;
    char type, c1, c2, transform;
    strcpy(msgrasp, msg);
    int i, nr = 0;
    // get the coordinates
    for (i = 0; msgrasp[i]; i++)
    {
        if (msgrasp[i] != ' ')
        {
            switch (nr)
            {
            case 0:
            {
                sc = (int)msgrasp[i] - 96;
                c1 = msgrasp[i];
            }
            case 1:
                sr = (int)msgrasp[i] - 48;
            case 2:
            {
                dc = (int)msgrasp[i] - 96;
                c2 = msgrasp[i];
            }
            case 3:
                dr = (int)msgrasp[i] - 48;

            case 4:
                transform = msgrasp[i];
            }
            nr++;
        }
    }

    save = A[dr][dc];
    type = A[sr][sc];
    // visits parts to check if the cast can be executed or not. If the pieces have been moved, castling cannot happen.
    if (type == 'K' && transform != 'F')
        vizA[2] = 1;
    else if (type == 'R' && sc == 1 && transform != 'F')
        vizA[1] = 1;
    else if (type == 'R' && sc == 8 && transform != 'F')
        vizA[3] = 1;
    else if (type == 'k' && transform != 'F')
        vizB[2] = 1;
    else if (type == 'r' && sc == 1 && transform != 'F')
        vizB[1] = 1;
    else if (type == 'r' && sc == 8 && transform != 'F')
        vizB[3] = 1;

    // cout << type << " " << sr << " " << sc << " " << dr << " " << dc << " " << transform << endl;

    if (move == 'a' && is_black_piece(type))
    {
        strcpy(msg, "Invalid move");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if (move == 'b' && is_white_piece(type))
    {
        strcpy(msg, "Invalid move");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if (A[sr][sc] == '-')
    {
        strcpy(msg, "Invalid move");
        write(fd, msg, strlen(msg));
        return -2;
    }

    else
    {
        string move_played(msg);
        moves_played += to_string(fd) + ":" + move_played;
        // cout << "current moves played: " << moves_played << endl;
        if (strcmp(msg, "surrender\n") == 0)
        {
            send_result(fd, opponent_fd, moves_played);
            return -1;
        }
        else if (move == 'a' && check_mate(A, 'K'))
        {

            send_result(fd, opponent_fd, moves_played);
            return -1;
        }
        else if (move == 'b' && check_mate(A, 'k'))
        {

            send_result(opponent_fd, fd, moves_played);
            return -1;
        }

        else if (transform == 'F')
        { // ROCADA
            int ok = 0;
            if (sr == 1 && dr == 1)
            {
                char t1, t2;
                t1 = A[sr][sc], t2 = A[dr][dc];
                if (sc == 1 && dc == 5)
                {
                    if (is_white_piece(t1) && is_white_piece(t2))
                    {
                        if (!is_checked(A, t1, dr, dc - 1) && !is_checked(A, t2, dr, dc - 2))
                        {
                            if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
                                if (!vizA[1] && !vizA[2])
                                {
                                    A[sr][sc] = '-';
                                    A[dr][dc] = '-';
                                    A[sr][sc + 2] = t2;
                                    A[sr][dc - 1] = t1;
                                    vizA[1] = 1;
                                    vizA[2] = 1;
                                    ok = 1;
                                }
                        }
                    }
                }
                else if (sc == 5 && dc == 8)
                {
                    if (is_white_piece(t1) && is_white_piece(t2))
                    {
                        if (!is_checked(A, t1, dr, sc + 1) && !is_checked(A, t2, dr, sc + 2))
                        {
                            if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
                                if (!vizA[2] && !vizA[3])
                                {
                                    A[sr][sc] = '-';
                                    A[dr][dc] = '-';
                                    A[sr][sc + 1] = t2;
                                    A[sr][dc - 1] = t1;
                                    vizA[2] = 1;
                                    vizA[3] = 1;
                                    ok = 1;
                                }
                        }
                    }
                }
            }
            else if (sr == 8 && dr == 8)
            {
                char t1, t2;
                t1 = A[sr][sc], t2 = A[dr][dc];
                if (sc == 1 && dc == 5)
                {
                    if (is_black_piece(t1) && is_black_piece(t2))
                    {
                        if (!is_checked(A, t1, dr, dc - 1) && !is_checked(A, t2, dr, dc - 2))
                        {
                            if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
                                if (!vizB[1] && !vizB[2])
                                {
                                    A[sr][sc] = '-';
                                    A[dr][dc] = '-';
                                    A[sr][sc + 2] = t2;
                                    A[sr][dc - 1] = t1;
                                    vizB[1] = 1;
                                    vizB[2] = 1;
                                    ok = 1;
                                }
                        }
                    }
                }
                else if (sc == 5 && dc == 8)
                {
                    if (is_black_piece(t1) && is_black_piece(t2))
                    {
                        if (!is_checked(A, t1, dr, sc + 1) && !is_checked(A, t2, dr, sc + 2))
                        {
                            if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
                                if (!vizB[2] && !vizB[3])
                                {
                                    A[sr][sc] = '-';
                                    A[dr][dc] = '-';
                                    A[sr][sc + 1] = t2;
                                    A[sr][dc - 1] = t1;
                                    vizB[2] = 1;
                                    vizB[3] = 1;
                                    ok = 1;
                                }
                        }
                    }
                }
            }

            if (!ok)
            {
                strcpy(msg, "Castling is not possible!");
                write(fd, msg, strlen(msg));
                return -2;
            }
            else if (ok)
            {
                // PrintTable(A);
                s = convert(A);
                write(fd, s.c_str(), s.size());
                cout << "Castling performed!" << endl;
                verify = 1;
                return s.size();
            }
        }
        else if (!is_valid_move(A, type, sr, sc, dr, dc))
        {
            strcpy(msg, "Invalid move");
            write(fd, msg, strlen(msg));
            return -2;
        }
        else if (is_valid_move(A, type, sr, sc, dr, dc))
        {
            A[sr][sc] = '-';
            A[dr][dc] = type;
            if (move == 'a' && check(A, 'K'))
            {
                strcpy(msg, "Invalid move! check!");
                write(fd, msg, strlen(msg));
                A[sr][sc] = type;
                A[dr][dc] = save;
                return -2;
            }
            else if (move == 'b' && check(A, 'k'))
            {
                strcpy(msg, "Invalid move! check!");
                write(fd, msg, strlen(msg));
                A[sr][sc] = type;
                A[dr][dc] = save;
                return -2;
            }

            // Transformation invalid for pawn when reaching the enemy's last row.
            if (type == 'p' && dr == 1 && is_black_piece(transform) && (transform == 'b' || transform == 'c' || transform == 'q' || transform == 'r'))
                A[sr][sc] = '-', A[dr][dc] = transform;
            else if (type == 'P' && dr == 8 && is_white_piece(transform) && (transform == 'B' || transform == 'C' || transform == 'Q' || transform == 'R'))
                A[sr][sc] = '-', A[dr][dc] = transform;
            else if ((transform != 'b' || transform != 'c' || transform != 'q' || transform != 'r' || transform != 'B' || transform != 'C' || transform != 'Q' || transform != 'R') && (type == 'p' || type == 'P'))
            {
                if (dr == 1 || dr == 8)
                {
                    strcpy(msg, "Invalid transformation!");
                    write(fd, msg, strlen(msg));
                    return -2;
                }
            }

            if (move == 'a' && check(A, 'k'))
            {
                strcpy(msg, "Move executed! The enemy's king is in check!");
                s = convert(A);
                bytes = s.size();
            }
            else if (move == 'b' && check(A, 'K'))
            {
                strcpy(msg, "Move executed! The enemy's king is in check!");
                s = convert(A);
                bytes = s.size();
            }
            else
            {
                strcpy(msg, "Move executed!");
                s = convert(A);
                bytes = s.size();
            }

            // PrintTable(A);

            // cout << type << " It was moved from position " << c1 << " " << sr << " to position " << c2 << " " << dr << endl;
            // cout << "Waiting for the other player's move!" << endl;

            if (bytes && write(opponent_fd, s.c_str(), bytes) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, strlen(msg)) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }

            verify = 1;
            return bytes;
        }
    }
}

char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    strcpy(str, inet_ntoa(address.sin_addr));
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

void print_server_state()
{
    queue<Player *> match_making_temp = match_making_players;

    cout << "Match Making Players ";
    while (match_making_temp.empty() != 1)
    {
        Player *player = match_making_temp.front();
        cout << (*player).fd << '-' << (*player).free << ' ';
        match_making_temp.pop();
    }
    cout << endl;

    vector<Player *> temp = online_players;
    cout << "Players online ";
    for (Player *i : temp)
        cout << i->fd << ' ';
    cout << endl;
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;

    struct timeval tv;
    int sd, client;
    int optval = 1;
    int fd;
    int nfds;
    pid_t childpid;
    unsigned int len;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Error in socket() function.\n");
        return errno;
    }

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Error in bind.\n");
        return errno;
    }

    if (listen(sd, 10) == -1)
    {
        perror("[server] Error in listen().\n");
        return errno;
    }

    FD_ZERO(&actfds);
    FD_SET(sd, &actfds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    nfds = sd;

    printf("[server] We are waiting at port %d...\n", PORT);
    fflush(stdout);

    pthread_t match_making_system_tid;
    pthread_create(&match_making_system_tid, NULL, &match_making_system, NULL);

    while (1)
    {
        bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));
        if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror("[server] Error in select().\n");
            return errno;
        }

        if (FD_ISSET(sd, &readfds))
        {
            len = sizeof(from);
            bzero(&from, sizeof(from));

            client = accept(sd, (struct sockaddr *)&from, &len);

            if (client < 0)
            {
                perror("[server] Error in accept().\n");
                continue;
            }

            if (nfds < client)
                nfds = client;

            FD_SET(client, &actfds);
            printf("[server] Client with descriptor %d has connected, from address %s.\n", client, conv_addr(from));
            fflush(stdout);

            Player *player = new Player;
            player->fd = client;
            player->free = 1;
            player->logged_in = 0;

            cout << "got new one " << &player << ':' << player->fd << endl;
            unique_lock<mutex> lock(vector_mutex);
            online_players.push_back(player);
            lock.unlock();

            pthread_t tid;
            // new client connected, make a thread to handle him
            pthread_create(&tid, NULL, &client_operation, &client);
        }

        // handling matchmaking (can be put into a separate thread in the future if needed)
        // int i = 0;
        // while (match_making_players.size() >= 2)
        // {

        //     Player *player_a = match_making_players.front();
        //     match_making_players.pop();
        //     Player *player_b = match_making_players.front();
        //     match_making_players.pop();
        //     cout << player_a->fd << " and " << player_b->fd << endl;

        //     // change their state to busy (already done when chose to match make)
        //     // player_a->free = 0;
        //     // player_b->free = 0;

        //     // cout << "The size now is " << match_making_players.() << endl;

        //     // thread for managing game play
        //     PlayGameThreadData game_data;
        //     game_data.player_a = player_a;
        //     game_data.player_b = player_b;

        //     cout << "new game between " << player_a->fd << " and " << player_b->fd << endl;
        //     pthread_t tid;
        //     pthread_create(&tid, NULL, &play_game, &game_data);
        // }
    }
    close(sd);
}

// Thread function for the matchmaking system
void *match_making_system(void *arg)
{
    while (true)
    {
        char A[9][9];
        if (match_making_players.size() < 2)
            continue;
        unique_lock<mutex> lock(queue_mutex);

        // Pair the first two players in the queue
        Player *player_a = match_making_players.front();
        match_making_players.pop();
        Player *player_b = match_making_players.front();
        match_making_players.pop();

        lock.unlock();

        // make initial board
        create_table(A);
        string s = convert(A);
        // thread for managing game play
        PlayGameThreadData *game_data = new PlayGameThreadData;
        game_data->player_a = player_a;
        game_data->player_b = player_b;
        game_data->initial_board = s;

        pthread_t tid;
        int thread_check = pthread_create(&tid, NULL, &play_game, game_data);
        if (thread_check != 0)
        {
            cerr << "Failed to create thread." << endl;
            break;
        }

        // Wait for the game thread to finish
        // thread_check = pthread_join(tid, NULL);
        // if (thread_check != 0)
        // {
        //     cerr << "Failed to join thread." << endl;
        //     return;
        // }
    }
}

void *play_game(void *arg)
{
    char A[9][9];
    create_table(A);
    int vizA[4] = {0};
    int vizB[4] = {0};

    PlayGameThreadData *data = (PlayGameThreadData *)arg;

    Player(*a) = (data->player_a);
    Player(*b) = (data->player_b);
    string s = (data->initial_board);
    (*a).round = 1;
    (*b).round = 0;

    int ft = 0, current_fd;
    // Working ... need to constantly check surrender message from both players

    string moves_played = "";
    while (1)
    {
        cout << "Game between " << (*a).fd << " and " << (*b).fd << endl;

        if ((*a).round == 1)
        {
            current_fd = (*a).fd;
            int verify = 0;

            if (!ft)
            {
                int bytes = s.size();
                if (bytes && send(current_fd, s.c_str(), bytes, 0) < 0)
                {
                    perror("[server] Error in send() to the client.\n");
                    return 0;
                }
                ft = 1;
            }
            if (get_move(A, vizA, vizB, current_fd, (*b).fd, verify, 'a', moves_played) == -1)
            {
                break;
            }
            else if (verify == 1)
            {
                (*a).round = 0;
                (*b).round = 1;
                verify = 0;
            }
        }
        if ((*b).round == 1)
        {
            current_fd = (*b).fd;
            int verify = 0;
            if (get_move(A, vizA, vizB, current_fd, (*a).fd, verify, 'b', moves_played) == -1)
            {
                break;
            }
            else if (verify == 1)
            {
                (*b).round = 0;
                (*a).round = 1;
                verify = 0;
            }
        }
    }
    // game finished, make them free
    a->free = 1;
    b->free = 1;
    cout << a->fd << " and " << b->fd << " are free " << endl;
    int *result = new int(42);
    pthread_exit(result);
}

std::vector<User> readAccountsFile()
{
    std::vector<User> users;
    std::ifstream accountsFile("accounts.txt");
    if (!accountsFile)
    {
        std::cerr << "Failed to open accounts file" << std::endl;
        return users;
    }

    std::string line;
    while (std::getline(accountsFile, line))
    {
        std::istringstream iss(line);
        User user;
        if (iss >> user.username >> user.password >> user.elo)
        {
            users.push_back(user);
        }
    }

    return users;
}

bool isUserValid(const std::string &username, const std::string &password)
{
    std::ifstream accounts(ACCOUNTS_FILE);
    if (!accounts)
    {
        std::cerr << "Failed to open accounts file" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(accounts, line))
    {
        std::string storedUsername, storedPassword;
        std::istringstream iss(line);
        if (iss >> storedUsername >> storedPassword)
        {
            if (storedUsername == username && storedPassword == password)
            {
                return true;
            }
        }
    }

    return false;
}

void handleLoginRequest(const json &requestData, int clientSocket)
{
    std::string username = requestData["username"];
    std::string password = requestData["password"];

    std::cout << username << std::endl;
    std::cout << password << std::endl;

    // Perform login validation/authentication logic
    User user = findUserByUsername(username);
    bool isValid = (user.username == username && user.password == password);
    // Craft the response JSON
    json response;
    response["type"] = static_cast<int>(RequestType::Login);
    response["success"] = isValid;
    response["message"] = isValid ? "Login successful" : "Invalid username or password";

    if (isValid)
    {
        response["data"] = {{"username", user.username},
                            {"password", user.password},
                            {"elo", user.elo}};
    }

    // Serialize the response JSON
    std::string responseStr = response.dump();
    std::cout << "response: " << responseStr << std::endl;

    // Send the response back to the client
    if (send(clientSocket, responseStr.c_str(), responseStr.size(), 0) == -1)
    {
        std::cerr << "Failed to send response to client" << std::endl;
    }
}

void *client_operation(void *arg)
{
    int client_fd = *((int *)arg);

    unique_lock<mutex> lock(vector_mutex);
    Player *this_player = find_online_player(client_fd);
    lock.unlock();

    int connected = 1;
    while (connected)
    {
        cout << "waiting for " << client_fd << " to choose an option" << endl;
        std::array<char, 1024> buffer{};
        ssize_t bytesRead = recv(client_fd, buffer.data(), buffer.size(), 0);
        if (bytesRead <= 0)
        {
            cerr << "Error receiving data" << endl;
            close(client_fd);
            continue;
        }
        // Parse the received data into JSON
        string requestData(buffer.data(), bytesRead);
        cout << requestData << endl;
        // Find the position of the first opening brace '{'
        size_t bracePos = requestData.find_first_of('{');
        if (bracePos != string::npos)
        {
            // Extract the substring from the brace position until the end of the string
            string jsonSubstring = requestData.substr(bracePos);

            // Parse the extracted JSON substring
            json jsonData = json::parse(jsonSubstring);

            // Determine the type of request and dispatch to the appropriate handler
            int requestType = jsonData["type"];

            if (requestType == static_cast<int>(RequestType::Login))
            {
                cout << "Received login request from " << client_fd << endl;
                handleLoginRequest(jsonData["data"], client_fd);
            }
            else if (requestType == static_cast<int>(RequestType::MatchMaking))
            {
                cout << "Received matchmaking request from " << client_fd << endl;
                handleMatchMakingRequest(jsonData["data"], client_fd);
            }
            else if (requestType == static_cast<int>(RequestType::GetOnlinePlayersList))
            {
                cout << "Received online players list request from " << client_fd << endl;
                handleGetOnlinePlayersListRequest(jsonData["data"], client_fd);
            }
            else if (requestType == static_cast<int>(RequestType::Challenge))
            {
                cout << "Received challenge request from " << client_fd << endl;
                handleChallengeRequest(jsonData["data"], client_fd);
            }
            else if (requestType == static_cast<int>(RequestType::Logout))
            {
                cout << "Received logout request from " << client_fd << endl;
                if (handleLogoutRequest(jsonData["data"], client_fd))
                    connected = 0;
            }

            // else
            // {
            //     // Handle unknown or unsupported request types
            //     cerr << "Unknown request type: " << requestType << endl;
            // }
        }
        else
        {
            // Handle the case where no opening brace is found
            cerr << "Invalid request data: " << requestData << endl;
        }

        print_server_state();
    }
}