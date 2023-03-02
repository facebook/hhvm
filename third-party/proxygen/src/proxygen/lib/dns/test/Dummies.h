/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/dns/CAresResolver.h"
#include "proxygen/lib/dns/CachingDNSResolver.h"

namespace proxygen {

class DummyDNSResolver : public DNSResolver {
 public:
  void resolveHostname(
      DNSResolver::ResolutionCallback* cb,
      const std::string& name,
      std::chrono::milliseconds /*timeout*/ = std::chrono::milliseconds(100),
      sa_family_t family = AF_INET,
      TraceEventContext /*teContext*/ = TraceEventContext()) override {

    if (!isRunning_) {
      cb->resolutionError(folly::make_exception_wrapper<Exception>(
          UNKNOWN, "dummy DNS server is down."));
      return;
    }

    int ttl;
    if (name == "foo") {
      ttl = 2;
    } else {
      ttl = 2000;
    }

    Answer ans(std::chrono::seconds(ttl), folly::SocketAddress("1.2.3.4", 0));
    Answer ans6(std::chrono::seconds(ttl),
                folly::SocketAddress("2401:db00:20:700a:face:0:17:0", 0));

    std::vector<Answer> results;
    switch (family) {
      case AF_INET:
        results.push_back(ans);
        break;
      case AF_INET6:
        results.push_back(ans6);
        break;
      default:
        results.push_back(ans);
        results.push_back(ans6);
    }

    hitCount_++;
    cb->resolutionSuccess(results);
  }

  // NOPS
  void resolveAddress(DNSResolver::ResolutionCallback* /*cb*/,
                      const folly::SocketAddress& /*address*/,
                      std::chrono::milliseconds /*timeout*/ =
                          std::chrono::milliseconds(100)) override {
  }

  void resolveMailExchange(DNSResolver::ResolutionCallback* /*cb*/,
                           const std::string& /*domain*/,
                           std::chrono::milliseconds /*timeout*/ =
                               std::chrono::milliseconds(100)) override {
  }

  void setStatsCollector(
      DNSResolver::StatsCollector* /*statsCollector*/) override {
  }

  DNSResolver::StatsCollector* getStatsCollector() const override {
    return nullptr;
  }

  [[nodiscard]] int getHitCount() const {
    return hitCount_;
  }

  [[nodiscard]] bool isRunning() const {
    return isRunning_;
  }

  void setIsRunning(bool value) {
    isRunning_ = value;
  }

 private:
  int hitCount_{0};
  bool isRunning_{true};
};

class DummyDNSClient : public DNSResolver::ResolutionCallback {
 public:
  void resolutionSuccess(
      std::vector<DNSResolver::Answer> answers) noexcept override {
    fail_ = false;
    answers_ = answers;
    numSuccesses_++;
  }

  void resolutionError(
      const folly::exception_wrapper& /*ew*/) noexcept override {
    fail_ = true;
    answers_.clear();
    numFailures_++;
  }

  std::vector<DNSResolver::Answer>& getAnswers() {
    return answers_;
  }

  [[nodiscard]] bool didLastResolutionFail() const {
    return fail_;
  }

  [[nodiscard]] int getNumFailures() const {
    return numFailures_;
  }

  [[nodiscard]] int getNumSuccesses() const {
    return numSuccesses_;
  }

 private:
  int numFailures_{0};
  int numSuccesses_{0};

  std::vector<DNSResolver::Answer> answers_;
  bool fail_{false};
};

} // namespace proxygen
