/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include "common/model/order.h"
#include <sstream>
#include <random>
#include <string>
#include <vector>

namespace common::model {
  class OrderFactory {

  public:
    static Order from_string(std::string s) {
      std::vector<std::string> tokens;
      std::stringstream input(s);
      std::string current;

      while (std::getline(input, current, ' ')) {
        if (current.length() > 0) {
          tokens.push_back(current);
        }
      }

      if (tokens.size() != 5) {
        std::stringstream error_text;
        error_text << "[" + s + "] is not a valid order input." << std::ends;

        throw std::invalid_argument(error_text.str());
      }
      auto side = tokens[1][0] == 'B' ? Order::SIDE::BUY : Order::SIDE::SELL;

      return Order{ .timestamp = common::get_current_milliseconds(),
                    .order_id = tokens[0],
                    .side = side,
                    .instrument = tokens[2],
                    .remaining_quantity = std::stoi(tokens[3]),
                    .price = std::stod(tokens[4]) };
    }
  };
} // namespace common::model
