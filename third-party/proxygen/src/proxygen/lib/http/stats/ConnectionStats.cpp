/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/ConnectionStats.h>

using namespace facebook::fb303;

namespace proxygen {

TLConnectionStats::TLConnectionStats(const std::string& prefix)
    : req_(prefix + "_req", SUM, RATE),
      resp_(prefix + "_resp", SUM, RATE),
      egressBytes_(prefix + "_egress_bytes", SUM, RATE),
      ingressBytes_(prefix + "_ingress_bytes", SUM, RATE),
      egressBodyBytes_(prefix + "_egress_body_bytes", SUM, RATE),
      ingressBodyBytes_(prefix + "_ingress_body_bytes", SUM, RATE),
      responseCodes_(prefix + "_"),
      totalDuration_(prefix + "_conn_duration",
                     100,
                     0,
                     5000,
                     facebook::fb303::AVG,
                     50,
                     95,
                     99),
      currConns_(prefix + "_conn"),
      newConns_(prefix + "_new_conn", SUM, RATE) {
}

void TLConnectionStats::recordConnectionOpen() {
  currConns_.incrementValue(1);
  newConns_.add(1);
}

void TLConnectionStats::recordConnectionClose() {
  currConns_.incrementValue(-1);
}

void TLConnectionStats::recordRequest() {
  req_.add(1);
}

void TLConnectionStats::recordResponse(folly::Optional<uint16_t> responseCode) {
  resp_.add(1);
  if (responseCode.has_value()) {
    responseCodes_.addStatus(responseCode.value());
  }
}

void TLConnectionStats::recordDuration(size_t duration) {
  totalDuration_.add(duration);
}

void TLConnectionStats::addEgressBytes(size_t bytes) {
  egressBytes_.add(bytes);
}

void TLConnectionStats::addIngressBytes(size_t bytes) {
  ingressBytes_.add(bytes);
}

void TLConnectionStats::addEgressBodyBytes(size_t bytes) {
  egressBodyBytes_.add(bytes);
}

void TLConnectionStats::addIngressBodyBytes(size_t bytes) {
  ingressBodyBytes_.add(bytes);
}

} // namespace proxygen
