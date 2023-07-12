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
#include <condition_variable>

#define PORT 3000
using namespace std;

extern int errno;
struct Player
{
    int round;
    int fd;
    int free;
};

fd_set readfds;
fd_set actfds;
int v[250] = {0};

vector<Player> online_players;
queue<Player *> match_making_players;

mutex queue_mutex;
condition_variable queue_condition;
struct PlayGameThreadData
{
    Player *player_a;
    Player *player_b;
};

void *client_operation(void *);
void *play_game(void *);

void remove_player_from_matchmaking()
{
    // Working
}

void remove_player_from_online()
{
    // Working
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

    // for (i = 1; i <= 8; i++)
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

void send_result(int loser_fd, int winner_fd)
{
    char msg[100];
    strcpy(msg, "`");
    if (write(winner_fd, msg, strlen(msg)) < 0)
    {
        cerr << "Error occurred while sending message to the Winner." << endl;
    }
    strcpy(msg, "loser");
    if (write(loser_fd, msg, strlen(msg)) < 0)
    {
        cerr << "Error occurred while sending message to the Loser" << endl;
    }
    cout << "The winner is " << winner_fd << endl;
}

int message(char A[9][9], int vizA[4], int vizB[4], int fd, int opponent_fd, int &verify, char move)
{
    string s;
    char buffer[100];
    int bytes;
    char msg[100], save;
    char msgrasp[100] = " ";

    bytes = read(fd, msg, sizeof(buffer));
    if (bytes < 0)
    {
        perror("Error in read() from the client.\n");
        return 0;
    }

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

    cout << type << " " << sr << " " << sc << " " << dr << " " << dc << " " << transform << endl;

    if (strcmp(msg, "surrender\n") == 0)
    {
        send_result(fd, opponent_fd);
        return -1;
    }
    else if (move == 'a' && check_mate(A, 'K'))
    {

        send_result(fd, opponent_fd);
        return -1;
    }
    else if (move == 'b' && check_mate(A, 'k'))
    {

        send_result(opponent_fd, fd);
        return -1;
    }
    else if (move == 'a' && is_black_piece(type))
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

        cout << type << " It was moved from position " << c1 << " " << sr << " to position " << c2 << " " << dr << endl;
        cout << "Waiting for the other player's move!" << endl;

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

            Player player;
            player.fd = client;
            player.free = 1;
            online_players.push_back(player);

            pthread_t tid;
            // new client connected, make a thread to handle him
            pthread_create(&tid, NULL, &client_operation, &client);
        }

        // handling matchmaking (can be put into a separate thread in the future if needed)
        int i = 0;
        while (match_making_players.size() >= 2)
        {
            Player *player_a = match_making_players.front();
            match_making_players.pop();
            Player *player_b = match_making_players.front();
            match_making_players.pop();

            // change their state to busy (already done when chose to match make)
            // player_a->free = 0;
            // player_b->free = 0;

            cout << "The size now is " << match_making_players.size() << endl;

            // thread for managing game play
            PlayGameThreadData game_data;
            game_data.player_a = player_a;
            game_data.player_b = player_b;

            pthread_t tid;
            pthread_create(&tid, NULL, &play_game, &game_data);
        }
    }
}

