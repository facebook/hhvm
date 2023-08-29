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

#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <folly/Memory.h>
#include <folly/Portability.h>

#include <fmt/core.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/experimental/coro/FutureUtil.h>
#include <folly/futures/Future.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/AsyncProcessorHelper.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/ClientSinkBridge.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/detail/meta.h>
#include <thrift/lib/cpp2/frozen/Frozen.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/Traits.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/util/Frozen2ViewHelpers.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

class BinaryProtocolReader;
class CompactProtocolReader;

namespace detail {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    bool, includeInRecentRequestsCount, const std::string_view /*methodName*/);

template <typename Protocol, typename IsSet>
struct Reader {
  Reader(
      Protocol* prot,
      IsSet& isset,
      int16_t fid,
      protocol::TType ftype,
      bool& success)
      : prot_(prot),
        isset_(isset),
        fid_(fid),
        ftype_(ftype),
        success_(success) {}
  template <typename FieldData>
  void operator()(FieldData& fieldData, int index) {
    if (fid_ != FieldData::fid) {
      return;
    }

    if (fieldData.read(prot_, ftype_)) {
      success_ = true;
      isset_.setIsSet(index);
    }
  }

 private:
  Protocol* prot_;
  IsSet& isset_;
  int16_t fid_;
  protocol::TType ftype_;
  bool& success_;
};

template <typename T>
T& maybe_deref(T& x) {
  return x;
}

template <typename T>
T& maybe_deref(T* x) {
  return *x;
}

template <bool hasIsSet, size_t count>
struct IsSetHelper {
  void setIsSet(size_t /*index*/, bool /*value*/ = true) {}
  bool getIsSet(size_t /*index*/) const { return true; }
};

template <size_t count>
struct IsSetHelper<true, count> {
  void setIsSet(size_t index, bool value = true) { isset_[index] = value; }
  bool getIsSet(size_t index) const { return isset_[index]; }

 private:
  std::array<bool, count> isset_ = {};
};

} // namespace detail

template <int16_t Fid, typename TC, typename T, typename Tag = void>
struct FieldData {
  static const constexpr int16_t fid = Fid;
  static const constexpr protocol::TType ttype = protocol_type_v<TC, T>;
  typedef TC type_class;
  typedef T type;
  typedef typename std::remove_pointer<T>::type value_type;
  using Ops = Cpp2Ops<value_type>;
  T value;
  static_assert(std::is_pointer_v<T> != std::is_base_of_v<TException, T>, "");

  value_type& ref() { return detail::maybe_deref(value); }
  const value_type& ref() const { return detail::maybe_deref(value); }

  template <typename Protocol>
  uint32_t write(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->writeFieldBegin("", thriftType(), fid);
    if constexpr (std::is_void_v<Tag>) {
      xfer += Ops::write(prot, &ref());
    } else {
      xfer += op::encode<Tag>(*prot, ref());
    }
    xfer += prot->writeFieldEnd();
    return xfer;
  }

  template <typename Protocol>
  uint32_t size(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->serializedFieldSize("", thriftType(), fid);
    if constexpr (std::is_void_v<Tag>) {
      xfer += Ops::serializedSize(prot, &ref());
    } else {
      xfer += op::serialized_size<false, Tag>(*prot, ref());
    }
    return xfer;
  }

  template <typename Protocol>
  uint32_t sizeZC(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->serializedFieldSize("", thriftType(), fid);
    if constexpr (std::is_void_v<Tag>) {
      xfer += Ops::serializedSizeZC(prot, &ref());
    } else {
      xfer += op::serialized_size<true, Tag>(*prot, ref());
    }
    return xfer;
  }

  template <typename Protocol>
  bool read(Protocol* prot, protocol::TType ftype) {
    if (ftype != thriftType()) {
      return false;
    }

    if constexpr (std::is_void_v<Tag>) {
      Ops::read(prot, &ref());
    } else {
      op::decode<Tag>(*prot, ref());
    }
    return true;
  }

 private:
  protocol::TType thriftType() const {
    if constexpr (std::is_void_v<Tag>) {
      return Ops::thriftType();
    } else {
      return op::detail::typeTagToTType<Tag>;
    }
  }
};

template <bool hasIsSet, typename... Field>
class ThriftPresult
    : private std::tuple<Field...>,
      public apache::thrift::detail::IsSetHelper<hasIsSet, sizeof...(Field)> {
  // The fields tuple and IsSetHelper are base classes (rather than members)
  // to employ the empty base class optimization when they are empty
  typedef std::tuple<Field...> Fields;
  typedef apache::thrift::detail::IsSetHelper<hasIsSet, sizeof...(Field)>
      CurIsSetHelper;

 public:
  using size = std::tuple_size<Fields>;

  CurIsSetHelper& isSet() { return *this; }
  const CurIsSetHelper& isSet() const { return *this; }
  Fields& fields() { return *this; }
  const Fields& fields() const { return *this; }

  // returns lvalue ref to the appropriate FieldData
  template <size_t index>
  auto get() -> decltype(std::get<index>(this->fields())) {
    return std::get<index>(this->fields());
  }

  template <size_t index>
  auto get() const -> decltype(std::get<index>(this->fields())) {
    return std::get<index>(this->fields());
  }

  template <class Protocol>
  uint32_t read(Protocol* prot) {
    auto xfer = prot->getCursorPosition();
    std::string fname;
    apache::thrift::protocol::TType ftype;
    int16_t fid;

    prot->readStructBegin(fname);

    while (true) {
      prot->readFieldBegin(fname, ftype, fid);
      if (ftype == apache::thrift::protocol::T_STOP) {
        break;
      }
      bool readSomething = false;
      detail::foreach_tuple(
          detail::Reader(prot, isSet(), fid, ftype, readSomething), fields());
      if (!readSomething) {
        prot->skip(ftype);
      }
      prot->readFieldEnd();
    }
    prot->readStructEnd();

    return folly::to_narrow(prot->getCursorPosition() - xfer);
  }

  template <class Protocol>
  uint32_t serializedSize(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->serializedStructSize("");
    detail::foreach_tuple(
        [&xfer, prot, isset = isSet()](const auto& fieldData, auto index) {
          if (!isset.getIsSet(index)) {
            return;
          }
          xfer += fieldData.size(prot);
        },
        fields());
    xfer += prot->serializedSizeStop();
    return xfer;
  }

  template <class Protocol>
  uint32_t serializedSizeZC(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->serializedStructSize("");
    detail::foreach_tuple(
        [&xfer, prot, isset = isSet()](const auto& fieldData, auto index) {
          if (!isset.getIsSet(index)) {
            return;
          }
          xfer += fieldData.sizeZC(prot);
        },
        fields());
    xfer += prot->serializedSizeStop();
    return xfer;
  }

  template <class Protocol>
  uint32_t write(Protocol* prot) const {
    uint32_t xfer = 0;
    xfer += prot->writeStructBegin("");
    detail::foreach_tuple(
        [&xfer, prot, isset = isSet()](const auto& fieldData, auto index) {
          if (!isset.getIsSet(index)) {
            return;
          }
          xfer += fieldData.write(prot);
        },
        fields());
    xfer += prot->writeFieldStop();
    xfer += prot->writeStructEnd();
    return xfer;
  }
};

template <typename PResults, typename StreamPresult>
struct ThriftPResultStream {
  using StreamPResultType = StreamPresult;
  using FieldsType = PResults;

  PResults fields;
  StreamPresult stream;
};

