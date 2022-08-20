/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "MessagePrinter.h"

#include <folly/lang/Bits.h>

namespace facebook {
namespace memcache {

namespace {

bool matchIPAddress(
    const folly::IPAddress& expectedIp,
    const folly::SocketAddress& address) {
  return !address.empty() && expectedIp == address.getIPAddress();
}

bool matchPort(uint16_t expectedPort, const folly::SocketAddress& address) {
  return !address.empty() && expectedPort == address.getPort();
}

std::string describeAddress(const folly::SocketAddress& address) {
  auto res = address.describe();
  if (address.getFamily() == AF_UNIX) {
    // Check if the path was truncated.
    if (res.size() >=
        MessageHeader::kAddressMaxSize - kUnixSocketPrefix.size() - 1) {
      return res + "...";
    }
  }
  return res;
}

} // anonymous namespace

MessagePrinter::MessagePrinter(
    Options options,
    Filter filter,
    std::unique_ptr<ValueFormatter> valueFormatter,
    std::ostream& targetOut)
    : options_(std::move(options)),
      filter_(std::move(filter)),
      valueFormatter_(std::move(valueFormatter)),
      targetOut_(targetOut) {
  if (options_.disableColor) {
    targetOut_.setColorOutput(false);
  }
}

bool MessagePrinter::matchAddress(
    const folly::SocketAddress& from,
    const folly::SocketAddress& to) const {
  // Initial filters
  if (!filter_.host.empty() && !matchIPAddress(filter_.host, from) &&
      !matchIPAddress(filter_.host, to)) {
    return false;
  }
  if (filter_.port != 0 && !matchPort(filter_.port, from) &&
      !matchPort(filter_.port, to)) {
    return false;
  }

  return true;
}

void MessagePrinter::countStats() {
  ++stats_.printedMessages;

  if (options_.maxMessages > 0 &&
      stats_.printedMessages >= options_.maxMessages) {
    assert(options_.stopRunningFn);
    options_.stopRunningFn();
  }

  if (options_.numAfterMatch > 0) {
    --afterMatchCount_;
  }
}

void MessagePrinter::printRawMessage(
    const struct iovec* iovsBegin,
    size_t iovsCount) {
  if (iovsBegin == nullptr) {
    return;
  }
  uint64_t rawMessageSize = 0;
  for (size_t i = 0; i < iovsCount; ++i) {
    rawMessageSize += iovsBegin[i].iov_len;
  }
  StyledString rawMessage;
  rawMessageSize = folly::Endian::little(rawMessageSize);
  rawMessage.append(
      std::string(reinterpret_cast<char*>(&rawMessageSize), sizeof(uint64_t)));
  for (size_t i = 0; i < iovsCount; ++i) {
    rawMessage.append(std::string(
        static_cast<char*>(iovsBegin[i].iov_base), iovsBegin[i].iov_len));
  }
  printMessage(rawMessage);
}

void MessagePrinter::printMessage(const StyledString& message) {
  targetOut_ << message;
  targetOut_.flush();
  countStats();
}

std::string MessagePrinter::serializeConnectionDetails(
    const folly::SocketAddress& from,
    const folly::SocketAddress& to,
    mc_protocol_t protocol) {
  std::string out;

  if (!from.empty()) {
    if (options_.script) {
      out.append(
          folly::sformat(",\n  \"from\": \"{}\"", describeAddress(from)));
    } else {
      out.append(describeAddress(from));
    }
  }
  if (!options_.script && (!from.empty() || !to.empty())) {
    out.append(" -> ");
  }
  if (!to.empty()) {
    if (options_.script) {
      out.append(folly::sformat(",\n  \"to\": \"{}\"", describeAddress(to)));
    } else {
      out.append(describeAddress(to));
    }
  }
  if ((!from.empty() || !to.empty()) && protocol != mc_unknown_protocol) {
    if (options_.script) {
      out.append(folly::sformat(
          ",\n  \"protocol\": \"{}\"", mc_protocol_to_string(protocol)));
    } else {
      out.append(folly::sformat(" ({})", mc_protocol_to_string(protocol)));
    }
  }

  return out;
}

std::string MessagePrinter::serializeMessageHeader(
    folly::StringPiece messageName,
    carbon::Result result,
    const std::string& key) {
  std::string out;

  if (options_.script) {
    out.append(folly::sformat("\"type\": \"{}\"", messageName.data()));
    if (result != carbon::Result::UNKNOWN) {
      out.append(folly::sformat(
          ",\n  \"result\": \"{}\"", carbon::resultToString(result)));
    }
    if (!key.empty()) {
      out.append(
          folly::sformat(",\n  \"key\": \"{}\"", folly::backslashify(key)));
    }
  } else {
    out.append(messageName.data());
    if (result != carbon::Result::UNKNOWN) {
      out.push_back(' ');
      out.append(carbon::resultToString(result));
    }
    if (!key.empty()) {
      out.push_back(' ');
      out.append(folly::backslashify(key));
    }
  }

  return out;
}

/**
 * Matches all the occurences of "pattern" in "text"
 *
 * @return A vector of pairs containing the index and size (respectively)
 *         of all ocurrences.
 */
std::vector<std::pair<size_t, size_t>> MessagePrinter::matchAll(
    folly::StringPiece text,
    const boost::regex& pattern) const {
  std::vector<std::pair<size_t, size_t>> result;

  boost::cregex_token_iterator it(text.begin(), text.end(), pattern);
  boost::cregex_token_iterator end;
  while (it != end) {
    result.emplace_back(it->first - text.begin(), it->length());
    ++it;
  }
  return result;
}
} // namespace memcache
} // namespace facebook
