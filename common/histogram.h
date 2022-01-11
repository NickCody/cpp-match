/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <tuple>
#include <numeric>
#include <type_traits>
#include <filesystem>

#include "common/common.h"

namespace common {
  template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>> class Histogram {

    const std::string HISTOGRAM_OUTPUT_DIRECTORY = "histogram";
    const std::string HISTOGRAM_EXTENSION = ".histogram";
    const std::string PER_SECOND_EXTENSION = ".bins";

    typedef std::tuple<std::chrono::milliseconds, T> Measurement;
    typedef std::vector<T> ValueArray;
    typedef std::tuple<double, double> Percentile;
    typedef std::vector<Percentile> PercentileArray;

    const std::vector<double> PERCENTILE_BINS = { 0.999, 0.99, 0.95, 0.90, 0.75, 0.5, 0.25 };

  private:
    std::string name;
    std::vector<Measurement> measurements;

  public:
    Histogram(const std::string& name)
        : name(name) {
      std::filesystem::create_directory(HISTOGRAM_OUTPUT_DIRECTORY);
    }

    ~Histogram() {}

    // record_value
    //

    void record_value(const T& v) { measurements.push_back(Measurement(get_current_milliseconds(), v)); }

    void record_value(std::chrono::milliseconds ms, const T& v) { measurements.push_back(ms, v); }

    // Per-second measurements
    //

    void write_sum_by_second(const std::string& notice) { write_sum_by_second(notice, name + PER_SECOND_EXTENSION); }

    void write_sum_by_second(const std::string& notice, const std::string& filename) {
      std::ofstream o(HISTOGRAM_OUTPUT_DIRECTORY + "/" + filename.c_str(), std::ofstream::out);
      o << notice << std::endl;
      write_sum_by_second(o);
      o.close();
    }

    void write_sum_by_second(std::ostream& o) {

      // typedef std::chrono::duration<long, std::ratio<1>> second_t;

      std::time_t current = 0;
      T total = 0;
      T grand_total = 0;
      bool initial = true;
      for (const Measurement& m : measurements) {
        std::time_t sec = std::chrono::duration_cast<std::chrono::seconds>(std::get<0>(m)).count();
        if (sec != current) {
          if (initial) {
            initial = false;
          } else {
            o << sec << " " << std::put_time(std::localtime(&sec), "%H:%M:%S") << " " << total << std::endl;
            grand_total += total;
            total = 0;
          }
          current = sec;
        }
        total += std::get<1>(m);
      }

      if (total > 0) {
        o << current << " " << std::put_time(std::localtime(&current), "%H:%M:%S") << " " << total << std::endl;
        grand_total += total;
      }

      o << "Grand total = " << std::fixed << std::setprecision(2) << grand_total << std::endl;
    }

    void write_max_by_second(const std::string& notice) { write_max_by_second(notice, name + PER_SECOND_EXTENSION); }

    // TODO: Almost identical to write_sum_by_second, should be collapsed/generalized
    //

    void write_max_by_second(const std::string& notice, const std::string& filename) {
      std::ofstream o(HISTOGRAM_OUTPUT_DIRECTORY + "/" + filename.c_str(), std::ofstream::out);
      o << notice << std::endl;
      write_max_by_second(o);
      o.close();
    }

    void write_max_by_second(std::ostream& o) {

      // typedef std::chrono::duration<long, std::ratio<1>> second_t;

      std::time_t current = 0;
      T maximum = 0;
      bool initial = true;
      for (const Measurement& m : measurements) {
        std::time_t sec = std::chrono::duration_cast<std::chrono::seconds>(std::get<0>(m)).count();
        if (sec != current) {
          if (initial) {
            initial = false;
          } else {
            o << sec << " " << std::put_time(std::localtime(&sec), "%H:%M:%S") << " " << maximum << std::endl;
            maximum = 0;
          }
          current = sec;
        }
        if (std::get<1>(m) > maximum)
          maximum = std::get<1>(m);
      }

      if (maximum > 0) {
        o << current << " " << std::put_time(std::localtime(&current), "%H:%M:%S") << " " << maximum << std::endl;
      }
    }

    // Percentiless
    //

    void write_percentiles() { write_percentiles(name + HISTOGRAM_EXTENSION); }

    void write_percentiles(const std::string& filename) {
      std::ofstream o(HISTOGRAM_OUTPUT_DIRECTORY + "/" + filename.c_str(), std::ofstream::out);
      write_percentiles(o);
      o.close();
    }

    void write_percentiles(std::ostream& o) {
      PercentileArray percentiles = calculate_percentiles(measurements);
      write_percentiles(o, measurements, percentiles);
    }

    PercentileArray calculate_percentiles(const std::vector<Measurement>& m) {
      PercentileArray percentiles;

      ValueArray values;
      std::transform(
          m.begin(), m.end(), std::back_inserter(values), [](const Measurement& m) { return std::get<1>(m); });

      std::sort(values.begin(), values.end());
      int vsize = values.size();

      if (vsize == 0)
        return percentiles;

      auto max_measurement = values[vsize - 1];
      auto min_measurement = values[0];
      // auto sum_measurement = std::accumulate(values.begin(), values.end(), 0);

      // push 100th percentile (max)
      //
      percentiles.push_back(Percentile(1.0, max_measurement));

      // Add bins
      //
      for (double p : PERCENTILE_BINS) {
        int index = (int)(p * values.size());
        percentiles.push_back(Percentile(p, values[index]));
      }
      // Push 0th percentile, min
      //
      percentiles.push_back(Percentile(0.0, min_measurement));

      return percentiles;
    }

    void write_percentiles(std::ostream& o, const std::vector<Measurement>& m, const PercentileArray& percentiles) {
      double basket = m.size() / 100.0;

      o << std::fixed;
      o << "Percentiles for [" << name << "]" << std::endl;
      o << "Total " << m.size() << " measurements; ";
      o << "Each percentile contains " << std::setprecision(2) << basket << " measurements; ";
      o << "MAX is percentile 1.000, MIN is percentile 0.000;" << std::endl;
      for (const Percentile& p : percentiles) {
        o << "  percentile " << std::setprecision(3) << std::get<0>(p) << " = " << std::setprecision(3)
          << std::get<1>(p) << std::endl;
      }
    }
  };
} // namespace common