template <
    typename PResults,
    typename SinkPresult,
    typename FinalResponsePresult>
struct ThriftPResultSink {
  using SinkPResultType = SinkPresult;
  using FieldsType = PResults;
  using FinalResponsePResultType = FinalResponsePresult;

  PResults fields;
  SinkPresult stream;
  FinalResponsePresult finalResponse;
};

template <bool hasIsSet, class... Args>
class Cpp2Ops<ThriftPresult<hasIsSet, Args...>> {
 public:
  typedef ThriftPresult<hasIsSet, Args...> Presult;
  static constexpr protocol::TType thriftType() { return protocol::T_STRUCT; }
  template <class Protocol>
  static uint32_t write(Protocol* prot, const Presult* value) {
    return value->write(prot);
  }
  template <class Protocol>
  static uint32_t read(Protocol* prot, Presult* value) {
    return value->read(prot);
  }
  template <class Protocol>
  static uint32_t serializedSize(Protocol* prot, const Presult* value) {
    return value->serializedSize(prot);
  }
  template <class Protocol>
  static uint32_t serializedSizeZC(Protocol* prot, const Presult* value) {
    return value->serializedSizeZC(prot);
  }
};

// Forward declaration
namespace detail {
namespace ap {

template <
    ErrorBlame Blame,
    typename Protocol,
    typename PResult,
    typename T,
    typename ErrorMapFunc>
class StreamElementEncoderImpl;

template <typename Protocol, typename PResult, typename T>
folly::Try<T> decode_stream_element(
    folly::Try<apache::thrift::StreamPayload>&& payload);

template <typename Protocol, typename PResult, typename T>
apache::thrift::ClientBufferedStream<T> decode_client_buffered_stream(
    apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
    const BufferOptions& bufferOptions);

template <typename Protocol, typename PResult, typename T>
std::unique_ptr<folly::IOBuf> encode_stream_payload(T&& _item);

template <typename Protocol, typename PResult>
std::unique_ptr<folly::IOBuf> encode_stream_payload(folly::IOBuf&& _item);

template <typename Protocol, typename PResult, typename ErrorMapFunc>
EncodedStreamError encode_stream_exception(folly::exception_wrapper ew);

template <typename Protocol, typename PResult, typename T>
T decode_stream_payload_impl(folly::IOBuf& payload, folly::tag_t<T>);

template <typename Protocol, typename PResult, typename T>
folly::IOBuf decode_stream_payload_impl(
    folly::IOBuf& payload, folly::tag_t<folly::IOBuf>);

template <typename Protocol, typename PResult, typename T>
T decode_stream_payload(folly::IOBuf& payload);

template <typename Protocol, typename PResult, typename T>
folly::exception_wrapper decode_stream_exception(folly::exception_wrapper ew);

struct EmptyExMapType {
  template <typename PResult>
  bool operator()(PResult&, folly::exception_wrapper) {
    return false;
  }
};

} // namespace ap
} // namespace detail

