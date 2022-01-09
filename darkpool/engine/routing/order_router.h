/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#pragma once

#include <memory>
#include <future>

#include "common/histogram.h"
#include "common/model/order.h"
#include "darkpool/engine/routing/routing_strategy.h"
#include "darkpool/engine/order_provider/order_provider.h"

namespace darkpool {

    class OrderRouter {

        public:
            OrderRouter(OrderProviderPtr order_provider, RoutingStrategyPtr routing_strategy);
            ~OrderRouter();
            void event_loop();

        private:
            OrderProviderPtr order_provider;
            RoutingStrategyPtr routing_strategy;
            common::Histogram<double> input_thread_lock_latencies_micros;

    };

    typedef std::shared_ptr<OrderRouter> OrderRouterPtr;
}
