/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/ContentHash.h"
#include <fmt/core.h>
#include <folly/ScopeGuard.h>
#include <string>
#include "watchman/Hash.h"
#include "watchman/Logging.h"
#include "watchman/ThreadPool.h"
#include "watchman/fs/FileSystem.h"
#include "watchman/watchman_stream.h"

#ifdef __APPLE__
#define COMMON_DIGEST_FOR_OPENSSL
#include "CommonCrypto/CommonDigest.h" // @manual
#elif defined(_WIN32)
#include <Wincrypt.h> // @manual
#else
#include <openssl/sha.h>
#endif

namespace watchman {

using HashValue = typename ContentHashCache::HashValue;
using Node = typename ContentHashCache::Node;

bool ContentHashCacheKey::operator==(const ContentHashCacheKey& other) const {
  return fileSize == other.fileSize && mtime.tv_sec == other.mtime.tv_sec &&
      mtime.tv_nsec == other.mtime.tv_nsec &&
      relativePath == other.relativePath;
}

std::size_t ContentHashCacheKey::hashValue() const {
  return hash_combine(
      {relativePath.hashValue(),
       fileSize,
       static_cast<uint64_t>(mtime.tv_sec),
       static_cast<uint64_t>(mtime.tv_nsec)});
}

ContentHashCache::ContentHashCache(
    const w_string& rootPath,
    size_t maxItems,
    std::chrono::milliseconds errorTTL)
    : cache_(maxItems, errorTTL), rootPath_(rootPath) {}

folly::Future<std::shared_ptr<const Node>> ContentHashCache::get(
    const ContentHashCacheKey& key) {
  return cache_.get(
      key, [this](const ContentHashCacheKey& k) { return computeHash(k); });
}

HashValue ContentHashCache::computeHashImmediate(const char* fullPath) {
  HashValue result;
  uint8_t buf[8192];

  auto stm = w_stm_open(fullPath, O_RDONLY);
  if (!stm) {
    throw std::system_error(
        errno, std::generic_category(), fmt::format("w_stm_open {}", fullPath));
  }

#ifndef _WIN32
  SHA_CTX ctx;
  SHA1_Init(&ctx);

  while (true) {
    auto n = stm->read(buf, sizeof(buf));
    if (n == 0) {
      break;
    }
    if (n < 0) {
      throw std::system_error(
          errno,
          std::generic_category(),
          fmt::format("while reading from {}", fullPath));
    }
    SHA1_Update(&ctx, buf, n);
  }

  SHA1_Final(result.data(), &ctx);
#else
  // Use the built-in crypt provider API on windows to avoid introducing a
  // dependency on openssl in the windows build.
  HCRYPTPROV provider{0};
  HCRYPTHASH ctx{0};

  if (!CryptAcquireContext(
          &provider,
          nullptr,
          nullptr,
          PROV_RSA_FULL,
          CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    throw std::system_error(
        GetLastError(), std::system_category(), "CryptAcquireContext");
  }
  SCOPE_EXIT {
    CryptReleaseContext(provider, 0);
  };

  if (!CryptCreateHash(provider, CALG_SHA1, 0, 0, &ctx)) {
    throw std::system_error(
        GetLastError(), std::system_category(), "CryptCreateHash");
  }
  SCOPE_EXIT {
    CryptDestroyHash(ctx);
  };

  while (true) {
    auto n = stm->read(buf, sizeof(buf));
    if (n == 0) {
      break;
    }
    if (n < 0) {
      throw std::system_error(
          errno,
          std::generic_category(),
          fmt::format("while reading from {}", fullPath));
    }

    if (!CryptHashData(ctx, buf, n, 0)) {
      throw std::system_error(
          GetLastError(), std::system_category(), "CryptHashData");
    }
  }

  DWORD size = result.size();
  if (!CryptGetHashParam(ctx, HP_HASHVAL, result.data(), &size, 0)) {
    throw std::system_error(
        GetLastError(), std::system_category(), "CryptGetHashParam HP_HASHVAL");
  }
#endif
  return result;
}

HashValue ContentHashCache::computeHashImmediate(
    const ContentHashCacheKey& key) const {
  auto fullPath = w_string::pathCat({rootPath_, key.relativePath});
  auto result = computeHashImmediate(fullPath.c_str());

  // Since TOCTOU is everywhere and everything, double check to make sure that
  // the file looks like we were expecting at the start.  If it isn't, then
  // we want to throw an exception and avoid associating the hash of whatever
  // state we just read with this cache key.
  auto stat = getFileInformation(fullPath.c_str());
  if (size_t(stat.size) != key.fileSize ||
      stat.mtime.tv_sec != key.mtime.tv_sec ||
      stat.mtime.tv_nsec != key.mtime.tv_nsec) {
    throw std::runtime_error(
        "metadata changed during hashing; query again to get latest status");
  }

  return result;
}

folly::Future<HashValue> ContentHashCache::computeHash(
    const ContentHashCacheKey& key) const {
  return folly::via(
      &getThreadPool(), [key, this] { return computeHashImmediate(key); });
}

const w_string& ContentHashCache::rootPath() const {
  return rootPath_;
}

CacheStats ContentHashCache::stats() const {
  return cache_.stats();
}
} // namespace watchman
