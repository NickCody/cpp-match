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
      // std::vector<std::string> op = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M" };
      return {
        common::get_current_milliseconds(), tokens[0], tokens[1], tokens[2], std::stoi(tokens[3]), std::stod(tokens[4])
        //       common::get_current_milliseconds(), "", "", op[rand() % op.size()], 0, 0.0
      };
    }
  };
} // namespace common::model
