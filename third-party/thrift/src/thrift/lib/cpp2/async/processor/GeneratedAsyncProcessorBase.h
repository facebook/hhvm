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

#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessorFunc.h>
#include <thrift/lib/cpp2/async/processor/RequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServerInterface.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>

namespace apache::thrift {

class ServerInterface;

class GeneratedAsyncProcessorBase : public AsyncProcessor,
                                    public AsyncProcessorFunc {
 public:
  // Sends an error response if validation fails.
  static bool validateRpcKind(
      const ResponseChannelRequest::UniquePtr& req, RpcKind kind);
  static bool validateRpcKind(const ServerRequest& req);

 protected:
  template <typename ProtocolIn, typename Args>
  static void deserializeRequest(
      Args& args,
      folly::StringPiece methodName,
      const SerializedRequest& serializedRequest,
      ContextStack* c);

  template <typename ProtocolIn, typename Args>
  static void simpleDeserializeRequest(
      Args& args, const SerializedRequest& serializedRequest);

  template <typename Response, typename ProtocolOut, typename Result>
  static Response serializeResponseImpl(
      std::string_view method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static LegacySerializedResponse serializeLegacyResponse(
      std::string_view method,
      ProtocolOut* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const Result& result);

  template <typename ProtocolOut, typename Result>
  static SerializedResponse serializeResponse(
      std::string_view method,
      ProtocolOut* prot,
      ContextStack* ctx,
      const Result& result);

  // Returns true if setup succeeded and sends an error response otherwise.
  // Always runs in eb thread.
  // tm is null if the method is annotated with thread='eb'.
  bool setUpRequestProcessing(
      const ResponseChannelRequest::UniquePtr& req,
      Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      RpcKind kind,
      ServerInterface* si,
      folly::StringPiece interaction = "",
      bool isInteractionFactoryFunction = false);

  bool setUpRequestProcessing(ServerRequest& req);

  void processInteraction(ServerRequest&& req) override;

 public:
  template <typename ChildType>
  static void processInThread(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm,
      RpcKind kind,
      ExecuteFunc<ChildType> executeFunc,
      ChildType* childClass);

 private:
  template <typename ChildType>
  static std::unique_ptr<concurrency::Runnable> makeEventTaskForRequest(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      Cpp2RequestContext* ctx,
      folly::Executor::KeepAlive<> executor,
      RpcKind kind,
      ExecuteFunc<ChildType> executeFunc,
      ChildType* childClass,
      Tile* tile);

  // Returns false if interaction id is duplicated.
  bool createInteraction(
      const ResponseChannelRequest::UniquePtr& req,
      int64_t id,
      std::string&& name,
      Cpp2RequestContext& ctx,
      concurrency::ThreadManager* tm,
      folly::EventBase& eb,
      ServerInterface* si,
      bool isFactoryFunction);

  // Returns false if interaction id is duplicated.
  bool createInteraction(ServerRequest& req);

 protected:
  virtual std::unique_ptr<Tile> createInteractionImpl(
      const std::string& name,
      // This is only used by Rust, since Rust implementations of interaction is
      // not fully compatible with standard interaction contract.
      int16_t protocol);

 public:
  void terminateInteraction(
      int64_t id, Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
  void destroyAllInteractions(
      Cpp2ConnContext& conn, folly::EventBase&) noexcept final;
};

template <typename ProtocolIn, typename Args>
void GeneratedAsyncProcessorBase::deserializeRequest(
    Args& args,
    folly::StringPiece methodName,
    const SerializedRequest& serializedRequest,
    ContextStack* c) {
  ProtocolIn iprot;
  iprot.setInput(serializedRequest.buffer.get());
  if (c) {
    c->preRead();
  }
  SerializedMessage smsg;
  smsg.protocolType = iprot.protocolType();
  smsg.buffer = serializedRequest.buffer.get();
  smsg.methodName = methodName;
  if (c) {
    c->onReadData(smsg);
  }
  uint32_t bytes = 0;
  try {
    bytes = apache::thrift::detail::deserializeRequestBody(&iprot, &args);
    iprot.readMessageEnd();
  } catch (const std::exception& ex) {
    throw TrustedServerException::requestParsingError(ex.what());
  } catch (...) {
    throw TrustedServerException::requestParsingError(
        folly::exceptionStr(folly::current_exception()).c_str());
  }
  if (c) {
    c->postRead(nullptr, bytes);
  }
}

template <typename ProtocolIn, typename Args>
void GeneratedAsyncProcessorBase::simpleDeserializeRequest(
    Args& args, const SerializedRequest& serializedRequest) {
  ProtocolIn iprot;
  iprot.setInput(serializedRequest.buffer.get());
  try {
    apache::thrift::detail::deserializeRequestBody(&iprot, &args);
    iprot.readMessageEnd();
  } catch (const std::exception& ex) {
    throw TrustedServerException::requestParsingError(ex.what());
  } catch (...) {
    throw TrustedServerException::requestParsingError(
        folly::exceptionStr(folly::current_exception()).c_str());
  }
}

template <typename Response, typename ProtocolOut, typename Result>
Response GeneratedAsyncProcessorBase::serializeResponseImpl(
    std::string_view method,
    ProtocolOut* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const Result& result) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  size_t bufSize =
      apache::thrift::detail::serializedResponseBodySizeZC(prot, &result);
  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    bufSize += prot->serializedMessageSize(method);
  }

  // Preallocate small buffer headroom for transports metadata & framing.
  constexpr size_t kHeadroomBytes = 128;
  auto buf = folly::IOBuf::create(kHeadroomBytes + bufSize);
  buf->advance(kHeadroomBytes);
  queue.append(std::move(buf));

  prot->setOutput(&queue, bufSize);
  if (ctx) {
    ctx->preWrite();
  }

  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    prot->writeMessageBegin(method, MessageType::T_REPLY, protoSeqId);
  }
  apache::thrift::detail::serializeResponseBody(prot, &result);
  if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
    prot->writeMessageEnd();
  }
  if (ctx) {
    SerializedMessage smsg;
    smsg.protocolType = prot->protocolType();
    smsg.methodName = method;
    if constexpr (std::is_same_v<Response, LegacySerializedResponse>) {
      apache::thrift::LegacySerializedResponse legacyResponse(
          queue.front()->clone());
      apache::thrift::SerializedResponse response(
          std::move(legacyResponse), prot->protocolType());

      smsg.buffer = response.buffer.get();
    } else {
      smsg.buffer = queue.front();
    }
    ctx->onWriteData(smsg);
  }
  DCHECK_LE(
      queue.chainLength(),
      static_cast<size_t>(std::numeric_limits<int>::max()));
  if (ctx) {
    ctx->postWrite(folly::to_narrow(queue.chainLength()));
  }
  return Response{queue.move()};
}

