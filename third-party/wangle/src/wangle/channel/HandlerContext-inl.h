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

#include <folly/Format.h>

namespace wangle {

class PipelineContext {
 public:
  virtual ~PipelineContext() = default;

  virtual void attachPipeline() = 0;
  virtual void detachPipeline() = 0;

  template <class H, class HandlerContext>
  void attachContext(H* handler, HandlerContext* ctx) {
    if (++handler->attachCount_ == 1) {
      handler->ctx_ = ctx;
    } else {
      handler->ctx_ = nullptr;
    }
  }

  template <class H, class HandlerContext>
  void detachContext(H* handler, HandlerContext* /*ctx*/) {
    if (handler->attachCount_ >= 1) {
      --handler->attachCount_;
    }
    handler->ctx_ = nullptr;
  }

  virtual void setNextIn(PipelineContext* ctx) = 0;
  virtual void setNextOut(PipelineContext* ctx) = 0;

  virtual HandlerDir getDirection() = 0;
};

template <class In>
class InboundLink {
 public:
  virtual ~InboundLink() = default;
  virtual void read(In msg) = 0;
  virtual void readEOF() = 0;
  virtual void readException(folly::exception_wrapper e) = 0;
  virtual void transportActive() = 0;
  virtual void transportInactive() = 0;
};

template <class Out>
class OutboundLink {
 public:
  virtual ~OutboundLink() = default;
  virtual folly::Future<folly::Unit> write(Out msg) = 0;
  virtual folly::Future<folly::Unit> writeException(
      folly::exception_wrapper e) = 0;
  virtual folly::Future<folly::Unit> close() = 0;
};

template <class H, class Context>
class ContextImplBase : public PipelineContext {
 public:
  ~ContextImplBase() override = default;

  H* getHandler() {
    return handler_.get();
  }

  void initialize(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    pipelineWeak_ = pipeline;
    pipelineRaw_ = pipeline.lock().get();
    handler_ = std::move(handler);
  }

  // PipelineContext overrides
  void attachPipeline() override {
    if (!attached_) {
      this->attachContext(handler_.get(), impl_);
      handler_->attachPipeline(impl_);
      attached_ = true;
    }
  }

  void detachPipeline() override {
    handler_->detachPipeline(impl_);
    attached_ = false;
    this->detachContext(handler_.get(), impl_);
  }

  void setNextIn(PipelineContext* ctx) override {
    if (!ctx) {
      nextIn_ = nullptr;
      return;
    }
    auto nextIn = dynamic_cast<InboundLink<typename H::rout>*>(ctx);
    if (nextIn) {
      nextIn_ = nextIn;
    } else {
      throw std::invalid_argument(folly::sformat(
          "inbound type mismatch after {}", folly::demangle(typeid(H))));
    }
  }

  void setNextOut(PipelineContext* ctx) override {
    if (!ctx) {
      nextOut_ = nullptr;
      return;
    }
    auto nextOut = dynamic_cast<OutboundLink<typename H::wout>*>(ctx);
    if (nextOut) {
      nextOut_ = nextOut;
    } else {
      throw std::invalid_argument(folly::sformat(
          "outbound type mismatch after {}", folly::demangle(typeid(H))));
    }
  }

  HandlerDir getDirection() override {
    return H::dir;
  }

 protected:
  Context* impl_;
  std::weak_ptr<PipelineBase> pipelineWeak_;
  PipelineBase* pipelineRaw_;
  std::shared_ptr<H> handler_;
  InboundLink<typename H::rout>* nextIn_{nullptr};
  OutboundLink<typename H::wout>* nextOut_{nullptr};

 private:
  bool attached_{false};
};

template <class H>
class ContextImpl : public HandlerContext<typename H::rout, typename H::wout>,
                    public InboundLink<typename H::rin>,
                    public OutboundLink<typename H::win>,
                    public ContextImplBase<
                        H,
                        HandlerContext<typename H::rout, typename H::wout>> {
 public:
  typedef typename H::rin Rin;
  typedef typename H::rout Rout;
  typedef typename H::win Win;
  typedef typename H::wout Wout;
  static const HandlerDir dir = HandlerDir::BOTH;

  explicit ContextImpl(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    this->impl_ = this;
    this->initialize(pipeline, std::move(handler));
  }

  // For StaticPipeline
  ContextImpl() {
    this->impl_ = this;
  }

  ~ContextImpl() override = default;

  // HandlerContext overrides
  void fireRead(Rout msg) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->read(std::forward<Rout>(msg));
    } else {
      LOG(WARNING) << "read reached end of pipeline";
    }
  }

  void fireReadEOF() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->readEOF();
    } else {
      LOG(WARNING) << "readEOF reached end of pipeline";
    }
  }

  void fireReadException(folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->readException(std::move(e));
    } else {
      LOG(WARNING) << "readException reached end of pipeline";
    }
  }

  void fireTransportActive() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->transportActive();
    }
  }

  void fireTransportInactive() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->transportInactive();
    }
  }

  folly::Future<folly::Unit> fireWrite(Wout msg) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->write(std::forward<Wout>(msg));
    } else {
      LOG(WARNING) << "write reached end of pipeline";
      return folly::makeFuture();
    }
  }

  folly::Future<folly::Unit> fireWriteException(
      folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->writeException(std::move(e));
    } else {
      LOG(WARNING) << "close reached end of pipeline";
      return folly::makeFuture();
    }
  }

  folly::Future<folly::Unit> fireClose() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->close();
    } else {
      LOG(WARNING) << "close reached end of pipeline";
      return folly::makeFuture();
    }
  }

  PipelineBase* getPipeline() override {
    return this->pipelineRaw_;
  }

  std::shared_ptr<PipelineBase> getPipelineShared() override {
    return this->pipelineWeak_.lock();
  }

  void setWriteFlags(folly::WriteFlags flags) override {
    this->pipelineRaw_->setWriteFlags(flags);
  }

  folly::WriteFlags getWriteFlags() override {
    return this->pipelineRaw_->getWriteFlags();
  }

  void setReadBufferSettings(uint64_t minAvailable, uint64_t allocationSize)
      override {
    this->pipelineRaw_->setReadBufferSettings(minAvailable, allocationSize);
  }

  std::pair<uint64_t, uint64_t> getReadBufferSettings() override {
    return this->pipelineRaw_->getReadBufferSettings();
  }

  // InboundLink overrides
  void read(Rin msg) override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->read(this, std::forward<Rin>(msg));
  }

  void readEOF() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->readEOF(this);
  }

  void readException(folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->readException(this, std::move(e));
  }

  void transportActive() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->transportActive(this);
  }

  void transportInactive() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->transportInactive(this);
  }

  // OutboundLink overrides
  folly::Future<folly::Unit> write(Win msg) override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->write(this, std::forward<Win>(msg));
  }

  folly::Future<folly::Unit> writeException(
      folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->writeException(this, std::move(e));
  }

  folly::Future<folly::Unit> close() override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->close(this);
  }
};

