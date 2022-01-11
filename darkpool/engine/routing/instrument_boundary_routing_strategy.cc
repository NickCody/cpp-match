/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <algorithm>
#include "darkpool/engine/routing/routing_strategy.h"

namespace darkpool {
  InstrumentBoundaryRoutingStrategy::InstrumentBoundaryRoutingStrategy(std::vector<std::string> ordered_partitions,
                                                                       bool concurrent)
      : ordered_partitions(ordered_partitions),
        concurrent(concurrent) {
    create_engines();
  }

  InstrumentBoundaryRoutingStrategy::InstrumentBoundaryRoutingStrategy(int num_partitions, bool concurrent)
      : concurrent(concurrent) {
    std::string all_syms = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    num_partitions = std::max(1, std::min(num_partitions, (int)all_syms.size()));

    std::vector<std::string> partitions;
    const int skip = all_syms.size() / num_partitions;

    for (int i = 1; i <= num_partitions; i++) {
      char c = all_syms[(i * skip) - 1];
      ordered_partitions.push_back(std::string(1, c));
    }

    create_engines();
  }

  InstrumentBoundaryRoutingStrategy::~InstrumentBoundaryRoutingStrategy() {}

  void InstrumentBoundaryRoutingStrategy::create_engines() {
    if (ordered_partitions.size() == 0)
      throw std::invalid_argument("no ordered partitions");

    for (std::string partition : ordered_partitions) {
      engines.push_back(MatchingEnginePtr(new MatchingEngine(partition, concurrent)));
    }
  }

  // Super-simple partitioning scheme
  // n partitions
  //
  // Order is in a partition if instrument < partition string using standard string lexicographical ASCII ordering.
  // partition n is final bucket so partition letter is ignored.
  //

  MatchingEnginePtr InstrumentBoundaryRoutingStrategy::find_engine(const Order& order) {
    for (size_t i = 0; i < ordered_partitions.size() - 1; i++) {
      if (order.instrument < ordered_partitions[i])
        return engines[i];
    }

    return engines[ordered_partitions.size() - 1];
  }

  std::vector<MatchingEnginePtr>& InstrumentBoundaryRoutingStrategy::get_engines() { return engines; }
} // namespace darkpool