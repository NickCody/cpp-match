/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */
#include <thread>

#include "darkpool/engine/darkpool_service.h"
#include "darkpool/engine/routing/routing_strategy.h"
#include "darkpool/engine/order_provider/order_provider.h"

namespace darkpool {
  DarkPoolService::DarkPoolService(OrderProviderPtr order_provider, int concurrency)
      : order_provider(order_provider),
        concurrency(concurrency) {

    // Setup routing strategy - hard-coded here but could be configurable
    //
    routing_strategy = RoutingStrategyPtr(new InstrumentBoundaryRoutingStrategy(concurrency, is_concurrent()));

    // Set up Router
    //
    router = OrderRouterPtr(new OrderRouter(order_provider, routing_strategy));
  }

  bool DarkPoolService::is_concurrent() { return concurrency > 0; }

  void DarkPoolService::run() {

    std::vector<std::thread> threads;

    if (is_concurrent()) {
      for (MatchingEnginePtr engine : routing_strategy->get_engines()) {
        threads.push_back(std::thread(&MatchingEngine::process_queued_orders, &(*engine)));
      }
    }

    // std::cout << "Created " << threads.size() << " threads." << std::endl;

    router->event_loop();

    if (is_concurrent()) {
      for (MatchingEnginePtr engine : routing_strategy->get_engines()) {
        engine->terminate_processing();
      }

      // Wait for threads to terminate
      //
      for (auto& t : threads) {
        if (t.joinable())
          t.join();
      }
    }

    // REQUIREMENT: Dump open orders at end
    //
    for (MatchingEnginePtr engine : routing_strategy->get_engines()) {
      engine->dump_orders();
    }
  }
} // namespace darkpool