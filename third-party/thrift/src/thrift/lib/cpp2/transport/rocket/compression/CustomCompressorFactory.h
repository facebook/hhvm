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

#include <memory>
#include <optional>
#include <string>

#include <folly/compression/Compression.h>

#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressor.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

class CustomCompressorFactory {
  /**
   * Setup Negotiation
   *
   * The following functions are used to setup the custom compressor
   * negotiation. The negotiation process is as follows:
   * 1. client constructs a CustomCompressionSetupRequest from higher level,
   *    and set the `customCompressorRequest` in `RequestSetupMetadata`.
   * 2. server receives the request, and calls
   *    `getCustomCompressorNegotiationResponse` to try to produce a response
   *    and return it to the client via `SetupResponse`.
   * 3. If successful, both server and client will now have a shared
   *    negotiation state stored in `CustomCompressionSetupResponse`.
   */

 public:
  enum class CompressorLocation { CLIENT, SERVER };

  virtual ~CustomCompressorFactory() = default;

  virtual std::string getCompressorName() const = 0;

  // Get the custom compressor negotiation response given a request.
  //
  // @param request The custom compressor negotiation request.
  // @return The custom compressor setup response. nullopt if the negotiation
  //         failed for any reason.
  virtual std::optional<CustomCompressionSetupResponse>
  createCustomCompressorNegotiationResponse(
      CustomCompressionSetupRequest const& request) const = 0;

  // Creates a new compressor from the negotiation response.
  //
  // shared_ptr is used since both payload serializer as well as the
  // client/server connection may need to hold onto the compressor.
  virtual std::shared_ptr<CustomCompressor> make(
      CustomCompressionSetupResponse response,
      CompressorLocation location) const = 0;
};

} // namespace apache::thrift::rocket
