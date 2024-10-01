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

#include <vector>

#include <folly/Function.h>
#include <folly/Range.h>

namespace apache::thrift {
class TProcessorEventHandler;
class ClientInterceptorBase;
class ThriftServer;
} // namespace apache::thrift

namespace apache::thrift::runtime {

using ClientInterceptorList =
    std::vector<std::shared_ptr<apache::thrift::ClientInterceptorBase>>;

struct InitOptions {
  std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>
      legacyClientEventHandlers;
  /**
   * Global set of client interceptors that will be applied to all AsyncClient
   * objects (by default).
   */
  ClientInterceptorList clientInterceptors;

  using ThriftServerInitializer = folly::Function<void(ThriftServer&) const>;
  /**
   * Initialization functions to be called on ThriftServer instances during
   * their construction. This is useful for always-on customizations such as:
   *   - Adding ServerModules via ThriftServer::addModule
   *   - Configuration via ThriftServer::set* methods
   */
  std::vector<ThriftServerInitializer> serverInitializers;
};
void init(InitOptions);
bool wasInitialized() noexcept;

folly::Range<std::shared_ptr<apache::thrift::TProcessorEventHandler>*>
getGlobalLegacyClientEventHandlers();

std::shared_ptr<ClientInterceptorList> getGlobalClientInterceptors();

folly::Range<const InitOptions::ThriftServerInitializer*>
getGlobalServerInitializers();

} // namespace apache::thrift::runtime
