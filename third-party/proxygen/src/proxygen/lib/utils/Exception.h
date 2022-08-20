/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <string>
#include <utility>

namespace proxygen {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4521) // 'proxygen::Exception': multiple copy ctors
#endif
/**
 * Base class for all exceptions.
 */
class Exception : public std::exception {
 public:
  explicit Exception(std::string const& msg);
  explicit Exception(const char* msg);
  Exception(const Exception&);
  Exception(Exception& other) : Exception(folly::as_const(other)) {
  }
  Exception(Exception&&) noexcept;

  template <typename... Args>
  explicit Exception(Args&&... args)
      : msg_(folly::to<std::string>(std::forward<Args>(args)...)), code_(0) {
  }

  ~Exception(void) noexcept override {
  }

  // std::exception methods
  const char* what(void) const noexcept override;

  // Accessors for code
  void setCode(int code) {
    code_ = code;
  }

  int getCode() const {
    return code_;
  }

  // Accessors for ProxygenError
  bool hasProxygenError() const {
    return (proxygenError_ != kErrorNone);
  }

  void setProxygenError(ProxygenError proxygenError) {
    proxygenError_ = proxygenError;
  }

  ProxygenError getProxygenError() const {
    return proxygenError_;
  }

 private:
  const std::string msg_;
  int code_;
  ProxygenError proxygenError_{kErrorNone};
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace proxygen
