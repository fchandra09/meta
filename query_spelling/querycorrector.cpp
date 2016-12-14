#include "querycorrector.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <queue>
#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// #define LAMBDA 0.5
// #define MU 1
#define K 5
#define MERGE_EM 200
#define SPLIT_EM 120
#define NULL_TRANS 0.8
#define DEN 500
#define EDITDIST 2
#define SAMEWORD 200

//double lambda = 0.5;
//double mu = 1.0;

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

double QueryCorrector::computeScore(string word1, string word2, double emission) {
  auto transition = getTransProb(word1, word2);
  return lambda * transition + mu * emission;
}

double QueryCorrector::compScore(double transition, double emission) {
  return lambda * transition + mu * emission;
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

void loadDict() {
  ifstream file("dict_lower_new.txt");
  string str;
  while (getline(file, str))
  {
    Dict.insert(str);
  }
  file.close();
  for (auto word : Dict) {
    Dict_len[word.length()].push_back(word);
  }
}

vector<pair<string, float>> getCandidates(string word, int dist) {
  vector<pair<string, float>> candidates;
  for (auto c : word) {
    if (!isalpha(c)) return candidates;
  }
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
  //cout << "here" << endl;
  for (int i = 1; i < word.length(); ++i) {
    auto left = word.substr(0, i);
    auto right = word.substr(i);
    //cout << left + ',' << right << endl;
    if (Dict.find(left) != Dict.end() && Dict.find(right) != Dict.end()) {
      res.emplace_back(left, right);
    }
  }
  return res;
}

vector<pair<string, vector<double>>> QueryCorrector::generateCorrections(string query) {
  stringstream qs(query);
  string buf;
  vector<string> que;
  while (qs >> buf) que.push_back(toLowerCase(buf));

  vector<pair<vector<string>, vector<double>>> state_seqs;
  state_seqs.push_back(make_pair(vector<string>(1, " "), vector<double>(3, 0.0)));

  auto comp = [](pair<int, double> a, pair<int, double> b) {
    return a.second < b.second;
  };

  vector<pair<string, vector<double>>> res;

  for (int i = 0; i < que.size(); ++i) {
    auto candidates = getCandidates(que[i], EDITDIST);
    auto split_candidates = getSplit(que[i]);

    vector<pair<vector<string>, vector<double>>> tmp_seqs;
    vector<pair<vector<string>, vector<double>>> tmp_seqs_null;
    priority_queue<pair<int, double>, vector<pair<int, double>>, decltype(comp)> pq(comp);

    for (auto seq : state_seqs) {
      auto tmp = seq;
      auto current_word = tmp.first.back();

      // Merge, merge the current word with the previous one
      if (i > 0 && current_word == " ") {
        string merge = que[i - 1] + que[i];
        if (Dict.find(merge) == Dict.end()) continue;
        current_word = tmp.first[tmp.first.size() - 2];
        auto next_word = merge;
        auto trans_score = getTransProb(current_word, next_word);
        auto emi_score = MERGE_EM;
        auto score = compScore(trans_score, emi_score);
        tmp.first.push_back(next_word);
        score += tmp.second[0];
        tmp.second[0] = score;
        tmp.second[1] += trans_score;
        tmp.second[2] += emi_score;
        tmp_seqs.push_back(tmp);
        pq.emplace(tmp_seqs.size() - 1, score);
      }
      else {
        // In-word transformation, insert, delete, replace
        for (auto candidate : candidates) {
          tmp = seq;
          auto next_word = candidate.first;

          auto trans_score = getTransProb(current_word, next_word);
          auto emi_score = candidate.second;
          auto score = compScore(trans_score, emi_score);

          tmp.first.push_back(next_word);
          score += tmp.second[0];
          tmp.second[0] = score;
          tmp.second[1] += trans_score;
          tmp.second[2] += emi_score;

          tmp_seqs.push_back(tmp);
          pq.emplace(tmp_seqs.size() - 1, score);
        }
        // float penalty = Dict.find(que[i]) != Dict.end() ? 5 : 1; // if the query word is correct, reduce it's chance to be split
        // Split
        for (auto split_cand : split_candidates) {
          tmp = seq;
          current_word = split_cand.first;
          auto next_word = split_cand.second;

          auto trans_score = getTransProb(current_word, next_word);
          auto emi_score = SPLIT_EM;
          auto score = compScore(trans_score, emi_score);

          tmp.first.push_back(current_word);
          tmp.first.push_back(next_word);

          score += tmp.second[0];
          tmp.second[0] = score;
          tmp.second[1] += trans_score;
          tmp.second[2] += emi_score;

          tmp_seqs.push_back(tmp);
          pq.emplace(tmp_seqs.size() - 1, score);
        }
        if (candidates.empty()) {
          tmp = seq;
          tmp.first.push_back(que[i]);
          tmp_seqs.push_back(tmp);
          pq.emplace(tmp_seqs.size() - 1, 1 + tmp.second[0]);
        }
        // Add a null state
        if (i < que.size() - 1) {
          tmp = seq;
          tmp.first.push_back(" ");
          tmp_seqs_null.push_back(tmp);
        }
      }
    }
    state_seqs.clear();

    int limit = K < pq.size() ? K : pq.size();
    auto max_num = pq.top().second;
    for (int j = 0; j < limit; ++j) {
      auto tmp_seq = tmp_seqs[pq.top().first];
      if (i < que.size() - 1) {
        state_seqs.push_back(tmp_seq);
      }
      else {
        string tmp = "";
        for (auto state : tmp_seq.first) {
          if (state != " ") {
            tmp += state;
            tmp += ' ';
          }
        }
        if (!tmp.empty()) tmp.pop_back();
        res.emplace_back(tmp, tmp_seq.second);
        // res.emplace_back(tmp, pq.top().second / max_num);
      }
      pq.pop();
    }
    for (auto seq : tmp_seqs_null) {
      state_seqs.push_back(seq);
    }
  }
  return res;
}

void QueryCorrector::runTestData()
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
    bool precision_done = false;

    //cout << str << endl;

    for (int i = 0; i < candidates.size(); i++)
    {
      auto candidate = candidates[i];
      auto correction = candidate.first;

      //cout << "Candidate: " << correction << endl;

      for (int i = 1; i < queries.size(); i++)
      {
        if (correction == queries[i])
        {
          if (!precision_done) {
            precision_sum += 1;
            precision_done = true;
          }
          candidate_match_count++;
          break;
        }
      }
    }

    recall_sum += (candidate_match_count * 1.0 / correction_truth_count);

    //cout << "Precision sum: " << precision_sum << endl;
    //cout << "Recall sum: " << recall_sum << endl;
    //cout << "*****************************************************" << endl;
  }
  file.close();

  float precision = precision_sum / test_data_count;
  float recall = recall_sum / test_data_count;

  //cout << "Test data count: " << test_data_count << endl;
  cout << "Precision: " << precision << endl;
  cout << "Recall: " << recall << endl;
}


