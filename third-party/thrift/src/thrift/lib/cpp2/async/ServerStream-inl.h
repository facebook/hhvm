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
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache {
namespace thrift {

template <typename T>
class CompactSerializerEncoder final : public detail::StreamElementEncoder<T> {
  folly::Try<StreamPayload> operator()(T&& in) override {
    folly::IOBufQueue buf;
    CompactSerializer::serialize(in, &buf);
    return folly::Try<StreamPayload>({buf.move(), {}});
  }

  folly::Try<StreamPayload> operator()(folly::exception_wrapper&& e) override {
    return folly::Try<StreamPayload>(e);
  }
};

template <typename T>
ClientBufferedStream<T> ServerStream<T>::toClientStreamUnsafeDoNotUse(
    folly::EventBase* eb, int32_t bufferSize) && {
  struct : public apache::thrift::detail::ClientStreamBridge::
               FirstResponseCallback {
    void onFirstResponse(
        FirstResponsePayload&&,
        apache::thrift::detail::ClientStreamBridge::ClientPtr
            clientStreamBridge) override {
      ptr = std::move(clientStreamBridge);
      baton.post();
    }
    void onFirstResponseError(folly::exception_wrapper) override {}
    apache::thrift::detail::ClientStreamBridge::ClientPtr ptr;
    folly::Baton<> baton;
  } firstResponseCallback;
  auto streamBridge = apache::thrift::detail::ClientStreamBridge::create(
      &firstResponseCallback);

  auto decode = [](folly::Try<StreamPayload>&& in) {
    if (in.hasValue()) {
      T out;
      CompactSerializer::deserialize<T>(in.value().payload.get(), out);
      return folly::Try<T>(std::move(out));
    } else if (in.hasException()) {
      return folly::Try<T>(in.exception());
    } else {
      return folly::Try<T>();
    }
  };

  static CompactSerializerEncoder<T> encode;
  eb->add([factory = (*this)(eb, &encode), eb, streamBridge]() mutable {
    factory({nullptr, {}}, streamBridge, eb);
  });
  firstResponseCallback.baton.wait();
  firstResponseCallback.ptr->requestN(bufferSize);
  return ClientBufferedStream<T>(
      std::move(firstResponseCallback.ptr), decode, {bufferSize});
}

} // namespace thrift
} // namespace apache
