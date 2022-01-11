/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include "common/model/order.h"

namespace common::model {
  std::ostream& operator<<(std::ostream& os, const Order& o) {
    os << "order_id=" << o.order_id << ", "
       << "side" << (o.side == Order::SIDE::BUY ? "BUY" : "SELL") << ", "
       << "instrument=" << o.instrument << ", "
       << "remaining_quantity=" << o.remaining_quantity << ", "
       << "price=" << o.price;

    return os;
  }

  bool operator<(const OrderPtr& lhs, const OrderPtr& rhs) { return *lhs < *rhs; }
} // namespace common::model
