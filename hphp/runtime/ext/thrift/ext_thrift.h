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

#include <folly/Overload.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <variant>
#include <exception>
#include <tuple>
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
namespace thrift {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_THRIFT_MARK_LEGACY_ARRAYS;

void HHVM_FUNCTION(
    thrift_protocol_write_binary,
    const Object& transportobj,
    const String& method_name,
    int64_t msgtype,
    const Object& request_struct,
    int64_t seqid,
    bool strict_write,
    bool oneway = false);

Object HHVM_FUNCTION(
    thrift_protocol_read_binary,
    const Object& transportobj,
    const String& obj_typename,
    bool strict_read,
    int64_t options);

Variant HHVM_FUNCTION(
    thrift_protocol_read_binary_struct,
    const Object& transportobj,
    const String& obj_typename,
    int64_t options);

int64_t HHVM_FUNCTION(thrift_protocol_set_compact_version, int64_t version);

void HHVM_FUNCTION(
    thrift_protocol_write_compact,
    const Object& transportobj,
    const String& method_name,
    int64_t msgtype,
    const Object& request_struct,
    int64_t seqid,
    bool oneway = false);

void HHVM_FUNCTION(
    thrift_protocol_write_compact2,
    const Object& transportobj,
    const String& method_name,
    int64_t msgtype,
    const Object& request_struct,
    int64_t seqid,
    bool oneway = false,
    int64_t version = 2);

Variant HHVM_FUNCTION(
    thrift_protocol_read_compact,
    const Object& transportobj,
    const String& obj_typename,
    int64_t options);

Object HHVM_FUNCTION(
    thrift_protocol_read_compact_struct,
    const Object& transportobj,
    const String& obj_typename,
    int64_t options);

///////////////////////////////////////////////////////////////////////////////

struct InteractionId {
  static Object newInstance() {
    return Object{PhpClass()};
  }

  static Class* PhpClass();

  ~InteractionId() {
    sweep();
  }

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

  void sweep() {
    close(true);
  }

  void close(bool /*sweeping*/ = false) {}

  static Class* PhpClass() {
    if (!c_RpcOptions) {
      c_RpcOptions = Class::lookup(s_RpcOptions.get());
      assert(c_RpcOptions);
    }
    return c_RpcOptions;
  }

  static Object newInstance() {
    return Object{PhpClass()};
  }

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

struct TClientStreamError {
 public:
  TClientStreamError() {}
  TClientStreamError(std::unique_ptr<folly::IOBuf> encodedErrorMsg)
      : errorMsg_(std::move(encodedErrorMsg)), isEncoded_(true) {}
  TClientStreamError(std::string errorStr)
      : errorMsg_(folly::IOBuf::copyBuffer(errorStr, errorStr.size())),
        isEncoded_(false) {}

  static TClientStreamError create(folly::exception_wrapper ew) {
    std::unique_ptr<folly::IOBuf> msgBuffer;
    std::string msgStr = ew.what().toStdString();
    ew.handle(
        [&](apache::thrift::detail::EncodedError& err) {
          msgBuffer = std::move(err.encoded);
        },
        [&](apache::thrift::detail::EncodedStreamError& err) {
          auto& payload = err.encoded;
          DCHECK(payload.metadata.payloadMetadata().has_value());
          DCHECK_EQ(
              payload.metadata.payloadMetadata()->getType(),
              apache::thrift::PayloadMetadata::exceptionMetadata);
          auto& exceptionMetadataBase =
              payload.metadata.payloadMetadata()->get_exceptionMetadata();
          if (auto exceptionMetadataRef =
                  exceptionMetadataBase.metadata()) {
            if (exceptionMetadataRef->getType() ==
                apache::thrift::PayloadExceptionMetadata::declaredException) {
              msgBuffer = std::move(payload.payload);
              if (!msgBuffer) {
                msgStr = "Failed to parse declared exception";
              }
            } else {
              msgStr = exceptionMetadataBase.what_utf8().value_or("");
            }
          } else {
            msgStr = "Missing payload exception metadata";
          }
        },
        [&](apache::thrift::detail::EncodedStreamRpcError& err) {
          apache::thrift::StreamRpcError streamRpcError;
          apache::thrift::CompactProtocolReader reader;
          reader.setInput(err.encoded.get());
          streamRpcError.read(&reader);
          msgStr = streamRpcError.what_utf8().value_or("");
        },
        [](...) {});

    if (msgBuffer) {
      return TClientStreamError(std::move(msgBuffer));
    }
    return TClientStreamError(msgStr);
  }

  std::unique_ptr<folly::IOBuf> errorMsg_;
  bool isEncoded_;
};

const StaticString s_TClientBufferedStream("TClientBufferedStream");

struct TClientBufferedStream {
  TClientBufferedStream() = default;
  TClientBufferedStream(const TClientBufferedStream&) = delete;
  TClientBufferedStream& operator=(const TClientBufferedStream&) = delete;
  ~TClientBufferedStream() {
    sweep();
  }

