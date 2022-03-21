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

template <typename M, typename T> void sync_send(caf::blocking_actor* sender, caf::actor target, const M& msg, const T& arg) {
  sender->request(target, caf::infinite, msg, arg).receive([&](const bool&) {}, [&](caf::error&) {});
}

void wait_for_finish(caf::blocking_actor* sender, caf::actor target, size_t counter) {
  bool done = false;
  while (!done) {

    sender->request(target, caf::infinite, dump_book_summary(), counter)
        .receive(
            [&](size_t num_processed) {
              fmt::print(stderr, "waiting... {} ?? {} \n", num_processed, counter);
              done = (num_processed == counter);
            },
            [&](caf::error&) {});
    this_thread::sleep_for(chrono::milliseconds(200));
  }
}

void process(caf::actor_system& sys) {

  // How you grab arguments from CAF_MAIN
  //
  auto [argc, argv] = sys.config().c_args_remainder();

  size_t report_mod = 0;
  if (argc > 1)
    report_mod = (size_t)atol(argv[1]);

  int async_mode = 1;
  if (argc > 2)
    async_mode = atol(argv[2]);

  // scope-based actor (needed for blocking requests)
  //
  caf::scoped_actor self{ sys };

  // Group allows messages to be sent to all actors in group
  //
  auto books_group = sys.groups().get_local("books");

  // router will route order to appropriate order book
  //
  auto router = sys.spawn<RouterActor>(books_group);

  size_t counter = 0;
  for (std::string line; std::getline(std::cin, line);) {
    Order order = OrderFactory::from_string(line);

    if (async_mode != 0) {
      caf::anon_send(router, new_order(), move(order));
    } else {
      sync_send<new_order, Order>(self.ptr(), router, new_order(), order);
    }

    counter++;
    if (report_mod != 0 && counter % report_mod == 0) {
      self->send(router, dump_book_summary(), counter);
    }
  }

  fmt::print(stderr, "Done reading {} orders; waiting to finish processing...\n", counter);

  wait_for_finish(self.ptr(), router, counter);

  fmt::print(stderr, "All {} orders processed!\n", counter);

  self->send(books_group, dump_book_and_exit());
  anon_send_exit(router, caf::exit_reason::user_shutdown);
}

void caf_main(caf::actor_system& sys) {
  process(sys);
  sys.await_all_actors_done();
}

CAF_MAIN(caf::id_block::match_id_block)
