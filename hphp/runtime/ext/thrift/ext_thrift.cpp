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

#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/runtime/base/types.h"

namespace HPHP::thrift {

///////////////////////////////////////////////////////////////////////////////

const int64_t k_THRIFT_MARK_LEGACY_ARRAYS = 1LL << 0;

const StaticString s_InteractionId("InteractionId");

Class* InteractionId::PhpClass() {
  Class* c_InteractionId = Class::lookup(s_InteractionId.get());
  assert(c_InteractionId);
  return c_InteractionId;
}

Class* RpcOptions::c_RpcOptions = nullptr;
Class* TClientBufferedStream::c_TClientBufferedStream = nullptr;
Class* TClientSink::c_TClientSink = nullptr;


Object HHVM_METHOD(
    TClientSink,
    genCreditsOrFinalResponse) {
  auto data = TClientSink::GetDataOrThrowException(this_);
  if (!data->sinkBridge_) {
    return null_object;
  }
  auto event = new ThriftSinkEvent();

  // Make sure event is abandoned, in case of an error
  auto guard = folly::makeGuard([=] { event->abandon(); });

  class ReadyCallback : public apache::thrift::detail::ClientSinkConsumer {
   public:
    explicit ReadyCallback(
        HPHP::thrift::TClientSink* sink,
        HPHP::ThriftSinkEvent* event)
        : sink(sink), event(event) {}
    void consume() override {
      event->finish(sink->getCreditsOrFinalResponse());
      delete this;
    }

    void canceled() override {
      // If we reach here then that means the sink is cancelled by
      // destroying the object. So finish
      // the event immediately, with an error message to be safe.
      std::string msg = "Something went wrong, queue is empty";
      std::unique_ptr<folly::IOBuf> msgBuffer =
          folly::IOBuf::copyBuffer(msg, msg.size());
      event->error(std::move(msgBuffer));
      delete this;
    }

   private:
    HPHP::thrift::TClientSink* sink;
    HPHP::ThriftSinkEvent* event;
  };

  auto callback = new ReadyCallback(data, event);
  if (!data->sinkBridge_->wait(callback)) {
    callback->consume();
  }

  guard.dismiss();
  return Object{event->getWaitHandle()};
}

bool HHVM_METHOD(
    TClientSink,
    sendPayloadOrSinkComplete,
    const Variant& payload) {
  auto data = TClientSink::GetDataOrThrowException(this_);
  if (!data->sinkBridge_) {
    return false;
  }
  if (data->sinkBridge_->hasServerCancelled()) {
    // If server has already cancelled, then final response is available
    // and we should not send any more payloads
    return false;
  }

  // we don't need to check if we have credits since this is only called
  // if we passed some credits to php
  if (payload.isNull()) {
    data->sinkBridge_->push({});
    return false;
  } else if (payload.isString()) {
    const String& sVal = payload.toString();
    data->sinkBridge_->push(
        folly::Try<apache::thrift::StreamPayload>(apache::thrift::StreamPayload(
            folly::IOBuf::copyBuffer(sVal.c_str(), sVal.size()), {})));
    return true;
  } else {
    raise_error("Payload needs to be a string");
  }
}

void HHVM_METHOD(
    TClientSink,
    sendClientException,
    const String& ex_encoded_string,
    const Variant& ex_msg) {
  auto data = TClientSink::GetDataOrThrowException(this_);
  if (!data->sinkBridge_) {
    return;
  }

  if (data->sinkBridge_->hasServerCancelled()) {
    // server has already cancelled, so we can skip sending exception
    return;
  }
  auto buf = folly::IOBuf::copyBuffer(
      ex_encoded_string.c_str(), ex_encoded_string.size());
  apache::thrift::PayloadExceptionMetadata exceptionMetadata;
  apache::thrift::PayloadExceptionMetadataBase exceptionMetadataBase;
  if (ex_msg.isNull()) {
    exceptionMetadata.declaredException_ref() = apache::thrift::PayloadDeclaredExceptionMetadata();
  } else if (ex_msg.isString()) {
    exceptionMetadataBase.what_utf8() = ex_msg.toString().c_str();
    apache::thrift::PayloadAppUnknownExceptionMetdata aue;
    aue.errorClassification().ensure().blame() =
        apache::thrift::ErrorBlame::CLIENT;
    exceptionMetadata.appUnknownException_ref() = std::move(aue);
  }

  exceptionMetadataBase.metadata() = std::move(exceptionMetadata);
  apache::thrift::StreamPayloadMetadata streamPayloadMetadata;
  apache::thrift::PayloadMetadata payloadMetadata;
  payloadMetadata.exceptionMetadata_ref() = std::move(exceptionMetadataBase);
  streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
  data->sinkBridge_->push(folly::Try<apache::thrift::StreamPayload>(
            folly::make_exception_wrapper<apache::thrift::detail::EncodedStreamError>(
          apache::thrift::StreamPayload(
              std::move(buf), std::move(streamPayloadMetadata)))));
  data->endSink();
}

Object HHVM_METHOD(TClientBufferedStream, genNext) {
  auto data = TClientBufferedStream::GetDataOrThrowException(this_);
  if (!data->streamBridge_) {
    return null_object;
  }
  auto event = new Thrift2StreamEvent();

  // Make sure event is abandoned, in case of an error
  auto guard = folly::makeGuard([=] { event->abandon(); });

  if (data->shouldRequestMore()) {
    data->streamBridge_->requestN(
        data->bufferOptions_.chunkSize - data->outstanding_);
    data->outstanding_ = data->bufferOptions_.chunkSize;
    data->payloadDataSize_ = 0;
  }

  if (!data->queue_.empty()) {
    event->finish(data->clientQueueToVec());
    guard.dismiss();
    return Object{event->getWaitHandle()};
  }

  class ReadyCallback : public apache::thrift::detail::ClientStreamConsumer {
   public:
    explicit ReadyCallback(
        HPHP::thrift::TClientBufferedStream* stream,
        HPHP::Thrift2StreamEvent* event)
        : stream(stream), event(event) {}
    void consume() override {
      stream->queue_ = stream->streamBridge_->getMessages();
      event->finish(stream->clientQueueToVec());
      delete this;
    }
    void canceled() override {
      // If we reach here then that means the stream is cancelled by
      // destroying the object. So finish
      // the event immediately, with an error message to be safe.
      std::string msg = "Something went wrong, queue is empty";
      std::unique_ptr<folly::IOBuf> msgBuffer =
          folly::IOBuf::copyBuffer(msg, msg.size());
      event->error(std::move(msgBuffer));
      delete this;
    }

   private:
    HPHP::thrift::TClientBufferedStream* stream;
    HPHP::Thrift2StreamEvent* event;
  };

  auto callback = new ReadyCallback(data, event);
  if (!data->streamBridge_->wait(callback)) {
    callback->consume();
  }

  guard.dismiss();
  return Object{event->getWaitHandle()};
}

Object HHVM_METHOD(RpcOptions, setChunkBufferSize, int64_t chunk_buffer_size) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  if (UNLIKELY(chunk_buffer_size < 0)) {
    raise_warning("servicerouter: The chunk buffer size can't be negative (" +
      std::to_string(chunk_buffer_size) + "). The setting won't be changed.");
  } else if (UNLIKELY(chunk_buffer_size > std::numeric_limits<int32_t>::max())) {
    raise_warning("servicerouter: The chunk buffer size " +
      std::to_string(chunk_buffer_size) + " shouldn't exceed " +
      std::to_string(std::numeric_limits<int32_t>::max()) +
      ". The setting won't be changed.");
  } else {
    data->rpcOptions.setChunkBufferSize(chunk_buffer_size);
  }
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setRoutingKey, const String& routing_key) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setRoutingKey(
    std::string(routing_key.c_str(), routing_key.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setShardId, const String& shard_id) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setShardId(std::string(shard_id.c_str(), shard_id.size()));
  return Object(this_);
}

