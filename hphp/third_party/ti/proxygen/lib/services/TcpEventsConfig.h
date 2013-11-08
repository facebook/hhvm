// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include "thrift/lib/cpp/async/TAsyncSocket.h"

#include <stdint.h>
#include <chrono>
#include <iosfwd>

namespace facebook {

namespace json {
class JsonObject;
}

namespace proxygen {

class TcpEventsConfig {
 public:
  static const bool     DEFAULT_ENABLED{false};
  static const double   DEFAULT_SAMPLE_RATE;
  static const uint32_t KERNEL_LIMIT_ACK_TRACKED{16};
  static const uint32_t DEFAULT_ACK_TRACKED{4};
  static const uint64_t DEFAULT_ACK_TIMEOUT_MS{10000};
  static const char * const ENTITY;

  static TcpEventsConfig parse(const facebook::json::JsonObject& configObj);

  friend std::ostream& operator<<(std::ostream& os, const TcpEventsConfig& cfg);

  TcpEventsConfig();

  virtual ~TcpEventsConfig() {}


  std::chrono::milliseconds getTimeout() const {
    return ackTimeout_;
  }

  uint32_t getMaxAckTracked() const {
    return maxAckTracked_;
  }

  void setMaxAckTracked(uint32_t n);

  bool shouldSample() const;

  void updateSocketOptions(
    apache::thrift::async::TAsyncSocket::OptionMap& opts);

private:
  bool enabled_{DEFAULT_ENABLED};
  double sampleRate_{DEFAULT_SAMPLE_RATE};
  uint32_t samplingKey_{0};
  uint32_t maxAckTracked_{DEFAULT_ACK_TRACKED};
  std::chrono::milliseconds ackTimeout_{DEFAULT_ACK_TIMEOUT_MS};
};

}}
