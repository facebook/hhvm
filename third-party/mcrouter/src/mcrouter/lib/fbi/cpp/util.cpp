/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "util.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <utime.h>

#include <chrono>

#include <boost/filesystem.hpp>

#include <openssl/md5.h>

#include <folly/FileUtil.h>
#include <folly/Random.h>
#include <folly/ScopeGuard.h>
#include <folly/hash/SpookyHashV2.h>
#include <folly/json/json.h>

namespace facebook {
namespace memcache {

std::string
randomString(size_t minLen, size_t maxLen, folly::StringPiece range) {
  assert(minLen <= maxLen);
  assert(!range.empty());

  std::string result(folly::Random::rand32(minLen, maxLen + 1), '\0');
  for (char& c : result) {
    c = range[folly::Random::rand32(range.size())];
  }
  return result;
}

uint32_t getMemcacheKeyHashValue(folly::StringPiece key) {
  return folly::hash::SpookyHashV2::Hash32(
      key.begin(), key.size(), /* seed= */ 0);
}

bool determineIfSampleKeyForViolet(
    uint32_t routingKeyHash,
    uint32_t sample_period) {
  assert(sample_period > 0);
  constexpr uint32_t m = std::numeric_limits<uint32_t>::max();
  uint32_t keyHashMax = m / sample_period;

  return routingKeyHash <= keyHashMax;
}

std::string Md5Hash(folly::StringPiece input) {
  unsigned char result[MD5_DIGEST_LENGTH];
  MD5(reinterpret_cast<const unsigned char*>(input.data()),
      input.size(),
      result);

  std::string ret;
  const std::string kHexBytes = "0123456789abcdef";
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    // Hex should print, for this, in a weird bytewise order:
    // If the bytes are b0b1b2b3..., we print:
    //       hi(b0) lo(b0) hi(b1) lo(b1) ...
    // Where hi(x) is the hex char of its upper 4 bits, and lo(x) is lower 4
    ret += kHexBytes[(result[i] >> 4) & 0x0F];
    ret += kHexBytes[result[i] & 0x0F];
  }

