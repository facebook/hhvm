/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/ThreadLocalHTTPSessionStats.h>

namespace proxygen {

TLHTTPSessionStats::TLHTTPSessionStats(const std::string& prefix)
    : txnsOpen(prefix + "_transactions_open"),
      pendingBufferedReadBytes(prefix + "_pending_buffered_read_bytes"),
      pendingBufferedWriteBytes(prefix + "_pending_buffered_write_bytes"),
      txnsOpened(prefix + "_txn_opened", facebook::fb303::SUM),
      txnsFromSessionReuse(prefix + "_txn_session_reuse", facebook::fb303::SUM),
      txnsTransactionStalled(prefix + "_txn_transaction_stall",
                             facebook::fb303::SUM),
      txnsSessionStalled(prefix + "_txn_session_stall", facebook::fb303::SUM),
      egressContentLengthMismatches(
          prefix + "_egress_content_length_mismatches", facebook::fb303::SUM),
      sessionPeriodicPingProbeTimeout(
          prefix + "_session_periodic_ping_probe_timeout",
          facebook::fb303::SUM),
      presendIoSplit(prefix + "_presend_io_split", facebook::fb303::SUM),
      presendExceedLimit(prefix + "_presend_exceed_limit",
                         facebook::fb303::SUM),
      ttlbaTracked(prefix + "_ttlba_tracked", facebook::fb303::SUM),
      ttlbaReceived(prefix + "_ttlba_received", facebook::fb303::SUM),
      ttlbaTimeout(prefix + "_ttlba_timeout", facebook::fb303::SUM),
      ttlbaNotFound(prefix + "_ttlba_not_found", facebook::fb303::SUM),
      ttlbaExceedLimit(prefix + "_ttlba_exceed_limit", facebook::fb303::SUM),
      ttbtxTracked(prefix + "_ttbtx_tracked", facebook::fb303::SUM),
      ttbtxReceived(prefix + "_ttbtx_received", facebook::fb303::SUM),
      ttbtxTimeout(prefix + "_ttbtx_timeout", facebook::fb303::SUM),
      ttbtxNotFound(prefix + "_ttbtx_not_found", facebook::fb303::SUM),
      ttbtxExceedLimit(prefix + "_ttbtx_exceed_limit", facebook::fb303::SUM),
      ctrlMsgsRateLimited(prefix + "_ctrl_msgs_rate_limited",
                          facebook::fb303::SUM),
      txnsPerSession(prefix + "_txn_per_session",
                     1,
                     0,
                     999,
                     facebook::fb303::AVG,
                     50,
                     95,
                     99),
      sessionIdleTime(prefix + "_session_idle_time",
                      1,
                      0,
                      150,
                      facebook::fb303::AVG,
                      50,
                      75,
                      95,
                      99),
      ctrlMsgsInInterval(
          prefix + "_ctrl_msgs_in_interval",
          1 /* bucketWidth */,
          0 /* min */,
          500 /* max, keep in sync with kDefaultMaxControlMsgsPerInterval */,
          facebook::fb303::AVG,
          50,
          99,
          100) {
}

void TLHTTPSessionStats::recordTransactionOpened() noexcept {
  txnsOpen.incrementValue(1);
  txnsOpened.add(1);
}

void TLHTTPSessionStats::recordTransactionClosed() noexcept {
  txnsOpen.incrementValue(-1);
}

void TLHTTPSessionStats::recordSessionReused() noexcept {
  txnsFromSessionReuse.add(1);
}

void TLHTTPSessionStats::recordPresendIOSplit() noexcept {
  presendIoSplit.add(1);
}

void TLHTTPSessionStats::recordPresendExceedLimit() noexcept {
  presendExceedLimit.add(1);
}

void TLHTTPSessionStats::recordTTLBAExceedLimit() noexcept {
  ttlbaExceedLimit.add(1);
}

void TLHTTPSessionStats::recordTTLBANotFound() noexcept {
  ttlbaNotFound.add(1);
}

void TLHTTPSessionStats::recordTTLBAReceived() noexcept {
  ttlbaReceived.add(1);
}

void TLHTTPSessionStats::recordTTLBATimeout() noexcept {
  ttlbaTimeout.add(1);
}

void TLHTTPSessionStats::recordTTLBATracked() noexcept {
  ttlbaTracked.add(1);
}

void TLHTTPSessionStats::recordTTBTXExceedLimit() noexcept {
  ttbtxExceedLimit.add(1);
}

void TLHTTPSessionStats::recordTTBTXReceived() noexcept {
  ttbtxReceived.add(1);
}

void TLHTTPSessionStats::recordTTBTXTimeout() noexcept {
  ttbtxTimeout.add(1);
}

void TLHTTPSessionStats::recordTTBTXNotFound() noexcept {
  ttbtxNotFound.add(1);
}

void TLHTTPSessionStats::recordTTBTXTracked() noexcept {
  ttbtxTracked.add(1);
}

void TLHTTPSessionStats::recordTransactionsServed(uint64_t num) noexcept {
  txnsPerSession.add(num);
}

void TLHTTPSessionStats::recordSessionIdleTime(
    std::chrono::seconds idleTime) noexcept {
  sessionIdleTime.add(idleTime.count());
}

void TLHTTPSessionStats::recordTransactionStalled() noexcept {
  txnsTransactionStalled.add(1);
}

void TLHTTPSessionStats::recordSessionStalled() noexcept {
  txnsSessionStalled.add(1);
}

void TLHTTPSessionStats::recordEgressContentLengthMismatches() noexcept {
  egressContentLengthMismatches.add(1);
}

void TLHTTPSessionStats::recordSessionPeriodicPingProbeTimeout() noexcept {
  sessionPeriodicPingProbeTimeout.add(1);
}

void TLHTTPSessionStats::recordPendingBufferedReadBytes(
    int64_t amount) noexcept {
  pendingBufferedReadBytes.incrementValue(amount);
}

void TLHTTPSessionStats::recordPendingBufferedWriteBytes(
    int64_t amount) noexcept {
  pendingBufferedWriteBytes.incrementValue(amount);
}

void TLHTTPSessionStats::recordControlMsgsInInterval(
    int64_t quantity) noexcept {
  ctrlMsgsInInterval.add(quantity);
}

void TLHTTPSessionStats::recordControlMsgRateLimited() noexcept {
  ctrlMsgsRateLimited.add(1);
}

} // namespace proxygen
