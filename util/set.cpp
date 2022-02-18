#include <set>
#include <fmt/format.h>
#include <chrono>
#include <thread>
#include <vector>

#include "common/common.h"

using namespace std;
using namespace std::chrono;

class Foo {
public:
  Foo(int n)
      : num(n) {
  }

  int num;
};

bool operator<(const Foo& lhs, const Foo& rhs) {
  return lhs.num < rhs.num;
}

template <typename Proc> chrono::nanoseconds benchmark(Proc fn) {
  auto start = system_clock::now();
  fn();
  return duration_cast<nanoseconds>(system_clock::now() - start);
}

int main() {

  // {
  //   auto result = benchmark([]() {
  //     set<Foo> myset;
  //     myset.insert(Foo{ 1 });
  //     myset.insert(Foo{ 4 });
  //     myset.insert(Foo{ 5 });
  //     myset.insert(Foo{ 0 });
  //     myset.insert(Foo{ 6 });
  //     myset.insert(Foo{ 2 });
  //     myset.insert(Foo{ 3 });

  //     auto it = myset.find(4);
  //     for (; it != myset.end(); it++) {
  //       fmt::print("{}\n", it->num);
  //     }
  //   });

  //   fmt::print("Simple inserts: {} ns\n", result.count());
  // }

  int MAX_ID = 100'000'000;

  set<Foo> myset;

  {
    auto result = benchmark([&]() {
      for (int i = 0; i < MAX_ID; i++) {
        myset.insert(Foo{ i });
      }
    });
    fmt::print("                      set inserts for {:>15} items: {:>15} ns\n",
               common::numberFormatWithCommas(MAX_ID),
               common::numberFormatWithCommas(result.count()));
  }

  vector<Foo> myvec;
  {
    auto result = benchmark([&]() {
      for (int i = 0; i < MAX_ID; i++) {
        myvec.push_back(Foo{ i });
      }
    });
    fmt::print("                   vector inserts for {:>15} items: {:>15} ns\n",
               common::numberFormatWithCommas(MAX_ID),
               common::numberFormatWithCommas(result.count()));
  }

  {
    auto result = benchmark([&]() {
      Foo key(MAX_ID / 2);
      auto it = myset.find(MAX_ID / 2);
      int sum = 0;
      int count = 0;
      for (; it != myset.end(); it++) {
        count++;
        sum += it->num;
      }
      // fmt::print("sum {}, count {}\n", sum, count);
    });

    fmt::print("     set seek from {:>15} to {:>15} items: {:>15} ns\n",
               common::numberFormatWithCommas(MAX_ID / 2),
               common::numberFormatWithCommas(MAX_ID),
               common::numberFormatWithCommas(result.count()));
  }

  {
    auto result = benchmark([&]() {
      Foo key(MAX_ID / 2);
      auto it = find_if(myvec.begin(), myvec.end(), [&key](const auto& e) { return e.num == key.num; });
      int sum = 0;
      int count = 0;
      for (; it != myvec.end(); it++) {
        count++;
        sum += it->num;
      }
      // fmt::print("sum {}, count {}\n", sum, count);
    });

    fmt::print("  vector seek from {:>15} to {:>15} items: {:>15} ns\n",
               common::numberFormatWithCommas(MAX_ID / 2),
               common::numberFormatWithCommas(MAX_ID),
               common::numberFormatWithCommas(result.count()));
  }

  return 0;
}