  return ret;
}

namespace {
bool writeToFile(
    folly::StringPiece contents,
    const std::string& path,
    int flags) {
  int fd = folly::openNoInt(path.data(), flags, 0664);
  if (fd == -1) {
    return false;
  }
  auto written = folly::writeFull(fd, contents.data(), contents.size());
  if (folly::closeNoInt(fd) != 0) {
    return false;
  }
  return written >= 0 && size_t(written) == contents.size();
}
} // namespace

bool writeStringToFile(folly::StringPiece contents, const std::string& path) {
  return writeToFile(contents, path, O_CREAT | O_WRONLY | O_TRUNC);
}

bool appendStringToFile(folly::StringPiece contents, const std::string& path) {
  return writeToFile(contents, path, O_CREAT | O_WRONLY | O_APPEND);
}

bool atomicallyWriteFileToDisk(
    folly::StringPiece contents,
    const std::string& absFilename) {
  boost::filesystem::path tempFilePath;
  auto tempFileGuard = folly::makeGuard([&tempFilePath]() {
    if (!tempFilePath.empty()) {
      boost::system::error_code ec;
      boost::filesystem::remove(tempFilePath.c_str(), ec);
    }
  });

  try {
    const boost::filesystem::path filePath(absFilename);
    auto fileDir = filePath.parent_path();
    if (fileDir.empty()) {
      return false;
    }
    auto tempFileName = filePath.filename().string() + ".temp-" +
        randomString(/* minLen */ 10, /* maxLen */ 10);
    tempFilePath = fileDir / tempFileName;

    boost::filesystem::create_directories(fileDir);

    if (!writeStringToFile(contents, tempFilePath.string())) {
      return false;
    }

    boost::filesystem::rename(tempFilePath, filePath);
    return true;
  } catch (const boost::filesystem::filesystem_error&) {
    return false;
  } catch (const boost::system::system_error&) {
    return false;
  }
}

bool touchFile(const std::string& path) {
  struct stat fileStats;
  if (stat(path.data(), &fileStats)) {
    if (!writeStringToFile("", path)) {
      return false;
    }
  }
  return utime(path.data(), nullptr) == 0;
}

// one day we should move it to folly/ThreadName.h
std::string getThreadName() {
#if defined(__GLIBC__) && !defined(__APPLE__)
#if __GLIBC_PREREQ(2, 12)

  char threadName[32];
  if (pthread_getname_np(pthread_self(), threadName, sizeof(threadName)) == 0) {
    return threadName;
  }

#endif
#endif

  return "unknown";
}

// TODO make this return a class containing folly::dynamic and
// folly::json::metadata_map as suggested by Andre in the comments of
// D15286671
folly::dynamic parseJsonString(
    folly::StringPiece s,
    folly::json::metadata_map* metadataMap,
    bool allow_trailing_comma) {
  folly::json::serialization_opts opts;
  opts.allow_trailing_comma = allow_trailing_comma;
  assert(!opts.allow_non_string_keys);
  return folly::parseJsonWithMetadata(s, opts, metadataMap);
}

std::string shorten(folly::StringPiece s, size_t maxLength) {
  if (s.size() <= maxLength || s.size() <= 3) {
    return s.str();
  }
  return s.subpiece(0, maxLength - 3).str() + "...";
}

std::string
replaceAll(std::string s, const std::string& from, const std::string& to) {
  auto pos = s.find(from);
  while (pos != std::string::npos) {
    s.replace(pos, from.size(), to);
    pos = s.find(from);
  }
  return s;
}

std::string toPrettySortedJson(const folly::dynamic& json) {
  folly::json::serialization_opts opts;
  opts.pretty_formatting = true;
  opts.sort_keys = true;
  return folly::json::serialize(json, opts);
}

namespace {
folly::Expected<folly::Unit, std::runtime_error> ensureHasPermissionImpl(
    const std::string& path,
    mode_t mode) {
  struct stat st;
  if (::stat(path.c_str(), &st) != 0) {
    auto statErr = errno;
    return folly::makeUnexpected(std::runtime_error(folly::errnoStr(statErr)));
  }

  if ((st.st_mode & mode) == mode) {
    return folly::Unit();
  }

  if (::chmod(path.c_str(), mode) != 0) {
    auto chmodErr = errno;
    return folly::makeUnexpected(std::runtime_error(folly::errnoStr(chmodErr)));
  }

  return folly::Unit();
}

folly::Expected<folly::Unit, std::runtime_error> ensureDirExistsAndWritableImpl(
    const std::string& path) {
  boost::system::error_code ec;

  boost::filesystem::create_directories(path, ec);
  if (ec) {
    return folly::makeUnexpected(std::runtime_error(ec.message()));
  }

  return ensureHasPermissionImpl(path, 0777);
}
} // namespace

bool ensureHasPermission(const std::string& path, mode_t mode) {
  auto res = ensureHasPermissionImpl(path, mode);
  return !res.hasError();
}

folly::Expected<folly::Unit, std::runtime_error>
ensureHasPermissionOrReturnError(const std::string& path, mode_t mode) {
  return ensureHasPermissionImpl(path, mode);
}

bool ensureDirExistsAndWritable(const std::string& path) {
  auto res = ensureDirExistsAndWritableImpl(path);
  return !res.hasError();
}

folly::Expected<folly::Unit, std::runtime_error>
ensureDirExistsAndWritableOrReturnError(const std::string& path) {
  return ensureDirExistsAndWritableImpl(path);
}

bool intervalOverlap(std::vector<std::vector<size_t>>& intervals) {
  std::sort(
      intervals.begin(),
      intervals.end(),
      [](const std::vector<size_t>& a, const std::vector<size_t>& b) {
        return a[0] < b[0];
      });
  for (size_t i = 1; i < intervals.size(); ++i) {
    assert(intervals[i - 1].size() == 1 || intervals[i - 1].size() == 2);
    if ((intervals[i - 1].size() == 1) &&
        (intervals[i][0] < intervals[i - 1][0])) {
      return true;
    } else if (
        (intervals[i - 1].size() == 2) &&
        (intervals[i][0] < intervals[i - 1][1])) {
      return true;
    }
  }
  return false;
}

} // namespace memcache
} // namespace facebook
