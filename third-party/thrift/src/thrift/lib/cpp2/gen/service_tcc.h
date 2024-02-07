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
#include <memory>
#include <utility>

#include <glog/logging.h>

#include <folly/Portability.h>

#include <folly/ExceptionString.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/ContextStack.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache {
namespace thrift {
namespace detail {

namespace ap {

template <typename Prot>
std::unique_ptr<folly::IOBuf> process_serialize_xform_app_exn(
    bool includeEnvelope,
    const TApplicationException& x,
    Cpp2RequestContext* const ctx,
    const char* const method) {
  Prot prot;
  size_t bufSize = x.serializedSizeZC(&prot);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  if (includeEnvelope) {
    bufSize += prot.serializedMessageSize(method);
    prot.setOutput(&queue, bufSize);
    prot.writeMessageBegin(
        method, MessageType::T_EXCEPTION, ctx->getProtoSeqId());
  } else {
    prot.setOutput(&queue, bufSize);
  }
  x.write(&prot);
  if (includeEnvelope) {
    prot.writeMessageEnd();
  }
  queue.append(transport::THeader::transform(
      queue.move(), ctx->getHeader()->getWriteTransforms()));
  return queue.move();
}

void inline sendTrustedServerExceptionHelper(
    ResponseChannelRequest::UniquePtr request,
    const TrustedServerException& trustedServerEx) {
  request->sendErrorWrapped(
      folly::make_exception_wrapper<TApplicationException>(
          trustedServerEx.toApplicationException()),
      std::string(trustedServerEx.errorCode()));
}

void inline sendExceptionHelper(
    ResponseChannelRequest::UniquePtr req, ResponsePayload&& payload) {
#if !FOLLY_HAS_COROUTINES
  if (req->isSink()) {
    DCHECK(false);
    return;
  }
#endif
  req->sendException(std::move(payload));
}

// Temporary for backwards compatibility whilst all users are migrated to
// optionally include the envelope.
template <typename Prot>
std::unique_ptr<folly::IOBuf> process_serialize_xform_app_exn(
    const TApplicationException& x,
    Cpp2RequestContext* const ctx,
    const char* const method) {
  return process_serialize_xform_app_exn<Prot>(true, x, ctx, method);
}

template <typename Prot>
void process_handle_exn_deserialization(
    const folly::exception_wrapper& ew,
    ResponseChannelRequest::UniquePtr req,
    Cpp2RequestContext* const ctx,
    folly::EventBase* const eb,
    const char* const method) {
  if (auto trustedServerEx =
          dynamic_cast<const TrustedServerException*>(ew.get_exception())) {
    if (eb) {
      eb->runInEventBaseThread(
          [request = std::move(req), ex = *trustedServerEx]() mutable {
            sendTrustedServerExceptionHelper(std::move(request), ex);
          });
    } else {
      sendTrustedServerExceptionHelper(std::move(req), *trustedServerEx);
    }
    return;
  }
  ::apache::thrift::util::appendExceptionToHeader(ew, *ctx);
  auto buf = process_serialize_xform_app_exn<Prot>(
      req->includeEnvelope(),
      ::apache::thrift::util::toTApplicationException(ew),
      ctx,
      method);
  if (eb) {
    eb->runInEventBaseThread(
        [buf = std::move(buf), req = std::move(req)]() mutable {
          sendExceptionHelper(std::move(req), std::move(buf));
        });
  } else {
    sendExceptionHelper(std::move(req), std::move(buf));
  }
}

template <typename Prot>
void process_throw_wrapped_handler_error(
    const folly::exception_wrapper& ew,
    apache::thrift::ResponseChannelRequest::UniquePtr req,
    Cpp2RequestContext* const ctx,
    ContextStack* const stack,
    const char* const method) {
  if (auto trustedServerEx =
          dynamic_cast<const TrustedServerException*>(ew.get_exception())) {
    req->sendErrorWrapped(
        folly::make_exception_wrapper<TApplicationException>(
            trustedServerEx->toApplicationException()),
        std::string(trustedServerEx->errorCode()));
    return;
  }

  FB_LOG_EVERY_MS(ERROR, 1000)
      << "Service handler threw an uncaught exception in method" << method
      << ": " << ew
      << ". This indicates an error in user code that implements this method. "
      << "Note: only exceptions declared in service definition in thrift IDL are allowed to "
      << "be thrown from method handler.";

  if (stack) {
    stack->userExceptionWrapped(false, ew);
    stack->handlerErrorWrapped(ew);
  }
  ::apache::thrift::util::appendExceptionToHeader(ew, *ctx);
  auto xp = ew.get_exception<TApplicationException>();
  auto x = xp ? std::move(*xp) : TApplicationException(ew.what().toStdString());
  auto buf = process_serialize_xform_app_exn<Prot>(
      req->includeEnvelope(), x, ctx, method);
  sendExceptionHelper(std::move(req), std::move(buf));
}

} // namespace ap

} // namespace detail
} // namespace thrift
} // namespace apache
