/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncLog.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <memory>

#include <folly/Conv.h>
#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/fibers/EventBaseLoopController.h>
#include <folly/json/json.h>
#include <folly/system/ThreadName.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/options.h"
#include "mcrouter/stats.h"

constexpr folly::StringPiece kAsyncLogMagic{"AS1.0"};
constexpr folly::StringPiece kAsyncLogMagic2{"AS2.0"};

using folly::dynamic;

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {

std::unique_ptr<folly::File> createFile(int fd) {
  if (fd < 0) {
    return nullptr;
  }
  return std::make_unique<folly::File>(fd, true);
}

void closeFd(int fd) {
  if (fd != -1) {
    close(fd);
  }
}

} // anonymous namespace

bool AsyncLog::openFile() {
  char path[PATH_MAX + 1];
  time_t now = time(nullptr);
  pid_t tid = syscall(SYS_gettid);
  struct tm date;
  struct stat st;

  if (file_ && now - spoolTime_ <= DEFAULT_ASYNCLOG_LIFETIME) {
    return true;
  }

  if (file_) {
    file_.reset();
  }

  localtime_r(&now, &date);
  char hour_path[PATH_MAX + 1];
  time_t hour_time = now - (now % 3600);
  if (snprintf(
          hour_path,
          PATH_MAX,
          "%s/%04d%02d%02dT%02d-%lld",
          options_.async_spool.c_str(),
          date.tm_year + 1900,
          date.tm_mon + 1,
          date.tm_mday,
          date.tm_hour,
          (long long)hour_time) > PATH_MAX) {
    hour_path[PATH_MAX] = '\0';
    MC_LOG_FAILURE(
        options_,
        failure::Category::kSystemError,
        "async log hourly spool path is too long: {}",
        hour_path);
    return false;
  }

  if (stat(hour_path, &st) != 0) {
    mode_t old_umask = umask(0);
    int ret = mkdir(hour_path, 0777);
    int mkdir_errno = 0;
    if (ret != 0) {
      mkdir_errno = errno;
    }
    if (old_umask != 0) {
      umask(old_umask);
    }
    /* EEXIST is possible due to a race. We don't care. */
    if (ret != 0 && mkdir_errno != EEXIST) {
      MC_LOG_FAILURE(
          options_,
          failure::Category::kSystemError,
          "couldn't create async log hour spool path: {}. Reason: {}",
          hour_path,
          folly::errnoStr(mkdir_errno));
      return false;
    }
  }

  if (snprintf(
          path,
          PATH_MAX,
          "%s/%04d%02d%02dT%02d%02d%02d-%lld-%s-%s-t%d-%p",
          hour_path,
          date.tm_year + 1900,
          date.tm_mon + 1,
          date.tm_mday,
          date.tm_hour,
          date.tm_min,
          date.tm_sec,
          (long long)now,
          options_.service_name.c_str(),
          options_.router_name.c_str(),
          tid,
          this) > PATH_MAX) {
    path[PATH_MAX] = '\0';
    MC_LOG_FAILURE(
        options_,
        failure::Category::kSystemError,
        "async log path is too long: {}",
        path);
    return false;
  }

  int fd = -1;
  /*
   * Just in case, append to the log if it exists
   */
  if (stat(path, &st) != 0) {
    fd = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
      MC_LOG_FAILURE(
          options_,
          failure::Category::kSystemError,
          "Can't create and open async store {}: {}",
          path,
          folly::errnoStr(errno));
      return false;
    }
  } else {
    fd = open(path, O_WRONLY | O_APPEND, 0666);
    if (fd < 0) {
      MC_LOG_FAILURE(
          options_,
          failure::Category::kSystemError,
          "Can't re-open async store {}: {}",
          path,
          folly::errnoStr(errno));
      return false;
    }
  }

  if (fstat(fd, &st)) {
    MC_LOG_FAILURE(
        options_,
        failure::Category::kSystemError,
        "Can't stat async store {}: {}",
        path,
        folly::errnoStr(errno));
    closeFd(fd);
    return false;
  }
  if (!S_ISREG(st.st_mode)) {
    MC_LOG_FAILURE(
        options_,
        failure::Category::kSystemError,
        "Async store exists but is not a file: {}: {}",
        path,
        folly::errnoStr(errno));
    closeFd(fd);
    return false;
  }

  file_ = createFile(fd);
  if (!file_) {
    MC_LOG_FAILURE(
        options_,
        failure::Category::kSystemError,
        "Unable to allocate memory for file_: {}",
        folly::errnoStr(errno));
    closeFd(fd);
    return false;
  }

  spoolTime_ = now;

  VLOG(1) << "Opened async store for " << path;

  return true;
}

AsyncLog::AsyncLog(const McrouterOptions& options) : options_(options) {}

/** Adds an asynchronous request to the event log. */
bool AsyncLog::writeDelete(
    const AccessPoint& ap,
    folly::StringPiece key,
    folly::StringPiece poolName,
    std::unordered_map<std::string, uint64_t> attributes) {
  dynamic json = dynamic::array;
  const auto& host = ap.getHost();
  const auto port = options_.asynclog_port_override == 0
      ? ap.getPort()
      : options_.asynclog_port_override;

  if (options_.use_asynclog_version2) {
    json = dynamic::object;
    json["s"] = options_.service_name;
    json["f"] = options_.flavor_name;
    json["r"] = options_.default_route.getRegion();
    json["h"] = folly::sformat("[{}]:{}", host, port);
    json["p"] = poolName.str();
    json["k"] = key.str();
    attributes.emplace(AsyncLog::kAsyncLogMarker, 1);
    json["a"] = folly::toDynamic(attributes);
  } else {
    /* ["host", port, escaped_command] */
    json.push_back(host);
    json.push_back(port);
    json.push_back(folly::sformat("delete {}\r\n", key));
  }

  if (!openFile()) {
    MC_LOG_FAILURE(
        options_,
        memcache::failure::Category::kSystemError,
        "asynclog_open() failed (key {}, pool {})",
        key,
        poolName);
    return false;
  }

  // ["AS1.0", 1289416829.836, "C", ["10.0.0.1", 11302, "delete foo\r\n"]]
  // OR ["AS2.0", 1289416829.836, "C", {"f":"flavor","h":"[10.0.0.1]:11302",
  //                                    "p":"pool_name","k":"foo\r\n"}]
  dynamic jsonOut = dynamic::array;
  if (options_.use_asynclog_version2) {
    jsonOut.push_back(kAsyncLogMagic2);
  } else {
    jsonOut.push_back(kAsyncLogMagic);
  }

  auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  jsonOut.push_back(1e-3 * timestamp_ms);

  jsonOut.push_back(std::string("C"));

  jsonOut.push_back(json);

  auto jstr = folly::toJson(jsonOut) + "\n";

  ssize_t size = folly::writeFull(file_->fd(), jstr.data(), jstr.size());
  if (size == -1 || size_t(size) < jstr.size()) {
    MC_LOG_FAILURE(
        options_,
        memcache::failure::Category::kSystemError,
        "Error fully writing asynclog request (key {}, pool {})",
        key,
        poolName);
    return false;
  }
  return true;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
