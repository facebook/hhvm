// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include <chrono>
#include <netinet/tcp.h>

namespace apache { namespace thrift { namespace async {
class TAsyncSocket;
}}}

namespace facebook { namespace proxygen {

struct TransportInfo {

  TransportInfo();

  /*
   * timestamp of when the connection handshake was completed
   */
  std::chrono::system_clock::time_point acceptTime;

  /*
   * connection RTT (Round-Trip Time)
   */
  std::chrono::microseconds rtt;

  /*
   * TCP information as fetched from getsockopt(2)
   */
  struct tcp_info tcpinfo;

  /*
   * true if the tcpinfo was successfully read from the kernel
   */
  bool validTcpinfo;

  /*
   * value of errno in case of getsockopt() error
   */
  int tcpinfoErrno;

  /*
   * time for setting the connection, from the moment in was accepted until it
   * is established.
   */
  std::chrono::milliseconds setupTime;

  /*
   * total number of bytes sent over the connection
   */
  int64_t totalBytes;

  /*
   * time to first byte
   */
  int64_t timeToFirstByte;

  /*
   * time to last byte
   */
  int64_t timeToLastByte;

  /*
   * time it took the client to ACK the last byte, from the moment when the
   * kernel sent the last byte to the client and until it received the ACK
   * for that byte
   */
  std::chrono::milliseconds lastByteAckLatency;

  /*
   * time spent inside proxygen
   */
  int64_t proxyLatency;

  /*
   * time between connection accepted and client message headers completed
   */
  int64_t clientLatency;

  /*
   * latency for communication with the server
   */
  int64_t serverLatency;

  /*
   * time used to get a usable connection.
   */
  int64_t connectLatency;

  /*
   * get the RTT value in milliseconds
   */
  std::chrono::milliseconds getRttMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(rtt);
  }

  /*
   * initialize the fields related with tcp_info
   */
  bool initWithSocket(const apache::thrift::async::TAsyncSocket* sock);

  virtual ~TransportInfo();

  /*
   * Get the kernel's estimate of round-trip time (RTT) to the transport's peer
   * in microseconds. Returns -1 on error.
   */
  static int64_t readRTT(const apache::thrift::async::TAsyncSocket* sock);

  /*
   * perform the getsockopt(2) syscall to fetch TCP info for a given socket
   */
  static bool readTcpInfo(struct tcp_info* tcpinfo,
                          const apache::thrift::async::TAsyncSocket* sock);
};

}} // facebook::proxygen
