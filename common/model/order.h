/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include "common/common.h"
#include <chrono>
#include <memory>
#include <sstream>

#include <caf/inspector_access.hpp>

namespace common::model {
  struct Order {
    enum SIDE { BUY, SELL };

    std::chrono::milliseconds timestamp;
    std::string order_id; // NOTE: Would get perf increase if we maintained symbol
                          // dictionary
    SIDE side;
    std::string instrument; // NOTE: Would get perf increase if we maintained
                            // symbol dictionary
    long remaining_quantity;
    double price;

    Order() {}

    Order(const Order& order) {
      timestamp = order.timestamp;
      order_id = order.order_id;
      side = order.side;
      instrument = order.instrument;
      remaining_quantity = order.remaining_quantity;
      price = order.price;
    }

    Order(std::chrono::milliseconds timestamp, const std::string& order_id, SIDE side, const std::string& instrument,
          long remaining_quantity, double price) {
      this->timestamp = timestamp;
      this->order_id = order_id;
      this->side = side;
      this->instrument = instrument;
      this->remaining_quantity = remaining_quantity;
      this->price = price;
    }

    Order(std::chrono::milliseconds timestamp, const std::string& order_id, const std::string& side,
          const std::string& instrument, long remaining_quantity, double price) {
      this->timestamp = timestamp;
      this->order_id = order_id;
      if (side.compare("BUY") == 0)
        this->side = Order::SIDE::BUY;
      else
        this->side = Order::SIDE::SELL;
      this->instrument = instrument;
      this->remaining_quantity = remaining_quantity;
      this->price = price;
    }

    std::string to_string(char sep = common::FIELD_SEP) const {
      std::ostringstream rep;
      rep << order_id << sep << (side == SIDE::BUY ? "BUY" : "SELL") << sep << instrument << sep << remaining_quantity
          << sep << price;

      return rep.str();
    }

    // Used for ranking, not meant to compare buys to sells, only buy/buy and
    // sell/sell less means ranked lower low buy price is ranked lower than high
    // buy high sell price is ranked lower than low sell later time is ranked
    // lower than earlier time

    bool operator<(const Order& rhs) {
      // silly equal
      if (std::abs(price - rhs.price) < 0.0001) {
        return timestamp > rhs.timestamp;
      }

      if (side == SIDE::BUY) {
        return price < rhs.price;
      } else {
        return price > rhs.price;
      }
    }

    friend std::ostream& operator<<(std::ostream& os, const Order& o);

    bool operator==(const Order& rhs) const { return order_id.compare(rhs.order_id) == 0; }

    bool operator!=(const Order& rhs) const { return !(*this == rhs); }
  };

  typedef std::shared_ptr<Order> OrderPtr;

  // NOTE: Only used if we have a collection container OrderPtr and we wanted
  // ordering based on Order order semantics
  //
  bool operator<(const OrderPtr& lhs, const OrderPtr& rhs);

  template <class Inspector> bool inspect(Inspector& f, Order& x) {
    return f.object(x).fields(f.field("order_id", x.order_id),
                              f.field("side", x.side),
                              f.field("instrument", x.instrument),
                              f.field("remaining_quantity", x.remaining_quantity),
                              f.field("price", x.price));
  }
} // namespace common::model

// namespace caf {
// template <>
// struct inspector_access<common::model::Order>
//     : inspector_access_base<common::model::Order> {
//   template <class Inspector>
//   static bool apply(Inspector &f, common::model::Order &x) {
//     return f.object(x).fields(
//         f.field("order_id", x.order_id), f.field("side", x.side),
//         f.field("instrument", x.instrument),
//         f.field("remaining_quantity", x.remaining_quantity),
//         f.field("price", x.price));
//   }
// };
// } // namespace caf