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

#include <thrift/lib/cpp2/server/ServiceInterceptorOnDrop.h>

#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>

namespace apache::thrift {

void processServiceInterceptorsOnDrop(
    const server::ServerConfigs& serverConfigs,
    const Cpp2RequestContext& reqCtx,
    ServiceInterceptorBase::DropReason reason) {
  const auto& interceptors = serverConfigs.getServiceInterceptors();
  if (interceptors.empty()) {
    return;
  }
  auto droppedInfo = ServiceInterceptorBase::DroppedRequestInfo{
      &reqCtx, reqCtx.getMethodName(), reason};
  for (std::size_t i = 0; i < interceptors.size(); ++i) {
    try {
      interceptors[i]->internal_onRequestDropped(droppedInfo);
    } catch (const std::exception& ex) {
      XLOG_EVERY_MS(WARNING, 10000)
          << "ServiceInterceptor::onRequestDropped threw: " << ex.what();
    } catch (...) {
      XLOG_EVERY_MS(WARNING, 10000)
          << "ServiceInterceptor::onRequestDropped threw non-std exception";
    }
  }
}

void processServiceInterceptorsOnDrop(
    const Cpp2RequestContext& reqCtx,
    ServiceInterceptorBase::DropReason reason) {
  auto* connCtx = reqCtx.getConnectionContext();
  if (!connCtx) {
    return;
  }
  auto* workerCtx = connCtx->getWorkerContext();
  if (!workerCtx) {
    return;
  }
  const auto* serverConfigs = workerCtx->getServerContext();
  if (!serverConfigs) {
    return;
  }
  processServiceInterceptorsOnDrop(*serverConfigs, reqCtx, reason);
}

} // namespace apache::thrift
