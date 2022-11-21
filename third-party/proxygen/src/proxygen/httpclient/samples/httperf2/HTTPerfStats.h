/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/json.h>
#include <iostream>
#include <mutex>

/**
 * Thread-local statistics for HTTPerf2.
 */
class HTTPerfStats {
 public:
  class SumStat {
   public:
    SumStat() = default;

    void addValue(size_t val) {
      sum += val;
    }

    size_t sum{0};
  };

  class AvgStat {
    struct AvgData {
      size_t count{0};
      size_t total{0};
    };

   public:
    AvgStat() = default;

    void addValue(size_t val, size_t num = 1) {
      avg_.count += num;
      avg_.total += val;
    }

    void addValue(const AvgStat& avgData) {
      addValue(avgData.avg_.total, avgData.avg_.count);
    }

    size_t avg() {
      if (avg_.count > 0) {
        return size_t(double(avg_.total) / double(avg_.count));
      } else {
        return 0;
      }
    }

   private:
    AvgData avg_;
  };

  HTTPerfStats() = default;
  ~HTTPerfStats() = default;

  void addConnection(uint32_t connLat) {
    conns_.addValue(1);
    connLatency_.addValue(connLat);
  }

  void addHandshake() {
    handshakes_.addValue(1);
  }

  void addResume() {
    resumes_.addValue(1);
  }

  void addRequest() {
    requests_.addValue(1);
  }

  void addResponse(uint32_t reqLat) {
    responses_.addValue(1);
    reqLatency_.addValue(reqLat);
  }

  void addErrorLat(uint32_t reqLat) {
    reqLatency_.addValue(reqLat);
  }

  void addResponseCode(uint32_t code) {
    if (code < 100 || code >= 600) {
      codeOther_.addValue(1);
    } else if (code < 200) {
      code1xx_.addValue(1);
    } else if (code < 300) {
      code2xx_.addValue(1);
    } else if (code < 400) {
      code3xx_.addValue(1);
    } else if (code < 500) {
      code4xx_.addValue(1);
    } else {
      code5xx_.addValue(1);
    }
  }

  void addBytesReceived(uint32_t count) {
    bytesReceived_.addValue(count);
  }

  void addConnectError() {
    connErrors_.addValue(1);
  }

  void addWriteError() {
    writeErrors_.addValue(1);
  }

  void addMessageError() {
    msgErrors_.addValue(1);
  }

  void addTimeoutError() {
    timeoutErrors_.addValue(1);
  }

  void addEOFResponse() {
    eofResponses_.addValue(1);
  }

  void addEOFError() {
    eofErrors_.addValue(1);
  }

  void merge(const HTTPerfStats& stats) {
    const std::lock_guard<std::mutex> lock(mutex_);
    conns_.addValue(stats.conns_.sum);
    handshakes_.addValue(stats.handshakes_.sum);
    resumes_.addValue(stats.resumes_.sum);
    requests_.addValue(stats.requests_.sum);
    codeOther_.addValue(stats.codeOther_.sum);
    code1xx_.addValue(stats.code1xx_.sum);
    code2xx_.addValue(stats.code2xx_.sum);
    code3xx_.addValue(stats.code3xx_.sum);
    code4xx_.addValue(stats.code4xx_.sum);
    code5xx_.addValue(stats.code5xx_.sum);
    bytesReceived_.addValue(stats.bytesReceived_.sum);
    connErrors_.addValue(stats.connErrors_.sum);
    msgErrors_.addValue(stats.msgErrors_.sum);
    writeErrors_.addValue(stats.writeErrors_.sum);
    timeoutErrors_.addValue(stats.timeoutErrors_.sum);
    eofResponses_.addValue(stats.eofResponses_.sum);
    eofErrors_.addValue(stats.eofErrors_.sum);
    connLatency_.addValue(stats.connLatency_);
    reqLatency_.addValue(stats.reqLatency_);
  }

  std::map<std::string, size_t> aggregateSums() {
    std::map<std::string, size_t> results;
    const std::lock_guard<std::mutex> lock(mutex_);
    results.emplace("HTTPerf_conns", conns_.sum);
    results.emplace("HTTPerf_ssl_hand", handshakes_.sum);
    results.emplace("HTTPerf_ssl_res", resumes_.sum);
    results.emplace("HTTPerf_reqs", requests_.sum);
    results.emplace("HTTPerf_resp", responses_.sum);
    results.emplace("HTTPerf_code_Other", codeOther_.sum);
    results.emplace("HTTPerf_code_1xx", code1xx_.sum);
    results.emplace("HTTPerf_code_2xx", code2xx_.sum);
    results.emplace("HTTPerf_code_3xx", code3xx_.sum);
    results.emplace("HTTPerf_code_4xx", code4xx_.sum);
    results.emplace("HTTPerf_code_5xx", code5xx_.sum);
    results.emplace("HTTPerf_bytes", bytesReceived_.sum);
    results.emplace("HTTPerf_conn_err", connErrors_.sum);
    results.emplace("HTTPerf_msg_err", msgErrors_.sum);
    results.emplace("HTTPerf_write_err", writeErrors_.sum);
    results.emplace("HTTPerf_timeout", timeoutErrors_.sum);
    results.emplace("HTTPerf_eof_resp", eofResponses_.sum);
    results.emplace("HTTPerf_eof_err", eofErrors_.sum);
    return results;
  }

  std::map<std::string, size_t> aggregateAvgs() {
    std::map<std::string, size_t> results;
    const std::lock_guard<std::mutex> lock(mutex_);
    results.emplace("HTTPerf_conn_lat", connLatency_.avg());
    results.emplace("HTTPerf_req_lat", reqLatency_.avg());
    return results;
  }

  void printStats(std::chrono::milliseconds durationMs) {
    auto results = aggregateSums();
    for (const auto& item : results) {
      double rate = double(item.second) * 1000.0 / double(durationMs.count());
      if (rate > 1000000) {
        rate /= 1000000;
        printf("  %-21s: %7.2fM/sec\n", item.first.c_str(), rate);
      } else {
        printf("  %-21s: %8d/sec\n", item.first.c_str(), (int)rate);
      }
    }
    results = aggregateAvgs();
    for (const auto& item : results) {
      printf("  %-21s: %7ld msec\n", item.first.c_str(), item.second);
    }
    printf("  %-21s: %9ld ms\n", "Run time", durationMs.count());
  }

  void printStatsInJson(const std::string& testname,
                        std::chrono::milliseconds durationMs) {
    auto results = aggregateSums();
    folly::dynamic d = folly::dynamic::object;
    for (const auto& item : results) {
      double rate = double(item.second) * 1000.0 / double(durationMs.count());
      d[testname + "." + item.first] = rate;
    }
    results = aggregateAvgs();
    for (const auto& item : results) {
      d[testname + "." + item.first] = item.second;
    }

    d[testname + ".runtime"] = durationMs.count();
    std::cout << toPrettyJson(d) << std::endl;
  }

 private:
  std::mutex mutex_;
  SumStat conns_;
  SumStat handshakes_;
  SumStat resumes_;
  SumStat requests_;
  SumStat responses_;
  SumStat codeOther_;
  SumStat code1xx_;
  SumStat code2xx_;
  SumStat code3xx_;
  SumStat code4xx_;
  SumStat code5xx_;
  SumStat bytesReceived_;
  SumStat connErrors_;
  SumStat msgErrors_;
  SumStat writeErrors_;
  SumStat timeoutErrors_;
  SumStat eofResponses_;
  SumStat eofErrors_;
  AvgStat connLatency_;
  AvgStat reqLatency_;
};
