/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <functional>

#include "common/common.h"
#include "common/histogram.h"

namespace common {

  // Comvenient way to time a lambda/code block
  //
  // D is std::chrono::milliseconds, microseconds, etc.

  template <typename T, typename D> class Stopwatch {
  private:
    common::Histogram<T>* optional_histogram;
    std::chrono::time_point<clock_t> start_time;

  public:
    Stopwatch(Histogram<T>* histogram = nullptr)
        : optional_histogram(histogram) {
    }

    double duration(std::function<void(void)> f) {
      start();

      f();

      auto d = stop();

      if (optional_histogram != nullptr)
        optional_histogram->record_value(d);

      return d;
    }

    void print_duration(const std::string& msg, int width, std::function<void(void)> f) {
      double t = duration(f);
      std::cout << msg << std::setw(width) << common::numberFormatWithCommas<>(t, 0) << std::endl;
    }

    void print_duration(const std::string& msg, std::function<void(void)> f) {
      double t = duration(f);
      std::cout << msg << std::setw(12) << common::numberFormatWithCommas<>(t, 0) << std::endl;
    }

    void start() {
      start_time = clock_t::now();
    }

    double stop() {
      std::chrono::time_point<clock_t> stop_time(clock_t::now());
      return std::chrono::duration_cast<D>(stop_time - start_time).count();
    }
  };
} // namespace common