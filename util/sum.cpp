#include <iostream>
#include <cstdlib>
#include <vector>
#include <time.h>
#include <utility>
#include <numeric>

using namespace std;

void print_seq(const string& message, vector<int>& seq) {
  cout << message << " ";
  for (auto i : seq) {
    cout << i << " ";
  }
  cout << endl;
}

int sum(vector<int>& seq) {
  return accumulate(seq.begin(), seq.end(), 0);
}

vector<int> max_ascending_sum(vector<int>& seq) {
  vector<int> largest_seq;
  vector<int> local_seq;

  int prev = -1;

  for (size_t i = 0; i < seq.size(); i++) {
    if (seq[i] > prev) {
      local_seq.push_back(seq[i]);
    } else {
      local_seq.clear();
      local_seq.push_back(seq[i]);
    }
    if (sum(local_seq) > sum(largest_seq)) {
      largest_seq = local_seq;
    }
    prev = seq[i];
  }

  return largest_seq;
}

int main() {
  // uncomment to randomize
  // srand(time(NULL));

  vector<int> seq;
  for (size_t i = 0; i < 100; i++) {
    seq.push_back(rand() % 100);
  }

  print_seq("Initial seq: ", seq);

  vector<int> largest_seq = max_ascending_sum(seq);

  print_seq("Subsequence with largest sum: ", largest_seq);

  int final_sum = accumulate(largest_seq.begin(), largest_seq.end(), 0);
  cout << "final_sum = " << final_sum << endl;
}