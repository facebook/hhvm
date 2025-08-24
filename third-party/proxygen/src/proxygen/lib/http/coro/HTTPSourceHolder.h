/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"

namespace proxygen::coro {

// A wrapper around an HTTPSource which ensures that either an EOM or error is
// read, or stopReading is called on the source.
class HTTPSourceHolder : public HTTPSourceFilter {
 public:
  HTTPSourceHolder() = default;

  /* implicit */ HTTPSourceHolder(HTTPSource* source)
      : HTTPSourceFilter(source) {
  }

  // Set move constructor and assignment operator overload
  HTTPSourceHolder(HTTPSourceHolder&& moved) noexcept {
    setSource(moved.release());
  }

  HTTPSourceHolder& operator=(HTTPSourceHolder&& moved) noexcept {
    setSource(moved.release());
    return *this;
  }

  // Delete implicit copy constructor and assignment operator overload
  HTTPSourceHolder(const HTTPSourceHolder&) = delete;
  HTTPSourceHolder& operator=(const HTTPSourceHolder&) = delete;
};

} // namespace proxygen::coro
