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

#include <thrift/lib/cpp2/async/HibernatingRequestChannel.h>

namespace apache::thrift {

// RequestCallback will keep owning the connection for inflight requests even
// after the timeout.
class HibernatingRequestChannel::RequestCallback
    : public apache::thrift::RequestClientCallback {
 public:
  RequestCallback(apache::thrift::RequestClientCallback::Ptr cob, ImplPtr impl)
      : impl_(std::move(impl)), cob_(std::move(cob)) {}

  void onResponse(ClientReceiveState&& state) noexcept override {
    cob_.release()->onResponse(std::move(state));
    delete this;
  }

  void onResponseError(folly::exception_wrapper ex) noexcept override {
    cob_.release()->onResponseError(std::move(ex));
    delete this;
  }

 private:
  HibernatingRequestChannel::ImplPtr impl_;
  apache::thrift::RequestClientCallback::Ptr cob_;
};

void HibernatingRequestChannel::sendRequestResponse(
    const RpcOptions& options,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& request,
    std::shared_ptr<transport::THeader> header,
    apache::thrift::RequestClientCallback::Ptr cob,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  auto implPtr = impl();
  auto& implRef = *implPtr;
  cob = apache::thrift::RequestClientCallback::Ptr(
      new RequestCallback(std::move(cob), std::move(implPtr)));
  implRef.sendRequestResponse(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(cob),
      std::move(frameworkMetadata));

  timeout_->scheduleTimeout(waitTime_.count());
}

HibernatingRequestChannel::ImplPtr& HibernatingRequestChannel::impl() {
  if (!impl_) {
    impl_ = implCreator_(evb_);
  }

  return impl_;
}
} // namespace apache::thrift
