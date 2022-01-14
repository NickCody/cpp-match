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
constexpr auto RESULT_WIDTH = 14;
constexpr auto NANOS_PER_MICRO = 1000;

std::atomic<uint64_t> g_actor_work_count = 0;
std::atomic<uint64_t> g_actor_x_work_count = 0;
std::atomic<uint64_t> g_actor_pool_work_count = 0;
uint64_t g_par_work_count = 0;
std::atomic<uint64_t> g_func_work_count = 0;

uint64_t g_actor_send_count = 0;
uint64_t g_actor_x_send_count = 0;
uint64_t g_actor_pool_send_count = 0;
uint64_t g_par_send_count = 0;
uint64_t g_func_send_count = 0;

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
    [&](const Foo& /*val*/) {
      g_actor_work_count++;
      busy_spin(BUSY_SPIN_MILLIS);
    },
  };
}

behavior posh_x(event_based_actor* /*self*/) {
  return {
    [&](const Foo& /*val*/) {
      g_actor_x_work_count++;
      busy_spin(BUSY_SPIN_MILLIS);
    },
  };
}

behavior posh_pool(event_based_actor* /*self*/) {
  return {
    [&](const Foo& /*val*/) {
      g_actor_pool_work_count++;
      busy_spin(BUSY_SPIN_MILLIS);
    },
  };
}

mutex g_mutex;
queue<uint64_t> g_queue;
atomic<bool> par_continue = true;

void par_func(int max_to_read) {
  while (par_continue.load()) {
    std::lock_guard<std::mutex> guard(g_mutex);
    int num_read = 0;
    while (!g_queue.empty() && (max_to_read == -1 || max_to_read < num_read)) {
      g_queue.pop();
      g_par_work_count++;
      num_read++;
      busy_spin(BUSY_SPIN_MILLIS);
    }
  }
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor(actor_system& sys, uint64_t iterations, std::chrono::microseconds work_send_delay) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_actor_send_count = 0;
  g_actor_work_count = 0;

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "single actor", iterations), RESULT_WIDTH, [&] {
    Foo f{ 1 };
    auto posh_actor = sys.spawn(posh);
    scoped_actor self{ sys };

    for (uint64_t i = 0; i < iterations; i++) {
      self->send(posh_actor, f);
      g_actor_send_count++;
      std::this_thread::sleep_for(work_send_delay);
    }
    self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
    self->await_all_other_actors_done();
  });
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor_x
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor_x(actor_system& sys, uint64_t iterations, std::chrono::microseconds work_send_delay) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_actor_x_send_count = 0;
  g_actor_x_work_count = 0;
  scoped_actor self{ sys };

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "actor per work item", iterations), RESULT_WIDTH, [&] {
    Foo f{ 1 };

    for (uint64_t i = 0; i < iterations; i++) {
      auto posh_actor = sys.spawn(posh);
      self->send(posh_actor, f);
      g_actor_x_send_count++;
      self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
      std::this_thread::sleep_for(work_send_delay);
    }
    self->await_all_other_actors_done();
  });
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor_x
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor_pool(actor_system& sys, uint64_t iterations, size_t pool_size, std::chrono::microseconds work_send_delay) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  scoped_actor self{ sys };
  scoped_execution_unit ctx{ &sys };
  g_actor_pool_send_count = 0;
  g_actor_pool_work_count = 0;

  auto pool = actor_pool::make(
      &ctx, pool_size, [&] { return sys.spawn(posh); }, actor_pool::round_robin());

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "actor pool", iterations), RESULT_WIDTH, [&] {
    Foo f{ 1 };

    for (uint64_t i = 0; i < iterations; i++) {
      g_actor_pool_send_count++;
      self->send(pool, f);
      std::this_thread::sleep_for(work_send_delay);
    }
    self->send_exit(pool, exit_reason::user_shutdown);
    self->await_all_other_actors_done();
  });
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_func
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_func(uint64_t iterations, std::chrono::microseconds work_send_delay) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_func_send_count = 0;
  g_func_work_count = 0;

  auto func = [&](int inc) {
    g_func_send_count += inc;
    busy_spin(BUSY_SPIN_MILLIS);
  };

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "raw function call", iterations), RESULT_WIDTH, [&] {
    for (uint64_t i = 0; i < iterations; i++) {
      func(1);
      std::this_thread::sleep_for(work_send_delay);
    }
  });
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_par_func
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_par_func(uint64_t iterations, uint64_t num_threads, std::chrono::microseconds work_send_delay) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  par_continue.store(true);
  int num_to_read = num_threads == 1 ? -1 : 1;
  g_par_send_count = 0;
  g_par_work_count = 0;

  std::vector<std::thread> thread_par_func;

  for (uint64_t i = 0; i < num_threads; i++) {
    thread_par_func.push_back(std::thread(par_func, num_to_read));
  }

  watch.print_duration(fmt::format("{:>25}: {} iterations (micros): ", "parallel thread", iterations), RESULT_WIDTH, [&] {
    for (uint64_t i = 0; i < iterations; i++) {
      {
        std::lock_guard<std::mutex> guard(g_mutex);
        g_queue.push(1);
      }
      g_par_send_count++;
      std::this_thread::sleep_for(work_send_delay);
    }
    par_continue.store(false);
    for (std::thread& thread : thread_par_func) {
      if (thread.joinable())
        thread.join();
    }
  });
}

