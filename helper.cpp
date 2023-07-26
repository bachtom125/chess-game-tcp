// #include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/time.h>
// #include <netinet/in.h>
// #include <unistd.h>
#include <errno.h>
#include <stdio.h>
// #include <arpa/inet.h>
#include <string.h>
// #include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <algorithm>
// #include <pthread.h>
#include <vector>
#include <queue>
#include <mutex>
// #include <nlohmann/json.hpp>
#include <fstream> // Add this line
#include <condition_variable>
#include <chrono>
#include <sstream>
#include <random>

using namespace std;
typedef long long ll;
vector<int> list;

void match_making_system()
{
    while (true)
    {
        if (match_making_players.size() < 2)
            continue;
        unique_lock<mutex> lock(queue_mutex);

        queue<Player *> match_making_players_copy = match_making_players;
        vector<Player *> match_making_players_vector;
        while (!match_making_players_copy.empty())
        {
            match_making_players_vector.push_back(match_making_players_copy.front());
            match_making_players_copy.pop();
        }

        cout <<
            // Pair the first two players in the queue
            Player *player_a = match_making_players.front();
        match_making_players.pop();
        Player *player_b = match_making_players.front();
        match_making_players.pop();

        lock.unlock();

        // make initial board
        char A[9][9];
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

int main()
{
    int a = 2;
    change(a);
    cout << a << endl;
}
