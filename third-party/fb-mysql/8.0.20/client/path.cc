/*
   Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "client/path.h"

#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"

#ifndef _WIN32
#include <pwd.h>
#endif

Path::Path() {}

Path::Path(const std::string &s) { path(s); }

Path::Path(const Path &p) {
  m_filename = p.m_filename;
  m_path = p.m_path;
}

bool Path::path_getcwd() {
  char path[FN_REFLEN];
  if (my_getwd(path, FN_REFLEN - 1, MYF(MY_WME))) return false;
  m_path.clear();
  m_path.append(path);
  trim();
  return true;
}

void Path::trim() {
  if (m_path.length() <= 1) return;
  std::string::iterator it = m_path.end();
  --it;

  while ((*it) == FN_LIBCHAR && m_path.length() > 1) {
    m_path.erase(it--);
  }
}

void Path::parent_directory(Path *out) {
  size_t idx = m_path.rfind(FN_DIRSEP);
  if (idx == std::string::npos) {
    out->path("");
  } else
    out->path(m_path.substr(0, idx));
}

Path &Path::up() {
  size_t idx = m_path.rfind(FN_DIRSEP);
  if (idx == std::string::npos) {
    m_path.clear();
  } else
    m_path.assign(m_path.substr(0, idx));
  return *this;
}

Path &Path::append(const std::string &path) {
  if (m_path.length() > 1 && path[0] != FN_LIBCHAR) m_path.append(FN_DIRSEP);
  m_path.append(path);
  trim();
  return *this;
}

Path &Path::filename_append(const std::string &ext) {
  m_filename.append(ext);
  trim();
  return *this;
}

void Path::path(const std::string &p) {
  m_path.clear();
  m_path.append(p);
  trim();
}

void Path::filename(const std::string &f) { m_filename = f; }

void Path::path(const Path &p) { path(p.m_path); }

void Path::filename(const Path &p) { path(p.m_filename); }

bool Path::qpath(const std::string &qp) {
  size_t idx = qp.rfind(FN_DIRSEP);
  if (idx == std::string::npos) {
    m_filename = qp;
    m_path.clear();
  } else {
    filename(qp.substr(idx + 1, qp.size() - idx));
    path(qp.substr(0, idx));
  }
  if (is_qualified_path())
    return true;
  else
    return false;
}

bool Path::normalize_path() {
  if (!m_path.length()) return false;

  char real_path[FN_REFLEN];
  size_t real_path_len;

  real_path_len = cleanup_dirname(real_path, m_path.c_str());
  if (real_path_len > FN_REFLEN) return false;

  if (my_realpath(real_path, real_path, MYF(0))) return false;

  m_path.clear();
  m_path.append(real_path);
  trim();

  return true;
}

bool Path::is_qualified_path() { return m_filename.length() > 0; }

bool Path::exists() {
  if (!is_qualified_path()) {
    MY_DIR *dir = my_dir(m_path.c_str(), MY_WANT_STAT | MY_DONT_SORT);
    if (dir == nullptr) return false;
    my_dirend(dir);
    return true;
  } else {
    MY_STAT s;
    std::string qpath(m_path);
    qpath.append(FN_DIRSEP).append(m_filename);
    if (my_stat(qpath.c_str(), &s, MYF(0)) == nullptr) return false;
    if (!MY_S_ISREG(s.st_mode)) return false;
    return true;
  }
}

const std::string Path::to_str() {
  std::string qpath(m_path);
  if (m_filename.length() != 0) {
    qpath.append(FN_DIRSEP);
    qpath.append(m_filename);
  }
  return qpath;
}

bool Path::empty() {
  if (!exists()) return true;
  MY_DIR *dir;
  bool ret = false;
  if (!(dir = my_dir(m_path.c_str(), MY_WANT_STAT | MY_DONT_SORT)))
    ret = false;
  else {
    if (dir->number_off_files == 2) ret = true;
  }
  my_dirend(dir);
  return ret;
}

#ifndef _WIN32
void Path::get_homedir() {
  struct passwd *pwd;
  pwd = getpwuid(geteuid());
  if (pwd == nullptr) return;
  if (pwd->pw_dir != nullptr)
    path(pwd->pw_dir);
  else
    path("");
}
#endif

std::ostream &operator<<(std::ostream &op, const Path &p) {
  std::string qpath(p.m_path);
  if (p.m_filename.length() != 0) {
    qpath.append(FN_DIRSEP);
    qpath.append(p.m_filename);
  }
  return op << qpath;
}
