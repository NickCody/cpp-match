#include <caf/actor_addr.hpp>
#include <caf/event_based_actor.hpp>
#include <caf/fwd.hpp>
#include <caf/group.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "caf/all.hpp"

#include "include/fmt/format.h"
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

class RouterActor : public event_based_actor {
public:
  explicit RouterActor(actor_config& cfg, actor_system& sys)
      : event_based_actor(cfg) {
    books_group = sys.groups().get_local("books");
  }

  behavior make_behavior() override {
    return {
      [this](new_order, const Order& order) -> bool {
        if (!book_map.order_books.contains(order.instrument)) {
          auto book = spawn_in_group<OrderBookActor>(books_group, order.instrument);
          // auto book = spawn<OrderBookActor>(order.instrument);
          book_map.order_books[order.instrument] = book;
        }
        send(book_map.order_books[order.instrument], new_order_v, order);
        return true;
      },
      [this](close_books) {
        for (auto book : book_map.order_books) {
          request(book.second, infinite, dump_book_v);
          send_exit(book.second, exit_reason::user_shutdown);
        }
      },
    };
  }

private:
  OrderBookMap book_map;

  caf::group books_group;
};

void process(actor_system& sys) {
  scoped_actor self{ sys };
  auto router = sys.spawn<RouterActor>(sys);

  for (std::string line; std::getline(std::cin, line);) {
    Order order = OrderFactory::from_string(line);

    // async send
    self->send(router, new_order_v, order);

    // synchronous send (slow)
    //
    if (false) {
      self->request(router, infinite, new_order_v, order)
          .receive(
              [&](const bool& added) {
                if (!added) {
                  aout(self) << "Order was not added: " << order.to_string() << endl;
                }
              },
              [&](error& err) { aout(self) << to_string(err) << endl; });
    }
  }

  self->request(router, infinite, close_books_v);
  self->send_exit(router, exit_reason::user_shutdown);
}

void caf_main(actor_system& sys) {
  process(sys);
  sys.await_all_actors_done();
}

CAF_MAIN(id_block::match_id_block)
