/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#include "mcrouter/lib/network/AsciiSerialized.h"
#include "mcrouter/lib/network/McSerializedRequest.h"
#include "mcrouter/lib/network/MessageHelpers.h"
#include "mcrouter/lib/network/ServerLoad.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/tools/mcpiper/Color.h"
#include "mcrouter/tools/mcpiper/Config.h"
#include "mcrouter/tools/mcpiper/McPiperVisitor.h"
#include "mcrouter/tools/mcpiper/Util.h"

namespace facebook {
namespace memcache {

namespace detail {

// Exptime
template <class M>
typename std::enable_if<HasExptimeTrait<M>::value, int32_t>::type getExptime(
    const M& req) {
  return *req.exptime_ref();
}
template <class M>
typename std::enable_if<!HasExptimeTrait<M>::value, int32_t>::type getExptime(
    const M& /* reply */) {
  return 0;
}

// Lease token
template <class M>
int64_t getLeaseToken(const M& /* msg */) {
  return 0;
}
inline int64_t getLeaseToken(const McLeaseGetReply& msg) {
  return *msg.leaseToken_ref();
}
inline int64_t getLeaseToken(const McLeaseSetRequest& msg) {
  return *msg.leaseToken_ref();
}

// Message
template <class M>
typename std::
    enable_if<!carbon::detail::HasMessage<M>::value, folly::StringPiece>::type
    getMessage(const M& /* msg */) {
  return folly::StringPiece();
}

template <class M>
typename std::
    enable_if<carbon::detail::HasMessage<M>::value, folly::StringPiece>::type
    getMessage(const M& msg) {
  return *msg.message_ref();
}

template <class M>
constexpr
    typename std::enable_if<carbon::IsRequestTrait<M>::value, const char*>::type
    getName() {
  return M::name;
}

template <class M>
constexpr typename std::
    enable_if<!carbon::IsRequestTrait<M>::value, const char*>::type
    getName() {
  return MatchingRequest<M>::name();
}

} // namespace detail

template <class Request>
void MessagePrinter::requestReady(
    uint64_t msgId,
    Request&& request,
    const folly::SocketAddress& from,
    const folly::SocketAddress& to,
    mc_protocol_t protocol) {
  if (auto out = filterAndBuildOutput(
          msgId,
          request,
          request.key_ref()->fullKey().str(),
          carbon::Result::UNKNOWN,
          from,
          to,
          protocol)) {
    if (options_.raw) {
      printRawRequest(msgId, request, protocol);
    } else {
      printMessage(out.value());
    }
  }
}

template <class Reply>
void MessagePrinter::replyReady(
    uint64_t msgId,
    Reply&& reply,
    std::string key,
    const folly::SocketAddress& from,
    const folly::SocketAddress& to,
    mc_protocol_t protocol,
    int64_t latencyUs,
    RpcStatsContext rpcStatsContext) {
  if (auto out = filterAndBuildOutput(
          msgId,
          reply,
          key,
          *reply.result_ref(),
          from,
          to,
          protocol,
          latencyUs,
          rpcStatsContext.serverLoad)) {
    stats_.numBytesBeforeCompression +=
        rpcStatsContext.replySizeBeforeCompression;
    stats_.numBytesAfterCompression +=
        rpcStatsContext.replySizeAfterCompression;
    if (options_.raw) {
      printRawReply(msgId, std::forward<Reply>(reply), protocol);
    } else {
      printMessage(out.value());
    }
  }
}

template <class Message>
folly::Optional<StyledString> MessagePrinter::filterAndBuildOutput(
    uint64_t msgId,
    const Message& message,
    const std::string& key,
    carbon::Result result,
    const folly::SocketAddress& from,
    const folly::SocketAddress& to,
    mc_protocol_t protocol,
    int64_t latencyUs,
    const ServerLoad& serverLoad) {
  ++stats_.totalMessages;

  if (!matchAddress(from, to)) {
    return folly::none;
  }
  if (filter_.protocol.hasValue() && filter_.protocol.value() != protocol) {
    return folly::none;
  }

  auto value = carbon::valueRangeSlow(const_cast<Message&>(message));
  if (value.size() < filter_.valueMinSize ||
      value.size() > filter_.valueMaxSize) {
    return folly::none;
  }

  // if latency is 0 and the filter is not set, we let it pass through
  if (latencyUs < filter_.minLatencyUs) {
    return folly::none;
  }

  StyledString out;
  out.append("\n");

  if (options_.script) {
    out.append("{\n", format_.dataOpColor);
    /* always present, makes comma accounting simpler */
    out.append(folly::sformat("  \"reqid\": {}", msgId));
  }

  if (options_.printTimeFn) {
    timeval ts;
    gettimeofday(&ts, nullptr);

    if (options_.script) {
      out.append(
          folly::sformat(",\n  \"ts\": \"{}\"", options_.printTimeFn(ts)));
    } else {
      out.append(options_.printTimeFn(ts));
      out.append(" ");
    }
  }

  out.append(serializeConnectionDetails(from, to, protocol));

  if (!options_.script) {
    out.append("\n{\n", format_.dataOpColor);
  }

  auto msgHeader =
      serializeMessageHeader(detail::getName<Message>(), result, key);
  if (!msgHeader.empty()) {
    if (options_.script) {
      out.append(",\n  ");
    } else {
      out.append("  ");
    }
    out.append(std::move(msgHeader), format_.headerColor);
  }

  // Msg attributes
  if (!options_.script) {
    /* Rendered above for script mode */
    out.append("\n  reqid: ", format_.msgAttrColor);
    out.append(folly::sformat("0x{:x}", msgId), format_.dataValueColor);
  }

  if (latencyUs > 0) { // it is 0 only for requests
    if (options_.script) {
      out.append(",\n  \"rtt_us\": ");
    } else {
      out.append("\n  request/response latency (us): ", format_.msgAttrColor);
    }
    out.append(folly::to<std::string>(latencyUs), format_.dataValueColor);
  }

  if (!serverLoad.isZero()) {
    if (options_.script) {
      out.append(",\n  \"server_load_percent\": ");
    } else {
      out.append("\n  server load: ", format_.msgAttrColor);
    }
    out.append(
        folly::sformat("{:.2f}%", serverLoad.percentLoad()),
        format_.dataValueColor);
  }

  if (options_.script) {
    out.append(",\n  \"flags\": ");
    out.append(folly::to<std::string>(getFlagsIfExist(message)));
  } else {
    out.append("\n  flags: ", format_.msgAttrColor);
    out.append(
        folly::sformat("0x{:x}", getFlagsIfExist(message)),
        format_.dataValueColor);
  }
  if (!options_.script && getFlagsIfExist(message)) {
    auto flagDesc = describeFlags(getFlagsIfExist(message));
    if (!flagDesc.empty()) {
      out.pushAppendColor(format_.attrColor);
      out.append(" [");
      bool first = true;
      for (auto& s : flagDesc) {
        if (!first) {
          out.append(", ");
        }
        first = false;
        out.append(s);
      }
      out.pushBack(']');
      out.popAppendColor();
    }
  }
  auto typeSpecificAttr = getTypeSpecificAttributes(message);
  if (options_.script && !typeSpecificAttr.empty()) {
    out.pushBack(',');
  }
  out.append(typeSpecificAttr);

  if (!value.empty()) {
    size_t uncompressedSize;
    auto formattedValue = valueFormatter_->uncompressAndFormat(
        value,
        getFlagsIfExist(message),
        format_,
        options_.script,
        uncompressedSize);

    if (options_.script) {
      out.append(folly::sformat(",\n  \"value_wire_bytes\": {}", value.size()));
      out.append(folly::sformat(
          ",\n  \"value_uncompressed_bytes\": {}", uncompressedSize));
    } else {
      out.append("\n  value size: ", format_.msgAttrColor);
      if (uncompressedSize != value.size()) {
        out.append(
            folly::sformat(
                "{} uncompressed, {} compressed, {:.2f}% savings",
                uncompressedSize,
                value.size(),
                100.0 - 100.0 * value.size() / uncompressedSize),
            format_.dataValueColor);
      } else {
        out.append(
            folly::to<std::string>(value.size()), format_.dataValueColor);
      }
    }

    if (!options_.quiet) {
      if (options_.script) {
        out.append(",\n  \"value\": ");
      } else {
        out.append("\n  value: ", format_.msgAttrColor);
      }
      out.append(formattedValue);
    }
  }

  if (options_.script) {
    out.append("\n},\n");
  } else {
    out.append("\n}\n", format_.dataOpColor);
  }

  // Match pattern
  if (filter_.pattern) {
    auto matches = matchAll(out.text(), *filter_.pattern);
    auto success = matches.empty() == filter_.invertMatch;

    if (!success && afterMatchCount_ == 0) {
      return folly::none;
    }
    if (!filter_.invertMatch) {
      for (auto& m : matches) {
        out.setFg(m.first, m.second, format_.matchColor);
      }
    }

    // Reset after match
    if (success && options_.numAfterMatch > 0) {
      afterMatchCount_ = options_.numAfterMatch + 1;
    }
  }

  return out;
}

template <class Reply>
void MessagePrinter::printRawReply(
    uint64_t msgId,
    Reply&& reply,
    mc_protocol_t protocol) {
  const struct iovec* iovsBegin = nullptr;
  size_t iovsCount = 0;
  CaretSerializedMessage caretSerializedMessage;
  switch (protocol) {
    case mc_ascii_protocol:
      LOG_FIRST_N(INFO, 1) << "ASCII protocol is not supported for raw data";
      return;
    case mc_caret_protocol:
      if (!caretSerializedMessage.prepare(
              std::move(reply),
              msgId,
              CodecIdRange::Empty,
              nullptr, /* codec map */
              ServerLoad::zero(),
              iovsBegin,
              iovsCount)) {
        LOG(ERROR) << "Serialization failed for caret reply " << msgId;
        return;
      }
      break;
    default:
      CHECK(false) << "Invalid protocol!";
  }

  printRawMessage(iovsBegin, iovsCount);
}

template <class Request>
void MessagePrinter::printRawRequest(
    uint64_t msgId,
    const Request& request,
    mc_protocol_t protocol) {
  if (protocol == mc_ascii_protocol) {
    LOG_FIRST_N(INFO, 1) << "ASCII protocol is not supported for raw data";
    return;
  }

  McSerializedRequest req(request, msgId, protocol, CodecIdRange::Empty);
  if (req.serializationResult() == McSerializedRequest::Result::OK) {
    printRawMessage(req.getIovs(), req.getIovsCount());
  } else {
    LOG(ERROR) << "Serialization failed for request " << msgId << ".";
  }
}

template <class Message>
StyledString MessagePrinter::getTypeSpecificAttributes(const Message& msg) {
  return carbon::print(msg, detail::getName<Message>(), options_.script);
}

} // namespace memcache
} // namespace facebook
