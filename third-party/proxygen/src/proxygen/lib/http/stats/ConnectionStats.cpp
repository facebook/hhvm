/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/stats/ConnectionStats.h"

using namespace facebook::fb303;

namespace proxygen {

MinimalConnectionStats::MinimalConnectionStats(const std::string& prefix,
                                               uint8_t verbosity) {
  req_.emplace(prefix + "_req", SUM, RATE); // RATE used
  egressBytes_.emplace(prefix + "_egress_bytes", SUM);
  upstreamLoadShed_.emplace(prefix + "_req_was_loadshed_by_upstream", SUM);

  if (verbosity > 8) {
    resp_.emplace(prefix + "_resp", SUM);
    ingressBytes_.emplace(prefix + "_ingress_bytes", SUM);
    egressBodyBytes_.emplace(
        prefix + "_egress_body_bytes",
        SUM,
        RATE); // RATE is being used for body throughout the code base
               // https://www.internalfb.com/code/search?q=repo%3Aall%20gress_body_bytes.rate.60
    ingressBodyBytes_.emplace(prefix + "_ingress_body_bytes", SUM, RATE);

    totalDuration_.emplace(prefix + "_conn_duration",
                           facebook::fb303::ExportTypeConsts::kAvg,
                           facebook::fb303::QuantileConsts::kP50_P95_P99);
    currConns_.emplace(prefix + "_conn");
    newConns_.emplace(prefix + "_new_conn", SUM);
    currTcpConns_.emplace(prefix + "_tcp_conn");
    newTcpConns_.emplace(prefix + "_new_tcp_conn", SUM);
  }
}

void MinimalConnectionStats::recordConnectionOpen() {
  BaseStats::incrementOptionalCounter(currConns_, 1);
  BaseStats::addToOptionalStat(newConns_, 1);
}

void MinimalConnectionStats::recordTcpConnectionOpen() {
  BaseStats::incrementOptionalCounter(currTcpConns_, 1);
  BaseStats::addToOptionalStat(newTcpConns_, 1);
}

void MinimalConnectionStats::recordConnectionClose() {
  BaseStats::incrementOptionalCounter(currConns_, -1);
}

void MinimalConnectionStats::recordRequest() {
  BaseStats::addToOptionalStat(req_, 1);
}

void MinimalConnectionStats::recordResponse(
    folly::Optional<uint16_t> responseCode, bool hasRetryAfterHeader) {
  BaseStats::addToOptionalStat(resp_, 1);
  if (responseCode.has_value() && upstreamLoadShed_ && hasRetryAfterHeader &&
      responseCode.value() == 503) {
    upstreamLoadShed_->add(1);
  }
}

void MinimalConnectionStats::recordDuration(size_t duration) {
  BaseStats::addValueToOptionalStat(totalDuration_, duration);
}

void MinimalConnectionStats::addEgressBytes(size_t bytes) {
  BaseStats::addToOptionalStat(egressBytes_, bytes);
}

void MinimalConnectionStats::addIngressBytes(size_t bytes) {
  BaseStats::addToOptionalStat(ingressBytes_, bytes);
}

void MinimalConnectionStats::addEgressBodyBytes(size_t bytes) {
  BaseStats::addToOptionalStat(egressBodyBytes_, bytes);
}

void MinimalConnectionStats::addIngressBodyBytes(size_t bytes) {
  BaseStats::addToOptionalStat(ingressBodyBytes_, bytes);
}

TLConnectionStats::TLConnectionStats(const std::string& prefix,
                                     uint8_t verbosity)
    : MinimalConnectionStats(prefix, verbosity) {
  responseCodes_.emplace(prefix + "_", verbosity);
}

void TLConnectionStats::recordResponse(folly::Optional<uint16_t> responseCode,
                                       bool hasRetryAfterHeader) {
  MinimalConnectionStats::recordResponse(responseCode, hasRetryAfterHeader);
  if (responseCodes_ && responseCode.has_value()) {
    responseCodes_->addStatus(responseCode.value());
  }
}

} // namespace proxygen