template <typename ProtocolOut, typename Result>
LegacySerializedResponse GeneratedAsyncProcessorBase::serializeLegacyResponse(
    std::string_view method,
    ProtocolOut* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const Result& result) {
  return serializeResponseImpl<LegacySerializedResponse>(
      method, prot, protoSeqId, ctx, result);
}

template <typename ProtocolOut, typename Result>
SerializedResponse GeneratedAsyncProcessorBase::serializeResponse(
    std::string_view method,
    ProtocolOut* prot,
    ContextStack* ctx,
    const Result& result) {
  return serializeResponseImpl<SerializedResponse>(
      method, prot, 0, ctx, result);
}

template <typename ChildType>
std::unique_ptr<concurrency::Runnable>
GeneratedAsyncProcessorBase::makeEventTaskForRequest(
    ResponseChannelRequest::UniquePtr req,
    SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::Executor::KeepAlive<> executor,
    RpcKind kind,
    ExecuteFunc<ChildType> executeFunc,
    ChildType* childClass,
    Tile* tile) {
  auto task = std::make_unique<RequestTask<ChildType>>(
      std::move(req),
      std::move(serializedRequest),
      std::move(executor),
      ctx,
      kind == RpcKind::SINGLE_REQUEST_NO_RESPONSE,
      childClass,
      executeFunc);
  if (tile) {
    task->setTile(
        {tile,
         ctx->getConnectionContext()
             ->getWorkerContext()
             ->getWorkerEventBase()});
  }
  return task;
}

template <typename ChildType>
void GeneratedAsyncProcessorBase::processInThread(
    ResponseChannelRequest::UniquePtr req,
    SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase*,
    concurrency::ThreadManager* tm,
    RpcKind kind,
    ExecuteFunc<ChildType> executeFunc,
    ChildType* childClass) {
  Tile* tile = nullptr;
  if (auto interactionId = ctx->getInteractionId()) { // includes create
    try {
      tile = &ctx->getConnectionContext()->getTile(interactionId);
    } catch (const std::out_of_range&) {
      req->sendErrorWrapped(
          TApplicationException(
              "Invalid interaction id " + std::to_string(interactionId)),
          kInteractionIdUnknownErrorCode);
      return;
    }
  }

  auto scope = ctx->getRequestExecutionScope();
  auto task = makeEventTaskForRequest(
      std::move(req),
      std::move(serializedRequest),
      ctx,
      tm ? tm->getKeepAlive(scope, concurrency::ThreadManager::Source::INTERNAL)
         : folly::Executor::KeepAlive<>{},
      kind,
      executeFunc,
      childClass,
      tile);

  if (tile && tile->maybeEnqueue(std::move(task), scope)) {
    return;
  }

  if (tm) {
    using Source = concurrency::ThreadManager::Source;
    auto source = tile && !ctx->getInteractionCreate()
        ? Source::EXISTING_INTERACTION
        : Source::UPSTREAM;
    tm->getKeepAlive(std::move(scope), source)->add([task = std::move(task)] {
      task->run();
    });
  } else {
    task->run();
  }
}

} // namespace apache::thrift
