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

#include <thrift/lib/cpp2/server/BaseThriftServer.h>

#include <fcntl.h>

#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>

#include <folly/Conv.h>
#include <folly/GLog.h>
#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/portability/Sockets.h>

namespace apache {
namespace thrift {
BaseThriftServer::BaseThriftServer() : thriftConfig_(), addresses_(1) {}

BaseThriftServer::BaseThriftServer(
    const ThriftServerInitialConfig& initialConfig)
    : thriftConfig_(initialConfig), addresses_(1) {}

/* static */ BaseThriftServer::ProcessedModuleSet
BaseThriftServer::processModulesSpecification(ModulesSpecification&& specs) {
  ProcessedModuleSet result;

  for (auto& info : specs.infos) {
    std::vector<std::shared_ptr<TProcessorEventHandler>> legacyEventHandlers =
        info.module->getLegacyEventHandlers();
    std::vector<std::shared_ptr<server::TServerEventHandler>>
        legacyServerEventHandlers = info.module->getLegacyServerEventHandlers();

    result.modules.emplace_back(std::move(info));

    result.coalescedLegacyEventHandlers.insert(
        result.coalescedLegacyEventHandlers.end(),
        std::make_move_iterator(legacyEventHandlers.begin()),
        std::make_move_iterator(legacyEventHandlers.end()));
    result.coalescedLegacyServerEventHandlers.insert(
        result.coalescedLegacyServerEventHandlers.end(),
        std::make_move_iterator(legacyServerEventHandlers.begin()),
        std::make_move_iterator(legacyServerEventHandlers.end()));
  }

  return result;
}

bool BaseThriftServer::getTaskExpireTimeForRequest(
    const apache::thrift::transport::THeader& requestHeader,
    std::chrono::milliseconds& queueTimeout,
    std::chrono::milliseconds& taskTimeout) const {
  return getTaskExpireTimeForRequest(
      requestHeader.getClientQueueTimeout(),
      requestHeader.getClientTimeout(),
      queueTimeout,
      taskTimeout);
}

bool BaseThriftServer::getTaskExpireTimeForRequest(
    std::chrono::milliseconds clientQueueTimeoutMs,
    std::chrono::milliseconds clientTimeoutMs,
    std::chrono::milliseconds& queueTimeout,
    std::chrono::milliseconds& taskTimeout) const {
  taskTimeout = getTaskExpireTime();

  // Client side always take precedence in deciding queue_timetout.
  queueTimeout = clientQueueTimeoutMs;
  if (queueTimeout == std::chrono::milliseconds(0)) {
    queueTimeout = getQueueTimeout();
  }
  auto useClientTimeout = getUseClientTimeout() && clientTimeoutMs.count() >= 0;
  // If queue timeout was set to 0 explicitly, this request has opt-out of queue
  // timeout.
  if (queueTimeout != std::chrono::milliseconds(0)) {
    auto queueTimeoutPct = getQueueTimeoutPct();
    // If queueTimeoutPct was set, we use it to calculate another queue timeout
    // based on client timeout. And then use the max of the explicite setting
    // and inferenced queue timeout.
    if (queueTimeoutPct > 0 && queueTimeoutPct < 100 && useClientTimeout) {
      queueTimeout =
          max(queueTimeout,
              std::chrono::milliseconds(
                  (clientTimeoutMs.count() * queueTimeoutPct / 100)));
    }
  }

  if (taskTimeout != std::chrono::milliseconds(0) && useClientTimeout) {
    // we add 10% to the client timeout so that the request is much more likely
    // to timeout on the client side than to read the timeout from the server
    // as a TApplicationException (which can be confusing)
    taskTimeout =
        std::chrono::milliseconds((uint32_t)(clientTimeoutMs.count() * 1.1));
  }
  // Queue timeout shouldn't be greater than task timeout
  if (taskTimeout < queueTimeout &&
      taskTimeout != std::chrono::milliseconds(0)) {
    queueTimeout = taskTimeout;
  }
  return queueTimeout != taskTimeout;
}

int64_t BaseThriftServer::getLoad(
    const std::string& counter, bool check_custom) const {
  if (check_custom && getLoad_) {
    return getLoad_(counter);
  }

  const auto activeRequests = getActiveRequests();

  if (VLOG_IS_ON(1)) {
    FB_LOG_EVERY_MS(INFO, 1000 * 10) << getLoadInfo(activeRequests);
  }

  return activeRequests;
}

std::string BaseThriftServer::getLoadInfo(int64_t load) const {
  std::stringstream stream;
  stream << "Load is: " << load << " active requests";
  return stream.str();
}

std::string BaseThriftServer::RuntimeServerActions::explain() const {
  std::string result;
  result = std::string(userSuppliedThreadManager ? "setThreadManager, " : "") +
      (userSuppliedResourcePools ? "userSuppliedResourcePools, " : "") +
      (interactionInService ? "interactionInService, " : "") +
      (wildcardMethods ? "wildcardMethods, " : "") +
      (noServiceRequestInfo ? "noServiceRequestInfo, " : "") +
      (activeRequestTrackingDisabled ? "activeRequestTrackingDisabled, " : "") +
      (setPreprocess ? "setPreprocess, " : "") +
      (setIsOverloaded ? "setIsOverloaded, " : "") +
      (codelEnabled ? "codelEnabled, " : "") +
      (setupThreadManagerBeforeHandler ? "setupThreadManagerBeforeHandler, "
                                       : "") +
      (!resourcePoolFlagSet ? "thriftFlagNotSet, " : "");
  return result;
}

} // namespace thrift
} // namespace apache
