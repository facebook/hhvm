/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <unordered_map>

#include <folly/File.h>
#include <folly/Range.h>

namespace facebook {
namespace memcache {

struct AccessPoint;
class McrouterOptions;

namespace mcrouter {

class AsyncLog {
 public:
  explicit AsyncLog(const McrouterOptions& options);

  /**
   * Appends a 'delete' request entry to the asynclog.
   * This call blocks until the entry is written to the file
   * or an error occurs. Returns true if the write was successful, false if not.
   */
  bool writeDelete(
      const AccessPoint& ap,
      folly::StringPiece key,
      folly::StringPiece poolName,
      std::unordered_map<std::string, uint64_t> attributes = {});

 private:
  const McrouterOptions& options_;
  std::unique_ptr<folly::File> file_;
  time_t spoolTime_{0};

  /**
   * Open async log file.
   *
   * @return True if the file is ready to use. False otherwise.
   */
  bool openFile();
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
