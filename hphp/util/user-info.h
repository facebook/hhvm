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

template <typename Entry>
struct PwdGrpBuffer {
  explicit PwdGrpBuffer(int name) : size{sysconf(name)} {
    if (size < 1) size = 1024;
    data = std::make_unique<char[]>(size);
  }

  Entry ent;
  std::unique_ptr<char[]> data;
  long size;
};

struct PasswdBuffer : PwdGrpBuffer<passwd> {
  explicit PasswdBuffer() : PwdGrpBuffer(_SC_GETPW_R_SIZE_MAX) {}
};

struct GroupBuffer : PwdGrpBuffer<group> {
  explicit GroupBuffer() : PwdGrpBuffer(_SC_GETGR_R_SIZE_MAX) {}
};

struct UserInfo final {
  explicit UserInfo(const char* name) {
    if (getpwnam_r(name, &buf.ent, buf.data.get(), buf.size, &pw)) {
      throw Exception("getpwnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (pw == nullptr) {
      throw Exception("getpwnam_r: no such user: %s", name);
    }
  }

  explicit UserInfo(uid_t uid) {
    if (getpwuid_r(uid, &buf.ent, buf.data.get(), buf.size, &pw)) {
      throw Exception("getpwuid_r: %s", folly::errnoStr(errno).c_str());
    }

    if (pw == nullptr) {
      throw Exception("getpwuid_r: no such uid: %u", uid);
    }
  }

  PasswdBuffer buf;
  passwd* pw;
};

struct GroupInfo final {
  explicit GroupInfo(const char* name) {
    if (getgrnam_r(name, &buf.ent, buf.data.get(), buf.size, &gr)) {
      throw Exception("getgrnam_r: %s", folly::errnoStr(errno).c_str());
    }

    if (gr == nullptr) {
      throw Exception("getgrnam_r: no such group: %s", name);
    }
  }

  GroupBuffer buf;
  group* gr;
};

} // namespace HPHP
