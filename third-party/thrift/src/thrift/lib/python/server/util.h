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
#include <thrift/lib/python/streaming/PythonUserException.h>
#include <thrift/lib/python/streaming/StreamElementEncoder.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::python {

constexpr size_t kMaxUexwSize = 1024;

namespace detail {

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

} // namespace apache::thrift::python