//  AsyncClient helpers
namespace detail {
namespace ac {

template <bool HasReturnType, typename PResult>
folly::exception_wrapper extract_exn(PResult& result) {
  using base = std::integral_constant<std::size_t, HasReturnType ? 1 : 0>;
  auto ew = folly::exception_wrapper();
  if (HasReturnType && result.getIsSet(0)) {
    return ew;
  }
  foreach_index<PResult::size::value - base::value>([&](auto index) {
    if (!ew && result.getIsSet(index.value + base::value)) {
      auto& fdata = result.template get<index.value + base::value>();
      ew = folly::exception_wrapper(std::move(fdata.ref()));
    }
  });
  if (!ew && HasReturnType) {
    ew = folly::make_exception_wrapper<TApplicationException>(
        TApplicationException::TApplicationExceptionType::MISSING_RESULT,
        "failed: unknown result");
  }
  return ew;
}

folly::exception_wrapper try_extract_any_exception(
    const apache::thrift::transport::THeader::StringToStringMap& headers);

template <typename Protocol, typename PResult>
folly::exception_wrapper recv_wrapped_helper(
    Protocol* prot, ClientReceiveState& state, PResult& result) {
  ContextStack* ctx = state.ctx();
  MessageType mtype = state.messageType();
  if (ctx) {
    ctx->preRead();
  }
  try {
    const folly::IOBuf& buffer = *state.serializedResponse().buffer;
    // TODO: re-enable checksumming after we properly adjust checksum on the
    // server to exclude the envelope.
    // if (state.header() && state.header()->getCrc32c().has_value() &&
    //     checksum::crc32c(buffer) != *state.header()->getCrc32c()) {
    //   return folly::make_exception_wrapper<TApplicationException>(
    //       TApplicationException::TApplicationExceptionType::CHECKSUM_MISMATCH,
    //       "corrupted response");
    // }
    if (mtype == MessageType::T_EXCEPTION) {
      if (state.header()) {
        if (auto anyEx =
                try_extract_any_exception(state.header()->getHeaders())) {
          return anyEx;
        }
      }
      TApplicationException x;
      apache::thrift::detail::deserializeExceptionBody(prot, &x);
      return folly::exception_wrapper(std::move(x));
    }
    if (mtype != MessageType::T_REPLY) {
      prot->skip(protocol::T_STRUCT);
      return folly::make_exception_wrapper<TApplicationException>(
          TApplicationException::TApplicationExceptionType::
              INVALID_MESSAGE_TYPE);
    }
    SerializedMessage smsg;
    smsg.protocolType = prot->protocolType();
    smsg.buffer = &buffer;
    if (ctx) {
      ctx->onReadData(smsg);
    }
    apache::thrift::detail::deserializeRequestBody(prot, &result);
    if (ctx) {
      ctx->postRead(
          state.header(), folly::to_narrow(buffer.computeChainDataLength()));
    }
    return folly::exception_wrapper();
  } catch (...) {
    return folly::exception_wrapper(std::current_exception());
  }
}

template <typename PResult, typename Protocol, typename... ReturnTs>
folly::exception_wrapper recv_wrapped(
    Protocol* prot, ClientReceiveState& state, ReturnTs&... _returns) {
  prot->setInput(state.serializedResponse().buffer.get());
  auto guard = folly::makeGuard([&] { prot->setInput(nullptr); });
  apache::thrift::ContextStack* ctx = state.ctx();
  PResult result;
  foreach(
      [&](auto index, auto& obj) {
        result.template get<index.value>().value = &obj;
      },
      _returns...);
  auto ew = recv_wrapped_helper(prot, state, result);
  if (!ew) {
    constexpr const auto kHasReturnType = sizeof...(_returns) != 0;
    ew = apache::thrift::detail::ac::extract_exn<kHasReturnType>(result);
  }
  if (ctx && ew) {
    ctx->handlerErrorWrapped(ew);
  }
  return ew;
}

template <typename PResult, typename Protocol, typename Response, typename Item>
folly::exception_wrapper recv_wrapped(
    Protocol* prot,
    ClientReceiveState& state,
    apache::thrift::ResponseAndClientBufferedStream<Response, Item>& _return) {
  prot->setInput(state.serializedResponse().buffer.get());
  auto guard = folly::makeGuard([&] { prot->setInput(nullptr); });
  apache::thrift::ContextStack* ctx = state.ctx();

  typename PResult::FieldsType result;
  result.template get<0>().value = &_return.response;

  auto ew = recv_wrapped_helper(prot, state, result);
  if (!ew) {
    ew = apache::thrift::detail::ac::extract_exn<true>(result);
  }
  if (ctx && ew) {
    ctx->handlerErrorWrapped(ew);
  }

  if (!ew) {
    _return.stream = apache::thrift::detail::ap::decode_client_buffered_stream<
        Protocol,
        typename PResult::StreamPResultType,
        Item>(state.extractStreamBridge(), state.bufferOptions());
  }
  return ew;
}

template <typename PResult, typename Protocol, typename Item>
folly::exception_wrapper recv_wrapped(
    Protocol* prot,
    ClientReceiveState& state,
    apache::thrift::ClientBufferedStream<Item>& _return) {
  prot->setInput(state.serializedResponse().buffer.get());
  auto guard = folly::makeGuard([&] { prot->setInput(nullptr); });
  apache::thrift::ContextStack* ctx = state.ctx();

  typename PResult::FieldsType result;

  auto ew = recv_wrapped_helper(prot, state, result);
  if (!ew) {
    ew = apache::thrift::detail::ac::extract_exn<false>(result);
  }
  if (ctx && ew) {
    ctx->handlerErrorWrapped(ew);
  }

  if (!ew) {
    _return = apache::thrift::detail::ap::decode_client_buffered_stream<
        Protocol,
        typename PResult::StreamPResultType,
        Item>(state.extractStreamBridge(), state.bufferOptions());
  }
  return ew;
}

#if FOLLY_HAS_COROUTINES

template <
    typename ProtocolReader,
    typename ProtocolWriter,
    typename SinkPResult,
    typename SinkType,
    typename FinalResponsePResult,
    typename FinalResponseType,
    typename ErrorMapFunc>
ClientSink<SinkType, FinalResponseType> createSink(
    apache::thrift::detail::ClientSinkBridge::ClientPtr impl) {
  static apache::thrift::detail::ap::StreamElementEncoderImpl<
      ErrorBlame::CLIENT,
      ProtocolWriter,
      SinkPResult,
      SinkType,
      std::decay_t<ErrorMapFunc>>
      encode;
  return ClientSink<SinkType, FinalResponseType>(
      std::move(impl),
      &encode,
      apache::thrift::detail::ap::decode_stream_element<
          ProtocolReader,
          FinalResponsePResult,
          FinalResponseType>);
}
#endif

template <
    typename PResult,
    typename ErrorMapFunc,
    typename ProtocolWriter,
    typename ProtocolReader,
    typename Response,
    typename Item,
    typename FinalResponse>
folly::exception_wrapper recv_wrapped(
    ProtocolReader* prot,
    ClientReceiveState& state,
    apache::thrift::detail::ClientSinkBridge::ClientPtr impl,
    apache::thrift::ResponseAndClientSink<Response, Item, FinalResponse>&
        _return) {
#if FOLLY_HAS_COROUTINES
  prot->setInput(state.serializedResponse().buffer.get());
  auto guard = folly::makeGuard([&] { prot->setInput(nullptr); });
  apache::thrift::ContextStack* ctx = state.ctx();

  typename PResult::FieldsType result;
  result.template get<0>().value = &_return.response;

  auto ew = recv_wrapped_helper(prot, state, result);
  if (!ew) {
    ew = apache::thrift::detail::ac::extract_exn<true>(result);
  }
  if (ctx && ew) {
    ctx->handlerErrorWrapped(ew);
  }

  if (!ew) {
    _return.sink = createSink<
        ProtocolReader,
        ProtocolWriter,
        typename PResult::SinkPResultType,
        Item,
        typename PResult::FinalResponsePResultType,
        FinalResponse,
        std::decay_t<ErrorMapFunc>>(std::move(impl));
  }
  return ew;
#else
  (void)prot;
  (void)state;
  (void)impl;
  (void)_return;
  std::terminate();
#endif
}

template <
    typename PResult,
    typename ErrorMapFunc,
    typename ProtocolWriter,
    typename ProtocolReader,
    typename Item,
    typename FinalResponse>
folly::exception_wrapper recv_wrapped(
    ProtocolReader* prot,
    ClientReceiveState& state,
    apache::thrift::detail::ClientSinkBridge::ClientPtr impl,
    apache::thrift::ClientSink<Item, FinalResponse>& _return) {
#if FOLLY_HAS_COROUTINES
  prot->setInput(state.serializedResponse().buffer.get());
  auto guard = folly::makeGuard([&] { prot->setInput(nullptr); });
  apache::thrift::ContextStack* ctx = state.ctx();

  typename PResult::FieldsType result;

  auto ew = recv_wrapped_helper(prot, state, result);
  if (!ew) {
    ew = apache::thrift::detail::ac::extract_exn<false>(result);
  }
  if (ctx && ew) {
    ctx->handlerErrorWrapped(ew);
  }

  if (!ew) {
    _return = createSink<
        ProtocolReader,
        ProtocolWriter,
        typename PResult::SinkPResultType,
        Item,
        typename PResult::FinalResponsePResultType,
        FinalResponse,
        std::decay_t<ErrorMapFunc>>(std::move(impl));
  }
  return ew;
#else
  (void)prot;
  (void)state;
  (void)impl;
  (void)_return;
  std::terminate();
#endif
}

[[noreturn]] void throw_app_exn(const char* msg);
} // namespace ac
} // namespace detail

//  AsyncProcessor helpers
namespace detail {
namespace ap {

//  Everything templated on only protocol goes here. The corresponding .cpp file
//  explicitly instantiates this struct for each supported protocol.
template <typename ProtocolReader, typename ProtocolWriter>
struct helper {
  static std::unique_ptr<folly::IOBuf> write_exn(
      bool includeEnvelope,
      const char* method,
      ProtocolWriter* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const TApplicationException& x);

  // Temporary for backwards compatibility
  static std::unique_ptr<folly::IOBuf> write_exn(
      const char* method,
      ProtocolWriter* prot,
      int32_t protoSeqId,
      ContextStack* ctx,
      const TApplicationException& x) {
    return write_exn(true, method, prot, protoSeqId, ctx, x);
  }

