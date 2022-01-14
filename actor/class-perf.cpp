#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdlib>
#include "include/fmt/format.h"

#include "caf/all.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"

#include "common/stopwatch.h"

auto BUSY_SPIN_MILLIS = std::chrono::microseconds(0);
auto SLEEP_BEFORE_SEND = std::chrono::microseconds(1);
constexpr auto RESULT_WIDTH = 14;

uint64_t posh_count = 0;
uint64_t posh_block_count = 0;
uint64_t par_count = 0;
uint64_t func_count = 0;

using namespace caf;
using namespace std;

// =-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// busy spin

void busy_spin(std::chrono::microseconds spin_time) {
  const auto start = std::chrono::system_clock::now();

  while (true) {
    const auto now = std::chrono::system_clock::now();
    if ((now - start) >= spin_time)
      break;
  }
}

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

behavior posh(event_based_actor* /*self*/) {
  return {
    [=](const Foo& /*val*/) { busy_spin(BUSY_SPIN_MILLIS); },
  };
}

mutex g_mutex;
queue<uint64_t> g_queue;
atomic<bool> par_continue = true;

void par_func() {
  par_continue.store(true);

  // fmt::print("Entering loop: {}\n", par_continue);
  while (par_continue.load()) {
    std::lock_guard<std::mutex> guard(g_mutex);
    while (g_queue.size() != 0) {
      // fmt::print("g_queue.size()={}\n", g_queue.size());
      g_queue.pop();
      busy_spin(BUSY_SPIN_MILLIS);
    }
  }
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_X
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor(actor_system& sys, uint64_t iterations) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  posh_count = 0;

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "single actor", iterations), RESULT_WIDTH, [&] {
    Foo f{ 1 };
    auto posh_actor = sys.spawn(posh);
    scoped_actor self{ sys };

    for (uint64_t i = 0; i < iterations; i++) {
      posh_count++;
      self->send(posh_actor, f);
      std::this_thread::sleep_for(SLEEP_BEFORE_SEND);
    }
    self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
    self->await_all_other_actors_done();
  });
}

void time_actor_x(actor_system& sys, uint64_t iterations) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  posh_count = 0;

  watch.print_duration(
      fmt::format("{:>25}: {} iterations (micros): ", "actor per work item", iterations), RESULT_WIDTH, [&] {
        Foo f{ 1 };
        scoped_actor self{ sys };

        for (uint64_t i = 0; i < iterations; i++) {
          auto posh_actor = sys.spawn(posh);
          posh_count++;
          self->send(posh_actor, f);
          std::this_thread::sleep_for(SLEEP_BEFORE_SEND);
          self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
        }
        self->await_all_other_actors_done();
      });
}

void time_func(uint64_t iterations) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  func_count = 0;

  auto func = [&](int inc) {
    func_count += inc;
    busy_spin(BUSY_SPIN_MILLIS);
  };

  watch.print_duration(
      fmt::format("{:>25}: {} iterations (micros): ", "raw function call", iterations), RESULT_WIDTH, [&] {
        for (uint64_t i = 0; i < iterations; i++) {
          func(1);
          std::this_thread::sleep_for(SLEEP_BEFORE_SEND);
        }
      });
}

void time_par_func(uint64_t iterations) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  std::thread thread_par_func(par_func);

  watch.print_duration(
      fmt::format("{:>25}: {} iterations (micros): ", "parallel thread", iterations), RESULT_WIDTH, [&] {
        for (par_count = 0; par_count < iterations; par_count++) {
          std::lock_guard<std::mutex> guard(g_mutex);
          g_queue.push(par_count);
          std::this_thread::sleep_for(SLEEP_BEFORE_SEND);
        }
        par_continue.store(false);
        thread_par_func.join();
      });
}

void harness(actor_system& sys, uint64_t iterations, std::chrono::microseconds busy_spin_micros) {
  BUSY_SPIN_MILLIS = busy_spin_micros;
  fmt::print("With {} microsecond work per iteration:\n\n", common::numberFormatWithCommas(BUSY_SPIN_MILLIS.count()));
  sys.clock();

  time_func(iterations);
  time_par_func(iterations);
  time_actor(sys, iterations);
  time_actor_x(sys, iterations);
  fmt::print("\n");
}

void caf_main(actor_system& sys) {
  constexpr uint64_t ITERATIONS = 10;
  fmt::print("Using {} iterations\n", common::numberFormatWithCommas(ITERATIONS, 0));

  harness(sys, ITERATIONS, std::chrono::microseconds(0));
  harness(sys, ITERATIONS, std::chrono::microseconds(1000));
  harness(sys, ITERATIONS, std::chrono::microseconds(100000));
  harness(sys, ITERATIONS, std::chrono::microseconds(1000000));
}

CAF_MAIN(id_block::custom_types_1)