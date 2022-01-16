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

auto g_busy_spin_millis = std::chrono::microseconds(0);
constexpr auto RESULT_WIDTH = 14;
constexpr auto NANOS_PER_MICRO = 1000;

uint64_t g_actor_work_count = 0;
uint64_t g_actor_x_work_count = 0;
uint64_t g_actor_pool_work_count = 0;
uint64_t g_par_work_count = 0;
uint64_t g_func_work_count = 0;

uint64_t g_actor_send_count = 0;
uint64_t g_actor_x_send_count = 0;
uint64_t g_actor_pool_send_count = 0;
uint64_t g_par_send_count = 0;
uint64_t g_func_send_count = 0;

using namespace caf;
using namespace std;

mutex g_mutex;
queue<uint64_t> g_queue;
queue<uint64_t> g_count_queue;
atomic<bool> g_par_continue;

struct WorkConfig {
  size_t par_threads;
  size_t actor_threads;
  size_t actor_pool_size;
  size_t iterations;
  std::chrono::microseconds work_send_delay;
  std::list<caf::timespan> work_per_iteration;
  volatile bool random_work;
};

WorkConfig g_work_config;

// =-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// busy spin

void busy_spin(std::chrono::microseconds spin_time) {
  const auto start = std::chrono::system_clock::now();

  std::chrono::microseconds actual_spin_time;
  if (g_work_config.random_work) {
    actual_spin_time = std::chrono::microseconds(rand() % (int)spin_time.count());
  } else {
    actual_spin_time = spin_time;
  }
  while (true) {
    const auto now = std::chrono::system_clock::now();
    if ((now - start) >= actual_spin_time)
      break;
  }
}

// =-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Actor setup
//
struct Foo;
struct Bar;

CAF_BEGIN_TYPE_ID_BLOCK(custom_types_1, first_custom_type_id)
CAF_ADD_TYPE_ID(custom_types_1, (Foo))
CAF_ADD_TYPE_ID(custom_types_1, (Bar))
CAF_END_TYPE_ID_BLOCK(custom_types_1)

struct Foo {
  int bar;
};

struct Bar {
  int foo;
};

template <class Inspector> bool inspect(Inspector& f, Foo& x) {
  return f.object(x).fields(f.field("bar", x.bar));
}

template <class Inspector> bool inspect(Inspector& f, Bar& x) {
  return f.object(x).fields(f.field("foo", x.foo));
}

behavior posh(event_based_actor* self, const actor& count_actor) {
  return {
    [=](const Foo& val) {
      self->send(count_actor, val);
      busy_spin(g_busy_spin_millis);
    },
  };
}

behavior posh_x(event_based_actor* self, const actor& count_actor) {
  return {
    [=](const Foo& val) {
      self->send(count_actor, val);
      busy_spin(g_busy_spin_millis);
    },
  };
}

behavior posh_pool(event_based_actor* self, const actor& count_actor) {
  return {
    [=](const Foo& val) {
      self->send(count_actor, val);
      busy_spin(g_busy_spin_millis);
    },
  };
}

behavior posh_counter(event_based_actor* /*self*/) {
  auto count = std::make_shared<uint64_t>(0);
  return {
    [=](const Foo& /*val*/) { (*count)++; },
    [=](const Bar& /*val*/) -> uint64_t { return *count; },
  };
}