  using BufferAndErrorPair =
      std::pair<std::vector<std::unique_ptr<folly::IOBuf>>, TClientStreamError>;

  void sweep() {
    endStream();
  }

  void init(
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      apache::thrift::BufferOptions bufferOptions) {
    streamBridge_ = std::move(streamBridge);
    bufferOptions_ = bufferOptions;
    if (bufferOptions_.chunkSize == 0) {
      streamBridge_->requestN(1);
      ++bufferOptions_.chunkSize;
    }
    outstanding_ = bufferOptions_.chunkSize;
    payloadDataSize_ = 0;
  }

  void endStream() {
    streamBridge_.reset();
  }

  bool shouldRequestMore() {
    return (outstanding_ <= bufferOptions_.chunkSize / 2) ||
        (payloadDataSize_ >= kRequestCreditPayloadSize);
  }

  BufferAndErrorPair clientQueueToVec() {
    std::vector<std::unique_ptr<folly::IOBuf>> bufferVec;
    TClientStreamError error;
    while (!queue_.empty()) {
      auto& payload = queue_.front();
      if (!payload.hasValue() && !payload.hasException()) {
        queue_.pop();
        endStream();
        break;
      }
      if (payload.hasException()) {
        error = TClientStreamError::create(payload.exception());
        queue_.pop();
        endStream();
        break;
      }
      if (payload->payload) {
        payloadDataSize_ += payload->payload->computeChainDataLength();
        bufferVec.push_back(std::move(payload->payload));
        --outstanding_;
      }
      queue_.pop();
      if (shouldRequestMore()) {
        break;
      }
    }
    return std::make_pair(std::move(bufferVec), std::move(error));
  }

  static Class* PhpClass() {
    if (!c_TClientBufferedStream) {
      c_TClientBufferedStream = Class::lookup(s_TClientBufferedStream.get());
      assert(c_TClientBufferedStream);
    }
    return c_TClientBufferedStream;
  }

  static Object newInstance() {
    return Object{PhpClass()};
  }

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
  Object genNext();

 public:
  apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge_;
  apache::thrift::BufferOptions bufferOptions_;
  apache::thrift::detail::ClientStreamBridge::ClientQueue queue_;
  int32_t outstanding_ = 0;
  size_t payloadDataSize_ = 0;
  static constexpr size_t kRequestCreditPayloadSize = 16384;

  static Class* c_TClientBufferedStream;
};

///////////////////////////////////////////////////////////////////////////////

using TClientSinkCreditsOrFinalResponse =
    std::variant<uint64_t, std::unique_ptr<folly::IOBuf>, TClientStreamError>;

const StaticString s_TClientSink("TClientSink");

struct TClientSink {
  TClientSink() = default;
  TClientSink(const TClientSink&) = delete;
  TClientSink& operator=(const TClientSink&) = delete;
  ~TClientSink() {
    sweep();
  }

  void sweep() {
    cancel();
  }

  void init(apache::thrift::detail::ClientSinkBridge::Ptr sinkBridge) {
    sinkBridge_ = std::move(sinkBridge);
  }

  void endSink() {
    sinkBridge_.reset();
  }

  void cancel() {
    if (sinkBridge_) {
      sinkBridge_->cancel(folly::Try<apache::thrift::StreamPayload>(
          apache::thrift::rocket::RocketException(
              apache::thrift::rocket::ErrorCode::CANCELED)));
      endSink();
    }
  }

  static Class* PhpClass() {
    if (!c_TClientSink) {
      c_TClientSink = Class::lookup(s_TClientSink.get());
      assert(c_TClientSink);
    }
    return c_TClientSink;
  }

  static Object newInstance() {
    return Object{PhpClass()};
  }

  static TClientSink* GetDataOrThrowException(ObjectData* object_) {
    if (object_ == nullptr) {
      throw_null_pointer_exception();
      not_reached();
    }
    if (!object_->getVMClass()->classofNonIFace(PhpClass())) {
      raise_error("TClientSink expected");
      not_reached();
    }
    return Native::data<TClientSink>(object_);
  }

  TClientSinkCreditsOrFinalResponse getCreditsOrFinalResponse() {
    uint64_t credits = 0;
    folly::Try<apache::thrift::StreamPayload> response;
    bool responseAvailable = false;

    auto creditsQueue_ = sinkBridge_->getMessages();
    while (!creditsQueue_.empty() && !responseAvailable) {
      auto& message = creditsQueue_.front();
      folly::variant_match(
          message,
          [&](folly::Try<apache::thrift::StreamPayload>& payload) {
            response = std::move(payload);
            responseAvailable = true;
          },
          [&](uint64_t n) { credits += n; });
      creditsQueue_.pop();
    }
    if (responseAvailable) {
      endSink();
      if (response.hasException()) {
        return TClientSinkCreditsOrFinalResponse(
            TClientStreamError::create(response.exception()));
      }
      if (response->payload) {
        return TClientSinkCreditsOrFinalResponse(std::move(response->payload));
      }
      return TClientSinkCreditsOrFinalResponse();
    }
    return TClientSinkCreditsOrFinalResponse(credits);
  }

