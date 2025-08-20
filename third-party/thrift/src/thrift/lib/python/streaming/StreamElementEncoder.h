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

#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/Try.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/python/streaming/PythonUserException.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::python::detail {

folly::Try<std::unique_ptr<folly::IOBuf>> decode_stream_element(
    folly::Try<apache::thrift::StreamPayload>&& payload);
folly::Try<std::unique_ptr<folly::IOBuf>> decode_stream_exception(
    folly::exception_wrapper ew);

template <
    class Protocol,
    apache::thrift::ErrorBlame Blame = apache::thrift::ErrorBlame::SERVER>
class PythonStreamElementEncoder final
    : public apache::thrift::detail::StreamElementEncoder<
          std::unique_ptr<::folly::IOBuf>> {
 public:
  folly::Try<apache::thrift::StreamPayload> operator()(
      std::unique_ptr<::folly::IOBuf>&& val) override {
    apache::thrift::StreamPayloadMetadata streamPayloadMetadata;
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata().ensure();
    streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
    return folly::Try<apache::thrift::StreamPayload>(
        {std::move(val), std::move(streamPayloadMetadata)});
  }

  folly::Try<apache::thrift::StreamPayload> operator()(
      folly::exception_wrapper&& e) override {
    Protocol prot;
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

    apache::thrift::PayloadExceptionMetadata exceptionMetadata;
    apache::thrift::PayloadExceptionMetadataBase exceptionMetadataBase;
    if (e.with_exception([&](const PythonUserException& py_ex) {
          prot.setOutput(&queue, 0);
          queue.append(*py_ex.buf());
          exceptionMetadata.declaredException() =
              apache::thrift::PayloadDeclaredExceptionMetadata();
        })) {
    } else {
      constexpr size_t kQueueAppenderGrowth = 4096;
      prot.setOutput(&queue, kQueueAppenderGrowth);
      apache::thrift::TApplicationException ex(e.what().toStdString());
      exceptionMetadataBase.what_utf8() = ex.what();
      apache::thrift::detail::serializeExceptionBody(&prot, &ex);
      apache::thrift::PayloadAppUnknownExceptionMetdata aue;
      aue.errorClassification().ensure().blame() = Blame;
      exceptionMetadata.appUnknownException() = std::move(aue);
    }

    exceptionMetadataBase.metadata() = std::move(exceptionMetadata);
    apache::thrift::StreamPayloadMetadata streamPayloadMetadata;
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.exceptionMetadata() = std::move(exceptionMetadataBase);
    streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
    return folly::Try<apache::thrift::StreamPayload>(
        folly::exception_wrapper(apache::thrift::detail::EncodedStreamError(
            apache::thrift::StreamPayload(
                std::move(queue).move(), std::move(streamPayloadMetadata)))));
  }
};

template <class Protocol>
using PythonSinkElementEncoder =
    PythonStreamElementEncoder<Protocol, apache::thrift::ErrorBlame::CLIENT>;

} // namespace apache::thrift::python::detail
