/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <iostream>
#include <vector>

#include "common/common.h"
#include "test_order_semantics.h"

common::CallbackMap commands;

void usage(const std::string& name, const std::string& extra = "") {
  if (!extra.empty()) {
    std::cerr << extra << std::endl;
  }

  std::cerr << "Usage: " << name << " <command>" << std::endl;
  std::cerr << "Where commands are:" << std::endl;
  std::cerr << "  all" << std::endl;
  for (const auto& pair : commands) {
    std::cerr << "  " << pair.first << std::endl;
  }
}

void setup_commands(int argc, char** argv) {

  commands["testOrderFromString"] = [argc, argv]() {
    std::string order_string;
    if (argc < 3) {
      order_string = "12345 BUY BTCUSD   5   10000";
    } else {
      order_string = argv[2];
    }
    testOrderFromString(order_string);
  };

  commands["testOrderCreation"] = testOrderCreation;
  commands["testOrderComparison"] = testOrderComparison;

  commands["testOrderHeapAllocation"] = [argc, argv]() {
    int count;
    if (argc < 3) {
      count = 1000000;
    } else {
      count = std::stoi(argv[2]);
    }
    testOrderHeapAllocation(count);
  };

  commands["testOrderGenerator"] = [argc, argv]() {
    int count = -1;

    if (argc >= 3) {
      count = std::stoi(argv[2]);
    }

    int num_instruments;
    if (count == -1)
      num_instruments = 100;
    else
      num_instruments = std::max(1, count / 100);

    if (argc >= 4) {
      num_instruments = std::stoi(argv[3]);
    }

    testOrderGenerator(count, num_instruments);
  };

  commands["testOrderRanking"] = testOrderRanking;
  commands["testOrderHeapOrdering"] = testOrderHeapOrdering;
  commands["testIntegerHistograms"] = testIntegerHistograms;
  commands["testDoubleHistograms"] = testDoubleHistograms;
}

int main(int argc, char** argv) {
  setup_commands(argc, argv);

  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  std::string command(argv[1]);

  if (command == "all") {
    for (const auto& pair : commands) {
      pair.second();
    }
  } else if (commands.find(command) != commands.end())
    commands[command]();
  else {
    usage(argv[0], std::string(command) + " invalid command");
  }
}