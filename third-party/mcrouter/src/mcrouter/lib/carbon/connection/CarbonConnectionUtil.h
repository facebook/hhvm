/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <stdexcept>
#include <string>

#include <mcrouter/lib/Reply.h>

namespace carbon {

class CarbonConnectionRecreateException : public std::runtime_error {
 public:
  explicit CarbonConnectionRecreateException(const std::string& what)
      : std::runtime_error(what) {}
};

class CarbonConnectionException : public std::runtime_error {
 public:
  explicit CarbonConnectionException(const std::string& what)
      : std::runtime_error(what) {}
};

template <class Request>
using RequestCb =
    std::function<void(const Request&, facebook::memcache::ReplyT<Request>&&)>;
} // namespace carbon