template <class H>
class InboundContextImpl
    : public InboundHandlerContext<typename H::rout>,
      public InboundLink<typename H::rin>,
      public ContextImplBase<H, InboundHandlerContext<typename H::rout>> {
 public:
  typedef typename H::rin Rin;
  typedef typename H::rout Rout;
  typedef typename H::win Win;
  typedef typename H::wout Wout;
  static const HandlerDir dir = HandlerDir::IN;

  explicit InboundContextImpl(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    this->impl_ = this;
    this->initialize(pipeline, std::move(handler));
  }

  // For StaticPipeline
  InboundContextImpl() {
    this->impl_ = this;
  }

  ~InboundContextImpl() override = default;

  // InboundHandlerContext overrides
  void fireRead(Rout msg) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->read(std::forward<Rout>(msg));
    } else {
      LOG(WARNING) << "read reached end of pipeline";
    }
  }

  void fireReadEOF() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->readEOF();
    } else {
      LOG(WARNING) << "readEOF reached end of pipeline";
    }
  }

  void fireReadException(folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->readException(std::move(e));
    } else {
      LOG(WARNING) << "readException reached end of pipeline";
    }
  }

  void fireTransportActive() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->transportActive();
    }
  }

  void fireTransportInactive() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextIn_) {
      this->nextIn_->transportInactive();
    }
  }

  PipelineBase* getPipeline() override {
    return this->pipelineRaw_;
  }

  std::shared_ptr<PipelineBase> getPipelineShared() override {
    return this->pipelineWeak_.lock();
  }

  // InboundLink overrides
  void read(Rin msg) override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->read(this, std::forward<Rin>(msg));
  }

  void readEOF() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->readEOF(this);
  }

  void readException(folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->readException(this, std::move(e));
  }

  void transportActive() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->transportActive(this);
  }

  void transportInactive() override {
    auto guard = this->pipelineWeak_.lock();
    this->handler_->transportInactive(this);
  }
};

template <class H>
class OutboundContextImpl
    : public OutboundHandlerContext<typename H::wout>,
      public OutboundLink<typename H::win>,
      public ContextImplBase<H, OutboundHandlerContext<typename H::wout>> {
 public:
  typedef typename H::rin Rin;
  typedef typename H::rout Rout;
  typedef typename H::win Win;
  typedef typename H::wout Wout;
  static const HandlerDir dir = HandlerDir::OUT;

  explicit OutboundContextImpl(
      std::weak_ptr<PipelineBase> pipeline,
      std::shared_ptr<H> handler) {
    this->impl_ = this;
    this->initialize(pipeline, std::move(handler));
  }

  // For StaticPipeline
  OutboundContextImpl() {
    this->impl_ = this;
  }

  ~OutboundContextImpl() override = default;

  // OutboundHandlerContext overrides
  folly::Future<folly::Unit> fireWrite(Wout msg) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->write(std::forward<Wout>(msg));
    } else {
      LOG(WARNING) << "write reached end of pipeline";
      return folly::makeFuture();
    }
  }

  folly::Future<folly::Unit> fireWriteException(
      folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->writeException(std::move(e));
    } else {
      LOG(WARNING) << "close reached end of pipeline";
      return folly::makeFuture();
    }
  }

  folly::Future<folly::Unit> fireClose() override {
    auto guard = this->pipelineWeak_.lock();
    if (this->nextOut_) {
      return this->nextOut_->close();
    } else {
      LOG(WARNING) << "close reached end of pipeline";
      return folly::makeFuture();
    }
  }

  PipelineBase* getPipeline() override {
    return this->pipelineRaw_;
  }

  std::shared_ptr<PipelineBase> getPipelineShared() override {
    return this->pipelineWeak_.lock();
  }

  // OutboundLink overrides
  folly::Future<folly::Unit> write(Win msg) override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->write(this, std::forward<Win>(msg));
  }

  folly::Future<folly::Unit> writeException(
      folly::exception_wrapper e) override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->writeException(this, std::move(e));
  }

  folly::Future<folly::Unit> close() override {
    auto guard = this->pipelineWeak_.lock();
    return this->handler_->close(this);
  }
};

template <class Handler>
struct ContextType {
  typedef typename std::conditional<
      Handler::dir == HandlerDir::BOTH,
      ContextImpl<Handler>,
      typename std::conditional<
          Handler::dir == HandlerDir::IN,
          InboundContextImpl<Handler>,
          OutboundContextImpl<Handler>>::type>::type type;
};

} // namespace wangle
