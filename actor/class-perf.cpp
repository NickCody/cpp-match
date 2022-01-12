#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>

#include "include/fmt/format.h"

#include "caf/all.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"

#include "common/stopwatch.h"

constexpr uint64_t ITERATIONS = 10000000;

using namespace caf;
using namespace std;

// =-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Actor setup
//
struct Foo;

CAF_BEGIN_TYPE_ID_BLOCK(custom_types_1, first_custom_type_id)
CAF_ADD_TYPE_ID(custom_types_1, (Foo))
CAF_END_TYPE_ID_BLOCK(custom_types_1)

struct Foo {
  int bar;
};

template <class Inspector> bool inspect(Inspector& f, Foo& x) {
  return f.object(x).fields(f.field("bar", x.bar));
}

uint64_t posh_count = 0;

behavior posh(event_based_actor* /*self*/) {
  return {
    [=](const Foo& val) { posh_count += val.bar; },
  };
}

void posh_block(blocking_actor* self) {
  bool running = true;
  self->receive_while(running)([&](const Foo& val) { posh_count += val.bar; },
                               [&](exit_msg& em) {
                                 if (em.reason) {
                                   self->fail_state(std::move(em.reason));
                                   running = false;
                                 }
                               });
}

// =-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// par_func setup
//

mutex g_mutex;
queue<uint64_t> g_queue;
atomic<bool> par_continue = true;

void par_func() {

  while (par_continue.load()) {
    std::lock_guard<std::mutex> guard(g_mutex);
    while (!g_queue.empty()) {
      g_queue.pop();
    }
  }
}

void time_actor(actor_system& sys) {
  common::Stopwatch<uint64_t, std::chrono::nanoseconds> watch;

  watch.print_duration(fmt::format("actor       {} iterations (nanos): ", ITERATIONS), [&] {
    Foo f{ 1 };
    auto posh_actor = sys.spawn(posh);
    scoped_actor self{ sys };

    for (uint64_t i = 0; i < ITERATIONS; i++) {
      self->send(posh_actor, f);
    }
  });
}

void time_actor_block(actor_system& sys) {
  common::Stopwatch<uint64_t, std::chrono::nanoseconds> watch;

  watch.print_duration(fmt::format("actor_block {} iterations (nanos): ", ITERATIONS), [&] {
    Foo f{ 1 };
    auto posh_actor = sys.spawn(posh_block);
    scoped_actor self{ sys };

    for (uint64_t i = 0; i < ITERATIONS; i++) {
      self->send(posh_actor, f);
    }

    self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
  });
}

void time_func() {
  common::Stopwatch<uint64_t, std::chrono::nanoseconds> watch;
  uint64_t func_count = 0;

  auto func = [&func_count](int inc) { func_count += inc; };

  watch.print_duration(fmt::format("func        {} iterations (nanos): ", ITERATIONS), [&] {
    for (uint64_t i = 0; i < ITERATIONS; i++) {
      func(1);
    }
  });
}

void time_par_func() {
  common::Stopwatch<uint64_t, std::chrono::nanoseconds> watch;

  std::thread thread_par_func(par_func);

  watch.print_duration(fmt::format("par_func    {} iterations (nanos): ", ITERATIONS), [&] {
    for (uint64_t i = 0; i < ITERATIONS; i++) {
      std::lock_guard<std::mutex> guard(g_mutex);
      g_queue.push(i);
    }
    par_continue = false;
    thread_par_func.join();
  });
}

void caf_main(actor_system& sys) {

  time_func();
  time_actor(sys);
  time_actor_block(sys);
  time_par_func();
}

CAF_MAIN(id_block::custom_types_1)