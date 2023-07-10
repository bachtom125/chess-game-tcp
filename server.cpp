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

#define PORT 3000
using namespace std;

extern int errno;

char A[9][9];
int vizA[4] = {0};
int vizB[4] = {0};
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

    for (i = 1; i <= 8; i++)
    {
        for (j = 1; j <= 8; j++)
            cout << A[i][j] << ' ';
        cout << endl;
    }
}

struct Player
{
    int round;
    int fd;
};

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

int move_straight_line(int sr, int sc, int fr, int fc)
{ // move along either row or column (1 only)
    int i, j;
    if (fr > sr && fc == sc)
    {
        i = sr + 1, j = sc;
        while (A[i][j] == '-' && i < fr)
            i++;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fr < sr && fc == sc)
    {
        i = sr - 1, j = sc;
        while (A[i][j] == '-' && i > fr)
            i--;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fc > sc && fr == sr)
    {
        i = sr, j = sc + 1;
        while (A[i][j] == '-' && j < fc)
            j++;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fc < sc && fr == sr)
    {
        i = sr, j = sc - 1;
        while (A[i][j] == '-' && j > fc)
            j--;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else
        return 0;
}

int move_diagonal_line(int sr, int sc, int fr, int fc)
{ // moving diagonally (long both rows and columns)
    int i, j;
    if (fr > sr && fc > sc)
    {
        i = sr + 1, j = sc + 1;
        while (A[i][j] == '-' && i < fr && j < fc)
            i++, j++;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fr > sr && fc < sc)
    {
        i = sr + 1, j = sc - 1;
        while (A[i][j] == '-' && i < fr && j > fc)
            i++, j--;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fr < sr && fc > sc)
    {
        i = sr - 1, j = sc + 1;
        while (A[i][j] == '-' && i > fr && j < fc)
            i--, j++;
        if (i == fr && j == fc)
            return 1;
        else
            return 0;
    }
    else if (fr < sr && fc < sc)
    {
        i = sr - 1, j = sc - 1;
        while (A[i][j] == '-' && i > fr && j > fc)
            i--, j--;
        if (i == fr && j == fc)
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

int pawn(char type, int sr, int sc, int fr, int fc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // Outside of matrix
        else if (fr <= sr)
            return 0; // The pawn is not allowed to move backwards.
        else if (fr - sr == 2 && sr == 2 && fc == sc && A[fr][fc] == '-')
        {
            if (is_black_piece(A[fr][fc]))
                return 0;
            else
                return 1;
        }
        else if (fr - sr == 1 && abs(fc - sc) == 1 && (is_black_piece(A[fr][fc])))
            return 1;
        else if (fr - sr == 1 && fc == sc && !is_black_piece(A[fr][fc]))
            return 1;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if (fr >= sr)
            return 0; // nu are voie pawnul sa fie dat inapoi
        else if (sr - fr == 2 && sr == 7 && fc == sc && A[fr][fc] == '-')
        {
            if (is_white_piece(A[fr][fc]))
                return 0;
            else
                return 1;
        }
        else if (sr - fr == 1 && abs(fc - sc) == 1 && (is_white_piece(A[fr][fc])))
            return 1;
        else if (sr - fr == 1 && fc == sc && !is_white_piece(A[fr][fc]))
            return 1;
    }
}

int king(char type, int sr, int sc, int fr, int fc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if ((abs(fr - sr) == 1 || abs(fr - sr) == 0) && (abs(fc - sc) == 1 || abs(fc - sc) == 0))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if ((abs(fr - sr) == 1 || abs(fr - sr) == 0) && (abs(fc - sc) == 1 || abs(fc - sc) == 0))
            return 1;
        else
            return 0;
    }
}

int knight(char type, int sr, int sc, int fr, int fc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        if ((abs(fr - sr) == 1 || abs(fr - sr) == 2) && (abs(fc - sc) == 1 || abs(fc - sc) == 2))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        if ((abs(fr - sr) == 1 || abs(fr - sr) == 2) && (abs(fc - sc) == 1 || abs(fc - sc) == 2))
            return 1;
        else
            return 0;
    }
}