// TODO (partisan): Deprecate in favor of more clear for extrnal users setHeader
Object HHVM_METHOD(RpcOptions, setWriteHeader, const String& key, const String& value) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setWriteHeader(std::string(key.c_str(), key.size()),
    std::string(value.c_str(), value.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setHeader, const String& key, const String& value) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setWriteHeader(std::string(key.c_str(), key.size()),
    std::string(value.c_str(), value.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setLoggingContext, const String& loggingContext) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setLoggingContext(
    std::string(loggingContext.c_str(), loggingContext.size()));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setOverallTimeout, int64_t overall_timeout) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setOverallTimeout(std::chrono::milliseconds(overall_timeout));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setProcessingTimeout, int64_t processing_timeout) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setProcessingTimeout(std::chrono::milliseconds(processing_timeout));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setChunkTimeout, int64_t chunk_timeout) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setChunkTimeout(std::chrono::milliseconds(chunk_timeout));
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setInteractionId, const Object& interaction_id) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  auto interaction = Native::data<InteractionId>(interaction_id.get());
  data->rpcOptions.setInteractionId(interaction->getInteractionId());
  return Object(this_);
}

Object HHVM_METHOD(RpcOptions, setSerializedAuthProofs, const String& payload) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  data->rpcOptions.setSerializedAuthProofs(
      apache::thrift::SerializedAuthProofs(folly::IOBuf::copyBuffer(payload.data(), payload.size())));
  return Object(this_);
}

