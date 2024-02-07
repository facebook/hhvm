/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>

#include <proxygen/lib/http/stats/TLResponseCodeStats.h>

#include <proxygen/lib/stats/StatsWrapper.h>

namespace proxygen {

/**
 * Connection stats abstract interface.
 */
class ConnectionStats {
 public:
  virtual ~ConnectionStats() = default;

  virtual void recordConnectionOpen() = 0;

  virtual void recordTcpConnectionOpen() = 0;

  virtual void recordConnectionClose() = 0;

  virtual void recordRequest() = 0;

  virtual void recordResponse(
      folly::Optional<uint16_t> responseCode = folly::none) = 0;

  virtual void recordDuration(size_t duration) = 0;

  virtual void addEgressBytes(size_t bytes) = 0;

  virtual void addIngressBytes(size_t bytes) = 0;

  virtual void addEgressBodyBytes(size_t bytes) = 0;

  virtual void addIngressBodyBytes(size_t bytes) = 0;
};

/**
 * Wraps connection stat counters.  One instance should be created per
 * uniquely named counter and shared accross threads as necessary.
 */
class TLConnectionStats : public ConnectionStats {
 public:
  explicit TLConnectionStats(const std::string& prefix, uint8_t verbosity);

  void recordConnectionOpen() override;

  void recordTcpConnectionOpen() override;

  void recordConnectionClose() override;

  void recordRequest() override;

  void recordResponse(
      folly::Optional<uint16_t> responseCode = folly::none) override;

  void recordDuration(size_t duration) override;

  void addEgressBytes(size_t bytes) override;

  void addIngressBytes(size_t bytes) override;

  void addEgressBodyBytes(size_t bytes) override;

  void addIngressBodyBytes(size_t bytes) override;

 private:
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> req_;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> resp_;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> egressBytes_;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> ingressBytes_;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> egressBodyBytes_;
  std::optional<StatsWrapper::TLTimeseriesMinuteAndAllTime> ingressBodyBytes_;
  std::optional<TLResponseCodeStats> responseCodes_;
  std::optional<StatsWrapper::TLHistogram> totalDuration_;

  std::optional<StatsWrapper::TLCounter> currConns_;
  std::optional<StatsWrapper::TLTimeseries> newConns_;

  std::optional<StatsWrapper::TLCounter> currTcpConns_;
  std::optional<StatsWrapper::TLTimeseries> newTcpConns_;
};

} // namespace proxygen
