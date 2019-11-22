/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#if !defined(SKIP_USER_CHANGE)

#include "hphp/util/capability.h"
#include "hphp/util/logger.h"
#include "hphp/util/user-info.h"
#include <folly/String.h>
#include <linux/types.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static bool setInitialCapabilities() {
  cap_t cap_d = cap_init();
  if (cap_d != nullptr) {
    cap_value_t cap_list[] = {CAP_NET_BIND_SERVICE, CAP_SYS_RESOURCE,
                              CAP_SETUID, CAP_SETGID, CAP_SYS_NICE};
    constexpr unsigned cap_size = sizeof(cap_list)/sizeof(*cap_list);

    cap_clear(cap_d);

    if (cap_set_flag(cap_d, CAP_PERMITTED, cap_size, cap_list, CAP_SET) < 0 ||
        cap_set_flag(cap_d, CAP_EFFECTIVE, cap_size, cap_list, CAP_SET) < 0) {
      Logger::Error("cap_set_flag failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    if (cap_set_proc(cap_d) == -1) {
      Logger::Error("cap_set_proc failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    if (cap_free(cap_d) == -1) {
      Logger::Error("cap_free failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) < 0) {
      Logger::Error("prctl(PR_SET_KEEPCAPS) failed: %s",
                    folly::errnoStr(errno).c_str());
      return false;
    }
    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    return true;
  }
  return false;
}

static bool setMinimalCapabilities() {
  cap_t cap_d = cap_init();

  if (cap_d != nullptr) {
    cap_value_t cap_list[] = {CAP_NET_BIND_SERVICE, CAP_SYS_RESOURCE,
                              CAP_SYS_NICE};
    constexpr unsigned cap_size = sizeof(cap_list)/sizeof(*cap_list);

    cap_clear(cap_d);

    if (cap_set_flag(cap_d, CAP_PERMITTED, cap_size, cap_list, CAP_SET) < 0 ||
        cap_set_flag(cap_d, CAP_EFFECTIVE, cap_size, cap_list, CAP_SET) < 0) {
      Logger::Error("cap_set_flag failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    if (cap_set_proc(cap_d) == -1) {
      Logger::Error("cap_set_proc failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    if (cap_free(cap_d) == -1) {
      Logger::Error("cap_free failed: %s", folly::errnoStr(errno).c_str());
      return false;
    }

    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);
    return true;
  }
  return false;
}

bool Capability::ChangeUnixUser(uid_t uid, bool allowRoot) {
  if (uid == 0 && !allowRoot) {
    Logger::Error("unable to change user to root");
    return false;
  }

  if (uid == getuid()) {
    return true;
  }

  if (setInitialCapabilities()) {
    auto buf = PasswdBuffer{};
    struct passwd *pw;

    if (getpwuid_r(uid, &buf.ent, buf.data.get(), buf.size, &pw)) {
      Logger::Error("unable to getpwuid(%d): %s", uid,
                    folly::errnoStr(errno).c_str());
      return false;
    }
    if (pw == nullptr) {
      Logger::Error("user id %d does not exist", uid);
      return false;
    }

    if (initgroups(pw->pw_name, pw->pw_gid) < 0) {
      Logger::Error("unable to drop supplementary group privs: %s",
                    folly::errnoStr(errno).c_str());
      return false;
    }

    if (pw->pw_gid == 0 || setgid(pw->pw_gid) < 0) {
      Logger::Error("unable to drop gid privs: %s",
                    folly::errnoStr(errno).c_str());
      return false;
    }

    if (uid == 0 || setuid(uid) < 0) {
      Logger::Error("unable to drop uid privs: %s",
                    folly::errnoStr(errno).c_str());
      return false;
    }

    if (!setMinimalCapabilities()) {
      Logger::Error("unable to set minimal server capabiltiies");
      return false;
    }
    return true;
  }
  return false;
}

bool Capability::ChangeUnixUser(const std::string &username, bool allowRoot) {
  if (!username.empty()) {
    auto buf = PasswdBuffer{};
    struct passwd *pw;
    if (getpwnam_r(username.c_str(), &buf.ent, buf.data.get(), buf.size, &pw)) {
      Logger::Error("Call to getpwnam_r failed for %s: %s",
                    username.c_str(),
                    folly::errnoStr(errno).c_str());
      return false;
    }
    if (!pw) {
      Logger::Error("unable to find user %s", username.c_str());
      return false;
    }
    return ChangeUnixUser(pw->pw_uid, allowRoot);
  }
  return false;
}

bool Capability::SetDumpable() {
  if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0)) {
    Logger::Error("Unable to make process dumpable: %s",
                  folly::errnoStr(errno).c_str());
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
