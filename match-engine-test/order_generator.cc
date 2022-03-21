/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <string>
#include <iostream>
#include <algorithm>
#include "order_generator.h"

using namespace common::model;

namespace testutil {
  OrderGenerator::OrderGenerator(size_t num_instruments, size_t instrument_length)
      : next_order_id(0) {
    for (size_t i = 0; i < num_instruments; i++) {
      std::string symbol = generate_random_string(instrument_length);
      std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);

      InstrumentPtr instrument = InstrumentPtr(new Instrument(symbol, generate_random_price()));

      instruments.push_back(instrument);
    }
  }

  std::string OrderGenerator::generate_random_string(int length) {
    char* r = new char[length + 1];
    for (int i = 0; i < length; i++) {
      r[i] = syms[(rand() % NUM_SYMS)];
    }

    r[length] = (char)0;

    return r;
  }

  double OrderGenerator::generate_random_price(int low, int high) {
    int r = ((rand() % (high - low)) + low);
    return (double)r;
  }

  int OrderGenerator::generate_random_quantity(int low, int high) {
    return (rand() % (high - low)) + low;
  }

  Order::SIDE OrderGenerator::generate_random_side() {
    if (rand() % 2 == Order::SIDE::BUY)
      return Order::SIDE::BUY;
    else
      return Order::SIDE::SELL;
  }

  InstrumentPtr OrderGenerator::pick_random_instrument() {
    return instruments[rand() % instruments.size()];
  }

  double OrderGenerator::pick_random_instrument_price(InstrumentPtr instrument, Order::SIDE side) {
    double price = instrument->last;
    double variation = generate_random_price(1, 6) - 3.0;
    if (side == Order::SIDE::BUY) {
      variation = -variation;
    }
    return price + variation;
  }

  OrderPtr OrderGenerator::generate_random_order() {
    auto instrument = pick_random_instrument();
    auto side = generate_random_side();
    auto order_id = next_order_id++;

    return OrderPtr(new Order(common::get_current_milliseconds(),
                              std::to_string(order_id),
                              side,
                              instrument->name,
                              generate_random_quantity(),
                              pick_random_instrument_price(instrument, side)));
  }
} // namespace testutil