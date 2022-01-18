#pragma once

#include "caf/actor.hpp"
#include "common/model/order.h"

namespace actor_match {
  struct OrderBookMap {

    std::map<std::string, caf::actor> order_books;
  };

  struct OrderBook {
    OrderBook() {
      std::make_heap(buys.begin(), buys.end());
      std::make_heap(sells.begin(), sells.end());
    }

    std::vector<common::model::Order> buys;
    std::vector<common::model::Order> sells;
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
