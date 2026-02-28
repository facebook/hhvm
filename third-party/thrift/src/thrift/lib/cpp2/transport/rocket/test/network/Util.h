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

#pragma once

#include <string>

#include <folly/ExceptionWrapper.h>
#include <folly/Range.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/framing/test/Util.h>

namespace folly {
class IOBuf;
} // namespace folly

namespace apache::thrift::rocket::test {

std::string repeatPattern(folly::StringPiece pattern, size_t nbytes);

void expectTransportExceptionType(
    transport::TTransportException::TTransportExceptionType expectedType,
    folly::exception_wrapper ew);

void expectRocketExceptionType(
    ErrorCode expectedCode, folly::exception_wrapper ew);

void expectEncodedError(folly::exception_wrapper ew);

} // namespace apache::thrift::rocket::test
