/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#ifndef __ORDER_GENERATOR_H__
#define __ORDER_GENERATOR_H__

#include <map>
#include <cstdlib>
#include "common/model/order.h"

using namespace common::model;

// IMPLEMENTATION NOTES
// =-==-=-==-=-===-=-=-==-=-=-=
//
// This order generator creates orders based around a static set of instruments
// Prices are fixed per instrument (like last price) and orders are generated around this price.
//
// The investment in time should pay off when it comes time to do realistic performance testing.
//

namespace testutil {
  struct Instrument {
    std::string name;
    double last;

    Instrument(std::string name, double last)
        : name(name),
          last(last) {}
  };

  typedef std::shared_ptr<Instrument> InstrumentPtr;
  typedef std::map<std::string, OrderPtr> DupeMap;

  class OrderGenerator {
    // Static API
    //
    static constexpr char syms[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static constexpr int NUM_SYMS = 62;

    static std::string generate_random_string(int length);

    static double generate_random_price(int low = 1, int high = 10000);
    static int generate_random_quantity(int low = 1, int high = 10);
    static Order::SIDE generate_random_side();

    std::vector<InstrumentPtr> instruments;

  public:
    // Stateful API dependent on static instrument set
    //
    OrderGenerator(int num_instruments = 10, int instrument_length = 5);
    InstrumentPtr pick_random_instrument();
    double pick_random_instrument_price(InstrumentPtr instrument, Order::SIDE side);
    OrderPtr generate_random_order(DupeMap& dupe_map);
  };
} // namespace testutil

#endif // __ORDER_GENERATOR_H__