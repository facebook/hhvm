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

#include <thrift/lib/python/streaming/StreamElementEncoder.h>

namespace apache::thrift::python::detail {

folly::Try<folly::IOBuf> decode_stream_element(
    folly::Try<apache::thrift::StreamPayload>&& payload) {
  if (payload.hasValue()) {
    return folly::Try<folly::IOBuf>(std::move(*payload->payload));
  } else if (payload.hasException()) {
    return decode_stream_exception(std::move(payload).exception());
  } else {
    return {};
  }
}

folly::Try<std::unique_ptr<folly::IOBuf>> decode_sink_element(
    folly::Try<apache::thrift::StreamPayload>&& payload) {
  folly::Try<std::unique_ptr<folly::IOBuf>> ret;
  if (payload.hasValue()) {
    ret = folly::Try<std::unique_ptr<folly::IOBuf>>(
        std::make_unique<folly::IOBuf>(std::move(*payload->payload)));
  } else if (payload.hasException()) {
    auto ex_try = decode_stream_exception(std::move(payload).exception());
    if (ex_try.hasValue()) {
      ret = folly::Try<std::unique_ptr<folly::IOBuf>>(
          std::make_unique<folly::IOBuf>(std::move(*ex_try)));
    } else if (ex_try.hasException()) {
      ret = folly::Try<std::unique_ptr<folly::IOBuf>>(
          std::move(ex_try).exception());
    }
  }
  return ret;
}

folly::Try<folly::IOBuf> decode_stream_exception(folly::exception_wrapper ew) {
  using IOBufTry = folly::Try<folly::IOBuf>;
  IOBufTry ret;
  ew.handle(
      [&ret](apache::thrift::detail::EncodedError& err) {
        ret = IOBufTry(std::move(*err.encoded));
      },
      [&ret](apache::thrift::detail::EncodedStreamError& err) {
        auto& payload = err.encoded;
        DCHECK_EQ(payload.metadata.payloadMetadata().has_value(), true);
        DCHECK_EQ(
            folly::to_underlying(payload.metadata.payloadMetadata()->getType()),
            folly::to_underlying(PayloadMetadata::Type::exceptionMetadata));
        auto& exceptionMetadataBase =
            payload.metadata.payloadMetadata()->get_exceptionMetadata();
        if (auto exceptionMetadataRef = exceptionMetadataBase.metadata()) {
          if (exceptionMetadataRef->getType() ==
              PayloadExceptionMetadata::Type::declaredException) {
            ret = IOBufTry(std::move(*payload.payload));
          } else {
            ret = IOBufTry(TApplicationException(
                exceptionMetadataBase.what_utf8().value_or("")));
          }
        } else {
          ret = IOBufTry(
              TApplicationException("Missing payload exception metadata"));
        }
      },
      [&ret](apache::thrift::detail::EncodedStreamRpcError& err) {
        StreamRpcError streamRpcError;
        CompactProtocolReader reader;
        reader.setInput(err.encoded.get());
        streamRpcError.read(&reader);
        TApplicationException::TApplicationExceptionType exType{
            TApplicationException::UNKNOWN};
        auto code = streamRpcError.code();
        if (code &&
            (code.value() == StreamRpcErrorCode::CREDIT_TIMEOUT ||
             code.value() == StreamRpcErrorCode::CHUNK_TIMEOUT)) {
          exType = TApplicationException::TIMEOUT;
        }
        ret = IOBufTry(TApplicationException(
            exType, streamRpcError.what_utf8().value_or("")));
      },
      [](...) {});

  return ret;
}

} // namespace apache::thrift::python::detail
