#pragma once

#include <caf/typed_response_promise.hpp>
#include <cstdio>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

#include "include/fmt//format.h"
#include "caf/all.hpp"
#include "caf/policy/select_any.hpp"
#include "caf/policy/select_all.hpp"
#include "actor_types.h"

#include "common/model/order.h"
#include "order_book_actor.h"

namespace actor_match {

  using namespace std;
  using namespace common::model;
  using book_actor_map_t = map<string, caf::actor>;

  class RouterActor : public caf::event_based_actor {

  public:
    explicit RouterActor(caf::actor_config& cfg, caf::group grp)
        : event_based_actor(cfg) {
      books_group = grp;
    }

    vector<caf::actor> vector_from_map(const book_actor_map_t& b_map) {
      vector<caf::actor> book_actors;
      book_actors.resize(b_map.size());
      for (auto& entry : b_map) {
        book_actors.push_back(entry.second);
      }
      return book_actors;
    }

    caf::behavior make_behavior() override {
      return {
        [this](new_order, Order order) -> bool {
          if (!book_map.contains(order.instrument)) {
            book_map[order.instrument] = spawn_in_group<OrderBookActor>(books_group, order.instrument);
          }

          send(book_map[order.instrument], new_order_v, move(order));
          return true;
        },
        [this](dump_book_summary, size_t count) -> caf::typed_response_promise<size_t> {
          vector<caf::actor> book_actors = vector_from_map(book_map);
          auto rp = make_response_promise<size_t>();
          fan_out_request<caf::policy::select_all, caf::message_priority::high>(book_actors, caf::infinite, get_book_stats())
              .then(
                  [count, rp](std::vector<BookStats> xs) mutable {
                    size_t total = 0;
                    stringstream book_stats;
                    for (auto stats : xs) {
                      book_stats << fmt::format(
                                        "  {}: total {} orders ({}, {})", stats.instrument, stats.total_orders, stats.open_buys, stats.open_sells)
                                 << endl;
                      total += stats.total_orders;
                    }
                    fmt::print(stderr, "After {} orders, stats =>\n{}", count, book_stats.str());

                    rp.deliver(total);
                  },
                  [](caf::error&) mutable {});
          return rp;
        },
      };
    }

  private:
    map<string, caf::actor> book_map;
    caf::group books_group;
  };

} // namespace actor_match