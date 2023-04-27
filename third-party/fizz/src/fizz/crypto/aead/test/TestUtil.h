/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>
#include <functional>

namespace fizz {
namespace test {

using BufCreator =
    std::function<std::unique_ptr<folly::IOBuf>(size_t len, size_t bufNum)>;

// Creates an IOBuf using an exact size (rather than the built in size
// heuristic) Useful for specifying exact sizes for tests.
std::unique_ptr<folly::IOBuf> createBufExact(size_t len);

// Converts the hex encoded string to an IOBuf.
std::unique_ptr<folly::IOBuf>
toIOBuf(std::string hexData, size_t headroom = 0, size_t tailroom = 0);

std::unique_ptr<folly::IOBuf> chunkIOBuf(
    std::unique_ptr<folly::IOBuf> input,
    size_t chunks,
    BufCreator creator = nullptr);
} // namespace test
} // namespace fizz
