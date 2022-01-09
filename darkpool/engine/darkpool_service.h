/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#pragma once

#include "darkpool/engine/routing/order_router.h"
#include "darkpool/engine/order_provider/order_provider.h"

namespace darkpool {

    class DarkPoolService {

        private:
            OrderProviderPtr order_provider;
            OrderRouterPtr router;
            RoutingStrategyPtr routing_strategy;

            std::ifstream file_input_stream;

            int concurrency;

        public:
            DarkPoolService(OrderProviderPtr order_provider, int concurrency);
            bool is_concurrent();
            void run();

    };


}