  static void process_exn(
      const char* func,
      const TApplicationException::TApplicationExceptionType type,
      const std::string& msg,
      ResponseChannelRequest::UniquePtr req,
      Cpp2RequestContext* ctx,
      folly::EventBase* eb,
      int32_t protoSeqId);
};

template <typename ProtocolReader>
using writer_of = typename ProtocolReader::ProtocolWriter;
template <typename ProtocolWriter>
using reader_of = typename ProtocolWriter::ProtocolReader;

template <typename ProtocolReader>
using helper_r = helper<ProtocolReader, writer_of<ProtocolReader>>;
template <typename ProtocolWriter>
using helper_w = helper<reader_of<ProtocolWriter>, ProtocolWriter>;

template <typename T>
inline constexpr bool is_root_async_processor =
    std::is_void_v<typename T::BaseAsyncProcessor>;

template <typename Derived>
GeneratedAsyncProcessorBase::ProcessFunc<Derived> getProcessFuncFromProtocol(
    folly::tag_t<CompactProtocolReader> /* unused */,
    const GeneratedAsyncProcessorBase::ProcessFuncs<Derived>& funcs) {
  return funcs.compact;
}
template <typename Derived>
GeneratedAsyncProcessorBase::ProcessFunc<Derived> getProcessFuncFromProtocol(
    folly::tag_t<BinaryProtocolReader> /* unused */,
    const GeneratedAsyncProcessorBase::ProcessFuncs<Derived>& funcs) {
  return funcs.binary;
}

template <typename Derived>
GeneratedAsyncProcessorBase::ExecuteFunc<Derived> getExecuteFuncFromProtocol(
    folly::tag_t<CompactProtocolReader> /* unused */,
    const GeneratedAsyncProcessorBase::ProcessFuncs<Derived>& funcs) {
  return funcs.compactExecute;
}
template <typename Derived>
GeneratedAsyncProcessorBase::ExecuteFunc<Derived> getExecuteFuncFromProtocol(
    folly::tag_t<BinaryProtocolReader> /* unused */,
    const GeneratedAsyncProcessorBase::ProcessFuncs<Derived>& funcs) {
  return funcs.binaryExecute;
}

inline void nonRecursiveProcessMissing(
    const std::string& methodName,
    ResponseChannelRequest::UniquePtr req,
    folly::EventBase* eb) {
  if (req) {
    eb->runInEventBaseThread([request = std::move(req),
                              name = std::string{methodName}]() mutable {
      AsyncProcessorHelper::sendUnknownMethodError(std::move(request), name);
    });
  }
}

template <class ProtocolReader, class Processor>
void recursiveProcessMissing(
    Processor* processor,
    const std::string& fname,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm);

template <class ProtocolReader, class Processor>
void recursiveProcessPmap(
    Processor* proc,
    const typename Processor::ProcessMap& pmap,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  const auto& fname = ctx->getMethodName();
  auto processFuncs = pmap.find(fname);
  if (processFuncs == pmap.end()) {
    recursiveProcessMissing<ProtocolReader>(
        proc, fname, std::move(req), std::move(serializedRequest), ctx, eb, tm);
    return;
  }

  auto pfn = getProcessFuncFromProtocol(
      folly::tag<ProtocolReader>, processFuncs->second);
  (proc->*pfn)(std::move(req), std::move(serializedRequest), ctx, eb, tm);
}

template <class ProtocolReader, class Processor>
void recursiveProcessMissing(
    Processor* processor,
    const std::string& fname,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  if constexpr (is_root_async_processor<Processor>) {
    nonRecursiveProcessMissing(fname, std::move(req), eb);
  } else {
    using BaseAsyncProcessor = typename Processor::BaseAsyncProcessor;
    recursiveProcessPmap<ProtocolReader, BaseAsyncProcessor>(
        processor,
        BaseAsyncProcessor::getOwnProcessMap(),
        std::move(req),
        std::move(serializedRequest),
        ctx,
        eb,
        tm);
  }
}

/**
 * Recursive implementation of method resolution based on
 * Processor::getOwnProcessMap(). This is the fallback/legacy implementation for
 * generated AsyncProcessor::processSerializedCompressedRequest, which is called
 * in the absence of MethodMetadata support.
 *
 * TODO(praihan): Remove this implementation once all services support
 * MethodMetadata.
 */
template <class ProtocolReader, class Processor>
void recursiveProcess(
    Processor* processor,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  return recursiveProcessPmap<ProtocolReader>(
      processor,
      Processor::getOwnProcessMap(),
      std::move(req),
      std::move(serializedRequest),
      ctx,
      eb,
      tm);
}

/**
 * Non-recursive implementation of method resolution based on
 * the passed-in MethodMetadata.
 * See ServerInterface::GeneratedMethodMetadata.
 */
template <class ProtocolReader, class Processor>
void nonRecursiveProcessForInteraction(
    Processor* processor,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    const apache::thrift::AsyncProcessor::MethodMetadata& untypedMethodMetadata,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  using Metadata = ServerInterface::GeneratedMethodMetadata<Processor>;
  static_assert(std::is_final_v<Metadata>);
  const auto& methodMetadata =
      AsyncProcessorHelper::expectMetadataOfType<Metadata>(
          untypedMethodMetadata);

  DCHECK(
      untypedMethodMetadata.interactionType ==
          AsyncProcessor::MethodMetadata::InteractionType::INTERACTION_V1 ||
      ctx->getInteractionId());
  auto pfn = getProcessFuncFromProtocol(
      folly::tag<ProtocolReader>, methodMetadata.processFuncs);
  (processor->*pfn)(std::move(req), std::move(serializedRequest), ctx, eb, tm);
}

inline void processViaExecuteRequest(
    AsyncProcessor* processor,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    const apache::thrift::AsyncProcessor::MethodMetadata& untypedMethodMetadata,
    protocol::PROTOCOL_TYPES protType,
    Cpp2RequestContext* ctx,
    concurrency::ThreadManager* tm) {
  DCHECK(
      untypedMethodMetadata.executorType !=
      AsyncProcessor::MethodMetadata::ExecutorType::UNKNOWN);
  DCHECK(
      untypedMethodMetadata.interactionType ==
          AsyncProcessor::MethodMetadata::InteractionType::NONE ||
      untypedMethodMetadata.isWildcard());
  DCHECK(untypedMethodMetadata.rpcKind || untypedMethodMetadata.isWildcard());

  if (!untypedMethodMetadata.isWildcard() &&
      !apache::thrift::GeneratedAsyncProcessorBase::validateRpcKind(
          req, *untypedMethodMetadata.rpcKind)) {
    return;
  }

  folly::Executor::KeepAlive<> executor;
  if (untypedMethodMetadata.executorType ==
          AsyncProcessor::MethodMetadata::ExecutorType::ANY &&
      tm) {
    executor = tm->getKeepAlive(
        ctx->getRequestExecutionScope(),
        concurrency::ThreadManager::Source::INTERNAL);
  }

  auto task = [serverRequest =
                   ServerRequest{
                       std::move(req),
                       std::move(serializedRequest),
                       ctx,
                       protType,
                       {},
                       {},
                       {}},
               processor = processor,
               executor = std::move(executor),
               &untypedMethodMetadata](bool runInline) mutable {
    if (!runInline) {
      if (serverRequest.requestContext()
              ->getTimestamps()
              .getSamplingStatus()
              .isEnabled()) {
        // Since this request was queued, reset the processBegin
        // time to the actual start time, and not the queue time.
        serverRequest.requestContext()->getTimestamps().processBegin =
            std::chrono::steady_clock::now();
      }
    }

    auto notOnewayOrWildcard = untypedMethodMetadata.isWildcard() ||
        (*untypedMethodMetadata.rpcKind != RpcKind::SINGLE_REQUEST_NO_RESPONSE);

    // only check queuetimeout if it is not oneway or it's
    // a wildcard method because wildcard method doesn't
    // mark oneway
    if (notOnewayOrWildcard &&
        !serverRequest.request()->getShouldStartProcessing()) {
      HandlerCallbackBase::releaseRequest(
          detail::ServerRequestHelper::request(std::move(serverRequest)),
          detail::ServerRequestHelper::eventBase(serverRequest));
      return;
    }

    detail::ServerRequestHelper::setExecutor(
        serverRequest, std::move(executor));

    processor->executeRequest(std::move(serverRequest), untypedMethodMetadata);
  };

  if (untypedMethodMetadata.executorType ==
          AsyncProcessor::MethodMetadata::ExecutorType::ANY &&
      tm) {
    tm->getKeepAlive(
          ctx->getRequestExecutionScope(),
          concurrency::ThreadManager::Source::UPSTREAM)
        ->add([task = std::move(task)]() mutable { task(false); });
  } else {
    task(true);
  }
}

// The below function overloads are for wildcard metadata only
// These are to make sure when executeRequest is called with a
// wildcard metadata, they will be routed to old logic
template <class ProtocolReader, class Processor>
void recursiveProcessMissing(
    Processor* processor, const std::string& fname, ServerRequest&& request);

template <class ProtocolReader, class Processor>
void recursiveProcessPmap(
    Processor* proc,
    const typename Processor::ProcessMap& pmap,
    ServerRequest&& request) {
  auto ctx = request.requestContext();

  const auto& fname = ctx->getMethodName();
  auto processFuncs = pmap.find(fname);
  if (processFuncs == pmap.end()) {
    recursiveProcessMissing<ProtocolReader>(proc, fname, std::move(request));
    return;
  }

  auto pfn = getExecuteFuncFromProtocol(
      folly::tag<ProtocolReader>, processFuncs->second);
  (proc->*pfn)(std::move(request));
}

template <class ProtocolReader, class Processor>
void recursiveProcessMissing(
    Processor* processor, const std::string& fname, ServerRequest&& request) {
  if constexpr (is_root_async_processor<Processor>) {
    using ServerRequestHelper = detail::ServerRequestHelper;
    auto req = ServerRequestHelper::request(std::move(request));
    auto eb = ServerRequestHelper::eventBase(request);

    nonRecursiveProcessMissing(fname, std::move(req), eb);
  } else {
    using BaseAsyncProcessor = typename Processor::BaseAsyncProcessor;
    recursiveProcessPmap<ProtocolReader, BaseAsyncProcessor>(
        processor, BaseAsyncProcessor::getOwnProcessMap(), std::move(request));
  }
}

template <class ProtocolReader, class Processor>
void recursiveProcess(Processor* processor, ServerRequest&& request) {
  return recursiveProcessPmap<ProtocolReader>(
      processor, Processor::getOwnProcessMap(), std::move(request));
}

template <class Processor>
void execute(
    Processor* processor,
    ServerRequest&& request,
    protocol::PROTOCOL_TYPES protType) {
  switch (protType) {
    case protocol::T_BINARY_PROTOCOL: {
      return recursiveProcess<BinaryProtocolReader>(
          processor, std::move(request));
    }
    case protocol::T_COMPACT_PROTOCOL: {
      return recursiveProcess<CompactProtocolReader>(
          processor, std::move(request));
    }
    default:
      LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
      return;
  }
}

// Generated AsyncProcessor::processSerializedCompressedRequest just calls
// this
template <class Processor>
void process(
    Processor* processor,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    protocol::PROTOCOL_TYPES protType,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  switch (protType) {
    case protocol::T_BINARY_PROTOCOL: {
      return recursiveProcess<BinaryProtocolReader>(
          processor, std::move(req), std::move(serializedRequest), ctx, eb, tm);
    }
    case protocol::T_COMPACT_PROTOCOL: {
      return recursiveProcess<CompactProtocolReader>(
          processor, std::move(req), std::move(serializedRequest), ctx, eb, tm);
    }
    default:
      LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
      return;
  }
}

// Generated AsyncProcessor::processSerializedCompressedRequestWithMetadata just
// calls this
template <class Processor>
void process(
    Processor* processor,
    ServerInterface* si,
    ResponseChannelRequest::UniquePtr req,
    apache::thrift::SerializedCompressedRequest&& serializedRequest,
    const apache::thrift::AsyncProcessor::MethodMetadata& methodMetadata,
    protocol::PROTOCOL_TYPES protType,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb,
    concurrency::ThreadManager* tm) {
  if (methodMetadata.isWildcard()) {
    process(
        processor,
        std::move(req),
        std::move(serializedRequest),
        protType,
        ctx,
        eb,
        tm);
    return;
  }

  using Metadata = ServerInterface::GeneratedMethodMetadata<Processor>;
  static_assert(std::is_final_v<Metadata>);
  AsyncProcessorHelper::expectMetadataOfType<Metadata>(methodMetadata);

  if (methodMetadata.interactionType !=
          AsyncProcessor::MethodMetadata::InteractionType::INTERACTION_V1 &&
      !ctx->getInteractionId()) {
    if (methodMetadata.executorType ==
            AsyncProcessor::MethodMetadata::ExecutorType::ANY &&
        tm) {
      ctx->setRequestExecutionScope(si->getRequestExecutionScope(
          ctx, methodMetadata.priority.value_or(concurrency::NORMAL)));
    }

    return processViaExecuteRequest(
        processor,
        std::move(req),
        std::move(serializedRequest),
        methodMetadata,
        protType,
        ctx,
        tm);
  }

  switch (protType) {
    case protocol::T_BINARY_PROTOCOL: {
      return nonRecursiveProcessForInteraction<BinaryProtocolReader>(
          processor,
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          ctx,
          eb,
          tm);
    }
    case protocol::T_COMPACT_PROTOCOL: {
      return nonRecursiveProcessForInteraction<CompactProtocolReader>(
          processor,
          std::move(req),
          std::move(serializedRequest),
          methodMetadata,
          ctx,
          eb,
          tm);
    }
    default:
      LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
      return;
  }
}

template <class Processor>
void execute(
    Processor* processor,
    ServerRequest&& request,
    protocol::PROTOCOL_TYPES protType,
    const AsyncProcessor::MethodMetadata& metadata) {
  using Metadata = ServerInterface::GeneratedMethodMetadata<Processor>;
  static_assert(std::is_final_v<Metadata>);

  // when generated execute accepts a wildcard metadata
  // it should use the old logic for function routing
  if (metadata.isWildcard()) {
    execute(processor, std::move(request), protType);
    return;
  }

  const auto& methodMetadata =
      AsyncProcessorHelper::expectMetadataOfType<Metadata>(metadata);
  switch (protType) {
    case protocol::T_BINARY_PROTOCOL: {
      auto pfn = getExecuteFuncFromProtocol(
          folly::tag<BinaryProtocolReader>, methodMetadata.processFuncs);
      (processor->*pfn)(std::move(request));
    } break;
    case protocol::T_COMPACT_PROTOCOL: {
      auto pfn = getExecuteFuncFromProtocol(
          folly::tag<CompactProtocolReader>, methodMetadata.processFuncs);
      (processor->*pfn)(std::move(request));
    } break;
    default:
      LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
      return;
  }
}
struct MessageBegin : folly::MoveOnly {
  std::string methodName;
  struct Metadata {
    std::string errMessage;
    size_t size{0};
    int32_t seqId{0};
    MessageType msgType{};
    bool isValid{true};
  } metadata;
};

bool setupRequestContextWithMessageBegin(
    const MessageBegin::Metadata& msgBegin,
    protocol::PROTOCOL_TYPES protType,
    ResponseChannelRequest::UniquePtr& req,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb);

MessageBegin deserializeMessageBegin(
    const folly::IOBuf& buf, protocol::PROTOCOL_TYPES protType);

/**
 * The function pointers themselves are contravariant on the Processor type but
 * ProcessFuncs is not because templates are invariant. This function performs
 * the conversion manually.
 */
template <class DerivedProcessor, class BaseProcessor>
std::enable_if_t<
    std::is_base_of_v<BaseProcessor, DerivedProcessor>,
    GeneratedAsyncProcessorBase::ProcessFuncs<DerivedProcessor>>
downcastProcessFuncs(
    const GeneratedAsyncProcessorBase::ProcessFuncs<BaseProcessor>&
        processFuncs) {
  return GeneratedAsyncProcessorBase::ProcessFuncs<DerivedProcessor>{
      processFuncs.compact,
      processFuncs.binary,
      processFuncs.compactExecute,
      processFuncs.binaryExecute};
}

template <
    class MostDerivedProcessor,
    class CurrentProcessor = MostDerivedProcessor>
void populateMethodMetadataMap(
    AsyncProcessorFactory::MethodMetadataMap& map,
    const ServiceRequestInfoMap& requestInfoMap) {
  for (const auto& [methodName, processFuncs] :
       CurrentProcessor::getOwnProcessMap()) {
    const auto& requestInfo = requestInfoMap.at(methodName);
    std::optional<std::string_view> interactionName =
        requestInfo.interactionName;
    if (!interactionName) {
      // If this is a normal RPC that creates an interaction
      interactionName = requestInfo.createdInteraction;
    }
    map.emplace(
        methodName,
        // Always create GeneratatedMethodMetadata<MostDerivedProcessor> so that
        // all entries in the map are of the same type.
        std::make_shared<
            ServerInterface::GeneratedMethodMetadata<MostDerivedProcessor>>(
            downcastProcessFuncs<MostDerivedProcessor>(processFuncs),
            requestInfo.isSync
                ? AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB
                : AsyncProcessorFactory::MethodMetadata::ExecutorType::ANY,
            requestInfo.interactionName
                ? AsyncProcessorFactory::MethodMetadata::InteractionType::
                      INTERACTION_V1
                : AsyncProcessorFactory::MethodMetadata::InteractionType::NONE,
            requestInfo.rpcKind,
            requestInfo.priority,
            std::move(interactionName),
            requestInfo.createdInteraction.has_value()));
  }
  if constexpr (!is_root_async_processor<CurrentProcessor>) {
    populateMethodMetadataMap<
        MostDerivedProcessor,
        typename CurrentProcessor::BaseAsyncProcessor>(map, requestInfoMap);
  }
}

template <class Processor>
AsyncProcessorFactory::MethodMetadataMap createMethodMetadataMap(
    const apache::thrift::ServiceRequestInfoMap& requestInfoMap) {
  AsyncProcessorFactory::MethodMetadataMap result;
  populateMethodMetadataMap<Processor>(result, requestInfoMap);
  return result;
}

template <typename Protocol, typename PResult, typename T>
std::unique_ptr<folly::IOBuf> encode_stream_payload(T&& _item) {
  PResult res;
  res.template get<0>().value = const_cast<T*>(&_item);
  res.setIsSet(0);

  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  Protocol prot;
  prot.setOutput(&queue, res.serializedSizeZC(&prot));

  res.write(&prot);
  return std::move(queue).move();
}

template <typename Protocol, typename PResult>
std::unique_ptr<folly::IOBuf> encode_stream_payload(folly::IOBuf&& _item) {
  return std::make_unique<folly::IOBuf>(std::move(_item));
}

template <
    ErrorBlame Blame,
    typename Protocol,
    typename PResult,
    typename ErrorMapFunc>
EncodedStreamError encode_stream_exception(folly::exception_wrapper ew) {
  ErrorMapFunc mapException;
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  PResult res;

  PayloadExceptionMetadata exceptionMetadata;
  PayloadExceptionMetadataBase exceptionMetadataBase;
  if (mapException(res, ew)) {
    prot.setOutput(&queue, res.serializedSizeZC(&prot));
    res.write(&prot);
    exceptionMetadata.declaredException_ref() =
        PayloadDeclaredExceptionMetadata();
  } else {
    constexpr size_t kQueueAppenderGrowth = 4096;
    prot.setOutput(&queue, kQueueAppenderGrowth);
    TApplicationException ex(ew.what().toStdString());
    exceptionMetadataBase.what_utf8() = ex.what();
    apache::thrift::detail::serializeExceptionBody(&prot, &ex);
    PayloadAppUnknownExceptionMetdata aue;
    aue.errorClassification().ensure().blame() = Blame;
    exceptionMetadata.appUnknownException_ref() = std::move(aue);
  }

  exceptionMetadataBase.metadata() = std::move(exceptionMetadata);
  StreamPayloadMetadata streamPayloadMetadata;
  PayloadMetadata payloadMetadata;
  payloadMetadata.exceptionMetadata_ref() = std::move(exceptionMetadataBase);
  streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
  return EncodedStreamError(
      StreamPayload(std::move(queue).move(), std::move(streamPayloadMetadata)));
}

template <
    ErrorBlame Blame,
    typename Protocol,
    typename PResult,
    typename T,
    typename ErrorMapFunc>
class StreamElementEncoderImpl final
    : public apache::thrift::detail::StreamElementEncoder<T> {
  folly::Try<StreamPayload> operator()(T&& val) override {
    StreamPayloadMetadata streamPayloadMetadata;
    PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata_ref().ensure();
    streamPayloadMetadata.payloadMetadata() = std::move(payloadMetadata);
    return folly::Try<StreamPayload>(
        {encode_stream_payload<Protocol, PResult>(std::move(val)),
         std::move(streamPayloadMetadata)});
  }

  folly::Try<StreamPayload> operator()(folly::exception_wrapper&& e) override {
    return folly::Try<StreamPayload>(folly::exception_wrapper(
        encode_stream_exception<Blame, Protocol, PResult, ErrorMapFunc>(e)));
  }
};

template <typename Protocol, typename PResult, typename T>
T decode_stream_payload_impl(folly::IOBuf& payload, folly::tag_t<T>) {
  PResult args;
  T res{};
  args.template get<0>().value = &res;

  Protocol prot;
  prot.setInput(&payload);
  args.read(&prot);
  return res;
}

template <typename Protocol, typename PResult, typename T>
folly::IOBuf decode_stream_payload_impl(
    folly::IOBuf& payload, folly::tag_t<folly::IOBuf>) {
  return std::move(payload);
}

template <typename Protocol, typename PResult, typename T>
T decode_stream_payload(folly::IOBuf& payload) {
  return decode_stream_payload_impl<Protocol, PResult, T>(
      payload, folly::tag_t<T>{});
}

template <typename Protocol, typename PResult, typename T>
folly::exception_wrapper decode_stream_exception(folly::exception_wrapper ew) {
  folly::exception_wrapper hijacked;
  ew.handle(
      [&hijacked](apache::thrift::detail::EncodedError& err) {
        PResult result;
        T res{};
        result.template get<0>().value = &res;
        Protocol prot;
        prot.setInput(err.encoded.get());
        result.read(&prot);

        CHECK(!result.getIsSet(0));

        foreach_index<PResult::size::value - 1>([&](auto index) {
          if (!hijacked && result.getIsSet(index.value + 1)) {
            auto& fdata = result.template get<index.value + 1>();
            hijacked = folly::exception_wrapper(std::move(fdata.ref()));
          }
        });

        if (!hijacked) {
          // Could not decode the error. It may be a TApplicationException
          TApplicationException x;
          prot.setInput(err.encoded.get());
          apache::thrift::detail::deserializeExceptionBody(&prot, &x);
          hijacked = folly::exception_wrapper(std::move(x));
        }
      },
      [&hijacked](apache::thrift::detail::EncodedStreamError& err) {
        auto& payload = err.encoded;
        DCHECK_EQ(payload.metadata.payloadMetadata().has_value(), true);
        DCHECK_EQ(
            payload.metadata.payloadMetadata()->getType(),
            PayloadMetadata::Type::exceptionMetadata);
        auto& exceptionMetadataBase =
            payload.metadata.payloadMetadata()->get_exceptionMetadata();
        if (auto exceptionMetadataRef = exceptionMetadataBase.metadata()) {
          if (exceptionMetadataRef->getType() ==
              PayloadExceptionMetadata::Type::declaredException) {
            PResult result;
            T res{};
            Protocol prot;
            result.template get<0>().value = &res;
            prot.setInput(payload.payload.get());
            result.read(&prot);
            CHECK(!result.getIsSet(0));
            foreach_index<PResult::size::value - 1>([&](auto index) {
              if (!hijacked && result.getIsSet(index.value + 1)) {
                auto& fdata = result.template get<index.value + 1>();
                hijacked = folly::exception_wrapper(std::move(fdata.ref()));
              }
            });

            if (!hijacked) {
              hijacked =
                  TApplicationException("Failed to parse declared exception");
            }
          } else {
            hijacked = TApplicationException(
                exceptionMetadataBase.what_utf8().value_or(""));
          }
        } else {
          hijacked =
              TApplicationException("Missing payload exception metadata");
        }
      },
      [&hijacked](apache::thrift::detail::EncodedStreamRpcError& err) {
        StreamRpcError streamRpcError;
        CompactProtocolReader reader;
        reader.setInput(err.encoded.get());
        streamRpcError.read(&reader);
        TApplicationException::TApplicationExceptionType exType{
            TApplicationException::UNKNOWN};
        auto code = streamRpcError.code();
        if (code &&
            (code.value() == StreamRpcErrorCode::CREDIT_TIMEOUT ||
             code.value() == StreamRpcErrorCode::CHUNK_TIMEOUT)) {
          exType = TApplicationException::TIMEOUT;
        }
        hijacked = TApplicationException(
            exType, streamRpcError.what_utf8().value_or(""));
      },
      [](...) {});

  if (hijacked) {
    return hijacked;
  }
  return ew;
}

template <
    typename Protocol,
    typename PResult,
    typename ErrorMapFunc,
    typename T>
ServerStreamFactory encode_server_stream(
    apache::thrift::ServerStream<T>&& stream,
    folly::Executor::KeepAlive<> serverExecutor) {
  static StreamElementEncoderImpl<
      ErrorBlame::SERVER,
      Protocol,
      PResult,
      T,
      ErrorMapFunc>
      encode;
  return stream(std::move(serverExecutor), &encode);
}

template <typename Protocol, typename PResult, typename T>
folly::Try<T> decode_stream_element(
    folly::Try<apache::thrift::StreamPayload>&& payload) {
  if (payload.hasValue()) {
    return folly::Try<T>(
        decode_stream_payload<Protocol, PResult, T>(*payload->payload));
  } else if (payload.hasException()) {
    return folly::Try<T>(decode_stream_exception<Protocol, PResult, T>(
        std::move(payload).exception()));
  } else {
    return folly::Try<T>();
  }
}

template <typename Protocol, typename PResult, typename T>
apache::thrift::ClientBufferedStream<T> decode_client_buffered_stream(
    apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
    const BufferOptions& bufferOptions) {
  return apache::thrift::ClientBufferedStream<T>(
      std::move(streamBridge),
      decode_stream_element<Protocol, PResult, T>,
      bufferOptions);
}

template <
    typename ProtocolReader,
    typename ProtocolWriter,
    typename SinkPResult,
    typename FinalResponsePResult,
    typename ErrorMapFunc,
    typename SinkType,
    typename FinalResponseType>
apache::thrift::detail::SinkConsumerImpl toSinkConsumerImpl(
    FOLLY_MAYBE_UNUSED SinkConsumer<SinkType, FinalResponseType>&& sinkConsumer,
    FOLLY_MAYBE_UNUSED folly::Executor::KeepAlive<> executor) {
#if FOLLY_HAS_COROUTINES
  auto consumer =
      [innerConsumer = std::move(sinkConsumer.consumer)](
          folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> gen) mutable
      -> folly::coro::Task<folly::Try<StreamPayload>> {
    folly::exception_wrapper ew;
    try {
      FinalResponseType finalResponse = co_await innerConsumer(
          [](folly::coro::AsyncGenerator<folly::Try<StreamPayload>&&> gen_)
              -> folly::coro::AsyncGenerator<SinkType&&> {
            while (auto item = co_await gen_.next()) {
              auto payload = std::move(*item);
              co_yield folly::coro::co_result(ap::decode_stream_element<
                                              ProtocolReader,
                                              SinkPResult,
                                              SinkType>(std::move(payload)));
            }
          }(std::move(gen)));
      co_return folly::Try<StreamPayload>(StreamPayload(
          ap::encode_stream_payload<ProtocolWriter, FinalResponsePResult>(
              std::move(finalResponse)),
          {}));
    } catch (...) {
      ew = folly::exception_wrapper(std::current_exception());
    }
    co_return folly::Try<StreamPayload>(ap::encode_stream_exception<
                                        ErrorBlame::SERVER,
                                        ProtocolWriter,
                                        FinalResponsePResult,
                                        ErrorMapFunc>(std::move(ew)));
  };
  return apache::thrift::detail::SinkConsumerImpl{
      std::move(consumer),
      sinkConsumer.bufferSize,
      sinkConsumer.sinkOptions.chunkTimeout,
      std::move(executor)};
#else
  std::terminate();
#endif
}

} // namespace ap
} // namespace detail

