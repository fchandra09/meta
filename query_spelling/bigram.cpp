#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <unordered_map>


using namespace std;
vector<string> split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

int main() 
{ 
    unordered_map<string, double> bigrams;
    std::ifstream file("bigrams_data.txt");
    std::string str; 
    while (std::getline(file, str))
    {
      auto tokens = split(str, ',');
      auto key = tokens[1] + ',' + tokens[2];
      auto value = stod(tokens[0]);
      bigrams[key] = value;
    }
}
