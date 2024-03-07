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

#include <boost/variant.hpp>
#include <folly/ExceptionWrapper.h>
#include <folly/Memory.h>
#include <folly/Optional.h>
#include <folly/Unit.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/AsyncUDPSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/Request.h>
#include <wangle/acceptor/SecureTransportType.h>
#include <wangle/acceptor/TransportInfo.h>
#include <wangle/channel/HandlerContext.h>

namespace wangle {

class PipelineBase;
class Acceptor;

class PipelineManager {
 public:
  virtual ~PipelineManager() = default;
  virtual void deletePipeline(PipelineBase* pipeline) = 0;
  virtual void refreshTimeout() {}
  virtual void adjustTimeout(std::chrono::milliseconds /*newTimeout*/) {}
};

class PipelineBase : public std::enable_shared_from_this<PipelineBase> {
 public:
  virtual ~PipelineBase() = default;

  void setPipelineManager(PipelineManager* manager) {
    manager_ = manager;
  }

  PipelineManager* getPipelineManager() {
    return manager_;
  }

  void deletePipeline() {
    if (manager_) {
      manager_->deletePipeline(this);
    }
  }

  void setTransport(std::shared_ptr<folly::AsyncTransport> transport) {
    transport_ = transport;
  }

  std::shared_ptr<folly::AsyncTransport> getTransport() {
    return transport_;
  }

  void setWriteFlags(folly::WriteFlags flags);
  folly::WriteFlags getWriteFlags();

  void setReadBufferSettings(uint64_t minAvailable, uint64_t allocationSize);
  std::pair<uint64_t, uint64_t> getReadBufferSettings();

  void setTransportInfo(std::shared_ptr<TransportInfo> tInfo);
  std::shared_ptr<TransportInfo> getTransportInfo();

  template <class H>
  PipelineBase& addBack(std::shared_ptr<H> handler);

  template <class H>
  PipelineBase& addBack(H&& handler);

  template <class H>
  PipelineBase& addBack(H* handler);

  template <class H>
  PipelineBase& addFront(std::shared_ptr<H> handler);

  template <class H>
  PipelineBase& addFront(H&& handler);

  template <class H>
  PipelineBase& addFront(H* handler);

  template <class H>
  PipelineBase& remove(H* handler);

  template <class H>
  PipelineBase& remove();

  PipelineBase& removeFront();

  PipelineBase& removeBack();

  template <class H>
  H* getHandler(size_t i);

  template <class H>
  H* getHandler();

  template <class H>
  typename ContextType<H>::type* getContext(size_t i);

  template <class H>
  typename ContextType<H>::type* getContext();

  // If one of the handlers owns the pipeline itself, use setOwner to ensure
  // that the pipeline doesn't try to detach the handler during destruction,
  // lest destruction ordering issues occur.
  // See thrift/lib/cpp2/async/Cpp2Channel.cpp for an example
  template <class H>
  bool setOwner(H* handler);

  virtual void finalize() = 0;

  size_t numHandlers() const;

 protected:
  template <class Context>
  void addContextFront(Context* ctx);

  void detachHandlers();

  std::vector<std::shared_ptr<PipelineContext>> ctxs_;
  std::vector<PipelineContext*> inCtxs_;
  std::vector<PipelineContext*> outCtxs_;

 private:
  PipelineManager* manager_{nullptr};
  std::shared_ptr<folly::AsyncTransport> transport_;
  std::shared_ptr<TransportInfo> transportInfo_;

  template <class Context>
  PipelineBase& addHelper(std::shared_ptr<Context>&& ctx, bool front);

  template <class H>
  PipelineBase& removeHelper(H* handler, bool checkEqual);

  using ContextIterator =
      std::vector<std::shared_ptr<PipelineContext>>::iterator;

  ContextIterator removeAt(const ContextIterator& it);

  folly::WriteFlags writeFlags_{folly::WriteFlags::NONE};
  std::pair<uint64_t, uint64_t> readBufferSettings_{2048, 2048};

