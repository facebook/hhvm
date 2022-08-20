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

#include <wangle/acceptor/SecureTransportType.h>
#include <wangle/ssl/SSLUtil.h>

#include <chrono>
#include <string>

#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/portability/Sockets.h>

namespace folly {

class AsyncSocket;

}

namespace wangle {

/**
 * A struct that can store additional information specific to the protocol being
 * used.
 */
struct ProtocolInfo {
  virtual ~ProtocolInfo() = default;
};

struct TransportInfo {
  /*
   * timestamp of when the connection handshake was completed
   */
  std::chrono::steady_clock::time_point acceptTime{};

  /*
   * timestamp of when the socket was accepted
   * and ready to be pushed into the socket queue
   */
  std::chrono::steady_clock::time_point timeBeforeEnqueue{};

  /*
   * connection RTT (Round-Trip Time)
   */
  std::chrono::microseconds rtt{0};

  /*
   * RTT variance in usecs (microseconds)
   */
  int64_t rtt_var{-1};

  /*
   * the total number of packets retransmitted during the connection lifetime.
   */
  int64_t rtx{-1};

  /*
   * the number of packets retransmitted due to timeout
   */
  int64_t rtx_tm{-1};

  /*
   * retransmission timeout (usec)
   */
  int64_t rto{-1};

  /*
   * The congestion window size in MSS
   */
  int64_t cwnd{-1};

  /*
   * The congestion window size in bytes
   */
  int64_t cwndBytes{-1};

  /*
   * The receiver flow control window for the entire connection in bytes.
   */
  int64_t recvwnd{-1};

  /*
   * MSS
   */
  int64_t mss{-1};

  /*
   * slow start threshold
   */
  int64_t ssthresh{-1};

  /*
   * Congestion avoidance algorithm
   */
  std::string caAlgo;

  /*
   * Socket max pacing rate
   */
  int32_t maxPacingRate{-1};

#ifdef __APPLE__
  typedef tcp_connection_info tcp_info;
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
  /*
   * TCP information as fetched from getsockopt(2)
   */
  tcp_info tcpinfo{};
#endif // defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

  /*
   * time for setting the connection, from the moment in was accepted until it
   * is established.
   */
  std::chrono::milliseconds setupTime{0};

  /*
   * NOTE: Avoid using any fields starting with "ssl" for anything other than
   * logging, as those field may not be populated for all security protocols.
   */

  /*
   * time for setting up the SSL connection or SSL handshake
   */
  std::chrono::milliseconds sslSetupTime{0};

  /*
   * The name of the SSL ciphersuite used by the transaction's
   * transport.  Returns null if the transport is not SSL.
   */
  std::shared_ptr<std::string> sslCipher{nullptr};

  /*
   * The SSL server name used by the transaction's
   * transport.  Returns null if the transport is not SSL.
   */
  std::shared_ptr<std::string> sslServerName{nullptr};

  /*
   * list of ciphers sent by the client
   */
  std::shared_ptr<std::string> sslClientCiphers{nullptr};

  /*
   * client ciphers as a series of 4-byte hex strings (e.g., 'cc14')
   */
  std::shared_ptr<std::string> sslClientCiphersHex{nullptr};

  /*
   * list of compression methods sent by the client
   */
  std::shared_ptr<std::string> sslClientComprMethods{nullptr};

  /*
   * list of TLS extensions sent by the client
   */
  std::shared_ptr<std::string> sslClientExts{nullptr};

  /*
   * list of hash and signature algorithms sent by the client
   */
  std::shared_ptr<std::string> sslClientSigAlgs{nullptr};

  /*
   * list of supported versions sent by client in supported versions extension
   */
  std::shared_ptr<std::string> sslClientSupportedVersions{nullptr};

  /*
   * hash of all the SSL parameters sent by the client
   */
  std::shared_ptr<std::string> sslSignature{nullptr};

  /*
   * list of ciphers supported by the server
   */
  std::shared_ptr<std::string> sslServerCiphers{nullptr};

  /*
   * guessed "(os) (browser)" based on SSL Signature
   */
  std::shared_ptr<std::string> guessedUserAgent{nullptr};

  /**
   * The application protocol running on the transport (h2, etc.)
   */
  std::shared_ptr<std::string> appProtocol{nullptr};

