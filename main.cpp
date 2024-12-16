#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <fstream>

using namespace std;

vector<string> vec;
void readArray(){
    ifstream f = ifstream();
    f.open("russian-nouns.txt");
    string str;
    while (f >> str) {
        vec.push_back(str);
    }
}

bool wordExists(const string &playerWrd) {
    return find(vec.begin(),vec.end(), playerWrd) != end(vec);
}

int main(){
    string playerWrd;
    cin >> playerWrd;
    cout << wordExists(playerWrd);
}