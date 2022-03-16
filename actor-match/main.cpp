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

class OrderBookActor : public event_based_actor {
public:
  explicit OrderBookActor(actor_config& cfg, const string& instrument)
      : event_based_actor(cfg),
        m_instrument(instrument) {
  }

  behavior make_behavior() override {

    return {
      [this](new_order, Order& order) { m_book.match_order(this, order); },
      [this](dump_book) {
        for (auto order : m_book.buys) {
          aout(this) << order.to_string() << endl;
        }

        for (auto order : m_book.sells) {
          aout(this) << order.to_string() << endl;
        }
      },
    };
  }

private:
  OrderBook m_book;
  const string m_instrument;
};

behavior route_order(stateful_actor<OrderBookMap>* self) {
  return {
    [=](new_order, const Order& order) -> bool {
      if (!self->state.order_books.contains(order.instrument)) {
        self->state.order_books[order.instrument] = self->spawn<OrderBookActor>(order.instrument);
      }
      self->send(self->state.order_books[order.instrument], new_order_v, order);
      return true;
    },
    [=](dump_book) {
      for (auto book : self->state.order_books) {
        self->send(book.second, dump_book_v);
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
      self->send(router, new_order_v, order);
    }

    self->request(router, infinite, dump_book_v);
    self->send_exit(router, exit_reason::user_shutdown);
  }
  sys.await_all_actors_done();
}

CAF_MAIN(id_block::match_id_block)
