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

#include <folly/Function.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializerStrategy.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

struct ChecksumPayloadSerializerStrategyOptions {
  folly::Function<void()> recordChecksumFailure;
  folly::Function<void()> recordChecksumSuccess;
  folly::Function<void()> recordChecksumCalculated;
};

/**
 * A payload serializer strategy that calculates and validates checksums. It
 * delegates to another strategy for the actual serialization and
 * deserialization.
 *
 * Metrics are supported for recording checksum failures, successes, and
 * calculations using the options. Metric are record via Folly functions
 * because this class is use both on the client and server so we need
 * to decouple the metrics from the implementation.
 */
template <typename DelegateStrategy>
class ChecksumPayloadSerializerStrategy final
    : PayloadSerializerStrategy<
          ChecksumPayloadSerializerStrategy<DelegateStrategy>> {
 public:
  ChecksumPayloadSerializerStrategy(
      ChecksumPayloadSerializerStrategyOptions options = {[] {}, [] {}, [] {}})
      : PayloadSerializerStrategy<
            ChecksumPayloadSerializerStrategy<DelegateStrategy>>(*this),
        delegate_(DelegateStrategy()),
        recordChecksumFailure_(std::move(options.recordChecksumFailure)),
        recordChecksumSuccess_(std::move(options.recordChecksumSuccess)),
        recordChecksumCalculated_(std::move(options.recordChecksumCalculated)) {
  }

  template <class T>
  FOLLY_ERASE folly::Try<T> unpackAsCompressed(Payload&& payload) {
    return unpackImpl<T>(
        std::move(payload), [this](Payload&& payload) -> folly::Try<T> {
          return delegate_.template unpackAsCompressed<T>(std::move(payload));
        });
  }

  template <typename T>
  FOLLY_ERASE folly::Try<T> unpack(Payload&& payload) {
    return unpackImpl<T>(
        std::move(payload), [this](Payload&& payload) -> folly::Try<T> {
          return delegate_.template unpack<T>(std::move(payload));
        });
  }

  template <typename T>
  FOLLY_ERASE std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
    return delegate_.packCompact(std::forward<T>(data));
  }

  template <typename Metadata>
  FOLLY_ERASE rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      folly::AsyncTransport* transport) {
    if (auto checksumOpt = calculateChecksum(*payload, metadata->checksum())) {
      metadata->checksum() = *checksumOpt;
    }
    return delegate_.packWithFds(
        metadata, std::move(payload), std::move(fds), transport);
  }

  template <class PayloadType>
  FOLLY_ERASE Payload
  pack(PayloadType&& payload, folly::AsyncTransport* transport) {
    auto metadata = std::forward<PayloadType>(payload).metadata;
    return packWithFds(
        &metadata,
        std::forward<PayloadType>(payload).payload,
        std::forward<PayloadType>(payload).fds,
        transport);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    return delegate_.unpackCompact(output, buffer);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackCompact(T& output, const folly::io::Cursor& cursor) {
    return delegate_.unpackCompact(output, cursor);
  }

 private:
  DelegateStrategy delegate_;

  FOLLY_ERASE apache::thrift::ChecksumAlgorithm getChecksumAlgorithm(
      ::apache::thrift::optional_field_ref<Checksum&> opt) {
    if (opt.has_value()) {
      return opt.value().algorithm().value();
    } else {
      return apache::thrift::ChecksumAlgorithm::NONE;
    }
  }

  template <typename Algo, apache::thrift::ChecksumAlgorithm ChecksumAlgo>
  apache::thrift::Checksum calculateChecksumImpl(folly::IOBuf& buf) {
    ChecksumGenerator<Algo> generator;
    auto ret = generator.calculateChecksumFromIOBuf(buf);
    Checksum checksum;
    checksum.algorithm() = ChecksumAlgo;
    checksum.checksum() = ret.checksum;
    checksum.salt() = ret.salt;
    recordChecksumCalculated_();
    return checksum;
  }

  std::optional<Checksum> calculateChecksum(
      folly::IOBuf& buf,
      ::apache::thrift::optional_field_ref<Checksum&> checksumOpt) {
    switch (getChecksumAlgorithm(checksumOpt)) {
      case apache::thrift::ChecksumAlgorithm::CRC32:
        return calculateChecksumImpl<
            CRC32C,
            apache::thrift::ChecksumAlgorithm::CRC32>(buf);

      case apache::thrift::ChecksumAlgorithm::XXH3_64:
        return calculateChecksumImpl<
            XXH3_64,
            apache::thrift::ChecksumAlgorithm::XXH3_64>(buf);

      case apache::thrift::ChecksumAlgorithm::NONE:
        return std::nullopt;

      default:
        throw std::runtime_error("Unsupported checksum algorithm");
    }
  }

  template <typename Algo>
  bool validateChecksumImpl(folly::IOBuf& buf, Checksum checksum) {
    ChecksumGenerator<Algo> generator;
    bool ret = generator.validateChecksumFromIOBuf(
        *checksum.checksum(), *checksum.salt(), buf);
    if (FOLLY_LIKELY(ret)) {
      recordChecksumSuccess_();
    } else {
      recordChecksumFailure_();
    }
    return ret;
  }

  bool validateChecksum(
      folly::IOBuf& buf,
      ::apache::thrift::optional_field_ref<Checksum&> checksumOpt) {
    switch (getChecksumAlgorithm(checksumOpt)) {
      case apache::thrift::ChecksumAlgorithm::CRC32:
        return validateChecksumImpl<CRC32C>(buf, *checksumOpt);

      case apache::thrift::ChecksumAlgorithm::XXH3_64:
        return validateChecksumImpl<XXH3_64>(buf, *checksumOpt);

      case apache::thrift::ChecksumAlgorithm::NONE:
        return true;

      default:
        throw std::runtime_error("Unsupported checksum algorithm");
    }
  }

 private:
  folly::Function<void()> recordChecksumFailure_;
  folly::Function<void()> recordChecksumSuccess_;
  folly::Function<void()> recordChecksumCalculated_;

  template <typename T, typename DelegateFunc>
  FOLLY_ERASE folly::Try<T> unpackImpl(Payload&& payload, DelegateFunc func) {
    if (payload.hasNonemptyMetadata()) {
      folly::Try<T> t = func(std::move(payload));
      folly::IOBuf& buf = *t->payload.get();
      if (t.hasException() || validateChecksum(buf, t->metadata.checksum())) {
        return t;
      } else {
        return folly::Try<T>(std::runtime_error("Checksum mismatch"));
      }
    } else {
      return func(std::move(payload));
    }
  }
};

} // namespace apache::thrift::rocket
