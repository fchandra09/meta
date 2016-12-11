#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <utility>
#include <queue>
#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define LAMBDA 0.5
#define MU 1
#define K 5
#define MERGE_EM 320
#define SPLIT_EM 120
#define NULL_TRANS 0.8
#define DEN 500
#define EDITDIST 2
#define SAMEWORD 200

vector<vector<int> > Wed_del(26, vector<int>(26, -1));
vector<vector<int> > Wed_ins(26, vector<int>(26, -1));
vector<vector<int> > Wed_rep(26, vector<int>(26, -1));

unordered_map<string, double> Bigrams;
unordered_set<string> Dict;
unordered_map<int, vector<string>> Dict_len;

int getWed(char a, char b, int type) {
  int y = a - 'a', x = b - 'a';
  if (0 > y || y >= 26 || 0 > x || x >= 26) return 0;
  if (type == 0) return Wed_del[y][x];
  if (type == 1) return Wed_ins[y][x];
  if (type == 2) return Wed_rep[y][x];
  return 0;
}

// unordered_map<string, int>
using namespace std;
int min3(int x, int y, int z)
{
  return min(min(x, y), z);
}

int stoint(string s) {
  int res = 0;
  for (auto c : s) {
    if (isdigit(c)) {
      res += res * 10 + (c - '0');
    }
  }
  return res;
}

