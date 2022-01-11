/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "darkpool/config/config.h"
#include "common/model/order.h"
#include "darkpool/engine/darkpool_service.h"
#include "darkpool/engine/order_provider/order_provider.h"

using namespace common::model;

void usage() {
  std::cout << "DarkPool" << std::endl;
  std::cout << "Usage: DarkPool [batch-size] [concurrency] [filename]" << std::endl;
  std::cout << "    `filename` can be blank or - for stdin" << std::endl;
  std::cout << "    `batch-size` specified number of orders read before dispatching to MatchingEngines" << std::endl;
  std::cout << "    `concurrency` can be:" << std::endl;
  std::cout << "      0 for single-threaded mode" << std::endl;
  std::cout << "      n for n worker nodes (1 means one input thread + 1 matcher thread)" << std::endl;
}

// TODO: I'd normally use getopt() for a more robust CLI interface
//

darkpool::Config parse_command_line(int argc, char** argv) {
  darkpool::Config config;

  try {
    if (argc > 1)
      config.batch_size = std::stoi(argv[1]);
    else
      config.batch_size = 1;

    if (argc > 2)
      config.concurrency = std::stoi(argv[2]);
    else
      config.concurrency = 0;

    if (argc > 3)
      config.filename = argv[3];
    else
      config.filename = "-";

  } catch (std::invalid_argument& e) {
    std::cout << "Invalid parameter: " << e.what() << std::endl;
    usage();
    exit(1);
  }

  return config;
}

darkpool::OrderProviderPtr create_order_provider(darkpool::Config config) {
  try {
    if (config.filename.compare("-") != 0) {
      try {
        return darkpool::OrderProviderPtr(
            new darkpool::FilenameStreamOrderProvider(config.filename, config.batch_size));
      } catch (const std::invalid_argument& e) {
        std::cerr << "Received " << e.what() << " when trying to open " << config.filename << std::endl;
        exit(EXIT_FAILURE);
      }
    } else {
      return darkpool::OrderProviderPtr(new darkpool::StdinOrderProvider(config.batch_size));
    }
  } catch (std::invalid_argument& e) {
    std::cout << "Invalid parameter." << std::endl;
    usage();
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {

  darkpool::Config config = parse_command_line(argc, argv);
  darkpool::OrderProviderPtr order_provider = create_order_provider(config);

  darkpool::DarkPoolService service(order_provider, config.concurrency);
  service.run();

  return EXIT_SUCCESS;
}
