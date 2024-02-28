/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cctype>
#include <folly/Conv.h>
#include <folly/File.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <memory>
#include <string>

namespace proxygen {

struct ConnIdLogSink : google::LogSink {
  using FileEntry =
      std::pair<folly::File, std::chrono::system_clock::time_point>;

  ConnIdLogSink(std::string logDir, std::string logPrefix)
      : logDir_(std::move(logDir)), prefix_(std::move(logPrefix)) {
  }

  void send(google::LogSeverity severity,
            const char* /*full_filename*/,
            const char* base_filename,
            int line,
            const struct ::tm* tm_time,
            const char* message,
            size_t message_len) override {
    folly::StringPiece testMsg(message, message_len);
    // The incoming string are expected to be in the format of
    // ".* CID=([a-f0-9]+)[, ].*"
    folly::StringPiece pre, post; // pre will be ignored
    folly::split("CID=", testMsg, pre, post);
    if (post.empty()) {
      return;
    }
    std::vector<folly::StringPiece> cids;
    folly::split(",", post, cids);
    for (const auto& cidSp : cids) {
      if (!std::all_of(cidSp.begin(), cidSp.end(), [](char c) {
            return std::isalnum(c);
          })) {
        continue;
      }
      auto cid = cidSp.str();
      int fd = -1;
      {
        auto now = std::chrono::system_clock::now();
        auto fdMap = fdMap_.wlock();
        auto it = fdMap->find(cid);
        if (it == fdMap->end()) {
          auto insert = fdMap->emplace(
              std::piecewise_construct,
              std::forward_as_tuple(cid),
              std::forward_as_tuple(
                  std::piecewise_construct,
                  std::forward_as_tuple(
                      folly::to<std::string>(
                          logDir_, "/", prefix_, ".", cid, ".html"),
                      O_CREAT | O_RDWR | O_APPEND),
                  std::forward_as_tuple(now)));
          fd = insert.first->second.first.fd();
        } else {
          fd = it->second.first.fd();
          it->second.second = now;
        }
        closeOldFds(*fdMap, now);
      }
      char timebuf[64];
      strftime(timebuf, sizeof(timebuf), "%m%d %R", tm_time);
      auto msg = folly::to<std::string>(severityMap[severity],
                                        timebuf,
                                        ' ',
                                        base_filename,
                                        ':',
                                        line,
                                        ' ',
                                        testMsg);
      [[maybe_unused]] auto writeRes = ::write(fd, msg.c_str(), msg.size());
      writeRes = ::write(fd, "<br/>", 5);
    } // else, not for a specific CID
  }

  [[nodiscard]] bool isValid() const {
    return !logDir_.empty() && ::access(logDir_.c_str(), W_OK) == 0;
  }

 private:
  using FdMap = std::map<std::string, FileEntry>;

  void closeOldFds(FdMap& fdMap, std::chrono::system_clock::time_point now) {
    // Close any logfiles open for > 1 min
    for (auto it = fdMap.begin(); it != fdMap.end();) {
      if (now > it->second.second + kMaxAge) {
        auto eraseIt = it++;
        fdMap.erase(eraseIt);
      } else {
        ++it;
      }
    }
  }

  std::string logDir_;
  std::string prefix_;
  folly::Synchronized<FdMap> fdMap_;
  std::array<char, 5> severityMap{{'V', 'I', 'W', 'E', 'F'}};
  const std::chrono::seconds kMaxAge{60};
};
} // namespace proxygen
