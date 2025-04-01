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

#include <thrift/lib/cpp2/runtime/Init.h>

#include <stdexcept>
#include <unordered_set>

#include <folly/Memory.h>
#include <folly/synchronization/DelayedInit.h>

namespace apache::thrift::runtime {

namespace {
struct RuntimeState {
  std::vector<std::shared_ptr<apache::thrift::TProcessorEventHandler>>
      legacyClientEventHandlers;
  std::shared_ptr<ClientInterceptorList> clientInterceptors;
  std::vector<InitOptions::ThriftServerInitializer> serverInitializers;
};
folly::DelayedInit<RuntimeState> gRuntimeState;
} // namespace

void init(InitOptions options) {
  bool didInitialize = false;
  gRuntimeState.try_emplace_with([&] {
    didInitialize = true;
    auto clientInterceptors = options.clientInterceptors.empty()
        ? nullptr
        : folly::copy_to_shared_ptr(std::move(options.clientInterceptors));
    return RuntimeState{
        std::move(options.legacyClientEventHandlers),
        std::move(clientInterceptors),
        std::move(options.serverInitializers)};
  });
  if (!didInitialize) {
    throw std::logic_error(
        "apache::thrift::runtime::init() was already called!");
  }
}

bool wasInitialized() noexcept {
  return gRuntimeState.has_value();
}

folly::Range<std::shared_ptr<apache::thrift::TProcessorEventHandler>*>
getGlobalLegacyClientEventHandlers() {
  if (!wasInitialized()) {
    return {};
  }
  return folly::range(gRuntimeState->legacyClientEventHandlers);
}

std::shared_ptr<ClientInterceptorList> getGlobalClientInterceptors() {
  if (!wasInitialized()) {
    return nullptr;
  }
  return gRuntimeState->clientInterceptors;
}

folly::Range<const InitOptions::ThriftServerInitializer*>
getGlobalServerInitializers() {
  if (!wasInitialized()) {
    return {};
  }
  return folly::range(gRuntimeState->serverInitializers);
}

} // namespace apache::thrift::runtime
