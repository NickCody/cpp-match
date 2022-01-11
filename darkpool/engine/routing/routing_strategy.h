/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <vector>
#include <string>
#include "common/model/order.h"
#include "darkpool/engine/matching_engine.h"

namespace darkpool {
  struct RoutingStrategy {
    virtual MatchingEnginePtr find_engine(const Order& order) = 0;
    virtual std::vector<MatchingEnginePtr>& get_engines() = 0;
  };

  class InstrumentBoundaryRoutingStrategy : public RoutingStrategy {
  private:
    std::vector<std::string> ordered_partitions;
    std::vector<MatchingEnginePtr> engines;
    void create_engines();

    const bool concurrent;

  public:
    InstrumentBoundaryRoutingStrategy(std::vector<std::string> ordered_partitions, bool concurrent);
    InstrumentBoundaryRoutingStrategy(int num_partitions, bool concurrent);
    virtual ~InstrumentBoundaryRoutingStrategy();
    virtual MatchingEnginePtr find_engine(const Order& order);
    virtual std::vector<MatchingEnginePtr>& get_engines();
  };

  typedef std::shared_ptr<RoutingStrategy> RoutingStrategyPtr;

} // namespace darkpool
