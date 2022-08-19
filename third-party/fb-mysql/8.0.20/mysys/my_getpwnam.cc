/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_getpwnam.cc
*/

#include "my_getpwnam.h"

#include <atomic>
#include <vector>

#include <errno.h>
#include <unistd.h>

namespace {
std::size_t start_bufsz() {
  long scsz = sysconf(_SC_GETPW_R_SIZE_MAX);
  return (scsz == -1L ? 256 : scsz);
}

template <class GETPW_CLOS>
PasswdValue my_getpw_(GETPW_CLOS &&getpwfunc) {
  passwd pwd;
  std::size_t bufsz = start_bufsz();
  std::vector<char> buf(bufsz);
  passwd *resptr = nullptr;

  while (true) {
    errno = getpwfunc(&pwd, &buf, &resptr);

    switch (errno) {
      case ERANGE:
        bufsz *= 2;
        buf.resize(bufsz);
        // fallthrough
      case EINTR:
        continue;
      default:
        break;
    }
    break;
  }

  return resptr ? PasswdValue{pwd} : PasswdValue{};
}
}  // namespace

/**
   Wrapper around the getpwnam_r() POSIX function which places the
   contents of the passwd struct into an object with value semantics
   and returns this.

   @param name Symbolic user id

   @retval PasswdValue representing user's passwd entry.
   PasswdValue::IsVoid() returns true if no such user exists or an error
   occured. In the latter case errno is set.
 */
PasswdValue my_getpwnam(const char *name) {
  return my_getpw_(
      [&name](passwd *pwd, std::vector<char> *bufp, passwd **resptr) {
        return getpwnam_r(name, pwd, &bufp->front(), bufp->size(), resptr);
      });
}

/**
   Wrapper around the getpwuid_r() POSIX function which places the
   contents of the passwd struct into an object with value semantics
   and returns this.

   @param uid Numeric user id

   @retval PasswdValue representing user's passwd entry.
   PasswdValue::IsVoid() returns true if no such user exists or an error
   occured. In the latter case errno is set.
 */
PasswdValue my_getpwuid(uid_t uid) {
  return my_getpw_(
      [uid](passwd *pwd, std::vector<char> *bufp, passwd **resptr) {
        return getpwuid_r(uid, pwd, &bufp->front(), bufp->size(), resptr);
      });
}