 public:
  apache::thrift::detail::ClientSinkBridge::Ptr sinkBridge_;
  static Class* c_TClientSink;
};
} // namespace thrift

inline String ioBufToString(const folly::IOBuf& ioBuf) {
  auto resultStringData = StringData::Make(ioBuf.computeChainDataLength());
  for (const auto& buf : ioBuf) {
    auto const data = reinterpret_cast<const char*>(buf.data());
    auto const piece = folly::StringPiece{data, buf.size()};
    resultStringData->append(piece);
  }
  return String::attach(resultStringData);
}

struct Thrift2StreamEvent final : AsioExternalThreadEvent {
  Thrift2StreamEvent() {}

  void finish(
      thrift::TClientBufferedStream::BufferAndErrorPair bufferAndError) {
    buffer_ = std::move(bufferAndError.first);
    error_ = std::move(bufferAndError.second);
    markAsFinished();
  }

  void error(std::unique_ptr<folly::IOBuf> err) {
    serverError_ = std::move(err);
    markAsFinished();
  }

 protected:
  // Called by PHP thread
  void unserialize(TypedValue& result) final {
    if (serverError_) {
      throw_object(
          "TTransportException",
          make_vec_array(
              ioBufToString(*serverError_),
              0 // error code UNKNOWN
              ));
    }

    String errorStr = null_string;
    if (error_.errorMsg_) {
      if (error_.isEncoded_) {
        // Encoded stream exceptions need to be deserialized by generated code.
        // Adding it at the end of the buffer vec so that it will be decoded
        // along with other payloads and will throw exception after returning
        // all payloads to client.
        buffer_.push_back(std::move(error_.errorMsg_));
      } else {
        errorStr = ioBufToString(*error_.errorMsg_);
      }
    }

    Array bufferVec;
    if (buffer_.empty()) {
      bufferVec = null_array;
    } else {
      VecInit resArr(buffer_.size());
      for (auto& ioBuf : buffer_) {
        resArr.append(ioBufToString(*ioBuf));
      }
      bufferVec = resArr.toArray();
    }
    tvCopy(
        make_array_like_tv(make_vec_array(bufferVec, errorStr).detach()),
        result);
  }

 private:
  std::vector<std::unique_ptr<folly::IOBuf>> buffer_;

  // any error while processing queue, this should be returned as error message
  // along with the buffer and should be sequenced after any received payloads
  thrift::TClientStreamError error_;

  // Unexpected server error which should be thrown immediately
  std::unique_ptr<folly::IOBuf> serverError_;
};

struct ThriftSinkEvent final : AsioExternalThreadEvent {
  ThriftSinkEvent() = default;

  void finish(thrift::TClientSinkCreditsOrFinalResponse credits) {
    creditsOrFinalResponse_ = std::move(credits);
    markAsFinished();
  }

  void error(std::unique_ptr<folly::IOBuf> err) {
    serverError_ = std::move(err);
    markAsFinished();
  }

 protected:
  // Called by PHP thread
  void unserialize(TypedValue& result) final {
    if (serverError_) {
      throw_object(
          "TTransportException",
          make_vec_array(
              ioBufToString(*serverError_),
              0 // error code UNKNOWN
              ));
    }
    auto responseStr = null_string;
    auto errorStr = null_string;
    uint64_t credits = 0;
    folly::variant_match(
        creditsOrFinalResponse_,
        [&](const std::unique_ptr<folly::IOBuf>& finalResponse) {
          responseStr = ioBufToString(*finalResponse);
        },
        [&](const thrift::TClientStreamError& finalResponseError) {
          if (finalResponseError.errorMsg_) {
            // Encoded stream exceptions need to be deserialized by generated
            // code. Returning it as the response string so that it will be
            // decoded
            if (finalResponseError.isEncoded_) {
              responseStr = ioBufToString(*finalResponseError.errorMsg_);
            } else {
              errorStr = ioBufToString(*finalResponseError.errorMsg_);
            }
          }
        },
        [&](const uint64_t& n) { credits = n; });
    return tvCopy(
        make_array_like_tv(
            make_vec_array(credits, responseStr, errorStr).detach()),
        result);
  }

 private:
  thrift::TClientSinkCreditsOrFinalResponse creditsOrFinalResponse_;

  // Unexpected server error which should be thrown immediately
  std::unique_ptr<folly::IOBuf> serverError_;
};
} // namespace HPHP
