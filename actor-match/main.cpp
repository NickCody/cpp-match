#include <chrono>
#include <iostream>
#include <string>

#include "caf/all.hpp"

#include "common/model/order.h"
#include "common/model/order_factory.h"

#include "actor_types.h"

using namespace std;
using namespace caf;
using namespace common::model;
using namespace actor_match;

constexpr auto ONE_SECOND = std::chrono::seconds(1);

behavior order_book(stateful_actor<OrderBook>* self, const string& instrument) {
  self->state.instrument = instrument;

  return {
    [=](new_order, Order& order) { self->state.match_order(self, order); },
    [=](dump_book, const string& /*instrument*/) {
      for (auto order : self->state.buys)
        aout(self) << order << endl;
      for (auto order : self->state.sells)
        aout(self) << order << endl;
    },
  };
}

behavior route_order(stateful_actor<OrderBookMap>* self) {
  return {
    [=](new_order, const Order& order) -> bool {
      if (!self->state.order_books.contains(order.instrument)) {
        self->state.order_books[order.instrument] = self->spawn(order_book, order.instrument);
      }
      self->send(self->state.order_books[order.instrument], new_order_v, order);
      return true;
    },
    [=](dump_book) {
      for (auto book : self->state.order_books) {
        self->send(book.second, dump_book_v, book.first);
      }
    },
  };
}

void caf_main(actor_system& sys) {
  cerr << "Actor Match" << endl;

  {
    scoped_actor self{ sys };

    auto router = sys.spawn(route_order);

    for (std::string line; std::getline(std::cin, line);) {
      Order order = OrderFactory::from_string(line);

      // this async send can only be used when caf.max_threads=1
      self->send(router, new_order_v, order);

      // use this when you use multiple threads as it will block and book won't be dumped into all orders sent
      // this is slow
      // self->request(router, infinite, new_order_v, order)
      //     .receive([&](const bool& /*added*/) {}, [&](error& err) { aout(self) << to_string(err) << endl; });
    }

    self->request(router, infinite, dump_book_v);
    self->send_exit(router, exit_reason::user_shutdown);
  }
  sys.await_all_actors_done();
}

CAF_MAIN(id_block::match_id_block)
