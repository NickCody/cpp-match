/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <iostream>
#include "darkpool/engine/order_book.h"

using namespace common::model;

namespace darkpool {
  OrderBook::OrderBook() {
    std::make_heap(buys.begin(), buys.end());
    std::make_heap(sells.begin(), sells.end());
  }

  // NOTE: not done with this...
  //

  bool OrderBook::remove_order(const std::string& order_id) {

    bool found = false;
    for (auto oi = buys.begin(); oi != buys.end(); oi++) {
      if (oi->order_id == order_id) {
        found = true;
        remove_order(oi, buys);
      }
    }

    // if (!found) {
    //     for (const Order& o : sells) {
    //         if (o.order_id == order_id) {
    //             found = true;
    //             remove_order(order_id, sells);
    //         }
    //     }
    // }

    return found;
  }

  void OrderBook::remove_order(OrderContainer::iterator oi, OrderContainer& orders) {
    orders.erase(oi);
    std::make_heap(orders.begin(), orders.end());
  }

  int OrderBook::get_order_book_size() {
    return buys.size() + sells.size();
  }

  void OrderBook::add_to_book(const Order& order) {
    if (order.side == Order::SIDE::BUY) {
      buys.push_back(order);
      std::push_heap(buys.begin(), buys.end());
    } else {
      sells.push_back(order);
      std::push_heap(sells.begin(), sells.end());
    }
  }

  OrderContainer& OrderBook::get_contra_orders(const Order& order) {
    if (order.side == Order::SIDE::BUY) {
      return sells;
    } else {
      return buys;
    }
  }

  bool OrderBook::has_contra_orders(const Order& order) {
    const OrderContainer& c = get_contra_orders(order);
    return c.size() > 0;
  }

  void OrderBook::purge() {

    purge_container(buys);
    purge_container(sells);
  }

  void OrderBook::purge_container(OrderContainer& c) {
    if (c.size() == 0)
      return;

    if (c.front().remaining_quantity == 0) {
      std::pop_heap(c.begin(), c.end());
      c.pop_back();
    }
  }

  void OrderBook::print_books() {
    print_book(buys);
    print_book(sells);
  }

  void OrderBook::print_book(Order::SIDE side) {
    if (side == Order::SIDE::BUY) {
      print_book(buys);
    } else {
      print_book(sells);
    }
  }

  void OrderBook::print_book(const OrderContainer& book) {
    for (const Order& o : book) {
      std::cout << o << std::endl;
    }
  }
} // namespace darkpool