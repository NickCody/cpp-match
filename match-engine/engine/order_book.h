/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <vector>
#include <memory>

#include "common/model/order.h"

using namespace common::model;

namespace darkpool {

  typedef std::vector<Order> OrderContainer;

  class OrderBook {
  private:
    OrderContainer buys;
    OrderContainer sells;

    void remove_order(OrderContainer::iterator oi, OrderContainer& orders);

  public:
    OrderBook();
    bool remove_order(const std::string& order_id);

    void add_to_book(const Order& order);
    bool has_contra_orders(const Order& order);
    OrderContainer& get_contra_orders(const Order& order);
    int get_order_book_size();

    void purge();
    void purge_container(OrderContainer& c);

    void print_books();
    void print_book(const Order::SIDE side);
    void print_book(const OrderContainer& book);
  };

  typedef std::shared_ptr<OrderBook> OrderBookPtr;
} // namespace darkpool