//  ServerInterface helpers
namespace detail {
namespace si {
template <typename T>
folly::Future<T> future(
    folly::SemiFuture<T>&& future, folly::Executor::KeepAlive<> keepAlive) {
  if (future.isReady()) {
    return std::move(future).toUnsafeFuture();
  }
  return std::move(future).via(keepAlive);
}

using CallbackBase = HandlerCallbackBase;
using CallbackBasePtr = std::unique_ptr<CallbackBase>;
template <typename T>
using Callback = HandlerCallback<T>;
template <typename T>
using CallbackPtr = std::unique_ptr<Callback<T>>;

class AsyncTmPrep {
  ServerInterface* si_;

 public:
  AsyncTmPrep(ServerInterface* si, CallbackBase* callback) : si_{si} {
    si->setEventBase(callback->getEventBase());
    if (auto threadManager = callback->getThreadManager_deprecated();
        threadManager != nullptr) {
      si->setThreadManager(threadManager);
    }
    si->setHandlerExecutor(callback->getHandlerExecutor());
    si->setRequestContext(callback->getRequestContext());
  }

  ~AsyncTmPrep() { si_->clearRequestParams(); }
};

inline void async_tm_future_oneway(
    CallbackBasePtr callback, folly::Future<folly::Unit>&& fut) {
  if (!fut.isReady()) {
    auto ka = callback->getInternalKeepAlive();
    std::move(fut)
        .via(std::move(ka))
        .thenValueInline([cb = std::move(callback)](auto&&) {});
  }
}

template <typename T>
void async_tm_future(
    CallbackPtr<T> callback, folly::Future<folly::lift_unit_t<T>>&& fut) {
  if (!fut.isReady()) {
    auto ka = callback->getInternalKeepAlive();
    std::move(fut)
        .via(std::move(ka))
        .thenTryInline([cb = std::move(callback)](
                           folly::Try<folly::lift_unit_t<T>>&& ret) {
          cb->complete(std::move(ret));
        });
  } else {
    callback->complete(std::move(fut).result());
  }
}

inline void async_tm_semifuture_oneway(
    CallbackBasePtr callback, folly::SemiFuture<folly::Unit>&& fut) {
  if (!fut.isReady()) {
    auto ka = callback->getInternalKeepAlive();
    std::move(fut)
        .via(std::move(ka))
        .thenValueInline([cb = std::move(callback)](auto&&) {});
  }
}

template <typename T>
void async_tm_semifuture(
    CallbackPtr<T> callback, folly::SemiFuture<folly::lift_unit_t<T>>&& fut) {
  if (!fut.isReady()) {
    auto ka = callback->getInternalKeepAlive();
    std::move(fut)
        .via(std::move(ka))
        .thenTryInline([cb = std::move(callback)](
                           folly::Try<folly::lift_unit_t<T>>&& ret) {
          cb->complete(std::move(ret));
        });
  } else {
    callback->complete(std::move(fut).result());
  }
}

#if FOLLY_HAS_COROUTINES
inline void async_tm_coro_oneway(
    CallbackBasePtr callback, folly::coro::Task<void>&& task) {
  auto ka = callback->getInternalKeepAlive();
  std::move(task)
      .scheduleOn(std::move(ka))
      .startInlineUnsafe([callback = std::move(callback)](auto&&) {});
}

template <typename T>
void async_tm_coro(CallbackPtr<T> callback, folly::coro::Task<T>&& task) {
  auto ka = callback->getInternalKeepAlive();
  std::move(task)
      .scheduleOn(std::move(ka))
      .startInlineUnsafe([callback = std::move(callback)](
                             folly::Try<folly::lift_unit_t<T>>&& tryResult) {
        callback->complete(std::move(tryResult));
      });
}
#endif

folly::exception_wrapper create_app_exn_unimplemented(const char* name);
[[noreturn]] void throw_app_exn_unimplemented(const char* name);

#if FOLLY_HAS_COROUTINES
/**
 * Exception thrown from default generated ServiceHandler coroutine methods to
 * signal to caller that the method is not implemented by the user. This
 * information can then be used by async_tm_* to try other overloads.
 *
 * The primary purpose of this mechanism is to avoid coroutine codegen, which
 * has a huge impact on compile times.
 *
 * In most cases, default generated methods wrap another implementation, e.g.
 * future_* wraps semifuture_*. So co_* could wrap future_*. That would involve
 * using co_await, co_yield, co_return etc. We want to avoid these because the
 * compiler is slow to compile coroutines.
 */
class UnimplementedCoroMethod : public std::exception {
 public:
  /**
   * Because the public interface of coroutine methods accept input by value
   * which are moved in (e.g. std::unique_ptr<T>), this exception needs to pass
   * the arguments back to the caller so that another overload can be called
   * with these values.
   */
  template <typename... Args>
  static UnimplementedCoroMethod withCapturedArgs(Args... args) {
    return UnimplementedCoroMethod(
        folly::make_erased_unique<std::tuple<Args...>>(std::move(args)...));
  }

