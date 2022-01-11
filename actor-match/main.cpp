#include <chrono>
#include <iostream>
#include <string>

#include "caf/actor_ostream.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"

#include "common/model/order.h"
#include "common/model/order_factory.h"

using namespace caf;
using namespace common::model;

behavior process_order(event_based_actor* self) {
  return { [=](const std::string& order) {
            aout(self) << "Received order: " << order << std::endl;
            aout(self) << "Received order: " << order << std::endl;
            aout(self) << "Received order: " << order << std::endl;
            aout(self) << "Received order: " << order << std::endl;
            aout(self) << "Received order: " << order << std::endl;
          },
           [=](int count) { aout(self) << "Received count: " << count << std::endl; } };
}

void read_stdin(event_based_actor* self, const actor& po) {
  auto one_sec = std::chrono::seconds(1);
  for (std::string line; std::getline(std::cin, line);) {
    Order order = OrderFactory::from_string(line);
    self->request(po, one_sec, order.to_string());
  }
}

void caf_main(actor_system& sys) {
  auto po = sys.spawn(process_order);
  sys.spawn(read_stdin, po);
  std::cout << sys.has_middleman() << std::endl;

  // for (std::string line; std::getline(std::cin, line);) {
  //   Order order = OrderFactory::from_string(line);
  //   std::cout << "Received order: " << order.to_string() << std::endl;
  // }
}

CAF_MAIN()