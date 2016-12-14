#include "querycorrector.h"
#include <iostream>

using namespace std;

int main()
{
  QueryCorrector queryCorrector;
  while (1) {
    cout << "Enter a query:" << endl;
    string query;
    getline(cin, query);
    auto res = queryCorrector.generateCorrections(query);
    cout << "Suggested corrections:" << endl;
    for (auto line : res) {
      cout << line.first + ' ' << line.second[0] << endl;
    }
  }
}
