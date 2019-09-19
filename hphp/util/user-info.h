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

#pragma once

#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <vector>

#include <folly/String.h>

#include "hphp/util/exception.h"

namespace HPHP {

struct UserInfo final {
  explicit UserInfo(const char* name) {
    ::passwd* retpwptr = nullptr;
    int pwbuflen = ::sysconf(_SC_GETPW_R_SIZE_MAX);
    if (pwbuflen < 1)   {
      throw Exception("Could not get _SC_GETPW_R_SIZE_MAX");
    }
    pwbuf.resize(pwbuflen);

    if (::getpwnam_r(name, &pwd, pwbuf.data(), pwbuf.size(), &retpwptr)) {
      throw Exception("getpwnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retpwptr) {
      throw Exception("getpwnam_r: no such user: %s", name);
    }
  }

  explicit UserInfo(uid_t uid) {
    ::passwd* retpwptr = nullptr;
    int pwbuflen = ::sysconf(_SC_GETPW_R_SIZE_MAX);
    if (pwbuflen < 1)   {
      throw Exception("Could not get _SC_GETPW_R_SIZE_MAX");
    }
    pwbuf.resize(pwbuflen);

    if (::getpwuid_r(uid, &pwd, pwbuf.data(), pwbuf.size(), &retpwptr)) {
      throw Exception("getpwuid_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retpwptr) {
      throw Exception("getpwuid_r: no such uid: %u", uid);
    }
  }

  ::passwd pwd;
  std::vector<char> pwbuf;
};

struct GroupInfo final {
  explicit GroupInfo(const char* name) {
    ::group* retgrptr = nullptr;
    int grbuflen = ::sysconf(_SC_GETGR_R_SIZE_MAX);
    if (grbuflen < 1)   {
      throw Exception("Could not get _SC_GETGR_R_SIZE_MAX");
    }
    grbuf.resize(grbuflen);

    if (::getgrnam_r(name, &gr, grbuf.data(), grbuf.size(), &retgrptr)) {
      throw Exception("getgrnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (!retgrptr) {
      throw Exception("getgrnam_r: no such group: %s", name);
    }
  }

  ::group gr;
  std::vector<char> grbuf;
};

} // namespace HPHP
