#pragma once

#include <map>
#include <algorithm>
#include "caf/all.hpp"
#include "common/model/order.h"

namespace actor_match {
  struct OrderBookMap {

    std::map<std::string, caf::actor> order_books;
  };

  struct OrderBook {
    using OrderVec = std::vector<common::model::Order>;

    struct BookTuple {
      OrderVec& contras;
      OrderVec& peers;
    };

    OrderBook() {
      std::make_heap(buys.begin(), buys.end());
      std::make_heap(sells.begin(), sells.end());
    }

    void match_order(caf::local_actor* self, common::model::Order& order) {
      BookTuple books = order.side == common::model::Order::SIDE::BUY ? BookTuple{ sells, buys } : BookTuple{ buys, sells };

      while (order.remaining_quantity > 0) {
        if (books.contras.empty()) {
          books.contras.push_back(order);
          std::push_heap(books.contras.begin(), books.contras.end());
          return;
        }

        auto& contra_order = books.contras.front();

        if ((order.side == common::model::Order::SIDE::BUY && contra_order.price <= order.price) ||
            (order.side == common::model::Order::SIDE::SELL && contra_order.price >= order.price)) {
          int trade_quantity = std::min(order.remaining_quantity, contra_order.remaining_quantity);
          double price = contra_order.price;

          // Print the trade
          //
          caf::aout(self) << "TRADE " << order.instrument << " " << order.order_id << " " << contra_order.order_id << " " << trade_quantity << " "
                          << price << std::endl;

          order.remaining_quantity -= trade_quantity;
          contra_order.remaining_quantity -= trade_quantity;

          purge_zero();
        } else {
          break;
        }
      }

      // If order still has quantity, add it to the book
      if (order.remaining_quantity > 0) {
        books.peers.push_back(order);
        std::push_heap(books.peers.begin(), books.peers.end());
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
        std::pop_heap(c.begin(), c.end());
        c.pop_back();
      }
    }

    OrderVec buys;
    OrderVec sells;
    std::string instrument;
  };

  template <class Inspector> bool inspect(Inspector& f, OrderBook& order_book) {
    return f.object(order_book)
        .fields(f.field("buys", order_book.buys), f.field("sells", order_book.sells), f.field("instrument", order_book.instrument));
  }

  template <class Inspector> bool inspect(Inspector& f, OrderBookMap& order_book_map) {
    return f.object(order_book_map).fields(f.field("order_books", order_book_map.order_books));
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
CAF_ADD_TYPE_ID(match_id_block, (actor_match::OrderBookMap))

CAF_ADD_ATOM(match_id_block, new_order)
CAF_ADD_ATOM(match_id_block, dump_book)

CAF_END_TYPE_ID_BLOCK(match_id_block)
