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

#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <wangle/channel/Pipeline.h>

namespace wangle {

template <class Context>
class HandlerBase {
 public:
  virtual ~HandlerBase() = default;

  virtual void attachPipeline(Context* /*ctx*/) {}
  virtual void detachPipeline(Context* /*ctx*/) {}

  Context* getContext() {
    if (attachCount_ != 1) {
      return nullptr;
    }
    CHECK(ctx_);
    return ctx_;
  }

 private:
  friend PipelineContext;
  uint64_t attachCount_{0};
  Context* ctx_{nullptr};
};

template <class Rin, class Rout = Rin, class Win = Rout, class Wout = Rin>
class Handler : public HandlerBase<HandlerContext<Rout, Wout>> {
 public:
  static const HandlerDir dir = HandlerDir::BOTH;

  using rin = Rin;
  using rout = Rout;
  using win = Win;
  using wout = Wout;
  using Context = HandlerContext<Rout, Wout>;
  ~Handler() override = default;

  virtual void read(Context* ctx, Rin msg) = 0;
  virtual void readEOF(Context* ctx) {
    ctx->fireReadEOF();
  }
  virtual void readException(Context* ctx, folly::exception_wrapper e) {
    ctx->fireReadException(std::move(e));
  }
  virtual void transportActive(Context* ctx) {
    ctx->fireTransportActive();
  }
  virtual void transportInactive(Context* ctx) {
    ctx->fireTransportInactive();
  }

  virtual folly::Future<folly::Unit> write(Context* ctx, Win msg) = 0;
  virtual folly::Future<folly::Unit> writeException(
      Context* ctx,
      folly::exception_wrapper e) {
    return ctx->fireWriteException(std::move(e));
  }
  virtual folly::Future<folly::Unit> close(Context* ctx) {
    return ctx->fireClose();
  }

  /*
  // Other sorts of things we might want, all shamelessly stolen from Netty
  // inbound
  virtual void exceptionCaught(
      HandlerContext* ctx,
      folly::exception_wrapper e) {}
  virtual void channelRegistered(HandlerContext* ctx) {}
  virtual void channelUnregistered(HandlerContext* ctx) {}
  virtual void channelReadComplete(HandlerContext* ctx) {}
  virtual void userEventTriggered(HandlerContext* ctx, void* evt) {}
  virtual void channelWritabilityChanged(HandlerContext* ctx) {}

  // outbound
  virtual folly::Future<folly::Unit> bind(
      HandlerContext* ctx,
      SocketAddress localAddress) {}
  virtual folly::Future<folly::Unit> connect(
          HandlerContext* ctx,
          SocketAddress remoteAddress, SocketAddress localAddress) {}
  virtual folly::Future<folly::Unit> disconnect(HandlerContext* ctx) {}
  virtual folly::Future<folly::Unit> deregister(HandlerContext* ctx) {}
  virtual folly::Future<folly::Unit> read(HandlerContext* ctx) {}
  virtual void flush(HandlerContext* ctx) {}
  */
};

template <class Rin, class Rout = Rin>
class InboundHandler : public HandlerBase<InboundHandlerContext<Rout>> {
 public:
  static const HandlerDir dir = HandlerDir::IN;

  using rin = Rin;
  using rout = Rout;
  using win = folly::Unit;
  using wout = folly::Unit;
  using Context = InboundHandlerContext<Rout>;
  ~InboundHandler() override = default;

  virtual void read(Context* ctx, Rin msg) = 0;
  virtual void readEOF(Context* ctx) {
    ctx->fireReadEOF();
  }
  virtual void readException(Context* ctx, folly::exception_wrapper e) {
    ctx->fireReadException(std::move(e));
  }
  virtual void transportActive(Context* ctx) {
    ctx->fireTransportActive();
  }
  virtual void transportInactive(Context* ctx) {
    ctx->fireTransportInactive();
  }
};

template <class Win, class Wout = Win>
class OutboundHandler : public HandlerBase<OutboundHandlerContext<Wout>> {
 public:
  static const HandlerDir dir = HandlerDir::OUT;

  using rin = folly::Unit;
  using rout = folly::Unit;
  using win = Win;
  using wout = Wout;
  using Context = OutboundHandlerContext<Wout>;
  ~OutboundHandler() override = default;

  virtual folly::Future<folly::Unit> write(Context* ctx, Win msg) = 0;
  virtual folly::Future<folly::Unit> writeException(
      Context* ctx,
      folly::exception_wrapper e) {
    return ctx->fireWriteException(std::move(e));
  }
  virtual folly::Future<folly::Unit> close(Context* ctx) {
    return ctx->fireClose();
  }
};

template <class R, class W = R>
class HandlerAdapter : public Handler<R, R, W, W> {
 public:
  using Context = typename Handler<R, R, W, W>::Context;

  void read(Context* ctx, R msg) override {
    ctx->fireRead(std::forward<R>(msg));
  }

  folly::Future<folly::Unit> write(Context* ctx, W msg) override {
    return ctx->fireWrite(std::forward<W>(msg));
  }
};

using BytesToBytesHandler =
    HandlerAdapter<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>;

using InboundBytesToBytesHandler =
    InboundHandler<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>;

using OutboundBytesToBytesHandler =
    OutboundHandler<std::unique_ptr<folly::IOBuf>>;

} // namespace wangle
