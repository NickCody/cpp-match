/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#include <sstream>
#include "darkpool/engine/order_provider/order_provider.h"
#include "common/model/order_factory.h"

namespace darkpool {
  InputStreamOrderProvider::InputStreamOrderProvider(std::istream& input_stream, int batch_size)
      : input_stream(input_stream),
        batch_size(batch_size) {}

  // TODO: Is this the right way to std::move() into a parameter?

  bool InputStreamOrderProvider::get_orders(std::vector<Order>& orders) {
    std::string line;

    int num_orders = batch_size;
    while (num_orders-- > 0) {
      if (std::getline(input_stream, line)) {
        try {
          orders.push_back(std::move(OrderFactory::from_string(line)));
        } catch (std::invalid_argument& e) {
          std::stringstream bad_data;
          bad_data << "While reading from input, encountered a bad input line: " << e.what() << std::ends;
          throw std::invalid_argument(bad_data.str());
        }

      } else {
        return false;
      }
    }

    return true;
  }
} // namespace darkpool