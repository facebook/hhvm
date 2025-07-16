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

#include <variant>
#include <folly/GLog.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/payload/ChecksumPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/CustomCompressionPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>

namespace apache::thrift::rocket {

/**
 * This class is a facade for the different strategies of serializing payloads.
 * Its to make changing the strategy easier in the future / safe. Hides the
 * strategy from the rest of the code (provides type-erasure) with exposing.
 *
 * For reaons with the existing code this class is accessed as a singleton. It
 * can be overridden by calling the initialize() method if you want to use a
 * different strategy.
 */
class PayloadSerializer : public folly::hazptr_obj_base<PayloadSerializer> {
 public:
  class Ptr {
   public:
    PayloadSerializer* operator->() const noexcept { return serializer_; }
    PayloadSerializer& operator*() const noexcept { return *serializer_; }
    operator bool() const noexcept { return serializer_; }
    friend bool operator==(Ptr const& ptr, std::nullptr_t) noexcept {
      return ptr.serializer_ == nullptr;
    }
    friend bool operator==(std::nullptr_t, Ptr const& ptr) noexcept {
      return ptr.serializer_ == nullptr;
    }
    friend bool operator!=(Ptr const& ptr, std::nullptr_t) noexcept {
      return ptr.serializer_ != nullptr;
    }
    friend bool operator!=(std::nullptr_t, Ptr const& ptr) noexcept {
      return ptr.serializer_ != nullptr;
    }
    PayloadSerializer* get() { return serializer_; }

   private:
    friend struct PayloadSerializerHolder;

    // Used for shared global instance. Co-owns the instance.
    explicit Ptr(std::atomic<PayloadSerializer*>& cell)
        : holder_(folly::make_hazard_pointer()),
          serializer_(holder_.protect(cell)) {}

    // Does not own the instance.
    explicit Ptr(PayloadSerializer& payloadSerializer)
        : holder_(folly::make_hazard_pointer()),
          serializer_(&payloadSerializer) {}

    folly::hazptr_holder<> holder_;
    PayloadSerializer* serializer_;

    friend class PayloadSerializer;
  };

  struct PayloadSerializerHolder {
    PayloadSerializerHolder() {}
    ~PayloadSerializerHolder();

    PayloadSerializer::Ptr get();

    template <typename Strategy>
    void initialize(Strategy&& strategy) {
      auto* serializer =
          new PayloadSerializer(std::forward<Strategy>(strategy));
      if (auto s =
              serializer_.exchange(serializer, std::memory_order_acq_rel)) {
        s->retire();
      }
    }

    void reset();

   private:
    alignas(folly::hardware_destructive_interference_size)
        std::atomic<PayloadSerializer*> serializer_{nullptr};
  };

 private:
  std::variant<
      DefaultPayloadSerializerStrategy,
      ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>,
      CustomCompressionPayloadSerializerStrategy<
          DefaultPayloadSerializerStrategy>>
      strategy_;

  /**
   * Visits the strategy and calls the delegate function with the strategy as
   * the parameter. Done manually instead of using std::visit for performance.
   */
  template <typename DelegateFunc>
  FOLLY_ALWAYS_INLINE decltype(auto) visit(DelegateFunc&& delegate) {
    if (std::holds_alternative<DefaultPayloadSerializerStrategy>(strategy_)) {
      auto& strategy = std::get<DefaultPayloadSerializerStrategy>(strategy_);
      return delegate(strategy);
    } else if (std::holds_alternative<ChecksumPayloadSerializerStrategy<
                   DefaultPayloadSerializerStrategy>>(strategy_)) {
      auto& strategy = std::get<
          ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>>(
          strategy_);
      return delegate(strategy);
    } else {
      auto& strategy = std::get<CustomCompressionPayloadSerializerStrategy<
          DefaultPayloadSerializerStrategy>>(strategy_);
      return delegate(strategy);
    }
  }

 public:
  template <
      typename Strategy,
      typename = std::enable_if_t<
          std::is_base_of_v<PayloadSerializerStrategy<Strategy>, Strategy>>>
  explicit PayloadSerializer(Strategy s) : strategy_(std::move(s)) {}

  /**
   * Lets you override the strategy to one of the supported strategies instead
   * of the default. Must be called before the getInstance() method is called
   * for the first time to take effect. Otherwise, it will be ignored.
   */
  template <typename Strategy>
  static void initialize(Strategy&& strategy) {
    getPayloadSerializerHolder().initialize(std::forward<Strategy>(strategy));
  }
  /**
   * Returns the singleton instance of the PayloadSerializer. Either returns
   * the default strategy or the one that was overridden by the initialize()
   * method.
   */
  static Ptr getInstance();

