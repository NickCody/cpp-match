/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 *
 */
#pragma once

#include <string>

namespace darkpool {
  struct Config {
    int concurrency;
    int batch_size;
    std::string filename;
  };
} // namespace darkpool
