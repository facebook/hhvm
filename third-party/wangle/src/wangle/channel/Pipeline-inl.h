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

#include <glog/logging.h>

namespace wangle {

template <class R, class W>
Pipeline<R, W>::Pipeline() : isStatic_(false) {}

template <class R, class W>
Pipeline<R, W>::Pipeline(bool isStatic) : isStatic_(isStatic) {
  CHECK(isStatic_);
}

template <class R, class W>
Pipeline<R, W>::~Pipeline() {
  if (!isStatic_) {
    detachHandlers();
  }
}

template <class H>
PipelineBase& PipelineBase::addBack(std::shared_ptr<H> handler) {
  typedef typename ContextType<H>::type Context;
  return addHelper(
      std::make_shared<Context>(shared_from_this(), std::move(handler)), false);
}

template <class H>
PipelineBase& PipelineBase::addBack(H&& handler) {
  return addBack(std::make_shared<H>(std::forward<H>(handler)));
}

template <class H>
PipelineBase& PipelineBase::addBack(H* handler) {
  return addBack(std::shared_ptr<H>(handler, [](H*) {}));
}

template <class H>
PipelineBase& PipelineBase::addFront(std::shared_ptr<H> handler) {
  typedef typename ContextType<H>::type Context;
  return addHelper(
      std::make_shared<Context>(shared_from_this(), std::move(handler)), true);
}

template <class H>
PipelineBase& PipelineBase::addFront(H&& handler) {
  return addFront(std::make_shared<H>(std::forward<H>(handler)));
}

template <class H>
PipelineBase& PipelineBase::addFront(H* handler) {
  return addFront(std::shared_ptr<H>(handler, [](H*) {}));
}

template <class H>
PipelineBase& PipelineBase::removeHelper(H* handler, bool checkEqual) {
  typedef typename ContextType<H>::type Context;
  bool removed = false;

  auto it = ctxs_.begin();
  while (it != ctxs_.end()) {
    auto ctx = std::dynamic_pointer_cast<Context>(*it);
    if (ctx && (!checkEqual || ctx->getHandler() == handler)) {
      it = removeAt(it);
      removed = true;
    } else {
      it++;
    }
  }

  if (!removed) {
    throw std::invalid_argument("No such handler in pipeline");
  }

  return *this;
}

template <class H>
PipelineBase& PipelineBase::remove() {
  return removeHelper<H>(nullptr, false);
}

template <class H>
PipelineBase& PipelineBase::remove(H* handler) {
  return removeHelper<H>(handler, true);
}

template <class H>
H* PipelineBase::getHandler(size_t i) {
  return getContext<H>(i)->getHandler();
}

template <class H>
H* PipelineBase::getHandler() {
  auto ctx = getContext<H>();
  return ctx ? ctx->getHandler() : nullptr;
}

template <class H>
typename ContextType<H>::type* PipelineBase::getContext(size_t i) {
  auto ctx = dynamic_cast<typename ContextType<H>::type*>(ctxs_[i].get());
  CHECK(ctx);
  return ctx;
}

template <class H>
typename ContextType<H>::type* PipelineBase::getContext() {
  for (auto pipelineCtx : ctxs_) {
    auto ctx = dynamic_cast<typename ContextType<H>::type*>(pipelineCtx.get());
    if (ctx) {
      return ctx;
    }
  }
  return nullptr;
}

template <class H>
bool PipelineBase::setOwner(H* handler) {
  typedef typename ContextType<H>::type Context;
  for (auto& ctx : ctxs_) {
    auto ctxImpl = dynamic_cast<Context*>(ctx.get());
    if (ctxImpl && ctxImpl->getHandler() == handler) {
      owner_ = ctx;
      return true;
    }
  }
  return false;
}

template <class Context>
void PipelineBase::addContextFront(Context* ctx) {
  addHelper(std::shared_ptr<Context>(ctx, [](Context*) {}), true);
}

template <class Context>
PipelineBase& PipelineBase::addHelper(
    std::shared_ptr<Context>&& ctx,
    bool front) {
  ctxs_.insert(front ? ctxs_.begin() : ctxs_.end(), ctx);
  if (Context::dir == HandlerDir::BOTH || Context::dir == HandlerDir::IN) {
    inCtxs_.insert(front ? inCtxs_.begin() : inCtxs_.end(), ctx.get());
  }
  if (Context::dir == HandlerDir::BOTH || Context::dir == HandlerDir::OUT) {
    outCtxs_.insert(front ? outCtxs_.begin() : outCtxs_.end(), ctx.get());
  }
  return *this;
}

namespace detail {

template <class T>
inline void logWarningIfNotUnit(const std::string& warning) {
  LOG(WARNING) << warning;
}

template <>
inline void logWarningIfNotUnit<folly::Unit>(const std::string& /*warning*/) {
  // do nothing
}

} // namespace detail

template <class R, class W>
template <class T>
typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
Pipeline<R, W>::read(R msg) {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);

