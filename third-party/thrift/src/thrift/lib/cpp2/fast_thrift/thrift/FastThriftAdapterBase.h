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

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Base class for app adapters used by generated fast thrift clients.
 *
 * Provides common response handling that all adapters (SR, Thrift, etc.)
 * share. Derived adapters call handleRequestResponse() to convert
 * error/unknown frames into Expected errors before dispatching to
 * generated client code, so generated code never sees non-PAYLOAD frames.
 */
class FastThriftAdapterBase {
 protected:
  /**
   * Handles a request-response frame: converts ERROR and unknown frame
   * types into folly::exception_wrapper. For PAYLOAD frames, deserializes
   * the response metadata, classifies declared/undeclared exceptions, and
   * returns the response data IOBuf for the generated client to deserialize.
   *
   * If the response carries a typed anyException variant (Rocket protocol
   * v10+), extracts the typed exception via the thrift Any registry using
   * `protocolId` to select the SemiAnyStruct deserializer. Returns the
   * extracted exception_wrapper to the caller.
   *
   * App adapters should call this before invoking the response handler,
   * passing their negotiated protocol id.
   */
  static folly::Expected<
      std::unique_ptr<folly::IOBuf>,
      folly::exception_wrapper>
  handleRequestResponse(ThriftResponseMessage&& response, uint16_t protocolId);
};

} // namespace apache::thrift::fast_thrift::thrift
