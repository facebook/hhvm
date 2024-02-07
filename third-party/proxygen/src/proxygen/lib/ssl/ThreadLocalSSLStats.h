/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Types.h>
#include <folly/ThreadLocal.h>
#include <proxygen/lib/stats/StatsWrapper.h>
#include <string>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/SSLUtil.h>

namespace proxygen {

class ProxygenSSLStats : public wangle::SSLStats {
 public:
  virtual ~ProxygenSSLStats() override = default;

  virtual void recordReplayCacheRequestComplete(uint64_t duration,
                                                bool cacheHit) noexcept = 0;
  virtual void recordReplayCacheRequestError() noexcept = 0;

  virtual void recordNewSSLHandshakeShed() = 0;

  virtual void recordPskType(folly::Optional<fizz::PskType> pskType) = 0;
  /**
   * Keep track of SSL handshake errors
   */
  virtual void recordSSLHandshake(bool success) = 0;

  virtual void recordFizzHandshake(bool success) = 0;

  virtual void recordSSLConnectionReuse() = 0;

  // Protocol level errors only
  virtual void recordFizzHandshakeProtocolError() = 0;

  virtual void recordTFOSuccess() = 0;

  virtual void recordServerCertExpiring() noexcept = 0;

  virtual void recordServerCertExpiringCritical() noexcept = 0;

  // TLS usage
  virtual void recordTLSVersion(fizz::ProtocolVersion tlsVersion) noexcept = 0;

  virtual void recordInsecureConnection() noexcept = 0;
};

class TLSSLStats : public ProxygenSSLStats {
 public:
  TLSSLStats(const std::string& prefix);
  virtual ~TLSSLStats() override = default;

  // downstream
  void recordSSLAcceptLatency(int64_t latency) noexcept override;
  void recordTLSTicket(bool ticketNew, bool ticketHit) noexcept override;
  void recordSSLSession(bool sessionNew,
                        bool sessionHit,
                        bool foreign) noexcept override;
  void recordSSLSessionRemove() noexcept override;
  void recordSSLSessionFree(uint32_t freed) noexcept override;
  void recordSSLSessionSetError(uint32_t err) noexcept override;
  void recordSSLSessionGetError(uint32_t err) noexcept override;
  void recordClientRenegotiation() noexcept override;
  void recordSSLClientCertificateMismatch() noexcept override;
  void recordTLSTicketRotation(bool valid) noexcept override;

  // upstream
  void recordSSLUpstreamConnection(bool handshake) noexcept override;
  void recordSSLUpstreamConnectionError(bool verifyError) noexcept override;

  // ProxygenSSLStats interface.

  void recordReplayCacheRequestComplete(uint64_t duration,
                                        bool cacheHit) noexcept override;
  void recordReplayCacheRequestError() noexcept override;

  void recordNewSSLHandshakeShed() override;

  void recordPskType(folly::Optional<fizz::PskType> pskType) override;
  /**
   * Keep track of SSL handshake errors
   */
  void recordSSLHandshake(bool success) override;

  void recordFizzHandshake(bool success) override;

  void recordSSLConnectionReuse() override;

  void recordFizzHandshakeProtocolError() override;

  void recordTFOSuccess() override;

  void recordServerCertExpiring() noexcept override;

  void recordServerCertExpiringCritical() noexcept override;

  void recordTLSVersion(fizz::ProtocolVersion tlsVersion) noexcept override;

  void recordInsecureConnection() noexcept override;

 private:
  // Forbidden copy constructor and assignment operator
  TLSSLStats(TLSSLStats const&) = delete;
  TLSSLStats& operator=(TLSSLStats const&) = delete;

  // downstream
  StatsWrapper::TLHistogram sslAcceptLatency_;
  StatsWrapper::TLTimeseries sslAcceptLatencyTS_;
  StatsWrapper::TLTimeseries tlsTicketNew_;
  StatsWrapper::TLTimeseries tlsTicketHit_;
  StatsWrapper::TLTimeseries tlsTicketMiss_;
  StatsWrapper::TLTimeseries sslSessionNew_;
  StatsWrapper::TLTimeseries sslSessionHit_;
  StatsWrapper::TLTimeseries sslSessionForeignHit_;
  StatsWrapper::TLTimeseries sslSessionTotalMiss_;
  StatsWrapper::TLTimeseries sslSessionRemove_;
  StatsWrapper::TLTimeseries sslSessionFree_;
  StatsWrapper::TLTimeseries sslSessionSetError_;
  StatsWrapper::TLTimeseries sslSessionGetError_;
  StatsWrapper::TLTimeseries sslClientRenegotiations_;
  StatsWrapper::TLTimeseries clientCertMismatch_;
  StatsWrapper::TLTimeseries tlsTicketInvalidRotation_;

  // upstream
  StatsWrapper::TLTimeseries sslUpstreamHandshakes_;
  StatsWrapper::TLTimeseries sslUpstreamResumes_;
  StatsWrapper::TLTimeseries sslUpstreamErrors_;
  StatsWrapper::TLTimeseries sslUpstreamVerifyErrors_;
  // replay_cache service
  StatsWrapper::TLTimeseries replayCacheNumRequests_;
  StatsWrapper::TLTimeseries replayCacheNumHits_;
  StatsWrapper::TLTimeseries replayCacheNumErrors_;
  StatsWrapper::TLHistogram replayCacheDuration_;

  // ssl handshake metrics
  StatsWrapper::TLTimeseries newSSLHandshakeShed_;
  StatsWrapper::TLTimeseries sslHandshakeErrors_;
  StatsWrapper::TLTimeseries sslHandshakeSuccesses_;
  StatsWrapper::TLTimeseries sslResumptions_;
  StatsWrapper::TLTimeseries fizzHandshakeErrors_;
  StatsWrapper::TLTimeseries fizzHandshakeProtocolErrors_;
  StatsWrapper::TLTimeseries fizzHandshakeSuccesses_;
  StatsWrapper::TLTimeseries tfoSuccess_;
  StatsWrapper::TLTimeseries sslServerCertExpiring_;
  StatsWrapper::TLTimeseries sslServerCertExpiringCritical_;
  // tlsUnknown_ is to track and make sure that we do not
  // support a TLS versions that is unintented. This is
  // also used for audits.
  StatsWrapper::TLTimeseries tlsUnknown_;
  StatsWrapper::TLTimeseries tlsVersion_1_0_;
  StatsWrapper::TLTimeseries tlsVersion_1_1_;
  StatsWrapper::TLTimeseries tlsVersion_1_2_;
  StatsWrapper::TLTimeseries tlsVersion_1_3_;
  StatsWrapper::TLTimeseries tlsInsecureConnection;

  // PskTypes counters
  StatsWrapper::TLTimeseries fizzPskTypeNotSupported_;
  StatsWrapper::TLTimeseries fizzPskTypeNotAttempted_;
  StatsWrapper::TLTimeseries fizzPskTypeRejected_;
  StatsWrapper::TLTimeseries fizzPskTypeExternal_;
  StatsWrapper::TLTimeseries fizzPskTypeResumption_;
};

} // namespace proxygen