  if (!front_) {
    throw std::invalid_argument("read(): no inbound handler in Pipeline");
  }
  front_->read(std::forward<R>(msg));
}

template <class R, class W>
template <class T>
typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
Pipeline<R, W>::readEOF() {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (!front_) {
    throw std::invalid_argument("readEOF(): no inbound handler in Pipeline");
  }
  front_->readEOF();
}

template <class R, class W>
template <class T>
typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
Pipeline<R, W>::transportActive() {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (front_) {
    front_->transportActive();
  }
}

template <class R, class W>
template <class T>
typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
Pipeline<R, W>::transportInactive() {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (front_) {
    front_->transportInactive();
  }
}

template <class R, class W>
template <class T>
typename std::enable_if<!std::is_same<T, folly::Unit>::value>::type
Pipeline<R, W>::readException(folly::exception_wrapper e) {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (!front_) {
    throw std::invalid_argument(
        "readException(): no inbound handler in Pipeline");
  }
  front_->readException(std::move(e));
}

template <class R, class W>
template <class T>
typename std::enable_if<
    !std::is_same<T, folly::Unit>::value,
    folly::Future<folly::Unit>>::type
Pipeline<R, W>::write(W msg) {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (!back_) {
    throw std::invalid_argument("write(): no outbound handler in Pipeline");
  }
  return back_->write(std::forward<W>(msg));
}

template <class R, class W>
template <class T>
typename std::enable_if<
    !std::is_same<T, folly::Unit>::value,
    folly::Future<folly::Unit>>::type
Pipeline<R, W>::writeException(folly::exception_wrapper e) {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (!back_) {
    throw std::invalid_argument(
        "writeException(): no outbound handler in Pipeline");
  }
  return back_->writeException(std::move(e));
}

template <class R, class W>
template <class T>
typename std::enable_if<
    !std::is_same<T, folly::Unit>::value,
    folly::Future<folly::Unit>>::type
Pipeline<R, W>::close() {
  OptionalReqCtxScopeGuard optGuard;
  fillRequestContextGuard(optGuard);
  if (!back_) {
    throw std::invalid_argument("close(): no outbound handler in Pipeline");
  }
  return back_->close();
}

// TODO Have read/write/etc check that pipeline has been finalized
template <class R, class W>
void Pipeline<R, W>::finalize() {
  front_ = nullptr;
  if (!inCtxs_.empty()) {
    front_ = dynamic_cast<InboundLink<R>*>(inCtxs_.front());
    for (size_t i = 0; i < inCtxs_.size() - 1; i++) {
      inCtxs_[i]->setNextIn(inCtxs_[i + 1]);
    }
    inCtxs_.back()->setNextIn(nullptr);
  }

  back_ = nullptr;
  if (!outCtxs_.empty()) {
    back_ = dynamic_cast<OutboundLink<W>*>(outCtxs_.back());
    for (size_t i = outCtxs_.size() - 1; i > 0; i--) {
      outCtxs_[i]->setNextOut(outCtxs_[i - 1]);
    }
    outCtxs_.front()->setNextOut(nullptr);
  }

  if (!front_) {
    detail::logWarningIfNotUnit<R>(
        "No inbound handler in Pipeline, inbound operations will throw "
        "std::invalid_argument");
  }
  if (!back_) {
    detail::logWarningIfNotUnit<W>(
        "No outbound handler in Pipeline, outbound operations will throw "
        "std::invalid_argument");
  }

  for (auto it = ctxs_.rbegin(); it != ctxs_.rend(); it++) {
    (*it)->attachPipeline();
  }
}

template <class R, class W>
void Pipeline<R, W>::fillRequestContextGuard(
    OptionalReqCtxScopeGuard& optGuard) {
  CHECK(!optGuard.has_value());
  if (requestContext_) {
    optGuard.emplace(requestContext_);
  }
}

} // namespace wangle