void par_func(int max_to_read, uint64_t id) {
  while (true) {
    int num_read = 0;
    {
      std::lock_guard<std::mutex> guard(g_mutex);
      if (!g_par_continue && g_queue.empty())
        return;

      while (!g_queue.empty() && (max_to_read == -1 || num_read < max_to_read)) {
        g_count_queue.push(id);
        g_queue.pop();
        num_read++;
      }
    }
    for (int i = 0; i < num_read; i++) {
      busy_spin(g_busy_spin_millis);
    }
  }
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor(actor_system& sys, const WorkConfig& work_config) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_actor_send_count = 0;
  g_actor_work_count = 0;
  while (!g_count_queue.empty())
    g_count_queue.pop();

  double micros = watch.duration([&] {
    Foo f{ 1 };
    auto counter = sys.spawn(posh_counter);
    auto posh_actor = sys.spawn(posh, counter);
    scoped_actor self{ sys };

    for (uint64_t i = 0; i < work_config.iterations; i++) {
      self->send(posh_actor, f);
      g_actor_send_count++;
      std::this_thread::sleep_for(work_config.work_send_delay);
    }
    self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
    self->await_all_other_actors_done();
  });

  fmt::print("{:>25}: {:>15} (micros)\n", "single actor", common::numberFormatWithCommas(micros));

  g_actor_work_count = g_count_queue.size();
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor_x
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor_x(actor_system& sys, const WorkConfig& work_config) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_actor_x_send_count = 0;
  g_actor_x_work_count = 0;
  scoped_actor self{ sys };
  auto counter = sys.spawn(posh_counter);

  while (!g_count_queue.empty())
    g_count_queue.pop();

  double micros = watch.duration([&] {
    Foo f{ 1 };

    for (uint64_t i = 0; i < work_config.iterations; i++) {
      auto posh_actor = sys.spawn(posh_x, counter);
      self->send(posh_actor, f);
      g_actor_x_send_count++;
      self->send(posh_actor, exit_msg{ self.address(), exit_reason::kill });
      std::this_thread::sleep_for(work_config.work_send_delay);
    }
    self->await_all_other_actors_done();
  });

  fmt::print("{:>25}: {:>15} (micros)\n", "actor per work item", common::numberFormatWithCommas(micros));

  g_actor_work_count = g_count_queue.size();
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_actor_x
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_actor_pool(actor_system& sys, const WorkConfig& work_config) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_actor_pool_send_count = 0;
  g_actor_pool_work_count = 0;

  double micros = watch.duration([&] {
    Foo f{ 1 };
    scoped_actor self{ sys };
    auto counter = sys.spawn(posh_counter);
    scoped_execution_unit ctx{ &sys };
    auto pool = actor_pool::make(
        &ctx, work_config.actor_pool_size, [&] { return sys.spawn(posh_pool, counter); }, actor_pool::round_robin());

    for (uint64_t i = 0; i < work_config.iterations; i++) {
      g_actor_pool_send_count++;
      self->send(pool, f);
      std::this_thread::sleep_for(work_config.work_send_delay);
    }

    while (g_actor_pool_work_count < g_actor_pool_send_count) {
      self->request(counter, infinite, Bar{ 1 })
          .receive([=](uint64_t c) { g_actor_pool_work_count = c; }, [&](error& err) { fmt::print("Error: {}\n", to_string(err)); });
    }

    self->send_exit(counter, exit_reason::user_shutdown);
    self->send_exit(pool, exit_reason::user_shutdown);
    self->await_all_other_actors_done();
  });

  fmt::print("{:>25}: {:>15} (micros)\n", "actor pool", common::numberFormatWithCommas(micros));

  sys.await_all_actors_done();
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_func
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_func(actor_system& /*sys*/, const WorkConfig& work_config) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_func_send_count = 0;
  g_func_work_count = 0;

  while (!g_count_queue.empty())
    g_count_queue.pop();

  auto func = [&](int inc) {
    g_func_send_count += inc;
    busy_spin(g_busy_spin_millis);
  };

  double micros = watch.duration([&] {
    for (uint64_t i = 0; i < work_config.iterations; i++) {
      func(1);
      std::this_thread::sleep_for(work_config.work_send_delay);
    }
  });

  fmt::print("{:>25}: {:>15} (micros)\n", "raw function", common::numberFormatWithCommas(micros));

  g_actor_work_count = g_count_queue.size();
}

// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==
// time_par_func
// -==------=-==-=-=-===-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-===-=-=-==

void time_par_func(actor_system& /*sys*/, const WorkConfig& work_config) {
  common::Stopwatch<uint64_t, std::chrono::microseconds> watch;
  g_par_continue.store(true);
  int num_to_read = work_config.par_threads == 1 ? -1 : 1;
  g_par_send_count = 0;
  g_par_work_count = 0;

  while (!g_queue.empty())
    g_queue.pop();

  while (!g_count_queue.empty())
    g_count_queue.pop();

  double micros = watch.duration([&] {
    
    // prime queue
    for (uint64_t i = 0; i < work_config.iterations; i++) {
      g_queue.push(g_par_send_count);
      g_par_send_count++;
      std::this_thread::sleep_for(work_config.work_send_delay);
    }

    // create threads
    std::vector<std::thread> thread_par_func;
    for (uint64_t i = 0; i < work_config.par_threads; i++) {
      thread_par_func.push_back(std::thread(par_func, num_to_read, i));
    }

    // teardown
    g_par_continue.store(false);
    for (std::thread& thread : thread_par_func) {
      if (thread.joinable())
        thread.join();
    }
  });

  fmt::print("{:>25}: {:>15} (micros)\n", "standard thread pool", common::numberFormatWithCommas(micros));

  g_par_work_count = g_count_queue.size();
}