vector<string> split(const string &s, char delim) {
  stringstream ss(s);
  string item;
  vector<string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

string toLowerCase(string a) {
  for (int i = 0; i < a.length(); ++i) {
    if (isalpha(a[i])) {
      a[i] = tolower(a[i]);
    }
  }
  return a;
}

double getTransProb(string a, string b) {
  if (a == " ") return NULL_TRANS;
  string key = a + ',' + b;
  if (Bigrams.find(key) != Bigrams.end()) {
    return Bigrams[key];
  }
  return 0.0;
}

double computeScore(string word1, string word2, double emission) {
  auto transition = getTransProb(word1, word2);
  return LAMBDA * transition + MU * emission;
}

pair<int, float> editDist(string str1, string str2) {
  int m = str1.size();
  int n = str2.size();
  vector<vector<pair<int, float>>> dp(m + 1, vector<pair<int, float>>(n + 1, make_pair(0, 0)));
  // int dp[m+1][n+1];
  for (int i=0; i<=m; i++) {
    for (int j=0; j<=n; j++) {
      if (i==0) {
        dp[i][j].first = j;
      }
      else if (j==0) {
        dp[i][j].first = i;
      }
      else if (str1[i-1] == str2[j-1]) {
        dp[i][j] = dp[i-1][j-1];
      }
      else {
        int min_val = min3(dp[i][j - 1].first,
                          dp[i - 1][j].first,
                          dp[i - 1][j - 1].first);
        dp[i][j].first = min_val + 1;
        if (!isalpha(str1[i - 1])) {
            continue;
        }
        if (dp[i][j - 1].first == min_val) { // Insert
          dp[i][j].second = dp[i][j - 1].second + getWed(str1[i - 1], str2[j - 1], 1);
        }
        else if (dp[i - 1][j].first == min_val) { // Remove
          dp[i][j].second = dp[i - 1][j].second + getWed(str2[j - 1], str1[i - 1], 0);
        }
        else { // Replace
          dp[i][j].second = dp[i - 1][j - 1].second + getWed(str1[i - 1], str2[j - 1], 2);
        }
      }
    }
  }
  return dp[m][n];

}

unordered_map<string, double> loadBigram() {
  unordered_map<string, double> bigrams;
  ifstream file("bigrams_data_new.txt");
  string str;
  while (getline(file, str))
  {
    auto tokens = split(str, ',');
    auto key = tokens[1] + ',' + tokens[2];
    auto value = stod(tokens[0]);
    bigrams[key] = value;
  }
  file.close();
  return bigrams;
}

void loadWed(string file_name, int type) {
  ifstream file(file_name);
  string str;
  int i = 0;
  while (getline(file, str))
  {
    stringstream ss(str);
    string tmp;
    int j = 0;
    while (ss >> tmp) {
      int num = stoint(tmp);
      switch (type) {
        case 0: Wed_del[i][j] = num; break;
        case 1: Wed_ins[i][j] = num; break;
        case 2: Wed_rep[i][j] = num; break;
        default: break;
      }
      ++j;
    }
    ++i;
  }
  file.close();
}

unordered_set<string> loadDict() {
  unordered_set<string> dict;
  ifstream file("dict_lower_new.txt");
  string str;
  while (getline(file, str))
  {
    dict.insert(str);
  }
  file.close();
  return dict;
}

unordered_map<int, vector<string>> loadDictByLen() {
  unordered_map< int, vector<string>> dict;
  std::ifstream file("dict_lower_new.txt");
  std::string str;
  while (std::getline(file, str))
  {
    dict[str.size()].push_back(str);
  }
  file.close();
  return dict;
}

vector<pair<string, float>> getCandidates(string word, int dist) {
  vector<pair<string, float>> candidates;
  int size = word.size();
  int lower = size - dist;
  int upper = size + dist;
  for (int i = lower; i <= upper; i++) {
    auto words = Dict_len[i];
    for (auto w : words) {
      if (w == word) {
        candidates.emplace_back(w, SAMEWORD);
      }
      else {
        auto tmp = editDist(word, w);
        if (tmp.first <= dist) {
          float bonus = dist - tmp.first;
          float emission = tmp.second / DEN + bonus;
          candidates.emplace_back(w, emission);
          // cout << w << ' ' << tmp.first << ' ' << emission << endl;
        }
      }
    }
  }
  return candidates;
}

vector<pair<string, string>> getSplit(string word) {
  if (word.size() < 2 || Dict.find(word) != Dict.end()) return vector<pair<string, string>>();
  vector<pair<string, string>> res;
  for (int i = 1; i < word.length(); ++i) {
    auto left = word.substr(0, i);
    auto right = word.substr(i);
    if (Dict.find(left) != Dict.end() && Dict.find(right) != Dict.end()) {
      res.emplace_back(left, right);
    }
  }
  return res;
}

vector<pair<string, double>> generateCorrections(string query) {
  stringstream qs(query);
  string buf;
  vector<string> que;
  while (qs >> buf) que.push_back(toLowerCase(buf));

  // cout << "Query length = " << que.size() << endl;
  vector<vector<string>> state_seqs;
  state_seqs.push_back(vector<string>(1, " "));

  auto comp = [](pair<int, double> a, pair<int, double> b) {
    return a.second < b.second;
  };

  vector<pair<string, double>> res;

  for (int i = 0; i < que.size(); ++i) {
    auto candidates = getCandidates(que[i], EDITDIST);
    auto split_candidates = getSplit(que[i]);

    vector<vector<string>> tmp_seqs;
    vector<vector<string>> tmp_seqs_null;
    // vector<pair<int, double>> idx_score;
    priority_queue<pair<int, double>, vector<pair<int, double>>, decltype(comp)> pq(comp);

    for (auto seq : state_seqs) {
      auto tmp = seq;
      auto current_word = tmp.back();

      // Merge, merge the current word with the previous one
      if (i > 0 && current_word == " ") {
        string merge = que[i - 1] + que[i];
        if (Dict.find(merge) == Dict.end()) continue;
        current_word = tmp[tmp.size() - 2];
        auto next_word = merge;
        auto score = computeScore(current_word, next_word, MERGE_EM);
        tmp.push_back(next_word);
        tmp_seqs.push_back(tmp);
        // idx_score.emplace_back(tmp_seqs.size() - 1, score);
        pq.emplace(tmp_seqs.size() - 1, score);
      }
      else {

        // In-word transformation, insert, delete, replace
        for (auto candidate : candidates) {
          tmp = seq;
          auto next_word = candidate.first;
          auto score = computeScore(current_word, next_word, candidate.second);
          tmp.push_back(next_word);
          tmp_seqs.push_back(tmp);
          // idx_score.emplace_back(tmp_seqs.size() - 1, score);
          pq.emplace(tmp_seqs.size() - 1, score);
        }

        // float penalty = Dict.find(que[i]) != Dict.end() ? 5 : 1; // if the query word is correct, reduce it's chance to be split
        // Split
        for (auto split_cand : split_candidates) {
          tmp = seq;
          current_word = split_cand.first;
          auto next_word = split_cand.second;
          auto score = computeScore(current_word, next_word, SPLIT_EM);
          tmp.push_back(current_word);
          tmp.push_back(next_word);
          tmp_seqs.push_back(tmp);
          // idx_score.emplace_back(tmp_seqs.size() - 1, score);
          pq.emplace(tmp_seqs.size() - 1, score);
        }

        if (candidates.empty()) {
          tmp = seq;
          tmp.push_back(que[i]);
          tmp_seqs.push_back(tmp);
          pq.emplace(tmp_seqs.size() - 1, 1);
        }

        // Add a null state
        if (i < que.size() - 1) {
          tmp = seq;
          tmp.push_back(" ");
          tmp_seqs_null.push_back(tmp);
        }
      }
    }
    state_seqs.clear();
    // sort(idx_score.begin(), idx_score.end(), comp);
    // int limit = K < idx_score.size()? K : idx_score.size();
    int limit = K < pq.size() ? K : pq.size();
    auto max_num = pq.top().second;
    for (int j = 0; j < limit; ++j) {
      // state_seqs.push_back(tmp_seqs[idx_score[j].first]);
      auto tmp_seq = tmp_seqs[pq.top().first];
      if (i < que.size() - 1) {
        state_seqs.push_back(tmp_seq);
      }
      else {
        string tmp = "";
        for (auto state : tmp_seq) {
          if (state != " ") {
            tmp += state + ' ';
          }
        }
        if (!tmp.empty()) tmp.pop_back();
        res.emplace_back(tmp, pq.top().second / max_num);
      }
      pq.pop();
    }
    for (auto seq : tmp_seqs_null) {
      state_seqs.push_back(seq);
    }
  }
  return res;
}

void runTestData()
{
  std::ifstream file("test_data.txt");
  std::string str;
  float precision_sum = 0;
  float recall_sum = 0;
  int test_data_count = 0;

  while (std::getline(file, str))
  {
    test_data_count++;

    auto queries = split(str, '\t');
    auto original_query = queries[0];
    auto correction_truth_count = queries.size() - 1;
    auto candidates = generateCorrections(original_query);
    int candidate_match_count = 0;

    for (auto candidate : candidates)
    {
      auto correction = candidate.first;
      auto score = candidate.second;

      for (int i = 1; i < queries.size(); i++)
      {
        if (correction == queries[i])
        {
          precision_sum += score;
          candidate_match_count++;
          break;
        }
      }
    }

    recall_sum += (candidate_match_count * 1.0 / correction_truth_count);
  }

  float precision = precision_sum / test_data_count;
  float recall = recall_sum / test_data_count;

  cout << "Precision: " << precision << endl;
  cout << "Recall: " << recall << endl;
}

int main()
{
  Bigrams = loadBigram();

  loadWed("wed_del.txt", 0);
  loadWed("wed_ins.txt", 1);
  loadWed("wed_rep.txt", 2);

  Dict_len = loadDictByLen();
  Dict = loadDict();

  //runTestData();

  while (1) {
    cout << "Enter a query:" << endl;
    string query;
    getline(cin, query);
    auto res = generateCorrections(query);
    cout << "Suggested corrections:" << endl;
    for (auto line : res) {
      cout << line.first << ' ' << line.second << endl;
    }
  }
}
