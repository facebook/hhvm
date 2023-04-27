/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <filesystem>
#include <stack>
#include <string>
#include <utility>

#include <grp.h>
#include <sys/stat.h>

#include <fmt/core.h>
#include <folly/Format.h>
#include <folly/hash/Hash.h>
#include <folly/logging/xlog.h>
#include <folly/portability/Fcntl.h>
#include <folly/portability/SysStat.h>

#include "hphp/runtime/ext/facts/sqlite-key.h"
#include "hphp/util/hash.h"
#include "hphp/util/optional.h"
#include "hphp/util/sqlite-wrapper.h"
#include "hphp/util/user-info.h"

namespace fs = std::filesystem;

namespace HPHP {
namespace Facts {

SQLiteKey SQLiteKey::readOnly(fs::path path) {
  return {
      std::move(path),
      SQLite::OpenMode::ReadOnly,
      static_cast<::gid_t>(-1),
      static_cast<::mode_t>(0)};
}

SQLiteKey SQLiteKey::readWrite(fs::path path) {
  return {
      std::move(path),
      SQLite::OpenMode::ReadWrite,
      static_cast<::gid_t>(-1),
      static_cast<::mode_t>(0)};
}

SQLiteKey
SQLiteKey::readWriteCreate(fs::path path, ::gid_t gid, ::mode_t perms) {
  return {std::move(path), SQLite::OpenMode::ReadWriteCreate, gid, perms};
}

bool SQLiteKey::operator==(const SQLiteKey& rhs) const noexcept {
  return m_path == rhs.m_path && m_writable == rhs.m_writable &&
      m_gid == rhs.m_gid && m_perms == rhs.m_perms;
}

namespace {

std::string formatGidForDebug(gid_t gid) {
  if (gid == static_cast<::gid_t>(-1)) {
    return "<unset>";
  }

  HPHP::UserInfo user(getuid());
  HPHP::GroupInfo group(gid);

  auto member = [&]() {
    const auto* username = user.pw->pw_name;
    if (user.pw->pw_gid == gid) {
      return folly::sformat("{} is a member", username);
    }

    int ngroups = 4096;
    gid_t groups[4096];
    if (getgrouplist(username, gid, groups, &ngroups) != -1) {
      for (int i = 0; i < ngroups; ++i) {
        if (gid == groups[i]) {
          return folly::sformat("{} is a member", username);
        }
      }
    } else {
      XLOGF(ERR, "Unabled to get group list for {}", username);
    }

    return folly::sformat("{} is not a member", username);
  }();

  return folly::sformat("{} (gid={}, {})", group.gr->gr_name, gid, member);
}

std::string getPermissionsForPath(const fs::path& path) {
  try {
    auto perms = static_cast<typename std::underlying_type<fs::perms>::type>(
        fs::status(path).permissions());
    if (perms == 0xffff) {
      return "unknown/may not exist";
    } else {
      return folly::sformat("{:04o}", perms);
    }
  } catch (...) {
  }
  return "??";
}

std::pair<std::string, std::string> getOwnerAndGroupForPath(
    const fs::path& path) {
  try {
    struct stat info;
    if (stat(path.native().c_str(), &info) == 0) {
      HPHP::UserInfo user(info.st_uid);
      HPHP::GroupInfo group(info.st_gid);
      return {user.pw->pw_name, group.gr->gr_name};
    }
  } catch (...) {
  }
  return {"??", "??"};
}

std::string getDirectoryTreeInformation(const fs::path& base_path) {
  std::stringstream ss;

  if (!base_path.has_root_path()) {
    LOG(INFO) << base_path.native() << " has no root path.";
    return "";
  }

  // Traverse the directory tree upwards to the root, finding all the
  // directories. We're doing this so we iterate next from root down.
  std::stack<fs::path> tree;
  fs::path path = base_path;
  while (true) {
    tree.push(path);
    if (path == path.root_path() || path.native().empty()) {
      break;
    }
    path = path.parent_path();
  }

  ss << "\n";
  while (!tree.empty()) {
    path = tree.top();
    try {
      auto permissions = getPermissionsForPath(path);
      auto [username, groupname] = getOwnerAndGroupForPath(path);
      XLOGF(
          ERR,
          "Database Permissions: {:<4} {:<16} {:<16} {}",
          permissions,
          username,
          groupname,
          path.native());

      ss << folly::sformat(
          "({} -> {} {}/{})\n",
          path.native(),
          permissions,
          username,
          groupname);
    } catch (std::exception& e) {
      XLOGF(
          ERR,
          "Exception thrown while getting directory permissions: {}",
          e.what());
      return folly::sformat("<error: {}>", e.what());
    }
    tree.pop();
  }

  return ss.str();
}

} // namespace

std::string SQLiteKey::toString() const {
  return folly::sformat(
      "SQLiteKey({}, {}, {})", m_path.native(), m_gid, m_perms);
}

std::string SQLiteKey::toDebugString() const {
  return folly::sformat(
      "SQLiteKey(Path: {}, Group: {}, Create Permissions: {:04o}, Mode: {}, "
      "Path Permissions: {}",
      m_path.native(),
      formatGidForDebug(m_gid),
      m_perms,
      SQLite::openModeName(m_writable),
      getDirectoryTreeInformation(m_path));
}

size_t SQLiteKey::hash() const {
  return folly::hash::hash_combine(
      hash_string_cs(m_path.native().c_str(), m_path.native().size()),
      std::hash<gid_t>{}(m_gid),
      std::hash<mode_t>{}(m_perms));
}

SQLiteKey::SQLiteKey(
    fs::path path,
    SQLite::OpenMode writable,
    ::gid_t gid,
    ::mode_t perms)
    : m_path{std::move(path)},
      m_writable{writable},
      m_gid{gid},
      m_perms{perms} {
  always_assert(m_path.is_absolute());

  // Coerce DB permissions into unix owner/group/other bits
  auto previous = m_perms;
  m_perms |= 0600;
  m_perms &= 0666;

  XLOGF_IF(
      INFO,
      previous != m_perms,
      "Coercing DB permission bits {:04o} to {:04o}",
      previous,
      m_perms);
}

} // namespace Facts
} // namespace HPHP
