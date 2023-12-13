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

TLConnectionStats::TLConnectionStats(const std::string& prefix,
                                     uint8_t verbosity) {
  req_.emplace(prefix + "_req", SUM, RATE); // RATE used
  egressBytes_.emplace(prefix + "_egress_bytes", SUM);
  responseCodes_.emplace(prefix + "_", verbosity);

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
                           100,
                           0,
                           5000,
                           facebook::fb303::AVG,
                           50,
                           95,
                           99);
    currConns_.emplace(prefix + "_conn");
    newConns_.emplace(prefix + "_new_conn", SUM);
    currTcpConns_.emplace(prefix + "_tcp_conn");
    newTcpConns_.emplace(prefix + "_new_tcp_conn", SUM);
  }
}

void TLConnectionStats::recordConnectionOpen() {
  BaseStats::incrementOptionalCounter(currConns_, 1);
  BaseStats::addToOptionalStat(newConns_, 1);
}

void TLConnectionStats::recordTcpConnectionOpen() {
  BaseStats::incrementOptionalCounter(currTcpConns_, 1);
  BaseStats::addToOptionalStat(newTcpConns_, 1);
}

void TLConnectionStats::recordConnectionClose() {
  BaseStats::incrementOptionalCounter(currConns_, -1);
}

void TLConnectionStats::recordRequest() {
  BaseStats::addToOptionalStat(req_, 1);
}

void TLConnectionStats::recordResponse(folly::Optional<uint16_t> responseCode) {
  BaseStats::addToOptionalStat(resp_, 1);
  if (responseCodes_ && responseCode.has_value()) {
    responseCodes_->addStatus(responseCode.value());
  }
}

void TLConnectionStats::recordDuration(size_t duration) {
  BaseStats::addToOptionalStat(totalDuration_, duration);
}

void TLConnectionStats::addEgressBytes(size_t bytes) {
  BaseStats::addToOptionalStat(egressBytes_, bytes);
}

void TLConnectionStats::addIngressBytes(size_t bytes) {
  BaseStats::addToOptionalStat(ingressBytes_, bytes);
}

void TLConnectionStats::addEgressBodyBytes(size_t bytes) {
  BaseStats::addToOptionalStat(egressBodyBytes_, bytes);
}

void TLConnectionStats::addIngressBodyBytes(size_t bytes) {
  BaseStats::addToOptionalStat(ingressBodyBytes_, bytes);
}

} // namespace proxygen
