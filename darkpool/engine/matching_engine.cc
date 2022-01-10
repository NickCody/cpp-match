/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#include <iostream>
#include <chrono>
#include <thread>
#include "common/stopwatch.h"
#include "darkpool/engine/matching_engine.h"

using namespace common::model;

namespace darkpool {

MatchingEngine::MatchingEngine(const std::string& name, bool concurrent) 
    : name(name), 
    terminate(false), 
    concurrent(concurrent), 
    orders_processed(std::string("MatchingEngine-OrdersProcessed-") + name), 
    match_latencies_micros(std::string("MatchingEngine-match-LatenciesMicros-") + name),
    lock_latencies_micros(std::string("MatchingEngine-Lock-LatenciesMicros-") + name),
    order_book_sizes(std::string("MatchingEngine-OrderBookSizes-") + name) {
}

MatchingEngine::~MatchingEngine() {
    
    // For orders processed, we're only interested in raw counts so don't print percentiles, just bin counts
    orders_processed.write_sum_by_second("Total orders processed (per second bin)");

    // for match latencies, we should get percentiles + bin stats
    match_latencies_micros.write_sum_by_second("Total micros spent in match() function (per second bin)");
    match_latencies_micros.write_percentiles();

    // for match engine thread lock stats, print percentiles + bin stats for total time spent getting locks
    //
    lock_latencies_micros.write_sum_by_second("Total micros spent waiting for locks (per second bin):");
    lock_latencies_micros.write_percentiles();

    // Report order book sizes per second
    order_book_sizes.write_max_by_second("Total (average) orders managed (buys/sells) by each order book in this engine (per second bin):");
}

bool MatchingEngine::is_concurrent() {
    return concurrent;
}

bool MatchingEngine::new_orders(std::vector<Order>& orders) {
    if (concurrent)
        order_mutex.lock();

    for(Order& o : orders) {
        incoming_orders.push_back(std::move(o));
    }

    if (concurrent)
        order_mutex.unlock();
    
    return true;
}

void MatchingEngine::pull_queued_orders(std::deque<Order>& q) {

    if (concurrent) {
        common::Stopwatch<long,std::chrono::microseconds> stopwatch(&lock_latencies_micros);
        stopwatch.duration( [this]() { order_mutex.lock(); });
    }

    std::copy(incoming_orders.begin(), incoming_orders.end(), std::back_inserter(q));
    incoming_orders = std::deque<Order>();

    if (concurrent)
        order_mutex.unlock();

}

void MatchingEngine::process_queued_orders() {

    while(true) {

        std::deque<Order> internal_queue;

        pull_queued_orders(internal_queue);

        //std::cout << "MatchingEngine(" << name << ") processing " << internal_queue.size() << " orders." << std::endl;

        for(Order& order : internal_queue) {

            auto obi = order_books.find(order.instrument);
            OrderBookPtr order_book;

            if (obi == order_books.end()) {
                // first order ever, no matching
                order_book = OrderBookPtr(new OrderBook());
                order_books[order.instrument] = order_book;
                order_book->add_to_book(order);
            } else {
                order_book = obi->second;

                common::Stopwatch<long,std::chrono::microseconds> stopwatch;
                stopwatch.start();
                match(order, order_book);
                match_latencies_micros.record_value(stopwatch.stop());
            }

            orders_processed.record_value(1);
        }

        // Calculate average order book sizes
        calculate_order_book_sizes();

        // Waiting on termination command, but ensure we have no remaining orders before quitting
        //
        if (concurrent && terminate.load()) {
            int last_orders;
            order_mutex.lock();
            last_orders = incoming_orders.size();
            order_mutex.unlock();
            if (last_orders == 0) {
                break;
            }
        } else if (!concurrent) {
            return;
        }
    }
}

void MatchingEngine::calculate_order_book_sizes() {
    int sum_orders = 0;
    int num_books = 0;
    for (const auto& o : order_books) {
        num_books++;
        sum_orders += std::get<1>(o)->get_order_book_size();
    }
    if (num_books > 0)
        order_book_sizes.record_value(sum_orders / num_books);
}

void MatchingEngine::terminate_processing() {
    terminate.store(true);
}

void MatchingEngine::match(Order& order, OrderBookPtr order_book) {

    // NOTE: Artificial delay for concurrency testing
    //
    // using namespace std::chrono_literals;
    // std::this_thread::sleep_for(10ms);

    while (order.remaining_quantity > 0) {
        // If no contras, just add order and return
        if ( !order_book->has_contra_orders(order) ) {
            order_book->add_to_book(order);
            return;
        }

        auto& contra_order = order_book->get_contra_orders(order).front();

        if ( (order.side == Order::SIDE::BUY && contra_order.price <= order.price)
             ||
             (order.side == Order::SIDE::SELL && contra_order.price >= order.price))
        {
            int trade_quantity = std::min(order.remaining_quantity, contra_order.remaining_quantity);
            double price = contra_order.price;

            // Print the trade
            //
            print_trade(order, contra_order, price, trade_quantity);

            order.remaining_quantity -= trade_quantity;
            contra_order.remaining_quantity -= trade_quantity;
            
            order_book->purge();
        } else {
            break;
        }

    }

    // If order still has quantity, add it to the book
    if (order.remaining_quantity > 0)
        order_book->add_to_book(order);
}

void MatchingEngine::print_trade(const Order& o1, const Order& o2, double price, int trade_quantity) {
    // TRADE BTCUSD abe14 12345 5 10000
    std::cout << "TRADE " << o1.instrument << " " << o1.order_id << " " << o2.order_id << " " << trade_quantity << " " << price << std::endl;
}

void MatchingEngine::dump_orders() {
    for (auto ob : order_books) {
        ob.second->print_books();
    }
}

} // namespace darkpool