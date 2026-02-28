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

#include <thrift/lib/cpp2/async/ClientStreamInterceptorContext.h>

#include <glog/logging.h>
#include <thrift/lib/cpp/ContextStack.h>

namespace apache::thrift {

ClientStreamInterceptorContext::ClientStreamInterceptorContext(
    std::shared_ptr<InterceptorList> interceptors,
    std::vector<detail::ClientInterceptorOnRequestStorage> requestStorages)
    : ownedInterceptors_(std::move(interceptors)),
      ownedRequestStorages_(std::move(requestStorages)) {
  DCHECK(ownedInterceptors_ != nullptr)
      << "interceptors must be non-null; use "
         "fromContextStack for null-safe construction";
}

ClientStreamInterceptorContext::ClientStreamInterceptorContext(
    ClientStreamInterceptorContext&& other) noexcept
    : ownedInterceptors_(std::move(other.ownedInterceptors_)),
      ownedRequestStorages_(std::move(other.ownedRequestStorages_)),
      payloadIndex_(other.payloadIndex_),
      ended_(other.ended_) {
  other.ended_ = true;
}

ClientStreamInterceptorContext::~ClientStreamInterceptorContext() {
  if (!ended_) {
    onStreamEnd(details::STREAM_ENDING_TYPES::CANCEL);
  }
}

const ClientStreamInterceptorContext::InterceptorList&
ClientStreamInterceptorContext::getInterceptors() const {
  return *ownedInterceptors_;
}

detail::ClientInterceptorOnRequestStorage*
ClientStreamInterceptorContext::getRequestStorage(std::size_t index) {
  return index < ownedRequestStorages_.size() ? &ownedRequestStorages_[index]
                                              : nullptr;
}

void ClientStreamInterceptorContext::onStreamBegin() {
  const auto& interceptors = getInterceptors();
  for (std::size_t i = 0; i < interceptors.size(); ++i) {
    interceptors[i]->internal_onStreamBegin(getRequestStorage(i));
  }
}

void ClientStreamInterceptorContext::onStreamPayloadImpl(
    util::TypeErasedRef payload) {
  const auto& interceptors = getInterceptors();
  for (std::size_t i = 0; i < interceptors.size(); ++i) {
    ClientInterceptorBase::StreamPayloadInfo info{
        .payload = payload, .payloadIndex = payloadIndex_};
    interceptors[i]->internal_onStreamPayload(getRequestStorage(i), info);
  }
  ++payloadIndex_;
}

void ClientStreamInterceptorContext::onStreamEnd(
    details::STREAM_ENDING_TYPES reason,
    const folly::exception_wrapper& error) {
  if (ended_) {
    return;
  }
  ended_ = true;

  const auto& interceptors = getInterceptors();
  for (std::size_t i = interceptors.size(); i > 0; --i) {
    ClientInterceptorBase::StreamEndInfo info{
        .endReason = reason, .error = error, .totalPayloads = payloadIndex_};
    interceptors[i - 1]->internal_onStreamEnd(getRequestStorage(i - 1), info);
  }
}

std::shared_ptr<ClientStreamInterceptorContext>
ClientStreamInterceptorContext::fromContextStack(ContextStack& ctx) {
  auto interceptors = ctx.getClientInterceptors();
  if (!interceptors || interceptors->empty()) {
    return nullptr;
  }

  auto requestStorages = ctx.extractClientInterceptorRequestStorages();

  return std::make_shared<ClientStreamInterceptorContext>(
      std::move(interceptors), std::move(requestStorages));
}

} // namespace apache::thrift