  /**
   * list of next protocols from client hello packet
   */
  std::shared_ptr<std::vector<std::string>> clientAlpns{nullptr};

  /*
   * total number of bytes sent over the connection
   */
  int64_t totalBytes{0};

  /**
   * the address of the remote side. If this is associated with a client socket,
   * it is a server side address. Otherwise, it is a client side address.
   */
  std::shared_ptr<folly::SocketAddress> remoteAddr;

  /**
   * the address of the local side. If the TransportInfo is associated with the
   * downstream transport in a proxy server, this is an VIP address.
   */
  std::shared_ptr<folly::SocketAddress> localAddr;

  /**
   * If the client passed through one of our L4 proxies (using PROXY Protocol),
   * then this will contain the IP address of the proxy host.
   */
  std::shared_ptr<folly::SocketAddress> clientAddrOriginal;

  /*
   * value of errno in case of getsockopt() error
   */
  int tcpinfoErrno{0};

  /*
   * bytes read & written during SSL Setup
   */
  uint32_t sslSetupBytesWritten{0};
  uint32_t sslSetupBytesRead{0};

  /**
   * SSL error detail
   */
  std::string sslError;

  /*
   * The SSL version used by the transaction's transport, in
   * OpenSSL's format: 4 bits for the major version, followed by 4 bits
   * for the minor version.  Returns zero for non-SSL.
   */
  uint16_t sslVersion{0};

  /*
   * The signature algorithm used in the certificate.
   */
  std::shared_ptr<std::string> sslCertSigAlgName{nullptr};

  /*
   * The SSL certificate size.
   */
  uint16_t sslCertSize{0};

  /*
   * The SSL mode for the transaction's transport: new session,
   * resumed session, or neither (non-SSL).
   */
  SSLResumeEnum sslResume{SSLResumeEnum::NA};

  /*
   * time used to get a usable connection.
   */
  int32_t connectLatency{-1};

  /*
   * true if the tcpinfo was successfully read from the kernel
   */
  bool validTcpinfo{false};

  /*
   * true if the connection is SSL, false otherwise
   */
  bool secure{false};

  /**
   * What is providing the security.
   */
  std::string securityType;

  /*
   * Additional protocol info.
   */
  std::shared_ptr<ProtocolInfo> protocolInfo{nullptr};

  /*
   * Hash of some of TCP/IP headers fields values, sometimes concatenated with
   * raw signature (that gives the hash).
   */
  std::shared_ptr<std::string> tcpSignature{nullptr};

  /*
   * Hash of some of TCP/IP headers fields (especially tcp_options) values,
   * sometimes concatenated with raw fingerprint (that gives the hash).
   */
  std::shared_ptr<std::string> tcpFingerprint{nullptr};

  /*
   * Whether or not TCP fast open succeded on this connection. Failure can occur
   * due to several reasons, including cookies not matching or TFO not being
   * advertised by the client.
   */
  bool tfoSucceded{false};

  /*
   * Stores the TokenBindingKeyParameter that was negotiatied during the
   * handshake. Needed for the validation step of Token Binding.
   */
  folly::Optional<uint8_t> negotiatedTokenBindingKeyParameters;

  /*
   * get the RTT value in milliseconds
   */
  std::chrono::milliseconds getRttMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(rtt);
  }

  /*
   * initialize the fields related with tcp_info
   */
  bool initWithSocket(const folly::AsyncSocket* sock);

#if defined(__linux__) || defined(__FreeBSD__)
  /*
   * Perform the getsockopt(2) syscall to fetch TCP congestion control algorithm
   * for a given socket.
   */
  bool readTcpCongestionControl(const folly::AsyncSocket* sock);

  /*
   * Perform the getsockopt(2) syscall to fetch max pacing rate for a given
   * socket.
   */
  bool readMaxPacingRate(const folly::AsyncSocket* sock);
#endif // defined(__linux__) || defined(__FreeBSD__)

  /*
   * Get the kernel's estimate of round-trip time (RTT) to the transport's peer
   * in microseconds. Returns -1 on error.
   */
  static int64_t readRTT(const folly::AsyncSocket* sock);

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
  /*
   * perform the getsockopt(2) syscall to fetch TCP info for a given socket
   */
  static bool readTcpInfo(tcp_info* tcpinfo, const folly::AsyncSocket* sock);
#endif
};

} // namespace wangle
