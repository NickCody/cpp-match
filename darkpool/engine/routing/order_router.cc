/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#include <list>
#include <future>

#include "common/stopwatch.h"
#include "darkpool/engine/order_provider/order_provider.h"
#include "darkpool/engine/routing/order_router.h"
#include "darkpool/engine/routing/routing_strategy.h"

namespace darkpool {

OrderRouter::OrderRouter(OrderProviderPtr order_provider, RoutingStrategyPtr routing_strategy) 
    : order_provider(order_provider), routing_strategy(routing_strategy),
    input_thread_lock_latencies_micros("InputThreadLockLatenciesMicros") {
}

OrderRouter::~OrderRouter() {
    // For input thread, we want to see latency distribution (percentiles) + total time spent in locks
    //
    input_thread_lock_latencies_micros.write_sum_by_second("Total micros spent waiting for locks (per second bin):");
    input_thread_lock_latencies_micros.write_percentiles();
}

void OrderRouter::event_loop() {

    bool more_orders = true;
    while(more_orders) {
        
        std::vector<Order> orders;
        more_orders = order_provider->get_orders(orders);

        if (orders.size() == 0)
            return;

        // Collect orders into batches for each matching engine
        //
        std::map<MatchingEnginePtr,std::vector<Order>> groups;
        for (Order& o : orders) {
            MatchingEnginePtr engine = routing_strategy->find_engine(o);
            if (groups.find(engine) == groups.end())
                groups[engine] = std::vector<Order>();
            groups[engine].push_back(std::move(o));
        }

        for (std::pair<MatchingEnginePtr, std::vector<Order>> engine : groups) {

            // Time how long we wait for locks in input thread
            //

            common::Stopwatch<double,std::chrono::microseconds> stopwatch;
            stopwatch.start();
            engine.first->new_orders(engine.second);
            input_thread_lock_latencies_micros.record_value(stopwatch.stop());

            if (!engine.first->is_concurrent()) {
                engine.first->process_queued_orders();
            }
        }

    }
}

} // namespace darkpool