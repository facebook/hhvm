/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/UserDir.h"
#include <folly/String.h>
#include "watchman/Logging.h"
#include "watchman/Options.h"
#include "watchman/fs/FileSystem.h"
#include "watchman/portability/WinError.h"

#ifdef _WIN32
#include <Lmcons.h> // @manual
#include <Shlobj.h> // @manual
#endif

namespace watchman {

namespace {

const char*
getEnvWithFallback(const char* name1, const char* name2, const char* fallback) {
  const char* val = getenv(name1);
  if (!val || *val == 0) {
    val = getenv(name2);
  }
  if (!val || *val == 0) {
    val = fallback;
  }

  return val;
}

#ifdef _WIN32
std::string getWatchmanAppDataPath() {
  PWSTR local_app_data = nullptr;
  auto res =
      SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data);
  if (res != S_OK) {
    logf(
        FATAL,
        "SHGetKnownFolderPath FOLDERID_LocalAppData failed: {}\n",
        win32_strerror(res));
  }
  SCOPE_EXIT {
    CoTaskMemFree(local_app_data);
  };
  // Perform path mapping from wide string to our preferred UTF8
  w_string temp_location(local_app_data, wcslen(local_app_data));
  // and use the watchman subdir of LOCALAPPDATA
  auto watchmanDir = folly::to<std::string>(temp_location.view(), "/watchman");
  if (mkdir(watchmanDir.c_str(), 0700) == 0 || errno == EEXIST) {
    return watchmanDir;
  }
  logf(
      ERR,
      "failed to create directory {}: {}\n",
      watchmanDir,
      folly::errnoStr(errno));
  exit(1);
}

const std::string& getCachedWatchmanAppDataPath() {
  static std::string path = getWatchmanAppDataPath();
  return path;
}
#endif

std::string computeTemporaryDirectory() {
#ifdef _WIN32
  if (!flags.test_state_dir.empty()) {
    return flags.test_state_dir;
  } else {
    return getCachedWatchmanAppDataPath();
  }
#else
  return getEnvWithFallback("TMPDIR", "TMP", "/tmp");
#endif
}

} // namespace

std::string computeUserName() {
#ifdef _WIN32
  // We don't trust the environment on Win32 because in some situations
  // the environment may contain the domain name like `WORKGROUP\user`
  // which can confuse some path construction we do later on.
  WCHAR userW[1 + UNLEN];
  DWORD size = static_cast<DWORD>(std::size(userW));
  if (GetUserNameW(userW, &size) && size > 0) {
    // Constructing a w_string from a WCHAR* will convert to UTF-8
    w_string user(userW, size);
    return user.string();
  }
  DWORD lastError = GetLastError();

  log(FATAL,
      "GetUserName failed: ",
      win32_strerror(lastError),
      ". I don't know who you are!?\n");
#else
  const char* user = getEnvWithFallback("USER", "LOGNAME", nullptr);
  if (user) {
    return user;
  }

  uid_t uid = getuid();
  struct passwd* pw = getpwuid(uid);
  if (!pw) {
    log(FATAL,
        "getpwuid(",
        uid,
        ") failed: ",
        folly::errnoStr(errno),
        ". I don't know who you are\n");
  }

  user = pw->pw_name;
  if (user) {
    return user;
  }

  log(FATAL, "watchman requires that you set $USER in your env\n");
#endif
  throw std::logic_error("unreachable");
}

const std::string& getTemporaryDirectory() {
  static std::string tmpdir = computeTemporaryDirectory();
  return tmpdir;
}

std::string computeWatchmanStateDirectory(const std::string& user) {
  if (!flags.test_state_dir.empty()) {
    return folly::to<std::string>(flags.test_state_dir, "/", user, "-state");
  }

#ifdef _WIN32
  return getCachedWatchmanAppDataPath();
#else
  auto state_parent =
#ifdef WATCHMAN_STATE_DIR
      WATCHMAN_STATE_DIR
#else
      getTemporaryDirectory().c_str()
#endif
      ;
  return folly::to<std::string>(state_parent, "/", user, "-state");
#endif
}

} // namespace watchman