void print_counts() {
  // fmt::print(" {:>30} = {:>15}\n", "g_func_work_count", common::numberFormatWithCommas(g_func_work_count.load()));
  // fmt::print(" {:>30} = {:>15}\n", "g_func_send_count", common::numberFormatWithCommas(g_func_send_count));

  fmt::print(" {:>30} = {:>15}\n", "g_par_work_count", common::numberFormatWithCommas(g_par_work_count));
  fmt::print(" {:>30} = {:>15}\n", "g_par_send_count", common::numberFormatWithCommas(g_par_send_count));

  // fmt::print(" {:>30} = {:15}\n", "g_actor_work_count", common::numberFormatWithCommas(g_actor_work_count.load()));
  // fmt::print(" {:>30} = {:>15}\n", "g_actor_send_count", common::numberFormatWithCommas(g_actor_send_count));

  fmt::print(" {:>30} = {:>15}\n", "g_actor_x_work_count", common::numberFormatWithCommas(g_actor_x_work_count.load()));
  fmt::print(" {:>30} = {:>15}\n", "g_actor_x_send_count", common::numberFormatWithCommas(g_actor_x_send_count));

  fmt::print(" {:>30} = {:>15}\n", "g_actor_pool_work_count", common::numberFormatWithCommas(g_actor_pool_work_count.load()));
  fmt::print(" {:>30} = {:>15}\n", "g_actor_pool_send_count", common::numberFormatWithCommas(g_actor_pool_send_count));
}

void harness(actor_system& sys, std::chrono::microseconds busy_spin_micros) {
  BUSY_SPIN_MILLIS = busy_spin_micros;
  int par_threads = get_or(sys.config(), "class-perf.par-threads", 1);
  int actor_threads = get_or(sys.config(), "caf.scheduler.max-threads", 1);
  size_t actor_pool_size = get_or(sys.config(), "class-perf.actor-pool-size", 1);
  uint64_t iterations = get_or(sys.config(), "class-perf.iterations", 100);
  std::chrono::microseconds work_send_delay =
      std::chrono::duration_cast<std::chrono::microseconds>(get_or(sys.config(), "class-perf.work-send-delay", caf::timespan(NANOS_PER_MICRO)));

  fmt::print("Using {} par_threads, {} actor_threads, {} actor_pool_size, {} itetarions, with {} microsecond work per "
             "iteration:\n\n",
             par_threads,
             actor_threads,
             common::numberFormatWithCommas(actor_pool_size),
             common::numberFormatWithCommas(iterations),
             common::numberFormatWithCommas(BUSY_SPIN_MILLIS.count()));
  sys.clock();

  // time_func(iterations,work_send_delay);
  time_par_func(iterations, par_threads, work_send_delay);
  // time_actor(sys, iterations,work_send_delay);
  time_actor_x(sys, iterations, work_send_delay);
  time_actor_pool(sys, iterations, actor_pool_size, work_send_delay);

  print_counts();

  fmt::print("\n");
}

void caf_main(actor_system& sys) {
  std::list<caf::timespan> default_work_per_iteration = { caf::timespan(NANOS_PER_MICRO * 10) };

  std::list<caf::timespan> work_per_iteration = get_or(sys.config(), "class-perf.work-per-iteration", default_work_per_iteration);

  for (auto work : work_per_iteration) {
    harness(sys, std::chrono::duration_cast<std::chrono::microseconds>(work));
  }
}

CAF_MAIN(id_block::custom_types_1)