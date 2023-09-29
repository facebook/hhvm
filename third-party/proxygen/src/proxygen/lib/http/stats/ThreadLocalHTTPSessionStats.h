/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <proxygen/lib/stats/BaseStats.h>
#include <string>

namespace proxygen {

class TLHTTPSessionStats : public HTTPSessionStats {
 public:
  explicit TLHTTPSessionStats(const std::string& prefix);

  void recordTransactionOpened() noexcept override;
  void recordTransactionClosed() noexcept override;
  void recordPresendIOSplit() noexcept override;
  void recordPresendExceedLimit() noexcept override;
  void recordTTLBAExceedLimit() noexcept override;
  void recordTTLBANotFound() noexcept override;
  void recordTTLBAReceived() noexcept override;
  void recordTTLBATimeout() noexcept override;
  void recordTTLBATracked() noexcept override;
  void recordTTBTXReceived() noexcept override;
  void recordTTBTXTimeout() noexcept override;
  void recordTTBTXNotFound() noexcept override;
  void recordTTBTXTracked() noexcept override;
  void recordTTBTXExceedLimit() noexcept override;
  void recordTransactionsServed(uint64_t) noexcept override;
  void recordSessionReused() noexcept override;
  void recordSessionIdleTime(std::chrono::seconds) noexcept override;
  void recordTransactionStalled() noexcept override;
  void recordSessionStalled() noexcept override;
  void recordPendingBufferedReadBytes(int64_t amount) noexcept override;
  void recordPendingBufferedWriteBytes(int64_t amount) noexcept override;
  void recordEgressContentLengthMismatches() noexcept override;
  void recordSessionPeriodicPingProbeTimeout() noexcept override;

  void recordControlMsgsInInterval(int64_t quantity) noexcept override;

  BaseStats::TLCounter txnsOpen;
  BaseStats::TLCounter pendingBufferedReadBytes;
  BaseStats::TLCounter pendingBufferedWriteBytes;
  BaseStats::TLTimeseries txnsOpened;
  BaseStats::TLTimeseries txnsFromSessionReuse;
  BaseStats::TLTimeseries txnsTransactionStalled;
  BaseStats::TLTimeseries txnsSessionStalled;
  BaseStats::TLTimeseries egressContentLengthMismatches;
  BaseStats::TLTimeseries sessionPeriodicPingProbeTimeout;
  // Time to Last Byte Ack (TTLBA)
  BaseStats::TLTimeseries presendIoSplit;
  BaseStats::TLTimeseries presendExceedLimit;
  BaseStats::TLTimeseries ttlbaTracked;
  BaseStats::TLTimeseries ttlbaReceived;
  BaseStats::TLTimeseries ttlbaTimeout;
  BaseStats::TLTimeseries ttlbaNotFound;
  BaseStats::TLTimeseries ttlbaExceedLimit;
  BaseStats::TLTimeseries ttbtxTracked;
  BaseStats::TLTimeseries ttbtxReceived;
  BaseStats::TLTimeseries ttbtxTimeout;
  BaseStats::TLTimeseries ttbtxNotFound;
  BaseStats::TLTimeseries ttbtxExceedLimit;
  BaseStats::TLHistogram txnsPerSession;
  BaseStats::TLHistogram sessionIdleTime;
  BaseStats::TLHistogram ctrlMsgsInInterval;
};

} // namespace proxygen
