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
  StdinOrderProvider::StdinOrderProvider(int batch_size)
      : InputStreamOrderProvider(std::cin, batch_size) {}

  StdinOrderProvider::~StdinOrderProvider() {}
} // namespace darkpool