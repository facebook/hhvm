/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/OpenSSL.h>
#include <glog/logging.h>

namespace proxygen {

class ProxygenSSL {
 public:
  static int getSSLAppContextConfigIndex() {
    static auto index = [] {
      auto idx =
          SSL_CTX_get_ex_new_index(0,
                                   (void*)"proxygen client context config",
                                   nullptr,
                                   nullptr,
                                   nullptr);
      CHECK(idx >= 0);
      return idx;
    }();
    return index;
  }

  static int getSSLAppContextStatsIndex() {
    static auto index = [] {
      auto idx = SSL_CTX_get_ex_new_index(
          0, (void*)"proxygen wangle::SSLStats", nullptr, nullptr, nullptr);
      CHECK(idx >= 0);
      return idx;
    }();
    return index;
  }
};

} // namespace proxygen
