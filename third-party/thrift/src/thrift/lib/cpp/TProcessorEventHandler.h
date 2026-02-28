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

#include <string_view>
#include <typeinfo>

#include <folly/Demangle.h>
#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp/BiDiEventHandler.h>
#include <thrift/lib/cpp/SerializedMessage.h>
#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp/server/TConnectionContext.h>
#include <thrift/lib/cpp/transport/THeader.h>

namespace apache::thrift {

using server::TConnectionContext;

/**
 * Virtual interface class that can handle events from the processor. To
 * use this you should subclass it and implement the methods that you care
 * about. Your subclass can also store local data that you may care about,
 * such as additional "arguments" to these methods (stored in the object
 * instance's state).
 */
class TProcessorEventHandler {
 public:
  virtual ~TProcessorEventHandler() {}

  /**
   * Returns the name of the TProcessorEventHandler implementation
   */
  virtual std::string getName() const {
    return folly::demangle(typeid(*this).name()).toStdString();
  }

  /**
   * Called before calling other callback methods.
   * Expected to return some sort of context object.
   * The return value is passed to all other callbacks
   * for that function invocation.
   */
  virtual void* getServiceContext(
      std::string_view /*service_name*/,
      std::string_view fn_name,
      TConnectionContext* connectionContext) {
    return getContext(fn_name, connectionContext);
  }
  virtual void* getContext(
      std::string_view /*fn_name*/, TConnectionContext* /*connectionContext*/) {
    return nullptr;
  }

  /**
   * Expected to free resources associated with a context.
   */
  virtual void freeContext(void* /*ctx*/, std::string_view /*fn_name*/) {}

  /**
   * Called before reading arguments.
   */
  virtual void preRead(void* /*ctx*/, std::string_view /*fn_name*/) {}

  /**
   * Called before postRead, after reading arguments (server) / after reading
   * reply (client), with the actual (unparsed, serialized) data.
   *
   * The data is framed by message begin / end entries, call readMessageBegin /
   * readMessageEnd on the protocol.
   *
   * Only called for Cpp2.
   */
  virtual void onReadData(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      const SerializedMessage& /*msg*/) {}

  /**
   * Called between reading arguments and calling the handler.
   */
  virtual void postRead(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      apache::thrift::transport::THeader* /*header*/,
      uint32_t /*bytes*/) {}

  /**
   * Called between calling the handler and writing the response.
   */
  virtual void preWrite(void* /*ctx*/, std::string_view /*fn_name*/) {}

  /**
   * Called before postWrite, after serializing response (server) / after
   * serializing request (client), with the actual (serialized) data.
   *
   * The data is framed by message begin / end entries, call readMessageBegin /
   * readMessageEnd on the protocol.
   *
   * Only called for Cpp2.
   */
  virtual void onWriteData(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      const SerializedMessage& /*msg*/) {}

  /**
   * Called after writing the response.
   */
  virtual void postWrite(
      void* /*ctx*/, std::string_view /*fn_name*/, uint32_t /*bytes*/) {}

  /**
   * Called when an interaction is terminated (not necessarily
   * part of a thrift request, unlike the other callbacks).
   * Must override `wantNonPerRequestCallbacks` to true to receive callbacks.
   */
  virtual void onInteractionTerminate(void* /*ctx*/, int64_t /*id*/) {}

  /**
   * defaults to false.
   *
   * When false, `getContext` is only called as part of a thrift request
   * (meaning typically has a connection + header + folly::RequestContext
   * available etc).
   * When true, `getContext` is also called as part of a non-thrift request,
   * such as interaction terminate. Impl must init/free contexts without
   * assuming the existence of per-request objects like header /
   * folly::RequestContext.
   */
  virtual bool wantNonPerRequestCallbacks() const { return false; }

  /**
   * Called if the handler throws an undeclared exception.
   */
  virtual void handlerError(void* /*ctx*/, std::string_view /*fn_name*/) {}
  virtual void handlerErrorWrapped(
      void* ctx,
      std::string_view fn_name,
      const folly::exception_wrapper& /*ew*/) {
    handlerError(ctx, fn_name);
  }

  /**
   * Called if the handler throws an exception.
   *
   * Only called for Cpp2
   */
  virtual void userException(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      std::string_view /*ex*/,
      std::string_view /*ex_what*/) {}
  virtual void userExceptionWrapped(
      void* ctx,
      std::string_view fn_name,
      bool declared,
      const folly::exception_wrapper& ew_) {
    const auto type = ew_.class_name();
    std::string what;
    if (declared) {
      if (auto* eptr = ew_.get_exception()) {
        what = eptr->what();
      } else {
        LOG(FATAL)
            << "declared exception does not derive from std::exception; what: "
            << ew_.what();
      }
    } else {
      what = ew_.what().toStdString();
    }
    return userException(ctx, fn_name, type, what);
  }

  StreamEventHandler* getStreamEventHandler() {
    return streamEventHandler_.get();
  }

  BiDiEventHandler* getBiDiEventHandler() { return bidiEventHandler_.get(); }

 protected:
  TProcessorEventHandler()
      : streamEventHandler_(nullptr), bidiEventHandler_(nullptr) {}
  explicit TProcessorEventHandler(
      std::unique_ptr<StreamEventHandler> streamEventHandler)
      : streamEventHandler_(std::move(streamEventHandler)),
        bidiEventHandler_(nullptr) {}
  explicit TProcessorEventHandler(
      std::unique_ptr<StreamEventHandler> streamEventHandler,
      std::unique_ptr<BiDiEventHandler> bidiEventHandler)
      : streamEventHandler_(std::move(streamEventHandler)),
        bidiEventHandler_(std::move(bidiEventHandler)) {}

 private:
  std::unique_ptr<StreamEventHandler> streamEventHandler_;
  std::unique_ptr<BiDiEventHandler> bidiEventHandler_;
};

/**
 * Derive from this interface if the ProcessorEventHandler is uninterested in
 * user exceptions.
 *
 * The ProcessorEventHandler user exception interface performs symbol demangling
 * which is relatively expensive. Many ProcessorEventHandlers are uninterested
 * in user exceptions which results in that work being discarded.
 */
class TProcessorEventHandlerNoUserExnCallbacks : public TProcessorEventHandler {
 public:
  void userException(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      std::string_view /*ex*/,
      std::string_view /*ex_what*/) final {}

  void userExceptionWrapped(
      void* /*ctx*/,
      std::string_view /*fn_name*/,
      bool /*declared*/,
      const folly::exception_wrapper& /*ew_*/) final {}
};

} // namespace apache::thrift
