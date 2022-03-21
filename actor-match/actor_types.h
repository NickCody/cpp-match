#pragma once

#include <map>
#include <string>
#include <algorithm>
#include "fmt/format.h"

#include "caf/all.hpp"

#include "common/model/order.h"

namespace actor_match {

  using namespace std;
  using namespace common::model;

  struct BookStats {
    string instrument;
    size_t total_orders;
    size_t open_buys;
    size_t open_sells;
  };

  struct OrderBook {
    using OrderVec = vector<Order>;

    struct BookTuple {
      OrderVec& contras;
      OrderVec& peers;
    };

    OrderBook() {
      make_heap(buys.begin(), buys.end());
      make_heap(sells.begin(), sells.end());
    }

    void delete_order(const string& name) {
      _delete_order(buys, name);
      _delete_order(sells, name);
    }

    void _delete_order(OrderVec& v, const string& name) {
      auto it = find_if(v.begin(), v.end(), [&](const Order& order) -> bool { return order.order_id == name; });
      if (it != v.end()) {
        v.erase(it);
        make_heap(v.begin(), v.end());
      }
    }

    void match_order(caf::local_actor* self, Order order) {
      size_t incoming_order_id = atoi(order.order_id.c_str());

      if (highest_order_id > incoming_order_id) {
        fmt::print(stderr, "ERROR: Got a message out of order {} last highest {}\n", order.order_id, highest_order_id);
      } else {
        highest_order_id = incoming_order_id;
      }

      BookTuple books = order.side == Order::SIDE::BUY ? BookTuple{ .contras = sells, .peers = buys } : BookTuple{ .contras = buys, .peers = sells };

      while (order.remaining_quantity > 0) {
        if (books.contras.empty()) {
          books.peers.push_back(move(order));
          push_heap(books.peers.begin(), books.peers.end());
          return;
        }

        auto& contra_order = books.contras.front();

        if ((order.side == Order::SIDE::BUY && contra_order.price <= order.price) ||
            (order.side == Order::SIDE::SELL && contra_order.price >= order.price)) {
          int trade_quantity = min(order.remaining_quantity, contra_order.remaining_quantity);
          double price = contra_order.price;

          // Print the trade
          //
          caf::aout(self) << fmt::format("TRADE {} {} {} {} {}", order.instrument, order.order_id, contra_order.order_id, trade_quantity, price)
                          << endl;

          order.remaining_quantity -= trade_quantity;
          contra_order.remaining_quantity -= trade_quantity;

          purge_zero();
        } else {
          break;
        }
      }

      // If order still has quantity, add it to the book
      if (order.remaining_quantity > 0) {
        books.peers.push_back(move(order));
        push_heap(books.peers.begin(), books.peers.end());
      }
    }

    void purge_zero() {

      purge_zero_container(buys);
      purge_zero_container(sells);
    }

    void purge_zero_container(OrderVec& c) {
      if (c.size() == 0)
        return;

      if (c.front().remaining_quantity == 0) {
        pop_heap(c.begin(), c.end());
        c.pop_back();
      }
    }

    OrderVec buys;
    OrderVec sells;
    string instrument;
    size_t highest_order_id;
  };

  template <class Inspector> bool inspect(Inspector& f, OrderBook& order_book) {
    return f.object(order_book)
        .fields(f.field("buys", order_book.buys), f.field("sells", order_book.sells), f.field("instrument", order_book.instrument));
  }

  template <class Inspector> bool inspect(Inspector& f, BookStats& book_stats) {
    return f.object(book_stats)
        .fields(f.field("instrument", book_stats.instrument),
                f.field("total_orders", book_stats.total_orders),
                f.field("open_buys", book_stats.open_buys),
                f.field("open_sells", book_stats.open_sells));
  }

} // namespace actor_match

namespace common::model {
  template <class Inspector> bool inspect(Inspector& f, Order& order) {
    return f.object(order).fields(f.field("timestamp", order.timestamp),
                                  f.field("order_id", order.order_id),
                                  f.field("side", order.side),
                                  f.field("instrument", order.instrument),
                                  f.field("remaining_quantity", order.remaining_quantity),
                                  f.field("price", order.price));
  }
} // namespace common::model

CAF_BEGIN_TYPE_ID_BLOCK(match_id_block, first_custom_type_id)

CAF_ADD_TYPE_ID(match_id_block, (common::model::Order))
CAF_ADD_TYPE_ID(match_id_block, (actor_match::OrderBook))
CAF_ADD_TYPE_ID(match_id_block, (actor_match::BookStats))

CAF_ADD_ATOM(match_id_block, new_order)
CAF_ADD_ATOM(match_id_block, delete_order)
CAF_ADD_ATOM(match_id_block, close_books)
CAF_ADD_ATOM(match_id_block, dump_book_and_exit)
CAF_ADD_ATOM(match_id_block, dump_book_summary)
CAF_ADD_ATOM(match_id_block, get_book_stats)
CAF_ADD_ATOM(match_id_block, wait_finished)

CAF_END_TYPE_ID_BLOCK(match_id_block)