  std::shared_ptr<PipelineContext> owner_;
};

/*
 * R is the inbound type, i.e. inbound calls start with pipeline.read(R)
 * W is the outbound type, i.e. outbound calls start with pipeline.write(W)
 *
 * Use Unit for one of the types if your pipeline is unidirectional.
 * If R is Unit, read(), readEOF(), and readException() will be disabled.
 * If W is Unit, write() and close() will be disabled.
 */
template <class R, class W = folly::Unit>
class Pipeline : public PipelineBase {
 public:
  using Ptr = std::shared_ptr<Pipeline>;

  static Ptr create() {
    return std::shared_ptr<Pipeline>(new Pipeline());
  }

  ~Pipeline() override;

  template <class T = R>
  typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type read(
      R msg);

  template <class T = R>
  typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type readEOF();

  template <class T = R>
  typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
  readException(folly::exception_wrapper e);

  template <class T = R>
  typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
  transportActive();

  template <class T = R>
  typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
  transportInactive();

  template <class T = W>
  typename std::enable_if<
      !std::is_same<T, folly::Unit>::value,
      folly::Future<folly::Unit>>::type
  write(W msg);

  template <class T = W>
  typename std::enable_if<
      !std::is_same<T, folly::Unit>::value,
      folly::Future<folly::Unit>>::type
  writeException(folly::exception_wrapper e);

  template <class T = W>
  typename std::enable_if<
      !std::is_same<T, folly::Unit>::value,
      folly::Future<folly::Unit>>::type
  close();

  void finalize() override;

  void setRequestContext(
      const std::shared_ptr<folly::RequestContext>& requestContext) {
    requestContext_ = requestContext;
  }

  void setRequestContext(
      std::shared_ptr<folly::RequestContext>&& requestContext) {
    requestContext_ = std::move(requestContext);
  }

 protected:
  Pipeline();
  explicit Pipeline(bool isStatic);

 private:
  using OptionalReqCtxScopeGuard =
      folly::Optional<folly::RequestContextScopeGuard>;

  void fillRequestContextGuard(OptionalReqCtxScopeGuard& optGuard);

  bool isStatic_{false};

  InboundLink<R>* front_{nullptr};
  OutboundLink<W>* back_{nullptr};

  std::shared_ptr<folly::RequestContext> requestContext_;
};

} // namespace wangle

namespace folly {

class AsyncSocket;
}

namespace wangle {

using DefaultPipeline =
    Pipeline<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>;

template <typename Pipeline>
class PipelineFactory {
 public:
  virtual typename Pipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransport>) = 0;

  virtual typename Pipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncUDPSocket> /* serverSocket */,
      const folly::SocketAddress& /* clientAddr */) {
    return nullptr;
  }

  virtual ~PipelineFactory() = default;
};

struct ConnInfo {
  folly::AsyncTransport* sock;
  const folly::SocketAddress* clientAddr;
  const std::string& nextProtoName;
  SecureTransportType secureType;
  const TransportInfo& tinfo;
};

enum class ConnEvent {
  CONN_ADDED,
  CONN_REMOVED,
};

using AcceptPipelineType = boost::variant<
    folly::IOBuf*,
    folly::AsyncTransport*,
    ConnInfo&,
    ConnEvent,
    std::tuple<
        folly::IOBuf*,
        std::shared_ptr<folly::AsyncUDPSocket>,
        folly::SocketAddress>,
    std::tuple<
        folly::IOBuf*,
        std::shared_ptr<folly::AsyncUDPSocket>,
        folly::SocketAddress,
        folly::AsyncUDPSocket::ReadCallback::OnDataAvailableParams>>;
using AcceptPipeline = Pipeline<AcceptPipelineType>;

class AcceptPipelineFactory {
 public:
  virtual typename AcceptPipeline::Ptr newPipeline(Acceptor* acceptor) = 0;

  virtual ~AcceptPipelineFactory() = default;
};

} // namespace wangle

#include <wangle/channel/Pipeline-inl.h>