int rook(char type, int sr, int sc, int fr, int fc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if (move_straight_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if (move_straight_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
}

int bishop(char type, int sr, int sc, int fr, int fc)
{
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0;
        else if (move_diagonal_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0;
        else if (move_diagonal_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
}

int queen(char type, int sr, int sc, int fr, int fc)
{
    int i, j;
    if (is_white_piece(type))
    {
        if (is_white_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if (move_diagonal_line(sr, sc, fr, fc) || move_straight_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
    else if (is_black_piece(type))
    {
        if (is_black_piece(A[fr][fc]))
            return 0;
        else if (fr > 8 || fr < 1 || fc > 8 || fc < 1)
            return 0; // in afara matricii
        else if (move_diagonal_line(sr, sc, fr, fc) || move_straight_line(sr, sc, fr, fc))
            return 1;
        else
            return 0;
    }
}

int is_valid_move(char type, int sr, int sc, int fr, int fc)
{
    switch (type)
    {
    case 'p':
        if (pawn(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'P':
        if (pawn(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'k':
        if (king(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'K':
        if (king(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'c':
        if (knight(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'C':
        if (knight(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'r':
        if (rook(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'R':
        if (rook(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'b':
        if (bishop(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'B':
        if (bishop(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'q':
        if (queen(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    case 'Q':
        if (queen(type, sr, sc, fr, fc))
            return 1;
        else
            return 0;
    default:
        return 0;
    }
}

void find_my_king(char type, int &fr, int &fc)
{
    int i, j;
    for (i = 1; i <= 8; i++)
        for (j = 1; j <= 8; j++)
            if (A[i][j] == type)
            {
                fr = i;
                fc = j;
                break;
            }
}

int is_checked(char type, int fr, int fc)
{
    int i, j;
    if (fr < 1 || fr > 8 || fc < 1 || fc > 8)
        return 0;
    else if (is_white_piece(type))
    {
        for (i = 1; i <= 8; i++)
            for (j = 1; j <= 8; j++)
                if (is_black_piece(A[i][j]))
                    if (is_valid_move(A[i][j], i, j, fr, fc))
                        return 1;

        return 0;
    }
    else if (is_black_piece(type))
    {
        for (i = 1; i <= 8; i++)
            for (j = 1; j <= 8; j++)
                if (is_white_piece(A[i][j]))
                    if (is_valid_move(A[i][j], i, j, fr, fc))
                        return 1;

        return 0;
    }
}

int check(char type)
{
    int fr, fc;
    find_my_king(type, fr, fc);
    if (is_checked(type, fr, fc))
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

int check_mate(char type)
{
    const int dir[8] = {0};
    char B[9][9], C[9][9];
    copy(B, A);
    copy(C, A);
    int i, j, fr, fc;
    find_my_king(type, fr, fc);
    if (is_black_piece(type))
    {
        if (C[fr - 1][fc] == '-')
            A[fr - 1][fc] = 'K';
        if (C[fr + 1][fc] == '-')
            A[fr + 1][fc] = 'K';
        if (C[fr - 1][fc - 1] == '-')
            A[fr - 1][fc - 1] = 'K';
        if (C[fr - 1][fc + 1] == '-')
            A[fr - 1][fc + 1] = 'K';
        if (C[fr + 1][fc + 1] == '-')
            A[fr + 1][fc + 1] = 'K';
        if (C[fr + 1][fc - 1] == '-')
            A[fr + 1][fc - 1] = 'K';
        if (C[fr][fc - 1] == '-')
            A[fr][fc - 1] = 'K';
        if (C[fr][fc + 1] == '-')
            A[fr][fc + 1] = 'K';
    }
    else if (is_white_piece(type))
    {
        if (C[fr - 1][fc] == '-')
            A[fr - 1][fc] = 'k';
        if (C[fr + 1][fc] == '-')
            A[fr + 1][fc] = 'k';
        if (C[fr - 1][fc - 1] == '-')
            A[fr - 1][fc - 1] = 'k';
        if (C[fr - 1][fc + 1] == '-')
            A[fr - 1][fc + 1] = 'k';
        if (C[fr + 1][fc + 1] == '-')
            A[fr + 1][fc + 1] = 'k';
        if (C[fr + 1][fc - 1] == '-')
            A[fr + 1][fc - 1] = 'k';
        if (C[fr][fc - 1] == '-')
            A[fr][fc - 1] = 'k';
        if (C[fr][fc + 1] == '-')
            A[fr][fc + 1] = 'k';
    }

    if (check(type))
    {
        int nr = 1;
        if (is_checked(type, fr - 1, fc) && B[fr - 1][fc] == '-')
            B[fr - 1][fc] = 'x', nr++;
        if (is_checked(type, fr + 1, fc) && B[fr + 1][fc] == '-')
            B[fr + 1][fc] = 'x', nr++;
        if (is_checked(type, fr - 1, fc - 1) && B[fr - 1][fc - 1] == '-')
            B[fr - 1][fc - 1] = 'x', nr++;
        if (is_checked(type, fr - 1, fc + 1) && B[fr - 1][fc + 1] == '-')
            B[fr - 1][fc + 1] = 'x', nr++;
        if (is_checked(type, fr + 1, fc + 1) && B[fr + 1][fc + 1] == '-')
            B[fr + 1][fc + 1] = 'x', nr++;
        if (is_checked(type, fr + 1, fc - 1) && B[fr + 1][fc - 1] == '-')
            B[fr + 1][fc - 1] = 'x', nr++;
        if (is_checked(type, fr, fc - 1) && B[fr][fc - 1] == '-')
            B[fr][fc - 1] = 'x', nr++;
        if (is_checked(type, fr, fc + 1) && B[fr][fc + 1] == '-')
            B[fr][fc + 1] = 'x', nr++;
        copy(A, C);
        if (B[fr][fc + 1] == '-' || B[fr][fc - 1] == '-' || B[fr + 1][fc - 1] == '-' || B[fr + 1][fc + 1] == '-' || B[fr - 1][fc + 1] == '-' || B[fr - 1][fc - 1] == '-' || B[fr + 1][fc] == '-' ||
            B[fr - 1][fc] == '-')
            return 0;
        if (nr > 1)
            return 1;
    }
    return 0;
}

int message(int fd, int &verify, char move)
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

    int sr, sc, fr, fc;
    char type, c1, c2, transform;
    strcpy(msgrasp, msg);
    int i, nr = 0;
    // Preluare coordonate.
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
                fc = (int)msgrasp[i] - 96;
                c2 = msgrasp[i];
            }
            case 3:
                fr = (int)msgrasp[i] - 48;

            case 4:
                transform = msgrasp[i];
            }
            nr++;
        }
    }

    save = A[fr][fc];
    type = A[sr][sc];
    // Vizitari piese pentru a verifica daca se poate executa rocada sau nu. Daca piesele au fost mutate rocada nu mai poate avea loc.
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

    cout << type << " " << sr << " " << sc << " " << fr << " " << fc << " " << transform << endl;

    if (strcmp(msg, "surrender\n") == 0)
        return -1;
    else if (move == 'a' && check_mate('K'))
    {
        strcpy(msg, "The winner is player B");
        write(fd, msg, strlen(msg));
        return -1;
    }
    else if (move == 'b' && check_mate('k'))
    {
        strcpy(msg, "The winner is player A");
        write(fd, msg, strlen(msg));
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
        if (sr == 1 && fr == 1)
        {
            char t1, t2;
            t1 = A[sr][sc], t2 = A[fr][fc];
            if (sc == 1 && fc == 5)
            {
                if (is_white_piece(t1) && is_white_piece(t2))
                {
                    if (!is_checked(t1, fr, fc - 1) && !is_checked(t2, fr, fc - 2))
                    {
                        if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
                            if (!vizA[1] && !vizA[2])
                            {
                                A[sr][sc] = '-';
                                A[fr][fc] = '-';
                                A[sr][sc + 2] = t2;
                                A[sr][fc - 1] = t1;
                                vizA[1] = 1;
                                vizA[2] = 1;
                                ok = 1;
                            }
                    }
                }
            }
            else if (sc == 5 && fc == 8)
            {
                if (is_white_piece(t1) && is_white_piece(t2))
                {
                    if (!is_checked(t1, fr, sc + 1) && !is_checked(t2, fr, sc + 2))
                    {
                        if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
                            if (!vizA[2] && !vizA[3])
                            {
                                A[sr][sc] = '-';
                                A[fr][fc] = '-';
                                A[sr][sc + 1] = t2;
                                A[sr][fc - 1] = t1;
                                vizA[2] = 1;
                                vizA[3] = 1;
                                ok = 1;
                            }
                    }
                }
            }
        }
        else if (sr == 8 && fr == 8)
        {
            char t1, t2;
            t1 = A[sr][sc], t2 = A[fr][fc];
            if (sc == 1 && fc == 5)
            {
                if (is_black_piece(t1) && is_black_piece(t2))
                {
                    if (!is_checked(t1, fr, fc - 1) && !is_checked(t2, fr, fc - 2))
                    {
                        if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-' && A[sr][sc + 3] == '-')
                            if (!vizB[1] && !vizB[2])
                            {
                                A[sr][sc] = '-';
                                A[fr][fc] = '-';
                                A[sr][sc + 2] = t2;
                                A[sr][fc - 1] = t1;
                                vizB[1] = 1;
                                vizB[2] = 1;
                                ok = 1;
                            }
                    }
                }
            }
            else if (sc == 5 && fc == 8)
            {
                if (is_black_piece(t1) && is_black_piece(t2))
                {
                    if (!is_checked(t1, fr, sc + 1) && !is_checked(t2, fr, sc + 2))
                    {
                        if (A[sr][sc + 1] == '-' && A[sr][sc + 2] == '-')
                            if (!vizB[2] && !vizB[3])
                            {
                                A[sr][sc] = '-';
                                A[fr][fc] = '-';
                                A[sr][sc + 1] = t2;
                                A[sr][fc - 1] = t1;
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
    else if (!is_valid_move(type, sr, sc, fr, fc))
    {
        strcpy(msg, "Invalid move");
        write(fd, msg, strlen(msg));
        return -2;
    }
    else if (is_valid_move(type, sr, sc, fr, fc))
    {
        A[sr][sc] = '-';
        A[fr][fc] = type;
        if (move == 'a' && check('K'))
        {
            strcpy(msg, "Invalid move! check!");
            write(fd, msg, strlen(msg));
            A[sr][sc] = type;
            A[fr][fc] = save;
            return -2;
        }
        else if (move == 'b' && check('k'))
        {
            strcpy(msg, "Invalid move! check!");
            write(fd, msg, strlen(msg));
            A[sr][sc] = type;
            A[fr][fc] = save;
            return -2;
        }

        // Transformation invalid for pawn when reaching the enemy's last row.
        if (type == 'p' && fr == 1 && is_black_piece(transform) && (transform == 'b' || transform == 'c' || transform == 'q' || transform == 'r'))
            A[sr][sc] = '-', A[fr][fc] = transform;
        else if (type == 'P' && fr == 8 && is_white_piece(transform) && (transform == 'B' || transform == 'C' || transform == 'Q' || transform == 'R'))
            A[sr][sc] = '-', A[fr][fc] = transform;
        else if ((transform != 'b' || transform != 'c' || transform != 'q' || transform != 'r' || transform != 'B' || transform != 'C' || transform != 'Q' || transform != 'R') && (type == 'p' || type == 'P'))
        {
            if (fr == 1 || fr == 8)
            {
                strcpy(msg, "Invalid transformation!");
                write(fd, msg, strlen(msg));
                return -2;
            }
        }

        if (move == 'a' && check('k'))
        {
            strcpy(msg, "Move executed! The enemy's king is in check!");
            s = convert(A);
            bytes = s.size();
        }
        else if (move == 'b' && check('K'))
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

        cout << type << " It was moved from position " << c1 << " " << sr << " to position " << c2 << " " << fr << endl;
        cout << "Waiting for the other player's move!" << endl;

        if (fd % 2 == 0)
        {
            if (bytes && write(fd + 1, s.c_str(), bytes) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, strlen(msg)) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }
        }
        else if (fd % 2 == 1)
        {
            if (bytes && write(fd - 1, s.c_str(), bytes) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }

            if (strlen(msg) && write(fd, msg, strlen(msg)) < 0)
            {
                perror("[server] Error in write() to the client.\n");
                return 0;
            }
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
    fd_set readfds;
    fd_set actfds;
    struct timeval tv;
    int sd, client;
    int optval = 1;
    int fd;
    int nfds;
    pid_t childpid;
    unsigned int len;
    int v[250] = {0};

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
        }

        if (nfds % 2 == 1 && nfds > 3 && !v[nfds - 1] && !v[nfds])
        {
            // cout << a.fd << " " << b.fd << endl;
            v[nfds - 1] = 1;
            v[nfds] = 1;
            if ((childpid = fork()) == 0)
            {
                create_table(A);
                // PrintTable(A);
                cout << "The game has started!" << endl;
                Player a;
                Player b;
                a.round = 1;
                a.fd = nfds - 1;
                b.round = 0;
                b.fd = nfds;
                close(sd);
                int ft = 0;
                while (1)
                {
                    if (a.round == 1)
                    {
                        fd = a.fd;
                        int verify = 0;
                        if (fd != sd)
                        {
                            if (!ft)
                            {
                                string s;
                                s = convert(A);
                                int bytes = s.size();
                                if (bytes && write(fd, s.c_str(), bytes) < 0)
                                {
                                    perror("[server] Error in write() to the client.\n");
                                    return 0;
                                }
                                ft = 1;
                            }
                            if (message(fd, verify, 'a') == -1)
                            {
                                close(fd);
                                FD_CLR(fd, &actfds);
                                close(fd + 1);
                                FD_CLR(fd + 1, &actfds);
                                v[a.fd] = 0;
                                v[b.fd] = 0;
                                exit(0);
                            }
                            else if (verify == 1)
                            {
                                a.round = 0;
                                b.round = 1;
                                verify = 0;
                            }
                        }
                    }
                    if (b.round == 1)
                    {
                        fd = b.fd;
                        int verify = 0;
                        if (fd != sd)
                        {
                            if (message(fd, verify, 'b') == -1)
                            {
                                close(fd);
                                FD_CLR(fd, &actfds);
                                close(fd - 1);
                                FD_CLR(fd - 1, &actfds);
                                v[a.fd] = 0;
                                v[b.fd] = 0;
                                exit(0);
                            }
                            else if (verify == 1)
                            {
                                b.round = 0;
                                a.round = 1;
                                verify = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}
