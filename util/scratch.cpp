#include "include/fmt/format.h"
#include "include/fmt/printf.h"

#include <memory>

using namespace std;

struct A {
  virtual void init() { fmt::printf("%s::init()\n", "A"); }
};

struct B : public A {
  virtual void init() { fmt::printf("%s::init()\n", "B"); }
};

int main(int /*argc*/, char** /*argv*/) {

  unique_ptr<A> a = make_unique<B>();
  a->init();

  return 0;
}