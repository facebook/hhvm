/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/ErrorCode.h>
#include <proxygen/lib/stats/BaseStats.h>
#include <string>

namespace proxygen {

/**
 * A stats interface for tracking HTTP events, primarily those on parallel
 * protocols.
 */
class HTTPCodecStats {
 public:
  virtual ~HTTPCodecStats() = default;

  virtual void incrementParallelConn(int64_t amount) = 0;

  virtual void recordIngressSynStream() = 0;
  virtual void recordIngressSynReply() = 0;
  virtual void recordIngressPushPromise() = 0;
  virtual void recordIngressExStream() = 0;
  virtual void recordIngressData() = 0;
  virtual void recordIngressRst(ErrorCode statusCode) = 0;
  virtual void recordIngressSettings() = 0;
  virtual void recordIngressPingRequest() = 0;
  virtual void recordIngressPingReply() = 0;
  virtual void recordIngressGoaway(ErrorCode statusCode) = 0;
  virtual void recordIngressGoawayDrain() = 0;
  virtual void recordIngressWindowUpdate() = 0;
  virtual void recordIngressPriority() = 0;

  virtual void recordEgressSynStream() = 0;
  virtual void recordEgressSynReply() = 0;
  virtual void recordEgressPushPromise() = 0;
  virtual void recordEgressExStream() = 0;
  virtual void recordEgressData() = 0;
  virtual void recordEgressRst(ErrorCode statusCode) = 0;
  virtual void recordEgressSettings() = 0;
  virtual void recordEgressPingRequest() = 0;
  virtual void recordEgressPingReply() = 0;
  virtual void recordEgressGoaway(ErrorCode statusCode) = 0;
  virtual void recordEgressGoawayDrain() = 0;
  virtual void recordEgressWindowUpdate() = 0;
  virtual void recordEgressPriority() = 0;
};

/**
 * A TCSD implementation of HTTPCodecStats
 */
class TLHTTPCodecStats : public HTTPCodecStats {
 public:
  explicit TLHTTPCodecStats(const std::string& prefix);
  explicit TLHTTPCodecStats(const TLHTTPCodecStats&) = delete;
  TLHTTPCodecStats& operator=(const TLHTTPCodecStats&) = delete;
  virtual ~TLHTTPCodecStats() override = default;

  void incrementParallelConn(int64_t amount) override;

  void recordIngressSynStream() override;
  void recordIngressSynReply() override;
  void recordIngressPushPromise() override;
  void recordIngressExStream() override;
  void recordIngressData() override;
  void recordIngressRst(ErrorCode statusCode) override;
  void recordIngressSettings() override;
  void recordIngressPingRequest() override;
  void recordIngressPingReply() override;
  void recordIngressGoaway(ErrorCode statusCode) override;
  void recordIngressGoawayDrain() override;
  void recordIngressWindowUpdate() override;
  void recordIngressPriority() override;

  void recordEgressSynStream() override;
  void recordEgressSynReply() override;
  void recordEgressPushPromise() override;
  void recordEgressExStream() override;
  void recordEgressData() override;
  void recordEgressRst(ErrorCode statusCode) override;
  void recordEgressSettings() override;
  void recordEgressPingRequest() override;
  void recordEgressPingReply() override;
  void recordEgressGoaway(ErrorCode statusCode) override;
  void recordEgressGoawayDrain() override;
  void recordEgressWindowUpdate() override;
  void recordEgressPriority() override;

 private:
  BaseStats::TLCounter openConn_;
  BaseStats::TLTimeseries ingressSynStream_;
  BaseStats::TLTimeseries ingressSynReply_;
  BaseStats::TLTimeseries ingressPushPromise_;
  BaseStats::TLTimeseries ingressExStream_;
  BaseStats::TLTimeseries ingressData_;
  BaseStats::TLTimeseries ingressRst_;
  std::vector<BaseStats::TLTimeseries> ingressRstStatus_;
  BaseStats::TLTimeseries ingressSettings_;
  BaseStats::TLTimeseries ingressPingRequest_;
  BaseStats::TLTimeseries ingressPingReply_;
  BaseStats::TLTimeseries ingressGoaway_;
  BaseStats::TLTimeseries ingressGoawayDrain_;
  std::vector<BaseStats::TLTimeseries> ingressGoawayStatus_;
  BaseStats::TLTimeseries ingressWindowUpdate_;
  BaseStats::TLTimeseries ingressPriority_;

  BaseStats::TLTimeseries egressSynStream_;
  BaseStats::TLTimeseries egressSynReply_;
  BaseStats::TLTimeseries egressPushPromise_;
  BaseStats::TLTimeseries egressExStream_;
  BaseStats::TLTimeseries egressData_;
  BaseStats::TLTimeseries egressRst_;
  std::vector<BaseStats::TLTimeseries> egressRstStatus_;
  BaseStats::TLTimeseries egressSettings_;
  BaseStats::TLTimeseries egressPingRequest_;
  BaseStats::TLTimeseries egressPingReply_;
  BaseStats::TLTimeseries egressGoaway_;
  BaseStats::TLTimeseries egressGoawayDrain_;
  std::vector<BaseStats::TLTimeseries> egressGoawayStatus_;
  BaseStats::TLTimeseries egressWindowUpdate_;
  BaseStats::TLTimeseries egressPriority_;
};

} // namespace proxygen