  /**
   * If the stack_arguments option is set, then arguments are not moved in so
   * capturing them is not necessary.
   */
  static UnimplementedCoroMethod withoutCapturedArgs() {
    return UnimplementedCoroMethod(folly::empty_erased_unique_ptr());
  }

  /**
   * Returns the arguments provided in withCapturedArgs.
   *
   * The template parameters must exactly match those passed to
   * withCapturedArgs. Otherwise, the behavior is undefined.
   */
  template <typename... Args>
  std::tuple<Args...> restoreArgs() && {
    DCHECK(args_ != nullptr);
    auto* args = reinterpret_cast<std::tuple<Args...>*>(args_.get());
    return std::move(*args);
  }

 private:
  explicit UnimplementedCoroMethod(folly::erased_unique_ptr args)
      : args_{std::move(args)} {}
  folly::erased_unique_ptr args_;
};
#endif // FOLLY_HAS_COROUTINES

/**
 * Helper function to mark that some objects are intentionally unused and avoid
 * tripping compiler warnings.
 */
template <typename... Ignore>
void ignore(Ignore&&...) {}

#if defined(THRIFT_SCHEMA_AVAILABLE)
inline std::optional<std::vector<schema::SchemaV1>> schemaAsOptionalVector(
    const schema::SchemaV1& schema) {
  std::vector<schema::SchemaV1> vec = {};
  vec.insert(vec.end(), schema);
  return std::optional<std::vector<schema::SchemaV1>>(vec);
}
#endif
} // namespace si
} // namespace detail

