#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <unordered_map>


using namespace std;
int min(int x, int y, int z)
{
    return min(min(x, y), z);
}
 
int editDist(string str1, string str2)
{
	int m = str1.size();
	int n = str2.size();
    int dp[m+1][n+1];
 
    for (int i=0; i<=m; i++)
    {
        for (int j=0; j<=n; j++)
        {
            if (i==0)
                dp[i][j] = j;
            else if (j==0)
                dp[i][j] = i;
            else if (str1[i-1] == str2[j-1])
                dp[i][j] = dp[i-1][j-1];
            else
                dp[i][j] = 1 + min(dp[i][j-1],  // Insert
                                   dp[i-1][j],  // Remove
                                   dp[i-1][j-1]); // Replace
        }
    }
 
    return dp[m][n];
}

unordered_map<int, vector<string>> loadDict() {
    unordered_map< int, vector<string>> dict;
    std::ifstream file("dic_lower.txt");
    std::string str; 
    while (std::getline(file, str))
    {
      dict[str.size()].push_back(str);
    }
	return dict;
}

vector<string> getCandidates(unordered_map<int, vector<string>>& dict, string word, int dist) {
	vector<string> candidates;
	int size = word.size();
	int lower = size - dist;
	int upper = size + dist;
    for (int i = lower; i <= upper ; i++)		
    {
      auto words = dict[i];
      for (int j = 0; j < words.size(); j++) {
        if (editDist(word, words[j]) <= dist) {
          candidates.push_back(words[j]);
        }
      }
    }
    return candidates;
}

int main() 
{ 
  string word = "chollenge";
  auto dict = loadDict();
  auto candidates = getCandidates(dict, word, 3);
  for (auto w : candidates)
    cout << w <<endl;
}
