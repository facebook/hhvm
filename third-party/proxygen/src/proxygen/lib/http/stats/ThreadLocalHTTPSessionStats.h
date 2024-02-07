/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <proxygen/lib/stats/StatsWrapper.h>
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
  void recordControlMsgRateLimited() noexcept override;
  void recordHeadersInInterval(int64_t quantity) noexcept override;
  void recordHeadersRateLimited() noexcept override;
  void recordResetsInInterval(int64_t quantity) noexcept override;
  void recordResetsRateLimited() noexcept override;

  StatsWrapper::TLCounter txnsOpen;
  StatsWrapper::TLCounter pendingBufferedReadBytes;
  StatsWrapper::TLCounter pendingBufferedWriteBytes;
  StatsWrapper::TLTimeseries txnsOpened;
  StatsWrapper::TLTimeseries txnsFromSessionReuse;
  StatsWrapper::TLTimeseries txnsTransactionStalled;
  StatsWrapper::TLTimeseries txnsSessionStalled;
  StatsWrapper::TLTimeseries egressContentLengthMismatches;
  StatsWrapper::TLTimeseries sessionPeriodicPingProbeTimeout;
  // Time to Last Byte Ack (TTLBA)
  StatsWrapper::TLTimeseries presendIoSplit;
  StatsWrapper::TLTimeseries presendExceedLimit;
  StatsWrapper::TLTimeseries ttlbaTracked;
  StatsWrapper::TLTimeseries ttlbaReceived;
  StatsWrapper::TLTimeseries ttlbaTimeout;
  StatsWrapper::TLTimeseries ttlbaNotFound;
  StatsWrapper::TLTimeseries ttlbaExceedLimit;
  StatsWrapper::TLTimeseries ttbtxTracked;
  StatsWrapper::TLTimeseries ttbtxReceived;
  StatsWrapper::TLTimeseries ttbtxTimeout;
  StatsWrapper::TLTimeseries ttbtxNotFound;
  StatsWrapper::TLTimeseries ttbtxExceedLimit;
  StatsWrapper::TLTimeseries ctrlMsgsRateLimited;
  StatsWrapper::TLTimeseries headersRateLimited;
  StatsWrapper::TLTimeseries resetsRateLimited;
  StatsWrapper::TLHistogram txnsPerSession;
  StatsWrapper::TLHistogram sessionIdleTime;
  StatsWrapper::TLHistogram ctrlMsgsInInterval;
  StatsWrapper::TLHistogram headersInInterval;
  StatsWrapper::TLHistogram resetsInInterval;
};

} // namespace proxygen