namespace util {

namespace detail {

constexpr ErrorKind fromExceptionKind(ExceptionKind kind) {
  switch (kind) {
    case ExceptionKind::TRANSIENT:
      return ErrorKind::TRANSIENT;

    case ExceptionKind::STATEFUL:
      return ErrorKind::STATEFUL;

    case ExceptionKind::PERMANENT:
      return ErrorKind::PERMANENT;

    default:
      return ErrorKind::UNSPECIFIED;
  }
}

constexpr ErrorBlame fromExceptionBlame(ExceptionBlame blame) {
  switch (blame) {
    case ExceptionBlame::SERVER:
      return ErrorBlame::SERVER;

    case ExceptionBlame::CLIENT:
      return ErrorBlame::CLIENT;

    default:
      return ErrorBlame::UNSPECIFIED;
  }
}

constexpr ErrorSafety fromExceptionSafety(ExceptionSafety safety) {
  switch (safety) {
    case ExceptionSafety::SAFE:
      return ErrorSafety::SAFE;

    default:
      return ErrorSafety::UNSPECIFIED;
  }
}

template <typename T>
std::string serializeExceptionMeta(const folly::exception_wrapper& ew) {
  ErrorClassification errorClassification;

  constexpr auto errorKind = apache::thrift::detail::st::struct_private_access::
      __fbthrift_cpp2_gen_exception_kind<T>();
  errorClassification.kind() = fromExceptionKind(errorKind);
  constexpr auto errorBlame = apache::thrift::detail::st::private_access::
      __fbthrift_cpp2_gen_exception_blame<T>();
  errorClassification.blame() = fromExceptionBlame(errorBlame);
  constexpr auto errorSafety = apache::thrift::detail::st::private_access::
      __fbthrift_cpp2_gen_exception_safety<T>();
  errorClassification.safety() = fromExceptionSafety(errorSafety);

  ew.with_exception(
      [&errorClassification](const ExceptionMetadataOverrideBase& ex) {
        if (ex.errorKind() != ExceptionKind::UNSPECIFIED) {
          errorClassification.kind() = fromExceptionKind(ex.errorKind());
        }
        if (ex.errorBlame() != ExceptionBlame::UNSPECIFIED) {
          errorClassification.blame() = fromExceptionBlame(ex.errorBlame());
        }
        if (ex.errorSafety() != ExceptionSafety::UNSPECIFIED) {
          errorClassification.safety() = fromExceptionSafety(ex.errorSafety());
        }
      });

  return apache::thrift::detail::serializeErrorClassification(
      errorClassification);
}

} // namespace detail

void appendExceptionToHeader(
    const folly::exception_wrapper& ew, Cpp2RequestContext& ctx);

template <typename T>
void appendErrorClassificationToHeader(
    const folly::exception_wrapper& ew, Cpp2RequestContext& ctx) {
  auto header = ctx.getHeader();
  if (!header) {
    return;
  }
  auto exMeta = detail::serializeExceptionMeta<T>(ew);
  header->setHeader(
      std::string(apache::thrift::detail::kHeaderExMeta), std::move(exMeta));
}

TApplicationException toTApplicationException(
    const folly::exception_wrapper& ew);

bool includeInRecentRequestsCount(const std::string_view methodName);

} // namespace util

} // namespace thrift
} // namespace apache
