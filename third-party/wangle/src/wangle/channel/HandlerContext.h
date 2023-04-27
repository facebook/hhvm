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

#include <folly/ExceptionWrapper.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncTransport.h>

namespace wangle {

class PipelineBase;

template <class In, class Out>
class HandlerContext {
 public:
  virtual ~HandlerContext() = default;

  virtual void fireRead(In msg) = 0;
  virtual void fireReadEOF() = 0;
  virtual void fireReadException(folly::exception_wrapper e) = 0;
  virtual void fireTransportActive() = 0;
  virtual void fireTransportInactive() = 0;

  virtual folly::Future<folly::Unit> fireWrite(Out msg) = 0;
  virtual folly::Future<folly::Unit> fireWriteException(
      folly::exception_wrapper e) = 0;
  virtual folly::Future<folly::Unit> fireClose() = 0;

  virtual PipelineBase* getPipeline() = 0;
  virtual std::shared_ptr<PipelineBase> getPipelineShared() = 0;
  std::shared_ptr<folly::AsyncTransport> getTransport() {
    return getPipeline()->getTransport();
  }

  virtual void setWriteFlags(folly::WriteFlags flags) = 0;
  virtual folly::WriteFlags getWriteFlags() = 0;

  virtual void setReadBufferSettings(
      uint64_t minAvailable,
      uint64_t allocationSize) = 0;
  virtual std::pair<uint64_t, uint64_t> getReadBufferSettings() = 0;

  /* TODO
  template <class H>
  virtual void addHandlerBefore(H&&) {}
  template <class H>
  virtual void addHandlerAfter(H&&) {}
  template <class H>
  virtual void replaceHandler(H&&) {}
  virtual void removeHandler() {}
  */
};

template <class In>
class InboundHandlerContext {
 public:
  virtual ~InboundHandlerContext() = default;

  virtual void fireRead(In msg) = 0;
  virtual void fireReadEOF() = 0;
  virtual void fireReadException(folly::exception_wrapper e) = 0;
  virtual void fireTransportActive() = 0;
  virtual void fireTransportInactive() = 0;

  virtual PipelineBase* getPipeline() = 0;
  virtual std::shared_ptr<PipelineBase> getPipelineShared() = 0;
  std::shared_ptr<folly::AsyncTransport> getTransport() {
    return getPipeline()->getTransport();
  }

  // TODO Need get/set writeFlags, readBufferSettings? Probably not.
  // Do we even really need them stored in the pipeline at all?
  // Could just always delegate to the socket impl
};

template <class Out>
class OutboundHandlerContext {
 public:
  virtual ~OutboundHandlerContext() = default;

  virtual folly::Future<folly::Unit> fireWrite(Out msg) = 0;
  virtual folly::Future<folly::Unit> fireWriteException(
      folly::exception_wrapper e) = 0;
  virtual folly::Future<folly::Unit> fireClose() = 0;

  virtual PipelineBase* getPipeline() = 0;
  virtual std::shared_ptr<PipelineBase> getPipelineShared() = 0;
  std::shared_ptr<folly::AsyncTransport> getTransport() {
    return getPipeline()->getTransport();
  }
};

// #include <windows.h> has blessed us with #define IN & OUT, typically mapped
// to nothing, so letting the preprocessor delete each of these symbols, leading
// to interesting compiler errors around HandlerDir.
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

enum class HandlerDir { IN, OUT, BOTH };

} // namespace wangle

#include <wangle/channel/HandlerContext-inl.h>