String HHVM_METHOD(RpcOptions, __toString) {
  auto data = RpcOptions::GetDataOrThrowException(this_);
  std::string result("RpcOptions(");
  result += "chunkBufferSize: " +
    std::to_string(data->rpcOptions.getChunkBufferSize()) + "; ";
  result += "routingKey: \"" + data->rpcOptions.getRoutingKey() + "\"; ";
  result += "shardId: \"" + data->rpcOptions.getShardId() + "\"; ";
  result += "loggingContext: \"" + data->rpcOptions.getLoggingContext() + "\"; ";
  result += "overallTimeout: " +
    std::to_string(data->rpcOptions.getOverallTimeout().count()) + "ms; ";
  result += "processingTimeout: " +
    std::to_string(data->rpcOptions.getProcessingTimeout().count()) + "ms; ";
  result += "interactionId: " +
    std::to_string(data->rpcOptions.getInteractionId()) + "; ";
  result += "headers: {";
  bool first = true;
  for (const auto& it : data->rpcOptions.getWriteHeaders()) {
    if (!first) {
      result += ", ";
    } else {
      first = false;
    }
    result += "\"" + it.first + "\": \"" + it.second + "\"";
  }
  result += "}";
  result += ")\n";
  return result;
}

///////////////////////////////////////////////////////////////////////////////

static struct ThriftExtension final : Extension {
  ThriftExtension() : Extension("thrift_protocol", NO_EXTENSION_VERSION_YET) {}
  void loadDecls() override {
    loadDeclsFrom("thrift");
  }
  void moduleInit() override {
    HHVM_RC_INT(THRIFT_MARK_LEGACY_ARRAYS, k_THRIFT_MARK_LEGACY_ARRAYS);

    HHVM_FE(thrift_protocol_write_binary);
    HHVM_FE(thrift_protocol_read_binary);
    HHVM_FE(thrift_protocol_read_binary_struct);
    HHVM_FE(thrift_protocol_set_compact_version);
    HHVM_FE(thrift_protocol_write_compact);
    HHVM_FE(thrift_protocol_write_compact2);
    HHVM_FE(thrift_protocol_read_compact);
    HHVM_FE(thrift_protocol_read_compact_struct);

    Native::registerNativeDataInfo<RpcOptions>(s_RpcOptions.get());
    HHVM_ME(RpcOptions, setChunkBufferSize);
    HHVM_ME(RpcOptions, setRoutingKey);
    HHVM_ME(RpcOptions, setShardId);
    HHVM_ME(RpcOptions, setWriteHeader);
    HHVM_ME(RpcOptions, setHeader);
    HHVM_ME(RpcOptions, setLoggingContext);
    HHVM_ME(RpcOptions, setOverallTimeout);
    HHVM_ME(RpcOptions, setProcessingTimeout);
    HHVM_ME(RpcOptions, setChunkTimeout);
    HHVM_ME(RpcOptions, setInteractionId);
    HHVM_ME(RpcOptions, setSerializedAuthProofs);
    HHVM_ME(RpcOptions, __toString);

    Native::registerNativeDataInfo<InteractionId>(s_InteractionId.get());

    Native::registerNativeDataInfo<TClientBufferedStream>(
      s_TClientBufferedStream.get(), Native::NDIFlags::NO_COPY);
    HHVM_ME(TClientBufferedStream, genNext);

    Native::registerNativeDataInfo<TClientSink>(
      s_TClientSink.get(), Native::NDIFlags::NO_COPY);
    HHVM_ME(TClientSink, sendPayloadOrSinkComplete);
    HHVM_ME(TClientSink, genCreditsOrFinalResponse);
    HHVM_ME(TClientSink, sendClientException);

    loadSystemlib("thrift");
  }
} s_thrift_extension;

}
