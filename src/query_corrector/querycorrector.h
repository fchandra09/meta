#ifndef QUERYCORRECTOR_H
#define QUERYCORRECTOR_H

#include <utility>
#include <vector>

using namespace std;

class QueryCorrector
{
  public:
    QueryCorrector();
    pair<double, double> discriminativeTraining(int training_size, int iterations);
    vector<pair<string, vector<double>>> generateCorrections(string query);
    void setParam(double lambdaVal, double muVal);
  private:
   double lambda = 0.5;
   double mu = 1.0;

   double computeScore(string word1, string word2, double emission);
   double compScore(double transition, double emission);
   void runTestData();
};
#endif
    
