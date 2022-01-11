/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>

#include "common.h"

namespace common {
  std::string trim(const std::string& str) {
    const size_t len = str.length();
    size_t a = 0, b = len - 1;
    char c;
    while (a < len && ((c = str.at(a)) == ' ' || c == '\t'))
      a++;
    while (b > a && ((c = str.at(b)) == ' ' || c == '\t'))
      b--;
    return str.substr(a, 1 + b - a);
  }

  std::chrono::milliseconds get_current_milliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
  }

  // parse_ip_port()
  //
  // Parses ip:port string into components

  std::pair<std::string, int> parse_ip_port(const std::string& arg) {
    std::istringstream target_arg(arg);
    std::string s;
    std::vector<std::string> ip_port;

    while (std::getline(target_arg, s, ':')) {
      ip_port.push_back(s);
    }

    if (ip_port.size() != 2) {
      throw std::invalid_argument("ip:port could not be parsed.");
    }

    return { ip_port[0], std::stoi(ip_port[1]) };
  }

  std::string collapse_line(const std::string& message) {
    std::string oneline = message;
    std::replace(oneline.begin(), oneline.end(), '\n', ',');
    return oneline;
  }
} // namespace common