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

#include <folly/Demangle.h>
#include <folly/Portability.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#if __has_include(<thrift/lib/thrift/gen-cpp2/any_rep_types.h>)
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/AnyValue.h>
#include <thrift/lib/cpp2/type/Runtime.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>
#define THRIFT_ANY_AVAILABLE
#endif

using namespace std;
using namespace folly;
using namespace apache::thrift::transport;

namespace apache::thrift {

namespace detail {

THRIFT_PLUGGABLE_FUNC_REGISTER(
    bool, includeInRecentRequestsCount, const std::string_view /*methodName*/) {
  // Users of the module will override the behavior
  return true;
}

} // namespace detail

namespace detail::ac {

[[noreturn]] void throw_app_exn(const char* const msg) {
  throw TApplicationException(msg);
}

folly::exception_wrapper try_extract_any_exception(
    const apache::thrift::transport::THeader::StringToStringMap& headers) {
#ifdef THRIFT_ANY_AVAILABLE
  auto anyexTypePtr = folly::get_ptr(
      headers, std::string(apache::thrift::detail::kHeaderAnyexType));
  if (!anyexTypePtr) {
    return {};
  }
  auto anyexPtr = folly::get_ptr(
      headers, std::string(apache::thrift::detail::kHeaderAnyex));
  if (!anyexPtr) {
    return {};
  }

  try {
    type::SemiAny builder;
    builder.type() = type::Type(type::exception_c{}, *anyexTypePtr);
    builder.protocol() = type::Protocol::get<type::StandardProtocol::Compact>();
    builder.data() = *protocol::base64Decode(*anyexPtr);
    type::AnyData data(builder);
    if (auto ew = apache::thrift::type::TypeRegistry::generated()
                      .load(data)
                      .asExceptionWrapper()) {
      return ew;
    }
  } catch (...) {
  }
#endif
  return {};
}

} // namespace detail::ac

namespace detail::ap {

template <typename ProtocolReader, typename ProtocolWriter>
std::unique_ptr<folly::IOBuf> helper<ProtocolReader, ProtocolWriter>::write_exn(
    bool includeEnvelope,
    const char* method,
    ProtocolWriter* prot,
    int32_t protoSeqId,
    ContextStack* ctx,
    const TApplicationException& x) {
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  size_t bufSize =
      apache::thrift::detail::serializedExceptionBodySizeZC(prot, &x);
  bufSize += prot->serializedMessageSize(method);
  prot->setOutput(&queue, bufSize);
  if (ctx) {
    ctx->handlerErrorWrapped(exception_wrapper(x));
  }
  if (includeEnvelope) {
    prot->writeMessageBegin(method, MessageType::T_EXCEPTION, protoSeqId);
  }
  apache::thrift::detail::serializeExceptionBody(prot, &x);
  if (includeEnvelope) {
    prot->writeMessageEnd();
  }
  return std::move(queue).move();
}

template <typename ProtocolReader, typename ProtocolWriter>
void helper<ProtocolReader, ProtocolWriter>::process_exn(
    const char* func,
    const TApplicationException::TApplicationExceptionType type,
    const string& msg,
    ResponseChannelRequest::UniquePtr req,
    Cpp2RequestContext* ctx,
    EventBase* eb,
    int32_t protoSeqId) {
  ProtocolWriter oprot;
  if (req) {
    LOG(ERROR) << msg << " in function " << func;
    TApplicationException x(type, msg);
    auto payload = THeader::transform(
        helper_w<ProtocolWriter>::write_exn(
            req->includeEnvelope(), func, &oprot, protoSeqId, nullptr, x),
        ctx->getHeader()->getWriteTransforms());
    eb->runInEventBaseThread(
        [payload = std::move(payload), request = std::move(req)]() mutable {
          if (request->isStream()) {
            request->sendStreamReply(
                ResponsePayload::create(std::move(payload)),
                detail::ServerStreamFactory{nullptr});
          } else if (request->isSink()) {
#if FOLLY_HAS_COROUTINES
            request->sendSinkReply(
                ResponsePayload::create(std::move(payload)),
                detail::SinkConsumerImpl{});
#else
            DCHECK(false);
#endif
          } else {
            request->sendReply(ResponsePayload::create(std::move(payload)));
          }
        });
  } else {
    LOG(ERROR) << msg << " in oneway function " << func;
  }
}

template struct helper<BinaryProtocolReader, BinaryProtocolWriter>;
template struct helper<CompactProtocolReader, CompactProtocolWriter>;

template <typename ProtocolReader>
static bool setupRequestContextWithMessageBegin(
    const MessageBegin::Metadata& msgBegin,
    ResponseChannelRequest::UniquePtr& req,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb) {
  using h = helper_r<ProtocolReader>;
  const char* fn = "process";
  if (!msgBegin.isValid) {
    LOG(ERROR) << "received invalid message from client: "
               << msgBegin.errMessage;
    auto type = TApplicationException::TApplicationExceptionType::UNKNOWN;
    const char* msg = "invalid message from client";
    h::process_exn(fn, type, msg, std::move(req), ctx, eb, msgBegin.seqId);
    return false;
  }
  if (msgBegin.msgType != MessageType::T_CALL &&
      msgBegin.msgType != MessageType::T_ONEWAY) {
    LOG(ERROR) << "received invalid message of type "
               << folly::to_underlying(msgBegin.msgType);
    auto type =
        TApplicationException::TApplicationExceptionType::INVALID_MESSAGE_TYPE;
    const char* msg = "invalid message arguments";
    h::process_exn(fn, type, msg, std::move(req), ctx, eb, msgBegin.seqId);
    return false;
  }

  ctx->setProtoSeqId(msgBegin.seqId);
  return true;
}

bool setupRequestContextWithMessageBegin(
    const MessageBegin::Metadata& msgBegin,
    protocol::PROTOCOL_TYPES protType,
    ResponseChannelRequest::UniquePtr& req,
    Cpp2RequestContext* ctx,
    folly::EventBase* eb) {
  switch (protType) {
    case protocol::T_BINARY_PROTOCOL:
      return setupRequestContextWithMessageBegin<BinaryProtocolReader>(
          msgBegin, req, ctx, eb);
    case protocol::T_COMPACT_PROTOCOL:
      return setupRequestContextWithMessageBegin<CompactProtocolReader>(
          msgBegin, req, ctx, eb);
    default:
      LOG(ERROR) << "invalid protType: " << folly::to_underlying(protType);
      return false;
  }
}

MessageBegin deserializeMessageBegin(
    const folly::IOBuf& buf, protocol::PROTOCOL_TYPES protType) {
  MessageBegin msgBegin;
  auto& meta = msgBegin.metadata;
  try {
    switch (protType) {
      case protocol::T_COMPACT_PROTOCOL: {
        CompactProtocolReader iprot;
        iprot.setInput(&buf);
        iprot.readMessageBegin(msgBegin.methodName, meta.msgType, meta.seqId);
        meta.size = iprot.getCursorPosition();
        break;
      }
      case protocol::T_BINARY_PROTOCOL: {
        BinaryProtocolReader iprot;
        iprot.setInput(&buf);
        iprot.readMessageBegin(msgBegin.methodName, meta.msgType, meta.seqId);
        meta.size = iprot.getCursorPosition();
        break;
      }
      default:
        meta.isValid = false;
        meta.errMessage = "unsupported protocol, unparseable message begin";
        LOG(ERROR) << "received unsupported protocol value: " << protType;
    }
  } catch (const TException& ex) {
    meta.isValid = false;
    meta.errMessage = ex.what();
    LOG(ERROR) << "received invalid message from client: " << ex.what();
  }
  return msgBegin;
}
} // namespace detail::ap

namespace detail::si {
namespace {
TrustedServerException createUnimplementedMethodException(
    std::string_view methodName) {
  return TrustedServerException::unimplementedMethodError(
      fmt::format("Function {} is unimplemented", methodName).c_str());
}

TrustedServerException createBadInteractionStateException(
    std::string_view interactionName, std::string_view methodName) {
  return TrustedServerException::unimplementedMethodError(
      fmt::format(
          "Interaction {} is in bad state for method {}",
          interactionName,
          methodName)
          .c_str());
}
} // namespace

folly::exception_wrapper create_app_exn_unimplemented(const char* name) {
  return folly::make_exception_wrapper<TrustedServerException>(
      createUnimplementedMethodException(name));
}

[[noreturn]] void throw_app_exn_unimplemented(const char* const name) {
  throw createUnimplementedMethodException(name);
}

folly::exception_wrapper create_app_exn_bad_interaction_state(
    std::string_view interactionName, std::string_view methodName) {
  return folly::make_exception_wrapper<TrustedServerException>(
      createBadInteractionStateException(interactionName, methodName));
}
} // namespace detail::si

namespace {

constexpr size_t kMaxUexwSize = 1024;

void setUserExceptionHeader(
    Cpp2RequestContext& ctx,
    std::string exType,
    std::string exReason,
    bool setClientCode) {
  auto header = ctx.getHeader();
  if (!header) {
    return;
  }

  if (setClientCode) {
    header->setHeader(std::string(detail::kHeaderEx), kAppClientErrorCode);
  }

  header->setHeader(std::string(detail::kHeaderUex), std::move(exType));
  header->setHeader(
      std::string(detail::kHeaderUexw),
      exReason.size() > kMaxUexwSize ? exReason.substr(0, kMaxUexwSize)
                                     : std::move(exReason));
}

} // namespace

namespace util {

void appendExceptionToHeader(
    const folly::exception_wrapper& ew, Cpp2RequestContext& ctx) {
  ctx.setException();
  auto* ex = ew.get_exception();
  if (const auto* aex = dynamic_cast<const AppBaseError*>(ex)) {
    setUserExceptionHeader(
        ctx,
        std::string(aex->name()),
        std::string(aex->what()),
        aex->isClientError());
    return;
  }

  const auto what = ew.what();
  folly::StringPiece whatsp(what);
  auto typeName = ew.class_name();

  ew.with_exception([&](const ExceptionMetadataOverrideBase& emob) {
    if (auto type = emob.type()) {
      typeName = folly::demangle(*type);
    }
  });

  whatsp.removePrefix(typeName);
  whatsp.removePrefix(": ");

  auto exName = typeName.toStdString();
  auto exWhat = whatsp.str();

  setUserExceptionHeader(ctx, std::move(exName), std::move(exWhat), false);
}

TApplicationException toTApplicationException(
    const folly::exception_wrapper& ew) {
  auto& ex = *ew.get_exception();
  auto msg = folly::exceptionStr(ex).toStdString();

  if (dynamic_cast<const AppBaseError*>(&ex)) { // customized app errors
    return TApplicationException(
        TApplicationException::TApplicationExceptionType::UNKNOWN, ex.what());
  } else {
    if (auto* te = dynamic_cast<const TApplicationException*>(&ex)) {
      return *te;
    } else {
      return TApplicationException(
          TApplicationException::TApplicationExceptionType::UNKNOWN,
          std::move(msg));
    }
  }
}

bool includeInRecentRequestsCount(const std::string_view methodName) {
  return apache::thrift::detail::includeInRecentRequestsCount(methodName);
}

} // namespace util

} // namespace apache::thrift
