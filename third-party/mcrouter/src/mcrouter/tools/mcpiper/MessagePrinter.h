/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <iostream>

#include <boost/regex.hpp>
#include <folly/IPAddress.h>
#include <folly/SocketAddress.h>

#include "mcrouter/lib/network/ServerLoad.h"
#include "mcrouter/tools/mcpiper/AnsiColorCodeStream.h"
#include "mcrouter/tools/mcpiper/PrettyFormat.h"
#include "mcrouter/tools/mcpiper/SnifferParser.h"
#include "mcrouter/tools/mcpiper/StyledString.h"
#include "mcrouter/tools/mcpiper/ValueFormatter.h"

namespace facebook {
namespace memcache {

/**
 * Class responsible for formatting and printing requests and replies.
 */
class MessagePrinter {
 public:
  /**
   * Format settings
   */
  struct Options {
    // Function responsible for printing the time of the messages.
    std::function<std::string(const struct timeval& ts)> printTimeFn{nullptr};

    // Number of messages to show after a match is found.
    uint32_t numAfterMatch{0};

    // If true, does not print values.
    bool quiet{false};

    // Number of messages to print before exiting (i.e. calling stopRunningFn).
    // 0 to disable.
    uint32_t maxMessages{0};

    // Disable nice coloring.
    bool disableColor{false};

    // Callback that will be called when application should stop
    // sending messages to MessagePrinter.
    std::function<void()> stopRunningFn = []() { exit(0); };

    // Getting raw data in binary format
    bool raw{false};

    // Machine-readable JSON format (has no effect if raw is true)
    bool script{false};
  };

  struct Stats {
    std::atomic<uint64_t> totalMessages{0};
    std::atomic<uint64_t> printedMessages{0};
    std::atomic<uint64_t> numBytesBeforeCompression{0};
    std::atomic<uint64_t> numBytesAfterCompression{0};
  };

  /**
   * Filter to be applied to requests/replies
   */
  struct Filter {
    folly::IPAddress host;
    uint16_t port{0};
    uint32_t valueMinSize{0};
    uint32_t valueMaxSize{std::numeric_limits<uint32_t>::max()};
    int64_t minLatencyUs{0}; // 0 means include all messages
    std::unique_ptr<boost::regex> pattern;
    bool invertMatch{false};
    folly::Optional<mc_protocol_t> protocol;
  };

  /**
   * Builds a message printer.
   *
   * @param options         General options.
   * @param filter          Message filter.
   * @param valueFormatter  Class used to format the values of messages.
   */
  MessagePrinter(
      Options options,
      Filter filter,
      std::unique_ptr<ValueFormatter> valueFormatter,
      std::ostream& targetOut = std::cout);

  /**
   * Return stats of message printer.
   */
  inline const Stats& stats() const noexcept {
    return stats_;
  }

  template <class Message>
  folly::Optional<StyledString> filterAndBuildOutput(
      uint64_t msgId,
      const Message& message,
      const std::string& key,
      carbon::Result result,
      const folly::SocketAddress& from,
      const folly::SocketAddress& to,
      mc_protocol_t protocol,
      int64_t latencyUs = 0,
      const ServerLoad& serverLoad = ServerLoad::zero());

 private:
  const Options options_;
  const Filter filter_;
  const PrettyFormat format_{}; // Default constructor = default coloring.

  std::unique_ptr<ValueFormatter> valueFormatter_;
  AnsiColorCodeStream targetOut_;
  Stats stats_;
  uint32_t afterMatchCount_{0};

  // SnifferParser Callbacks
  template <class Request>
  void requestReady(
      uint64_t msgId,
      Request&& request,
      const folly::SocketAddress& from,
      const folly::SocketAddress& to,
      mc_protocol_t protocol);

  template <class Reply>
  void replyReady(
      uint64_t msgId,
      Reply&& reply,
      std::string key,
      const folly::SocketAddress& from,
      const folly::SocketAddress& to,
      mc_protocol_t protocol,
      int64_t latencyUs,
      RpcStatsContext rpcStatsContext);

  template <class Request>
  void printRawRequest(
      uint64_t msgId,
      const Request& request,
      mc_protocol_t protocol);

  template <class Reply>
  void printRawReply(uint64_t msgId, Reply&& reply, mc_protocol_t protocol);

  void printRawMessage(const struct iovec* iovsBegin, size_t iovsCount);

  void printMessage(const StyledString& message);

  void countStats();

  friend class SnifferParserBase<MessagePrinter>;

  template <class Message>
  StyledString getTypeSpecificAttributes(const Message& msg);

  /**
   * Tells whether a message matches the ip/port filter.
   *
   * @param from  Address that sent the message.
   * @param to    Address to where the message was sent.
   *
   * @return      True if the addresses matches the specified filters
   *              (and thus should be printed). False otherwise.
   */
  bool matchAddress(
      const folly::SocketAddress& from,
      const folly::SocketAddress& to) const;

  /**
   * Matches all the occurences of "pattern" in "text"
   *
   * @return A vector of pairs containing the index and size (respectively)
   *         of all ocurrences.
   */
  std::vector<std::pair<size_t, size_t>> matchAll(
      folly::StringPiece text,
      const boost::regex& pattern) const;

  std::string serializeConnectionDetails(
      const folly::SocketAddress& from,
      const folly::SocketAddress& to,
      mc_protocol_t protocol);

  std::string serializeMessageHeader(
      folly::StringPiece messageName,
      carbon::Result result,
      const std::string& key);
};

} // namespace memcache
} // namespace facebook

#include "MessagePrinter-inl.h"
