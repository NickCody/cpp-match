#include <iostream>

#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"

using namespace caf;

struct Foo {
  int bar;

  Foo() { bar = 0; }

  Foo(const Foo& foo) { bar = foo.bar }

  template <class Inspector> bool inspect(Inspector& f, Foo& x) { return f.object(x).fields(f.field("bar", x.bar)); };
};

behavior mirror(event_based_actor* self) {
  return {
    [=](const Foo& foo) { std::cout << foo.bar << std::endl; },
  };
}

void begin(event_based_actor* self, const actor& other) {
  Foo foo{ 1 };
  self->request(other, std::chrono::seconds(10), foo);
}

void caf_main(actor_system& sys) {
  auto mirror_actor = sys.spawn(mirror);
  sys.spawn(mirror);
}

CAF_MAIN()