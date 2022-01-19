/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */

#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include "common/model/order.h"

using namespace common::model;

namespace darkpool {
  class OrderProvider {
  public:
    virtual bool get_orders(std::vector<Order>& orders) = 0;
  };

  class InputStreamOrderProvider : public OrderProvider {
  protected:
    std::istream& input_stream;
    int batch_size;

  public:
    InputStreamOrderProvider(std::istream& input_stream, int batch_size);
    virtual bool get_orders(std::vector<Order>& orders);
  };

  class FilenameStreamOrderProvider : public InputStreamOrderProvider {

    std::ifstream file_input_stream;

  public:
    FilenameStreamOrderProvider(std::string filename, int batch_size);
    virtual ~FilenameStreamOrderProvider();
  };

  class StdinOrderProvider : public InputStreamOrderProvider {

  public:
    StdinOrderProvider(int batch_size);
    virtual ~StdinOrderProvider();
  };

  typedef std::shared_ptr<OrderProvider> OrderProviderPtr;
} // namespace darkpool