// Param: Training set size
// Param: Iteration limit
// Return lamda, mu
pair<double, double> QueryCorrector::discriminativeTraining(int training_size, int iterations) {
  if (training_size < 1 || iterations < 1) return pair<double, double> ();
  double lambda_ini = 0.5;
  double mu_ini = 0.5;
  lambda = lambda_ini;
  mu = mu_ini;
  cout << "Initial lambda = " << lambda << endl;
  cout << "Initial mu = " << mu << endl;

  // read query and labeled answer
  ifstream file("training_data.txt");
  string line;
  int count = 0;
  double delta_lambda = 0.0;
  double delta_mu = 0.0;

  int it = 0;
  while (it < iterations) {

    while (getline(file, line)) {

      if (count == training_size) break;
      ++count;
      auto que_ans = split(line, '\t');
      auto query = que_ans[0];
      auto answer = que_ans[1];

      auto corrections = generateCorrections(query);
      auto first_correction = corrections[0].first;

      if (first_correction != answer) {
          double trans_score_training = 0;
          double trans_score_correction = 0;
          auto query_vec = split(query, ' ');
          auto training_vec = split(answer, ' ');
          auto correction_vec = split(first_correction, ' ');
          int i = 1;
          for (; i < training_vec.size(); ++i) {
            trans_score_training += getTransProb(training_vec[i - 1], training_vec[i]);
          }
          trans_score_training /= i;
          trans_score_correction = corrections[0].second[1] / i;
          delta_lambda = trans_score_correction - trans_score_correction;

          double emi_score_training = 0;
          double emi_score_correction = 0;
          i = 0;
          while (i < min3(training_vec.size(), correction_vec.size(), query_vec.size())) {
            if (training_vec[i] == query_vec[i]) {
              emi_score_training += SAMEWORD;
            }
            else {
              emi_score_training += editDist(query_vec[i], training_vec[i]).second;
            }
            ++i;
          }
          emi_score_correction = corrections[0].second[2];
          delta_mu += (emi_score_training - emi_score_correction) / (i + 0.01);
      }
    }
    file.close();
    lambda += delta_lambda / count;
    mu += delta_mu / count;
    cout << "Lambda = " << lambda << endl;
    cout << "Mu = " << mu << endl;
    ++it;
  }
  return make_pair(lambda, mu);
}


QueryCorrector::QueryCorrector() {
  cout << "loading bigrams and dictionaries. Please wait..." << endl;
  Bigrams = loadBigram();

  loadWed("wed_del.txt", 0);
  loadWed("wed_ins.txt", 1);
  loadWed("wed_rep.txt", 2);

  loadDict();
}
