#include <iostream>
using namespace std;
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

    for (i = 0; i <= 8; i++)
    {
        for (j = 0; j <= 8; j++)
            cout << A[i][j] << ' ';
        cout << endl;
    }
}
int main()
{
    char A[9][9];
    create_table(A);
    return 0;
}