void print_queue_analysis(uint64_t num_threads) {
  std::vector<uint64_t> queue_analysis;
  queue_analysis.reserve(num_threads);
  for (uint64_t i = 0; i < num_threads; i++) {
    queue_analysis[i] = 0;
  }
  while (!g_count_queue.empty()) {
    queue_analysis[g_count_queue.front()]++;
    g_count_queue.pop();
  }

  for (uint64_t i = 0; i < num_threads; i++) {
    fmt::print("queue_analysis[{}] = {}\n", i, queue_analysis[i]);
  }
}

void print_counts() {
  // fmt::print(" {:>30} = {:>15}\n", "g_func_work_count", common::numberFormatWithCommas(g_func_work_count));
  // fmt::print(" {:>30} = {:>15}\n", "g_func_send_count", common::numberFormatWithCommas(g_func_send_count));

  fmt::print(" {:>30} = {:>15}\n", "g_par_work_count", common::numberFormatWithCommas(g_par_work_count));
  fmt::print(" {:>30} = {:>15}\n", "g_par_send_count", common::numberFormatWithCommas(g_par_send_count));

  // fmt::print(" {:>30} = {:15}\n", "g_actor_work_count", common::numberFormatWithCommas(g_actor_work_count));
  // fmt::print(" {:>30} = {:>15}\n", "g_actor_send_count", common::numberFormatWithCommas(g_actor_send_count));

  // fmt::print(" {:>30} = {:>15}\n", "g_actor_x_work_count", common::numberFormatWithCommas(g_actor_x_work_count));
  // fmt::print(" {:>30} = {:>15}\n", "g_actor_x_send_count", common::numberFormatWithCommas(g_actor_x_send_count));

  fmt::print(" {:>30} = {:>15}\n", "g_actor_pool_work_count", common::numberFormatWithCommas(g_actor_pool_work_count));
  fmt::print(" {:>30} = {:>15}\n", "g_actor_pool_send_count", common::numberFormatWithCommas(g_actor_pool_send_count));
}

void harness(actor_system& sys) {

  fmt::print("Using {} par_threads, {} actor_threads, {} actor_pool_size, {} iterations, with {} microsecond work per "
             "iteration, random_work={}:\n\n",
             g_work_config.par_threads,
             g_work_config.actor_threads,
             common::numberFormatWithCommas(g_work_config.actor_pool_size),
             common::numberFormatWithCommas(g_work_config.iterations),
             common::numberFormatWithCommas(g_busy_spin_millis.count()),
             g_work_config.random_work);
  sys.clock();

  // time_func(sys, g_work_config.iterations,g_work_config.work_send_delay);
  time_par_func(sys, g_work_config);
  // time_actor(sys, g_work_config);
  // time_actor_x(sys, g_work_config);
  time_actor_pool(sys, g_work_config);

  // print_counts();

  fmt::print("\n");
}

void caf_main(actor_system& sys) {
  std::list<caf::timespan> default_work_per_iteration = { caf::timespan(NANOS_PER_MICRO * 10) };
  g_work_config = {
    get_or(sys.config(), "class-perf.par-threads", (size_t)1),
    get_or(sys.config(), "caf.scheduler.max-threads", (size_t)1),
    get_or(sys.config(), "class-perf.actor-pool-size", (size_t)1),
    get_or(sys.config(), "class-perf.iterations", (size_t)100),
    std::chrono::duration_cast<std::chrono::microseconds>(get_or(sys.config(), "class-perf.work-send-delay", caf::timespan(NANOS_PER_MICRO))),
    get_or(sys.config(), "class-perf.work-per-iteration", default_work_per_iteration),
    get_or(sys.config(), "class-perf.random-work", false),
  };

  for (auto work : g_work_config.work_per_iteration) {
    g_busy_spin_millis = std::chrono::duration_cast<std::chrono::microseconds>(work);
    // for (auto n : std::list<size_t>{ 1, 2, 4, 8 }) {
    //   config.par_threads = n;
    //   harness(sys, config);
    // }
    harness(sys);
  }
}

CAF_MAIN(id_block::custom_types_1)