/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <unordered_map>
#include <functional>
#include <chrono>
#include <locale>
#include <iomanip>
#include <string>

namespace common {
  const char FIELD_SEP = ' ';

  std::string trim(const std::string& str);

  typedef std::chrono::high_resolution_clock clock_t;
  typedef std::unordered_map<std::string, std::function<void()>> CallbackMap;

  std::chrono::milliseconds get_current_milliseconds();

  std::pair<std::string, int> parse_ip_port(const std::string& arg);
  std::string collapse_line(const std::string& message);

  template <class T> std::string numberFormatWithCommas(T value, uint8_t precision = 0) {
    struct Numpunct : public std::numpunct<char> {
    protected:
      virtual char do_thousands_sep() const {
        return ',';
      }

      virtual std::string do_grouping() const {
        return "\03";
      }
    };

    std::stringstream ss;
    ss.imbue({ std::locale(), new Numpunct });
    ss << std::setprecision(precision) << std::fixed << value;
    return ss.str();
  }
} // namespace common
