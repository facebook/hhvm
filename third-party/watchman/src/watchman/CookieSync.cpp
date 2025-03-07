/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CookieSync.h"
#include <fmt/core.h>
#include <folly/String.h>
#include <exception>
#include <optional>
#include "watchman/Logging.h"
#include "watchman/watchman_stream.h"

namespace watchman {

CookieSync::Cookie::Cookie(uint64_t numCookies) : numPending(numCookies) {}

CookieSync::CookieSync(FileSystem& fs, const w_string& dir) : fileSystem_{fs} {
  char hostname[256];
  gethostname(hostname, sizeof(hostname));
  hostname[sizeof(hostname) - 1] = '\0';

  auto prefix = w_string::build(kCookiePrefix, hostname, "-", ::getpid(), "-");

  auto guard = cookieDirs_.wlock();
  guard->cookiePrefix_ = prefix;
  guard->dirs_.insert(dir);
}

CookieSync::~CookieSync() {
  // Wake up anyone that might have been waiting on us
  abortAllCookies();
}

void CookieSync::addCookieDir(const w_string& dir) {
  logf(DBG, "Adding cookie dir: {}\n", dir);
  auto guard = cookieDirs_.wlock();
  guard->dirs_.insert(dir);
}

void CookieSync::removeCookieDir(const w_string& dir) {
  logf(DBG, "Removing cookie dir: {}\n", dir);
  {
    auto guard = cookieDirs_.wlock();
    guard->dirs_.erase(dir);
  }

  // Cancel the cookies in the removed directory. These are considered to be
  // serviced.
  auto cookies = cookies_.wlock();
  for (auto it = cookies->begin(); it != cookies->end();) {
    auto& [cookiePath, cookie] = *it;
    if (cookiePath.piece().startsWith(dir)) {
      cookie->notify();
      it = cookies->erase(it);
    } else {
      ++it;
    }
  }
}

void CookieSync::setCookieDir(const w_string& dir) {
  auto guard = cookieDirs_.wlock();
  guard->dirs_.clear();
  guard->dirs_.insert(dir);
}

std::vector<w_string> CookieSync::getOutstandingCookieFileList() const {
  std::vector<w_string> result;
  auto cookiesLocked = cookies_.rlock();
  for (auto& it : *cookiesLocked) {
    result.push_back(it.first);
  }

  return result;
}

folly::SemiFuture<CookieSync::SyncResult> CookieSync::sync() {
  std::shared_ptr<Cookie> cookie;
  std::vector<w_string> cookieFileNames;
  {
    // We need to hold the cookieDirs lock while we lay cookies on disk to
    // avoid a race where a cookie directory is removed after collecting all
    // the cookie directories. In that case, this function would lay cookies on
    // disk, but the cookie directory removal wouldn't be able to notify them,
    // thus leaving them in a never notified state.
    auto cookieDirsGuard = cookieDirs_.rlock();
    auto prefixes = cookiePrefixLocked(*cookieDirsGuard);
    auto serial = serial_++;

    cookie = std::make_shared<Cookie>(prefixes.size());

    // Even though we only write to the cookie at the end of the function, we
    // need to hold it while the files are written on disk to avoid a race where
    // cookies are detected on disk by the watcher, and notifyCookie is called
    // prior to all the pending cookies being added to cookies_. Holding the
    // lock will make sure that notifyCookie will be serialized with this code.
    auto cookiesLock = cookies_.wlock();

    CookieMap pendingCookies;
    std::optional<std::tuple<w_string, int>> lastError;

    cookieFileNames.reserve(prefixes.size());
    for (const auto& prefix : prefixes) {
      auto path_str = w_string::build(prefix, serial);
      cookieFileNames.push_back(path_str);

      /* then touch the file */
      try {
        fileSystem_.touch(path_str.c_str());
      } catch (const std::system_error& e) {
        lastError = {path_str, e.code().value()};
        cookie->numPending.fetch_sub(1, std::memory_order_acq_rel);
        logf(
            ERR,
            "sync cookie {} couldn't be created: {}\n",
            path_str,
            folly::errnoStr(e.code().value()));
        continue;
      }

      /* insert the cookie into the temporary map */
      pendingCookies[path_str] = cookie;
      logf(DBG, "sync created cookie file {}\n", path_str);
    }

    if (pendingCookies.size() == 0) {
      w_assert(lastError.has_value(), "no cookies written, but no errors set");
      auto errCode = std::get<int>(*lastError);
      throw std::system_error(
          errCode,
          std::generic_category(),
          fmt::format(
              "sync: creat({}) failed: {}",
              std::get<w_string>(*lastError),
              folly::errnoStr(errCode)));
    }

    cookiesLock->insert(pendingCookies.begin(), pendingCookies.end());
  }

  return cookie->promise.getSemiFuture().deferValue(
      [cookieFileNames = std::move(cookieFileNames)](folly::Unit) mutable {
        return SyncResult{std::move(cookieFileNames)};
      });
}

CookieSync::SyncResult CookieSync::syncToNow(
    std::chrono::milliseconds timeout) {
  /* compute deadline */
  using namespace std::chrono;
  auto deadline = system_clock::now() + timeout;

  while (true) {
    auto cookieFuture = sync();

    folly::Try<SyncResult> result;
    try {
      result = std::move(cookieFuture).getTry(timeout);
    } catch (const folly::FutureTimeout&) {
      auto why = fmt::format(
          "syncToNow: timed out waiting for cookie file to be observed by watcher within {} milliseconds",
          timeout.count());
      log(ERR, why, "\n");
      throw std::system_error(ETIMEDOUT, std::generic_category(), why);
    }

    if (result.hasValue()) {
      // Success!
      return std::move(result).value();
    }

    // Sync was aborted by a recrawl; recompute the timeout
    // and wait again if we still have time
    timeout = duration_cast<milliseconds>(deadline - system_clock::now());
    if (timeout.count() <= 0) {
      result.throwUnlessValue();
    }
  }
}

void CookieSync::abortAllCookies() {
  std::unordered_map<w_string, std::shared_ptr<Cookie>> cookies;

  {
    auto map = cookies_.wlock();
    std::swap(*map, cookies);
  }

  for (const auto& [path, cookie] : cookies) {
    log(ERR, "syncToNow: aborting cookie ", path, "\n");
    unlink(path.c_str());

    if (cookie->numPending.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      cookie->promise.setException(
          folly::make_exception_wrapper<CookieSyncAborted>());
    }
  }
}

void CookieSync::Cookie::notify() {
  if (numPending.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    promise.setValue();
  }
}

void CookieSync::notifyCookie(const w_string& path) {
  std::shared_ptr<Cookie> cookie;

  {
    auto map = cookies_.wlock();
    auto cookie_iter = map->find(path);
    log(DBG,
        "cookie for ",
        path,
        "? ",
        cookie_iter != map->end() ? "yes" : "no",
        "\n");

    if (cookie_iter != map->end()) {
      cookie = std::move(cookie_iter->second);
      map->erase(cookie_iter);
    }
  }

  if (cookie) {
    cookie->notify();

    // The file may not exist at this point; we're just taking this
    // opportunity to remove it if nothing else has done so already.
    // We don't care about the return code; best effort is fine.
    unlink(path.c_str());
  }
}

bool CookieSync::isCookiePrefix(w_string_piece path) const {
  auto cookieDirs = cookieDirs_.rlock();
  for (const auto& dir : cookieDirs->dirs_) {
    if (path.startsWith(dir) &&
        path.baseName().startsWith(cookieDirs->cookiePrefix_)) {
      return true;
    }
  }
  return false;
}

bool CookieSync::isCookieDir(w_string_piece path) const {
  auto cookieDirs = cookieDirs_.rlock();
  for (const auto& dir : cookieDirs->dirs_) {
    if (path == dir) {
      return true;
    }
  }
  return false;
}

std::unordered_set<w_string> CookieSync::cookiePrefixLocked(
    const CookieSync::CookieDirectories& guard) const {
  std::unordered_set<w_string> res;
  for (const auto& dir : guard.dirs_) {
    res.insert(w_string::build(dir, "/", guard.cookiePrefix_));
  }
  return res;
}

std::unordered_set<w_string> CookieSync::cookiePrefix() const {
  auto guard = cookieDirs_.rlock();
  return cookiePrefixLocked(*guard);
}

std::unordered_set<w_string> CookieSync::cookieDirs() const {
  std::unordered_set<w_string> res;
  auto guard = cookieDirs_.rlock();
  for (const auto& dir : guard->dirs_) {
    res.insert(dir);
  }
  return res;
}

} // namespace watchman
