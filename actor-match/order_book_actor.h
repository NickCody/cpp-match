#pragma once

#include <sstream>
#include <string>

#include "include/fmt//format.h"
#include "caf/all.hpp"

#include "actor_types.h"
#include "common/model/order.h"

namespace actor_match {

  using namespace std;
  using namespace common::model;

  class OrderBookActor : public caf::event_based_actor {
  public:
    explicit OrderBookActor(caf::actor_config& cfg, const string& instrument)
        : event_based_actor(cfg),
          m_instrument(instrument),
          orders_received(0) {
    }

    caf::behavior make_behavior() override {

      return {
        [this](new_order, Order& order) {
          orders_received++;
          m_book.match_order(this, order);
        },
        [this](get_book_stats) -> BookStats {
          return BookStats{ m_instrument, orders_received, m_book.buys.size(), m_book.sells.size() };
        },
        [this](delete_order, const string& orderid) { m_book.delete_order(orderid); },
        [this](dump_book_and_exit) {
          stringstream book_dump;

          for (auto order : m_book.buys) {
            book_dump << order << endl;
          }

          for (auto order : m_book.sells) {
            book_dump << order << endl;
          }

          aout(this) << book_dump.str();

          send_exit(this, caf::exit_reason::user_shutdown);
        },
      };
    }

  private:
    string m_instrument;
    size_t orders_received;
    OrderBook m_book;
  };

} // namespace actor_match
