/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/rocket/test/network/Util.h>

#include <algorithm>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/Range.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>

namespace apache::thrift::rocket::test {

std::string repeatPattern(folly::StringPiece pattern, size_t nbytes) {
  std::string rv;
  rv.reserve(nbytes);
  for (size_t remaining = nbytes; remaining != 0;) {
    const size_t toCopy = std::min<size_t>(pattern.size(), remaining);
    rv.append(pattern.data(), 0, toCopy);
    remaining -= toCopy;
  }
  return rv;
}

void expectTransportExceptionType(
    transport::TTransportException::TTransportExceptionType expectedType,
    folly::exception_wrapper ew) {
  const auto* const tex = ew.get_exception<transport::TTransportException>();
  ASSERT_NE(nullptr, tex);
  EXPECT_EQ(expectedType, tex->getType());
}

void expectRocketExceptionType(
    ErrorCode expectedCode, folly::exception_wrapper ew) {
  const auto* const rex = ew.get_exception<RocketException>();
  ASSERT_NE(nullptr, rex);
  EXPECT_EQ(expectedCode, rex->getErrorCode());
}

void expectEncodedError(folly::exception_wrapper ew) {
  const auto* const rex = ew.get_exception<thrift::detail::EncodedError>();
  ASSERT_NE(nullptr, rex);
}

} // namespace apache::thrift::rocket::test