// Thread function for the matchmaking system
void match_making_system()
{
    while (true)
    {
        unique_lock<mutex> lock(queue_mutex);

        // Wait until there are at least two players in the queue
        queue_condition.wait(lock, []
                             { return match_making_players.size() >= 2; });

        // Pair the first two players in the queue
        Player *player_a = match_making_players.front();
        match_making_players.pop();
        Player *player_b = match_making_players.front();
        match_making_players.pop();

        lock.unlock();

        // thread for managing game play
        PlayGameThreadData game_data;
        game_data.player_a = player_a;
        game_data.player_b = player_b;

        pthread_t tid;
        int thread_check = pthread_create(&tid, NULL, &play_game, &game_data);
        if (thread_check != 0)
        {
            cerr << "Failed to create thread." << endl;
            return;
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
    int vizA[4] = {0};
    int vizB[4] = {0};

    PlayGameThreadData *data = static_cast<PlayGameThreadData *>(arg);

    Player a = *(data->player_a);
    Player b = *(data->player_b);
    create_table(A);
    cout << "The game has started between " << a.fd << " and " << b.fd << endl;
    a.round = 1;
    b.round = 0;

    int ft = 0, current_fd;
    while (1)
    {
        if (a.round == 1)
        {
            current_fd = a.fd;
            int verify = 0;

            if (!ft)
            {
                string s;
                s = convert(A);
                int bytes = s.size();
                if (bytes && send(current_fd, s.c_str(), bytes, 0) < 0)
                {
                    perror("[server] Error in send() to the client.\n");
                    return 0;
                }
                ft = 1;
            }
            if (message(A, vizA, vizB, current_fd, b.fd, verify, 'a') == -1)
            {
                // close(current_fd);
                // // Working ...
                // FD_CLR(current_fd, &actfds);
                // close(b.fd);
                // FD_CLR(b.fd, &actfds);
                // v[a.fd] = 0;
                // v[b.fd] = 0;
                // exit(0);

                break;
            }
            else if (verify == 1)
            {
                a.round = 0;
                b.round = 1;
                verify = 0;
            }
        }
        if (b.round == 1)
        {
            current_fd = b.fd;
            int verify = 0;
            if (message(A, vizA, vizB, current_fd, a.fd, verify, 'b') == -1)
            {
                // // Working ...
                // close(current_fd);
                // FD_CLR(current_fd, &actfds);
                // close(a.fd);
                // FD_CLR(a.fd, &actfds);
                // v[a.fd] = 0;
                // v[b.fd] = 0;
                // exit(0);

                break;
            }
            else if (verify == 1)
            {
                b.round = 0;
                a.round = 1;
                verify = 0;
            }
        }
    }
    // game finished, make them free
    a.free = 1;
    b.free = 1;

    int *result = new int(42);
    pthread_exit(result);
}

void *client_operation(void *arg)
{
    int client_fd = *((int *)arg);
    // Working ...
    // Idea: Make an infinite loop the loop will check if there is an incoming match challange, or if the user select any option
    // and for the actual game, maybe make another thread/process specifically for that game
    // reference: https://chat.openai.com/share/41ad436c-48d2-450f-95e2-d3a0d93ab8f1
    Player *this_player;
    for (Player player : online_players)
    {
        if (player.fd == client_fd)
        {
            this_player = &player;
            break;
        }
    }

    while (1)
    {
        while (1)
        {
            if (this_player->free == 1)
                break;
        }
        int option;
        if (recv(client_fd, &option, sizeof(option), 0) <= 0)
        {
            cout << "Error occurred while receiving option from the client." << endl;
            close(client_fd);
        }
        cout << "Client with fd " << client_fd << " chose " << option << endl;

        switch (option)
        {
        case 1: // Random matchmaking:

            match_making_players.push(this_player);
            this_player->free = 0;
            break;
        case 2: // Play a friend
            break;
        case 3: // Challenge a friend
            break;
        default:
            break;
        }
    }
}

// the commented code below is implementation of playing a chess game between 2 players
//     if (nfds % 2 == 1 && nfds > 3 && !v[nfds - 1] && !v[nfds])
//     {
//         // cout << a.fd << " " << b.fd << endl;
//         v[nfds - 1] = 1;
//         v[nfds] = 1;
//         if ((childpid = fork()) == 0)
//         {
//             create_table(A);
//             // PrintTable(A);
//             cout << "The game has started!" << endl;
//             Player a;
//             Player b;
//             a.round = 1;
//             a.fd = nfds - 1;
//             b.round = 0;
//             b.fd = nfds;
//             close(sd);
//             int ft = 0;
//             while (1)
//             {
//                 if (a.round == 1)
//                 {
//                     fd = a.fd;
//                     int verify = 0;
//                     if (fd != sd)
//                     {
//                         if (!ft)
//                         {
//                             string s;
//                             s = convert(A);
//                             int bytes = s.size();
//                             if (bytes && write(fd, s.c_str(), bytes) < 0)
//                             {
//                                 perror("[server] Error in write() to the client.\n");
//                                 return 0;
//                             }
//                             ft = 1;
//                         }
//                         if (message(fd, verify, 'a') == -1)
//                         {
//                             close(fd);
//                             FD_CLR(fd, &actfds);
//                             close(fd + 1);
//                             FD_CLR(fd + 1, &actfds);
//                             v[a.fd] = 0;
//                             v[b.fd] = 0;
//                             exit(0);
//                         }
//                         else if (verify == 1)
//                         {
//                             a.round = 0;
//                             b.round = 1;
//                             verify = 0;
//                         }
//                     }
//                 }
//                 if (b.round == 1)
//                 {
//                     fd = b.fd;
//                     int verify = 0;
//                     if (fd != sd)
//                     {
//                         if (message(fd, verify, 'b') == -1)
//                         {
//                             close(fd);
//                             FD_CLR(fd, &actfds);
//                             close(fd - 1);
//                             FD_CLR(fd - 1, &actfds);
//                             v[a.fd] = 0;
//                             v[b.fd] = 0;
//                             exit(0);
//                         }
//                         else if (verify == 1)
//                         {
//                             b.round = 0;
//                             a.round = 1;
//                             verify = 0;
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
