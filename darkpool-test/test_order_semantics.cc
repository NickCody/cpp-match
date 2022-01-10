/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#include <cstdlib>
#include <cassert>
#include <string> 
#include <vector>
#include <bits/stdc++.h> 

#include "common/common.h"
#include "common/stopwatch.h"
#include "common/histogram.h"
#include "common/model/order.h"
#include "common/model/order_factory.h"

#include "test_order_semantics.h"
#include "order_generator.h"

void pass() {
    std::cout << "pass" << std::endl;
}

void testOrderFromString(const std::string& os) {
    try {
        auto order = OrderFactory::from_string(os);
        std::cout << order << std::endl;
    } catch (std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void testOrderCreation() {
    try {
        auto order = OrderFactory::from_string("plu401 SELL ETHUSD   5   170");
        
        assert(order.order_id.compare("plu401") == 0);
        assert(order.side == Order::SIDE::SELL);
        assert(order.instrument.compare("ETHUSD") == 0);
        assert(order.remaining_quantity == 5);
        assert(order.price == 170.0);

        pass();

    } catch (std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void testOrderComparison() {

    OrderPtr o1(new Order(common::get_current_milliseconds(), "1", Order::SIDE::BUY, "XOXO", 100L, 1000.0));
    OrderPtr o1a = o1;
    OrderPtr o2(new Order(common::get_current_milliseconds(), "2", Order::SIDE::SELL, "YNYM", 200L, 3000.0));

    assert(*o1 == *o1a);
    assert(*o1 != *o2);

    pass();
}

void testOrderHeapAllocation(int count) {
    common::Stopwatch<double,std::chrono::seconds> stopwatch;
 
    stopwatch.print_duration("Time (seconds) for creating objects: ", [count]() {
        std::vector<OrderPtr> orders;
        for (int i=0; i < count; i++) {
            std::string order_id = std::to_string(i);
            orders.push_back(OrderPtr(new Order(common::get_current_milliseconds(), order_id, Order::SIDE::BUY, "XOXO", 100L, 1000.0)));
        }
    });

    pass();
}

void testOrderGenerator(int count, int num_instruments) {
    testutil::OrderGenerator order_generator(num_instruments);
    testutil::DupeMap dupe_map;

    for (int i=0; count == -1 || i < count; i++) {
        OrderPtr order = order_generator.generate_random_order(dupe_map);
        std::cout << order->to_string(' ') << std::endl;
        dupe_map[order->order_id] = order;
    }

    std::cerr << "Generated " << count << " test orders using " << num_instruments << " instruments." << std::endl;
}


void testOrderRanking() {
    auto millis = common::get_current_milliseconds();
    std::chrono::milliseconds one(1);
    std::chrono::milliseconds two(2);

    // orders with same price are ranked by timestamp
    {
        auto o1 = Order(millis, "1", Order::SIDE::BUY, "XOXO", 100L, 1000.0);
        auto o2 = Order(millis+one, "2", Order::SIDE::BUY, "XOXO", 200L, 1000.0);
        auto o3 = Order(millis+two, "2", Order::SIDE::BUY, "XOXO", 200L, 1000.0);
        assert( o2 < o1 );
        assert( o3 < o1 );
        assert( o3 < o2 );
    }

    // buys are ranked by price, lower price is ranked lower
    {
        auto o1 = Order(millis, "1", Order::SIDE::BUY, "XOXO", 100L, 1000.0);
        auto o2 = Order(millis, "2", Order::SIDE::BUY, "XOXO", 200L, 1001.0);
        assert( o1 < o2 );
        assert( !(o2 < o1) );
    }

    // sells are ranked by price, higher price is ranked lower
    {
        auto o1 = Order(millis, "1", Order::SIDE::SELL, "XOXO", 100L, 1000.0);
        auto o2 = Order(millis, "2", Order::SIDE::SELL, "XOXO", 200L, 1001.0);
        assert( o2 < o1 );
        assert( !(o1 < o2) );
    }

    pass();
}

void testOrderHeapOrdering() {
    auto now = common::get_current_milliseconds();

    // works for heaps of Order
    {
        std::vector<Order> orders;

        orders.push_back(Order(now, "0", Order::SIDE::SELL, "XOXO", 100L, 1000.0));
        orders.push_back(Order(now, "1", Order::SIDE::SELL, "XOXO", 200L, 1003.0));
        orders.push_back(Order(now, "2", Order::SIDE::SELL, "XOXO", 300L, 1002.0));
        orders.push_back(Order(now, "3", Order::SIDE::SELL, "XOXO", 400L, 1004.0));
        orders.push_back(Order(now, "4", Order::SIDE::SELL, "XOXO", 500L, 1001.0));

        std::make_heap(orders.begin(), orders.end());

        assert(orders[0].remaining_quantity == 100);
        assert(orders[1].remaining_quantity == 500);
        assert(orders[2].remaining_quantity == 300);
        assert(orders[3].remaining_quantity == 400);
        assert(orders[4].remaining_quantity == 200);
    }

    // and also heaps of OrderPtr
    {
        std::vector<OrderPtr> orders;

        orders.push_back(OrderPtr(new Order(now, "0", Order::SIDE::SELL, "XOXO", 100L, 1000.0)));
        orders.push_back(OrderPtr(new Order(now, "1", Order::SIDE::SELL, "XOXO", 200L, 1003.0)));
        orders.push_back(OrderPtr(new Order(now, "2", Order::SIDE::SELL, "XOXO", 300L, 1002.0)));
        orders.push_back(OrderPtr(new Order(now, "3", Order::SIDE::SELL, "XOXO", 400L, 1004.0)));
        orders.push_back(OrderPtr(new Order(now, "4", Order::SIDE::SELL, "XOXO", 500L, 1001.0)));

        std::make_heap(orders.begin(), orders.end());

        assert(orders[0]->remaining_quantity == 100);
        assert(orders[1]->remaining_quantity == 500);
        assert(orders[2]->remaining_quantity == 300);
        assert(orders[3]->remaining_quantity == 400);
        assert(orders[4]->remaining_quantity == 200);
    }

    pass();
}

void testIntegerHistograms() {
    common::Histogram<int> integers("integers");
    for(int i=0; i < 10000; i++) {
        integers.record_value(rand() % 300);
    }

    std::stringstream result;
    integers.write_percentiles(result);

    std::cout << result.str() << std::endl;
}

void testDoubleHistograms() {
    common::Histogram<double> integers("doubles");
    for(int i=0; i < 10000; i++) {
        integers.record_value((rand() % 100)/100.0);
    }

    std::stringstream result;
    integers.write_percentiles(result);

    std::cout << result.str() << std::endl;
}