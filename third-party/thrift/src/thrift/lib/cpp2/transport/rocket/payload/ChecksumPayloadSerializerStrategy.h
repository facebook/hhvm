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
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializerStrategy.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

struct ChecksumPayloadSerializerStrategyOptions {
  folly::Function<void()> recordChecksumFailure;
  folly::Function<void()> recordChecksumSuccess;
  folly::Function<void()> recordChecksumCalculated;
  folly::Function<void()> recordChecksumSkipped;
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
      ChecksumPayloadSerializerStrategyOptions options = {})
      : PayloadSerializerStrategy<
            ChecksumPayloadSerializerStrategy<DelegateStrategy>>(*this),
        delegate_(DelegateStrategy()),
        recordChecksumFailure_(std::move(options.recordChecksumFailure)),
        recordChecksumSuccess_(std::move(options.recordChecksumSuccess)),
        recordChecksumCalculated_(std::move(options.recordChecksumCalculated)),
        recordChecksumSkipped_(std::move(options.recordChecksumSkipped)) {}

  bool supportsChecksum() { return true; }

  template <class T>
  FOLLY_ERASE folly::Try<T> unpackAsCompressed(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return unpackImpl<T>(
        std::move(payload),
        [this, decodeMetadataUsingBinary](Payload&& payload) -> folly::Try<T> {
          return delegate_.template unpackAsCompressed<T>(
              std::move(payload), decodeMetadataUsingBinary);
        });
  }

  template <typename T>
  FOLLY_ERASE folly::Try<T> unpack(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return unpackImpl<T>(
        std::move(payload),
        [this, decodeMetadataUsingBinary](Payload&& payload) -> folly::Try<T> {
          return delegate_.template unpack<T>(
              std::move(payload), decodeMetadataUsingBinary);
        });
  }

  template <typename T>
  FOLLY_ERASE std::unique_ptr<folly::IOBuf> packCompact(const T& data) {
    return delegate_.packCompact(data);
  }

  template <typename Metadata>
  bool isDataCompressed(Metadata* metadata) {
    return metadata->compression().has_value() &&
        metadata->compression().value() !=
        apache::thrift::CompressionAlgorithm::NONE;
  }

  template <typename Metadata>
  FOLLY_ERASE rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    if (payload != nullptr) {
      if (auto checksumOpt =
              calculateChecksum(*payload, metadata->checksum())) {
        metadata->checksum() = *checksumOpt;
      }
    }
    return delegate_.packWithFds(
        metadata,
        std::move(payload),
        std::move(fds),
        encodeMetadataUsingBinary,
        transport);
  }

  template <class PayloadType>
  FOLLY_ERASE Payload pack(
      PayloadType&& payload,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    auto metadata = std::forward<PayloadType>(payload).metadata;
    return packWithFds(
        &metadata,
        std::forward<PayloadType>(payload).payload,
        std::forward<PayloadType>(payload).fds,
        encodeMetadataUsingBinary,
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

  template <typename T>
  FOLLY_ERASE size_t unpackBinary(T& output, const folly::IOBuf* buffer) {
    return delegate_.unpackBinary(output, buffer);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackBinary(T& output, const folly::io::Cursor& cursor) {
    return delegate_.unpackBinary(output, cursor);
  }

  FOLLY_ERASE
  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return delegate_.compressBuffer(std::move(buffer), compressionAlgorithm);
  }

  FOLLY_ERASE
  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return delegate_.uncompressBuffer(std::move(buffer), compressionAlgorithm);
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
    tryRecordChecksumCalculated();
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

      default: {
        LOG(ERROR) << "calculateChecksum|Unsupported checksum algorithm";
        throw TApplicationException(
            TApplicationException::CHECKSUM_MISMATCH,
            "Unsupported checksum algorithm");
      }
    }
  }

  template <typename Algo>
  bool validateChecksumImpl(folly::IOBuf& buf, Checksum checksum) {
    ChecksumGenerator<Algo> generator;
    bool ret = generator.validateChecksumFromIOBuf(
        *checksum.checksum(), *checksum.salt(), buf);
    if (FOLLY_LIKELY(ret)) {
      tryRecordChecksumSuccess();
    } else {
      tryRecordChecksumFailure();
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
        tryRecordChecksumSkipped();
        return true;

      default: {
        LOG(ERROR) << "validateChecksum|Unsupported checksum algorithm";
        throw TApplicationException(
            TApplicationException::CHECKSUM_MISMATCH,
            "Unsupported checksum algorithm");
      }
    }
  }

 private:
  folly::Function<void()> recordChecksumFailure_;
  folly::Function<void()> recordChecksumSuccess_;
  folly::Function<void()> recordChecksumCalculated_;
  folly::Function<void()> recordChecksumSkipped_;

  /**
   * Helper function that makes checks to make sure that the checksum wasn't
   * invalid because of incorrect setup vs an actual checksum failure.
   */
  void validateInvalidChecksum(const Checksum& c) {
    auto value = c.checksum().value();

    if (value == 0) {
      XLOG_EVERY_MS(ERR, 1'000)
          << "Received a request to checksum the payload but received a checksumt that is zero. "
          << "Please make sure that the ChecksumPayloadSerializerStrategy is enabled on both the client and server.";
    }
  }

  template <typename T, typename DelegateFunc>
  FOLLY_ALWAYS_INLINE folly::Try<T> unpackImpl(
      Payload&& payload, DelegateFunc func) {
    if (payload.hasNonemptyMetadata()) {
      // Wrap the entire unpack and validation pipeline in try-catch to prevent
      // exceptions from escaping into noexcept contexts
      // (e.g., RocketClientChannel::onResponsePayload).
      // This catches:
      // 1. TProtocolException from delegate's unpack() (truncated data)
      // 2. TApplicationException from validateChecksum() (unsupported
      // algorithm)
      // 3. Any other exceptions during deserialization or validation
      try {
        folly::Try<T> t = func(std::move(payload));
        bool compressed = isDataCompressed(&t.value().metadata);
        folly::IOBuf& buf = *t->payload.get();
        if (t.hasException() || compressed) {
          return t;
        } else if (validateChecksum(buf, t->metadata.checksum())) {
          return t;
        } else {
          if (FOLLY_LIKELY(t->metadata.checksum().has_value())) {
            validateInvalidChecksum(t->metadata.checksum().value());
          }
          return folly::Try<T>(
              folly::make_exception_wrapper<TApplicationException>(
                  TApplicationException::CHECKSUM_MISMATCH,
                  "Checksum mismatch"));
        }
      } catch (...) {
        // Catch and wrap any exceptions to prevent termination in noexcept
        // contexts
        auto ex = std::current_exception();
        try {
          std::rethrow_exception(ex);
        } catch (const std::exception& e) {
          // Log detailed information to help identify the source of malformed
          // responses
          XLOG(ERR)
              << "ChecksumPayloadSerializer: Exception during unpack/validation. "
              << "Exception type: " << folly::demangle(typeid(e))
              << ", Message: " << e.what() << ", Payload metadata size: "
              << (payload.hasNonemptyMetadata() ? payload.metadataSize() : 0)
              << ", Payload data size: " << payload.dataSize();
        } catch (...) {
          XLOG(ERR)
              << "ChecksumPayloadSerializer: Unknown exception during unpack/validation. "
              << "Payload metadata size: "
              << (payload.hasNonemptyMetadata() ? payload.metadataSize() : 0)
              << ", Payload data size: " << payload.dataSize();
        }
        return folly::Try<T>(folly::exception_wrapper(ex));
      }
    } else {
      return func(std::move(payload));
    }
  }

  FOLLY_ERASE void tryRecordChecksumFailure() {
    if (recordChecksumFailure_ != nullptr) {
      recordChecksumFailure_();
    }
  }

  FOLLY_ERASE void tryRecordChecksumSuccess() {
    if (recordChecksumSuccess_ != nullptr) {
      recordChecksumSuccess_();
    }
  }

  FOLLY_ERASE void tryRecordChecksumCalculated() {
    if (recordChecksumCalculated_ != nullptr) {
      recordChecksumCalculated_();
    }
  }

  FOLLY_ERASE void tryRecordChecksumSkipped() {
    if (recordChecksumSkipped_ != nullptr) {
      recordChecksumSkipped_();
    }
  }
};

} // namespace apache::thrift::rocket
