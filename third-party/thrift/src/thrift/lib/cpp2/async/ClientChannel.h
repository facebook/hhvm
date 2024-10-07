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

#include <optional>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/HeaderChannel.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

struct ClientHostMetadata {
  std::optional<std::string> hostname;
  std::optional<apache::thrift::transport::THeader::StringToStringMap>
      otherMetadata;
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(ClientHostMetadata, getClientHostMetadata);
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, hookForClientTransport, folly::AsyncTransport* transport);
} // namespace detail

/**
 * Interface for Thrift Client channels
 */
class ClientChannel : public RequestChannel, public HeaderChannel {
 public:
  ClientChannel();
  ~ClientChannel() override {}

  struct SaturationStatus {
    enum class SaturationType {
      INVALID = 0,
      REQUEST = 1,
    };
    SaturationType type = SaturationType::INVALID;
    uint32_t usedCapacity = 0;
    uint32_t capacity = 0;

    SaturationStatus() = default;

    SaturationStatus(uint32_t usedCap, uint32_t cap)
        : type(SaturationType::REQUEST), usedCapacity(usedCap), capacity(cap) {}
  };

  typedef std::unique_ptr<ClientChannel, folly::DelayedDestruction::Destructor>
      Ptr;

  virtual folly::AsyncTransport* getTransport() = 0;

  virtual bool good() = 0;

  virtual SaturationStatus getSaturationStatus() = 0;

  virtual void attachEventBase(folly::EventBase*) = 0;
  virtual void detachEventBase() = 0;
  virtual bool isDetachable() = 0;

  virtual uint32_t getTimeout() = 0;
  virtual void setTimeout(uint32_t ms) = 0;

  virtual void closeNow() = 0;
  virtual CLIENT_TYPE getClientType() = 0;

  virtual void setOnDetachable(folly::Function<void()> onDetachable) {
    onDetachable_ = std::move(onDetachable);
  }

  virtual void unsetOnDetachable() { onDetachable_ = nullptr; }

 protected:
  // This should be called by the implementation to notify observer (if any)
  // that the channel became detachable.
  void notifyDetachable() {
    DCHECK(getEventBase()->isInEventBaseThread());
    DCHECK(isDetachable());
    if (!onDetachable_) {
      return;
    }
    onDetachable_();
  }

  static const std::optional<ClientHostMetadata>& getHostMetadata();

 private:
  folly::Function<void()> onDetachable_;
};
} // namespace apache::thrift
