#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

using namespace std;
typedef long long ll;
vector<int> list;
struct a
{
    string a = "";
};

void putout(a *var)
{
    cout << var->a << endl;
}
int main()
{
    string b = "BONGBONGBONGBONGBONG";

    a var;
    var.a = b;
    putout(&var);
    return 0;
}
