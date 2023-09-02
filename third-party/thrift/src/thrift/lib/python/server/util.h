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

#include <exception>
#include <map>
#include <memory>
#include <string>
#include <Python.h>
#include <glog/logging.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/gen/service_tcc.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace thrift {
namespace python {

constexpr size_t kMaxUexwSize = 1024;

class PythonUserException : public std::exception {
 public:
  PythonUserException(
      std::string type, std::string reason, std::unique_ptr<folly::IOBuf> buf)
      : type_(std::move(type)),
        reason_(std::move(reason)),
        buf_(std::move(buf)) {}
  PythonUserException(const PythonUserException& ex)
      : type_(ex.type_), reason_(ex.reason_), buf_(ex.buf_->clone()) {}

  PythonUserException& operator=(const PythonUserException& ex) {
    type_ = ex.type_;
    reason_ = ex.reason_;
    buf_ = ex.buf_->clone();
    return *this;
  }

  const std::string& type() const { return type_; }
  const std::string& reason() const { return reason_; }
  const folly::IOBuf* buf() const { return buf_.get(); }
  const char* what() const noexcept override { return reason_.c_str(); }

 private:
  std::string type_;
  std::string reason_;
  std::unique_ptr<folly::IOBuf> buf_;
};

namespace detail {

template <class Protocol>
class PythonStreamElementEncoder final
    : public apache::thrift::detail::StreamElementEncoder<
          std::unique_ptr<::folly::IOBuf>> {
  folly::Try<apache::thrift::StreamPayload> operator()(
      std::unique_ptr<::folly::IOBuf>&& val) override {
    apache::thrift::StreamPayloadMetadata streamPayloadMetadata;
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata_ref().ensure();
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
          exceptionMetadata.declaredException_ref() =
              apache::thrift::PayloadDeclaredExceptionMetadata();
        })) {
    } else {
      constexpr size_t kQueueAppenderGrowth = 4096;
      prot.setOutput(&queue, kQueueAppenderGrowth);
      apache::thrift::TApplicationException ex(e.what().toStdString());
      exceptionMetadataBase.what_utf8() = ex.what();
      apache::thrift::detail::serializeExceptionBody(&prot, &ex);
      apache::thrift::PayloadAppUnknownExceptionMetdata aue;
      aue.errorClassification().ensure().blame() =
          apache::thrift::ErrorBlame::SERVER;
      exceptionMetadata.appUnknownException_ref() = std::move(aue);
    }

    exceptionMetadataBase.metadata() = std::move(exceptionMetadata);
    apache::thrift::StreamPayloadMetadata streamPayloadMetadata;
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.exceptionMetadata_ref() = std::move(exceptionMetadataBase);
    streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
    return folly::Try<apache::thrift::StreamPayload>(
        folly::exception_wrapper(apache::thrift::detail::EncodedStreamError(
            apache::thrift::StreamPayload(
                std::move(queue).move(), std::move(streamPayloadMetadata)))));
  }
};

template <class ProtocolIn, class ProtocolOut>
static apache::thrift::SerializedResponse return_serialized(
    apache::thrift::ContextStack* ctx, const ::folly::IOBuf& _return) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  ProtocolOut prot;

  // Preallocate small buffer headroom for transports metadata & framing.
  constexpr size_t kHeadroomBytes = 128;
  auto buf = folly::IOBuf::create(kHeadroomBytes);
  buf->advance(kHeadroomBytes);
  queue.append(std::move(buf));

  prot.setOutput(&queue, 0);
  if (ctx) {
    ctx->preWrite();
  }
  queue.append(_return);
  if (ctx) {
    apache::thrift::SerializedMessage smsg;
    smsg.protocolType = prot.protocolType();
    smsg.methodName = "";
    smsg.buffer = queue.front();
    ctx->onWriteData(smsg);
  }
  DCHECK_LE(
      queue.chainLength(),
      static_cast<size_t>(std::numeric_limits<int>::max()));
  if (ctx) {
    ctx->postWrite(folly::to_narrow(queue.chainLength()));
  }
  return apache::thrift::SerializedResponse{queue.move()};
}

template <class ProtocolIn_, class ProtocolOut_>
static apache::thrift::ResponseAndServerStreamFactory return_streaming(
    apache::thrift::ContextStack* ctx,
    folly::Executor::KeepAlive<> executor,
    ::apache::thrift::ResponseAndServerStream<
        std::unique_ptr<::folly::IOBuf>,
        std::unique_ptr<::folly::IOBuf>>&& _return) {
  static PythonStreamElementEncoder<ProtocolOut_> encode;
  return {
      return_serialized<ProtocolIn_, ProtocolOut_>(ctx, *(_return.response)),
      std::move(_return.stream)(std::move(executor), &encode),
  };
}

template <class ProtocolIn_, class ProtocolOut_>
static void throw_wrapped(
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    int32_t protoSeqId,
    apache::thrift::ContextStack* ctx,
    folly::exception_wrapper ew,
    apache::thrift::Cpp2RequestContext* reqCtx) {
  if (!ew) {
    return;
  }
  {
    if (ew.with_exception([&](const PythonUserException& e) {
          auto header = reqCtx->getHeader();
          if (!header) {
            return;
          }

          // TODO: (ffrancet) error kind overrides currently usupported,
          // by python, add kHeaderExMeta header support when it is
          header->setHeader(
              std::string(apache::thrift::detail::kHeaderUex), e.type());
          const std::string reason = e.reason();
          header->setHeader(
              std::string(apache::thrift::detail::kHeaderUexw),
              reason.size() > kMaxUexwSize ? reason.substr(0, kMaxUexwSize)
                                           : reason);

          ProtocolOut_ prot;
          auto response =
              return_serialized<ProtocolIn_, ProtocolOut_>(ctx, *e.buf());
          auto payload = std::move(response).extractPayload(
              req->includeEnvelope(),
              prot.protocolType(),
              protoSeqId,
              apache::thrift::MessageType::T_REPLY,
              reqCtx->getMethodName().c_str());
          payload.transform(reqCtx->getHeader()->getWriteTransforms());
          return req->sendException(std::move(payload));
        })) {
    } else {
      apache::thrift::detail::ap::process_throw_wrapped_handler_error<
          ProtocolOut_>(
          ew, std::move(req), reqCtx, ctx, reqCtx->getMethodName().c_str());
    }
  }
}

} // namespace detail

} // namespace python
} // namespace thrift
