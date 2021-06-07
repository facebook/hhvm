/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"
#include "thrift/lib/cpp2/async/RequestCallback.h"
#include "thrift/lib/cpp2/async/RequestChannel.h"

namespace HPHP { namespace thrift {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_THRIFT_MARK_LEGACY_ARRAYS;

void HHVM_FUNCTION(thrift_protocol_write_binary,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool strict_write,
                   bool oneway = false);

Object HHVM_FUNCTION(thrift_protocol_read_binary,
                     const Object& transportobj,
                     const String& obj_typename,
                     bool strict_read,
                     int options);

Variant HHVM_FUNCTION(thrift_protocol_read_binary_struct,
                      const Object& transportobj,
                      const String& obj_typename,
                      int options);

int64_t HHVM_FUNCTION(thrift_protocol_set_compact_version,
                      int version);

void HHVM_FUNCTION(thrift_protocol_write_compact,
                   const Object& transportobj,
                   const String& method_name,
                   int64_t msgtype,
                   const Object& request_struct,
                   int seqid,
                   bool oneway = false);

Variant HHVM_FUNCTION(thrift_protocol_read_compact,
                      const Object& transportobj,
                      const String& obj_typename,
                      int options);

Object HHVM_FUNCTION(thrift_protocol_read_compact_struct,
                     const Object& transportobj,
                     const String& obj_typename,
                     int options);

///////////////////////////////////////////////////////////////////////////////

struct InteractionId {
  static Object newInstance() { return Object{PhpClass()}; }

  static Class* PhpClass();

  ~InteractionId() { sweep(); }

  void sweep() {
    if (interactionId_) {
      channel_->terminateInteraction(std::move(interactionId_));
      channel_.reset();
    }
  }

  void attach(
      const std::shared_ptr<apache::thrift::RequestChannel>& channel,
      apache::thrift::InteractionId&& id) {
    channel_ = channel;
    interactionId_ = std::move(id);
  }

  const apache::thrift::InteractionId& getInteractionId() const {
    return interactionId_;
  }

  private:
  apache::thrift::InteractionId interactionId_;
  std::shared_ptr<apache::thrift::RequestChannel> channel_;
};

/////////////////////////////////////////////////////////////////////////////

const StaticString s_RpcOptions("RpcOptions");

struct RpcOptions {
  RpcOptions() {}
  RpcOptions& operator=(const RpcOptions& that_) {
    return *this;
  }

  void sweep() { close(true); }

  void close(bool /*sweeping*/ = false) {}

  static Class* PhpClass() {
    if (!c_RpcOptions) {
      c_RpcOptions = Class::lookup(s_RpcOptions.get());
      assert(c_RpcOptions);
    }
    return c_RpcOptions;
  }

  static Object newInstance() { return Object{PhpClass()}; }

  static RpcOptions* GetDataOrThrowException(ObjectData* object_) {
    if (object_ == nullptr) {
      throw_null_pointer_exception();
      not_reached();
    }
    if (!object_->getVMClass()->classofNonIFace(PhpClass())) {
      raise_error("RpcOptions expected");
      not_reached();
    }
    return Native::data<RpcOptions>(object_);
  }

  apache::thrift::RpcOptions rpcOptions;
 private:
  static Class* c_RpcOptions;
  TYPE_SCAN_IGNORE_ALL;
};

///////////////////////////////////////////////////////////////////////////////

const StaticString s_TClientBufferedStream("TClientBufferedStream");

struct TClientBufferedStream {
  TClientBufferedStream() = default;
  TClientBufferedStream(const TClientBufferedStream&) = delete;
  TClientBufferedStream& operator=(const TClientBufferedStream&) = delete;
  ~TClientBufferedStream();

  void sweep() { close(true); }

  void close(bool /*sweeping*/ = false) {}

  void init(
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      apache::thrift::BufferOptions bufferOptions) {
    streamBridge_ = std::move(streamBridge);
    bufferOptions_ = bufferOptions;
  }

  static Class* PhpClass() {
    if (!c_TClientBufferedStream) {
      c_TClientBufferedStream = Class::lookup(s_TClientBufferedStream.get());
      assert(c_TClientBufferedStream);
    }
    return c_TClientBufferedStream;
  }

  static Object newInstance() { return Object{PhpClass()}; }

  static TClientBufferedStream* GetDataOrThrowException(ObjectData* object_) {
    if (object_ == nullptr) {
      throw_null_pointer_exception();
      not_reached();
    }
    if (!object_->getVMClass()->classofNonIFace(PhpClass())) {
      raise_error("TClientBufferedStream expected");
      not_reached();
    }
    return Native::data<TClientBufferedStream>(object_);
  }

 private:
  apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge_;
  apache::thrift::BufferOptions bufferOptions_;

  static Class* c_TClientBufferedStream;
};
}}
