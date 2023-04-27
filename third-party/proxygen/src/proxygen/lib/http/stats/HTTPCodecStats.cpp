/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/HTTPCodecStats.h>

using facebook::fb303::RATE;
using facebook::fb303::SUM;

namespace {
static std::array<const char*, 14> kErrorStrings{
    "Ok",
    "Protocol_Error",
    "Internal_Error",
    "Flow_Control_Error",
    "Settings_Timeout",
    "Stream_Closed",
    "Frame_Size_Error",
    "Refused_Stream",
    "Cancel",
    "Compression_Error",
    "Connect_Error",
    "Enhance_Your_Calm",
    "Inadequate_Security",
    "Http_1_1_Required",
};

} // namespace

namespace proxygen {

// TLHTTPCodecStats

TLHTTPCodecStats::TLHTTPCodecStats(const std::string& prefix)
    : openConn_(prefix + "_conn.sum"),
      ingressSynStream_(prefix + "_ingress_syn_stream", SUM, RATE),
      ingressSynReply_(prefix + "_ingress_syn_reply", SUM, RATE),
      ingressPushPromise_(prefix + "_ingress_push_promise", SUM, RATE),
      ingressExStream_(prefix + "_ingress_ex_stream", SUM, RATE),
      ingressData_(prefix + "_ingress_data", SUM, RATE),
      ingressRst_(prefix + "_ingress_rst", SUM, RATE),
      ingressSettings_(prefix + "_ingress_settings", SUM, RATE),
      ingressPingRequest_(prefix + "_ingress_ping_request", SUM, RATE),
      ingressPingReply_(prefix + "_ingress_ping_reply", SUM, RATE),
      ingressGoaway_(prefix + "_ingress_goaway", SUM, RATE),
      ingressGoawayDrain_(prefix + "_ingress_goaway_drain", SUM, RATE),
      ingressWindowUpdate_(prefix + "_ingress_window_update", SUM, RATE),
      ingressPriority_(prefix + "_ingress_priority", SUM, RATE),
      egressSynStream_(prefix + "_egress_syn_stream", SUM, RATE),
      egressSynReply_(prefix + "_egress_syn_reply", SUM, RATE),
      egressPushPromise_(prefix + "_egress_push_promise", SUM, RATE),
      egressExStream_(prefix + "_egress_ex_stream", SUM, RATE),
      egressData_(prefix + "_egress_data", SUM, RATE),
      egressRst_(prefix + "_egress_rst", SUM, RATE),
      egressSettings_(prefix + "_egress_settings", SUM, RATE),
      egressPingRequest_(prefix + "_egress_ping_request", SUM, RATE),
      egressPingReply_(prefix + "_egress_ping_reply", SUM, RATE),
      egressGoaway_(prefix + "_egress_goaway", SUM, RATE),
      egressGoawayDrain_(prefix + "_egress_goaway_drain", SUM, RATE),
      egressWindowUpdate_(prefix + "_egress_window_update", SUM, RATE),
      egressPriority_(prefix + "_egress_priority", SUM, RATE) {
  ingressRstStatus_.reserve(kErrorStrings.size());
  egressRstStatus_.reserve(kErrorStrings.size());
  ingressGoawayStatus_.reserve(kErrorStrings.size());
  egressGoawayStatus_.reserve(kErrorStrings.size());
  for (auto errString : kErrorStrings) {
    ingressRstStatus_.emplace_back(prefix + "_ingress_rst_" + errString, SUM);
    egressRstStatus_.emplace_back(prefix + "_egress_rst_" + errString, SUM);
    ingressGoawayStatus_.emplace_back(prefix + "_ingress_goaway_" + errString,
                                      SUM);
    egressGoawayStatus_.emplace_back(prefix + "_egress_goaway_" + errString,
                                     SUM);
  }
}

void TLHTTPCodecStats::incrementParallelConn(int64_t amount) {
  openConn_.incrementValue(amount);
}
void TLHTTPCodecStats::recordIngressSynStream() {
  ingressSynStream_.add(1);
}
void TLHTTPCodecStats::recordIngressSynReply() {
  ingressSynReply_.add(1);
}
void TLHTTPCodecStats::recordIngressPushPromise() {
  ingressPushPromise_.add(1);
}
void TLHTTPCodecStats::recordIngressExStream() {
  ingressExStream_.add(1);
}
void TLHTTPCodecStats::recordIngressData() {
  ingressData_.add(1);
}
void TLHTTPCodecStats::recordIngressRst(ErrorCode statusCode) {
  ingressRst_.add(1);
  uint32_t index = uint32_t(statusCode);
  if (index >= kErrorStrings.size()) {
    LOG(ERROR) << "Unknown ingress reset status code=" << index;
    index = (uint32_t)ErrorCode::PROTOCOL_ERROR;
  }
  ingressRstStatus_[index].add(1);
}
void TLHTTPCodecStats::recordIngressSettings() {
  ingressSettings_.add(1);
}
void TLHTTPCodecStats::recordIngressPingRequest() {
  ingressPingRequest_.add(1);
}
void TLHTTPCodecStats::recordIngressPingReply() {
  ingressPingReply_.add(1);
}
void TLHTTPCodecStats::recordIngressGoaway(ErrorCode statusCode) {
  ingressGoaway_.add(1);
  uint32_t index = uint32_t(statusCode);
  if (index >= kErrorStrings.size()) {
    LOG(ERROR) << "Unknown ingress goaway status code=" << index;
    index = (uint32_t)ErrorCode::PROTOCOL_ERROR;
  }
  ingressGoawayStatus_[index].add(1);
}
void TLHTTPCodecStats::recordIngressGoawayDrain() {
  ingressGoawayDrain_.add(1);
}
void TLHTTPCodecStats::recordIngressWindowUpdate() {
  ingressWindowUpdate_.add(1);
}
void TLHTTPCodecStats::recordIngressPriority() {
  ingressPriority_.add(1);
}
void TLHTTPCodecStats::recordEgressSynStream() {
  egressSynStream_.add(1);
}
void TLHTTPCodecStats::recordEgressSynReply() {
  egressSynReply_.add(1);
}
void TLHTTPCodecStats::recordEgressPushPromise() {
  egressPushPromise_.add(1);
}
void TLHTTPCodecStats::recordEgressExStream() {
  egressExStream_.add(1);
}
void TLHTTPCodecStats::recordEgressData() {
  egressData_.add(1);
}
void TLHTTPCodecStats::recordEgressRst(ErrorCode statusCode) {
  egressRst_.add(1);
  uint32_t index = uint32_t(statusCode);
  if (index >= kErrorStrings.size()) {
    LOG(ERROR) << "Unknown egress reset status code=" << index;
    index = (uint32_t)ErrorCode::PROTOCOL_ERROR;
  }
  egressRstStatus_[index].add(1);
}
void TLHTTPCodecStats::recordEgressSettings() {
  egressSettings_.add(1);
}
void TLHTTPCodecStats::recordEgressPingRequest() {
  egressPingRequest_.add(1);
}
void TLHTTPCodecStats::recordEgressPingReply() {
  egressPingReply_.add(1);
}
void TLHTTPCodecStats::recordEgressGoaway(ErrorCode statusCode) {
  egressGoaway_.add(1);
  uint32_t index = uint32_t(statusCode);
  if (index >= kErrorStrings.size()) {
    LOG(ERROR) << "Unknown egress goaway status code=" << index;
    index = (uint32_t)ErrorCode::PROTOCOL_ERROR;
  }
  egressGoawayStatus_[index].add(1);
}
void TLHTTPCodecStats::recordEgressGoawayDrain() {
  egressGoawayDrain_.add(1);
}
void TLHTTPCodecStats::recordEgressWindowUpdate() {
  egressWindowUpdate_.add(1);
}
void TLHTTPCodecStats::recordEgressPriority() {
  egressPriority_.add(1);
}

} // namespace proxygen
