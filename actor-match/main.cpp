#include <caf/blocking_actor.hpp>
#include <caf/fwd.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <thread>

#include "caf/all.hpp"
#include "caf/policy/select_all.hpp"

#include "include/fmt/format.h"

#include "actor_types.h"
#include "common/model/order.h"
#include "common/model/order_factory.h"
#include "order_book_actor.h"
#include "router_actor.h"

using namespace std;
using namespace common::model;
using namespace actor_match;
using namespace std::chrono;

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
//
template <typename M, typename T> void sync_send(caf::blocking_actor* sender, caf::actor target, const M& msg, const T& arg) {
  sender->request(target, caf::infinite, msg, arg).receive([&](const bool&) {}, [&](caf::error&) {});
}

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
//
void wait_for_finish(caf::blocking_actor* sender, caf::actor target, size_t counter) {
  bool done = false;
  while (!done) {
    sender->request(target, caf::infinite, dump_book_summary(), counter)
        .receive([&](size_t num_processed) { done = (num_processed == counter); }, [&](caf::error&) {});

    if (!done)
      this_thread::sleep_for(chrono::milliseconds(200));
  }
}

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
//
void send_order(caf::blocking_actor* self, caf::actor target, Order& order, bool async_mode) {
  if (async_mode) {
    caf::anon_send(target, new_order(), move(order));
  } else {
    sync_send<new_order, Order>(self, target, new_order(), order);
  }
}

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
// Message Flow
//
//               +----------+    +----> order book actor 0
//  +-------+    |          |    |
//  | stdin | -> |  router  | -> +----> order book actor 1
//  +-------+    |          |    |      ...
//               +----------+    +----> order book actor N
//
// one-to-many concepts:
// - groups
// - fan_out_request (map/reduce)
// - actor pools (see class-perf.cpp)
//
void process(caf::actor_system& sys) {

  auto [argc, argv] = sys.config().c_args_remainder(); // How you grab arguments from CAF_MAIN

  size_t report_mod = 0;
  if (argc > 1)
    report_mod = (size_t)atol(argv[1]);

  bool async_mode = true;
  if (argc > 2)
    async_mode = atol(argv[2]) != 0;

  fmt::print(stderr, "async_mode={}, report_mod={}\n", async_mode, report_mod);

  caf::scoped_actor self{ sys };                      // scope-based actor (needed for blocking requests)
  auto books_group = sys.groups().get_local("books"); // Group allows messages to be sent to all actors in group
  auto router = sys.spawn<RouterActor>(books_group);  // router will route order to appropriate order book

  size_t counter = 0;
  for (std::string line; std::getline(std::cin, line);) {
    Order order = OrderFactory::from_string(line);

    send_order(self.ptr(), router, order, async_mode);

    counter++;
    if (report_mod != 0 && counter % report_mod == 0) {
      self->send<caf::message_priority::high>(router, dump_book_summary(), counter);
    }
  }

  fmt::print(stderr, "{} orders read from stdin\n", counter);

  wait_for_finish(self.ptr(), router, counter);

  fmt::print(stderr, "{} orders processed\n", counter);

  self->send(books_group, dump_book_and_exit());
  anon_send_exit(router, caf::exit_reason::user_shutdown);
}

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
//
void caf_main(caf::actor_system& sys) {
  process(sys);
  sys.await_all_actors_done();
}

// --=-=-=-=-=-==-=-=-=-==-=-=-=--==-==-=-=-==-=-=-=-==-=-=-=--=-=-=-==-=-==-==
//
CAF_MAIN(caf::id_block::match_id_block)