  /**
   * Constructs a new PayloadSerializer with the given strategy.
   */
  template <
      typename Strategy = DefaultPayloadSerializerStrategy,
      typename... Args,
      typename = std::enable_if_t<
          std::is_base_of_v<PayloadSerializerStrategy<Strategy>, Strategy>>>
  static FOLLY_ALWAYS_INLINE PayloadSerializer make(Args... args) {
    if constexpr (std::is_same_v<Strategy, DefaultPayloadSerializerStrategy>) {
      return PayloadSerializer{DefaultPayloadSerializerStrategy(args...)};
    } else if constexpr (std::is_same_v<
                             Strategy,
                             ChecksumPayloadSerializerStrategy<
                                 DefaultPayloadSerializerStrategy>>) {
      return PayloadSerializer{
          ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>(
              args...)};
    } else if constexpr (std::is_same_v<
                             Strategy,
                             CustomCompressionPayloadSerializerStrategy<
                                 DefaultPayloadSerializerStrategy>>) {
      return PayloadSerializer{CustomCompressionPayloadSerializerStrategy<
          DefaultPayloadSerializerStrategy>(args...)};
    }
  }

  Ptr getNonOwningPtr() { return Ptr(*this); }

  bool supportsChecksum() {
    return visit([&](auto& strategy) { return strategy.supportsChecksum(); });
  }

  template <class T>
  folly::Try<T> unpackAsCompressed(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return visit([&](auto& strategy) {
      return strategy.template unpackAsCompressed<T>(
          std::move(payload), decodeMetadataUsingBinary);
    });
  }

  template <class T>
  folly::Try<T> unpack(
      rocket::Payload&& payload, bool decodeMetadataUsingBinary) {
    return visit([&](auto& strategy) {
      return strategy.template unpack<T>(
          std::move(payload), decodeMetadataUsingBinary);
    });
  }

  template <typename T>
  std::unique_ptr<folly::IOBuf> packCompact(const T& data) {
    return visit(
        [&](auto& strategy) { return strategy.template packCompact<T>(data); });
  }

  template <typename T>
  FOLLY_ERASE size_t
  unpack(T& output, const folly::IOBuf* buffer, bool useBinary) {
    if (useBinary) {
      return visit([&](auto& strategy) {
        return strategy.template unpackBinary<T>(output, buffer);
      });
    }
    return visit([&](auto& strategy) {
      return strategy.template unpackCompact<T>(output, buffer);
    });
  }

  template <typename T>
  FOLLY_ERASE size_t
  unpack(T& output, const folly::io::Cursor& cursor, bool useBinary) {
    if (useBinary) {
      return visit([&](auto& strategy) {
        return strategy.template unpackBinary<T>(output, cursor);
      });
    }
    return visit([&](auto& strategy) {
      return strategy.template unpackCompact<T>(output, cursor);
    });
  }

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    if (!supportsChecksum()) {
      if (metadata->checksum().has_value() &&
          metadata->checksum()->algorithm().value() !=
              ChecksumAlgorithm::NONE) {
        FB_LOG_ONCE(WARNING)
            << "Checksum is not supported, but checksum was set";
        Checksum c;
        c.algorithm() = ChecksumAlgorithm::NONE;
        metadata->checksum() = c;
      }
    }
    return visit([&](auto& strategy) {
      return strategy.template packWithFds<Metadata>(
          metadata,
          std::move(payload),
          std::move(fds),
          encodeMetadataUsingBinary,
          transport);
    });
  }

  template <class PayloadType>
  rocket::Payload pack(
      PayloadType&& payload,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    return visit([&](auto& strategy) {
      return strategy.template pack<PayloadType>(
          std::forward<PayloadType>(payload),
          encodeMetadataUsingBinary,
          transport);
    });
  }

  /**
   * Resets the singleton instance of the PayloadSerializer to empty. This
   * should only be called in tests.
   */
  static void reset();

 private:
  static PayloadSerializerHolder& getPayloadSerializerHolder();

 public:
  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return visit([&](auto& strategy) {
      return strategy.compressBuffer(std::move(buffer), compressionAlgorithm);
    });
  }

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return visit([&](auto& strategy) {
      return strategy.uncompressBuffer(std::move(buffer), compressionAlgorithm);
    });
  }
};

} // namespace apache::thrift::rocket
