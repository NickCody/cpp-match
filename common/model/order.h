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

namespace common::model {
  struct Order;
  using OrderPtr = std::shared_ptr<Order>;

  struct Order {

    enum SIDE { BUY, SELL };

    std::chrono::milliseconds timestamp;
    std::string order_id;
    SIDE side;
    std::string instrument;
    long remaining_quantity;
    double price;

    Order() {
    }

    Order(const Order& order) {
      timestamp = order.timestamp;
      order_id = order.order_id;
      side = order.side;
      instrument = order.instrument;
      remaining_quantity = order.remaining_quantity;
      price = order.price;
    }

    Order(std::chrono::milliseconds timestamp, const std::string& order_id, SIDE side, const std::string& instrument, long remaining_quantity,
          double price) {
      this->timestamp = timestamp;
      this->order_id = order_id;
      this->side = side;
      this->instrument = instrument;
      this->remaining_quantity = remaining_quantity;
      this->price = price;
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

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
      os << std::setw(8) << order.order_id << " " << ((order.side == Order::SIDE::BUY) ? "BUY" : "SELL") << " " << order.instrument << " "
         << order.remaining_quantity << " " << order.price;

      return os;
    }

    bool operator<(const OrderPtr& other) {
      return *this < other;
    }

    bool operator==(const Order& rhs) const {
      return order_id.compare(rhs.order_id) == 0;
    }

    bool operator!=(const Order& rhs) const {
      return !(*this == rhs);
    }
  };

} // namespace common::model
