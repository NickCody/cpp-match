/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <map>
#include <memory>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#include "common/histogram.h"
#include "common/model/order.h"
#include "darkpool/engine/order_book.h"

namespace darkpool {
  class MatchingEngine {
  private:
    std::string name;

    std::map<std::string, OrderBookPtr> order_books;

    // input queue
    std::deque<Order> incoming_orders;
    std::mutex order_mutex;
    std::atomic_bool terminate;

    bool concurrent;

    // stats
    common::Histogram<int> orders_processed;
    common::Histogram<long> match_latencies_micros;
    common::Histogram<long> lock_latencies_micros;
    common::Histogram<long> order_book_sizes;

    void calculate_order_book_sizes();

  public:
    MatchingEngine(const std::string& name, bool concurrent);
    ~MatchingEngine();

    // concurrency and order provider
    //
    bool is_concurrent();
    bool new_orders(std::vector<Order>& orders);
    void pull_queued_orders(std::deque<Order>& q);
    void process_queued_orders();
    void terminate_processing();

    // Core matching
    //
    void match(Order& order, OrderBookPtr order_book);

    // Utility
    void print_trade(const Order& o1, const Order& o2, double price, int trade_quantity);
    void dump_orders();
  };

  typedef std::shared_ptr<MatchingEngine> MatchingEnginePtr;
} // namespace darkpool
