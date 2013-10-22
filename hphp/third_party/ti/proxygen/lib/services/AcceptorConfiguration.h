// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include "folly/String.h"
#include "ti/proxygen/lib/utils/SocketOptions.h"
#include "ti/proxygen/lib/services/TcpEventsConfig.h"
#include "thrift/lib/cpp/async/TAsyncSocket.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#include <chrono>
#include <list>
#include <string>

namespace facebook { namespace proxygen {

/**
 * Configure a single Acceptor.
 *
 * This configures not only accept behavior, but also some types of SSL
 * behavior that may make sense to configure on a per-VIP basis (e.g. which
 * cert(s) we use, etc).
 */
class AcceptorConfiguration {
 public:

  /** Set/get the address to bind to */
  void setAddress(const apache::thrift::transport::TSocketAddress &addr) {
    addr_ = addr;
  }

  const apache::thrift::transport::TSocketAddress& getAddress() const {
    return addr_;
  }

  /**
   * Set/get whether or not the interface is being used to accept traffic from
   * internal or external clients. Internal clients will different behavior
   * (e.g. Via headers, etc).
   */
  void setInternal(bool internal) { internal_ = internal; }
  bool getInternal() const { return internal_; }

  /**
   * Set/get the depth of the accept queue backlog.
   */
  void setAcceptBacklog(uint32_t len) { acceptBacklog_ = len; }
  uint32_t getAcceptBacklog() const { return acceptBacklog_; }

  /**
   * Set/get the number of milliseconds that a connection can be idle before we
   * close it.
   */
  void setConnectionIdleTime(std::chrono::milliseconds time) {
    connIdleTime_ = time;
  }
  std::chrono::milliseconds getConnectionIdleTime() const {
    return connIdleTime_;
  }

  /**
   * Set/get the number of milliseconds that a transaction can be idle before we
   * close it.
   */
  void setTransactionIdleTime(std::chrono::milliseconds time) {
    transactionIdleTime_ = time;
  }
  std::chrono::milliseconds getTransactionIdleTime() const {
    return transactionIdleTime_;
  }

  /**
   * Set/get the socket options to apply on all the downstream connections
   */
  void setSocketOptions(
    const apache::thrift::async::TAsyncSocket::OptionMap& opts) {
    socketOptions_ = filterIPSocketOptions(opts, addr_.getFamily());
  }
  apache::thrift::async::TAsyncSocket::OptionMap&
  getSocketOptions() {
    return socketOptions_;
  }
  const apache::thrift::async::TAsyncSocket::OptionMap&
  getSocketOptions() const {
    return socketOptions_;
  }

  /**
   * Set/get the name of a protocol to use on non-TLS connections.
   */
  void setPlaintextProtocol(const std::string& proto) {
    plaintextProtocol_ = proto;
  }
  const std::string& getPlaintextProtocol() const {
    return plaintextProtocol_;
  }

  /**
   * Set/get the name of this acceptor; used for stats/reporting purposes.
   */
  void setName(const std::string &name) {
    name_ = name;
  }
  const std::string &getName() const { return name_; }

  void setTcpEventsConfig(TcpEventsConfig tcpEventsConfig) {
    tcpEvents_ = tcpEventsConfig;
  }

  const TcpEventsConfig& getTcpEventsConfig() const {
    return tcpEvents_;
  }

  void setUseNativeCodec(bool useNative) {
    nativeCodec_ = useNative;
  }
  bool getUseNativeCodec() const {
    return nativeCodec_;
  }

  void setCompressionLevel(int spdyCompressionLevel) {
    spdyCompressionLevel_ = spdyCompressionLevel;
  }
  int getSPDYCompressionLevel() const {
    return spdyCompressionLevel_;
  }

 private:
  apache::thrift::transport::TSocketAddress addr_;
  bool internal_{false};
  uint32_t acceptBacklog_{1024};
  std::chrono::milliseconds connIdleTime_{600000};
  std::chrono::milliseconds transactionIdleTime_{600000};
  std::string name_;
  std::string plaintextProtocol_;
  apache::thrift::async::TAsyncSocket::OptionMap socketOptions_;
  bool nativeCodec_{false};
  int spdyCompressionLevel_{Z_NO_COMPRESSION};
  TcpEventsConfig tcpEvents_;
};

}} // facebook::proxygen
