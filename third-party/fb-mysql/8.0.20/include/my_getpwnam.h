/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MY_GETPWNAM_INCLUDED
#define MY_GETPWNAM_INCLUDED

/**
  @file include/my_getpwnam.h
*/
#include <string>
#include "my_config.h"

#ifdef HAVE_GETPWNAM
#include <pwd.h>
#include <sys/types.h>

/**
   Shadow struct for passwd which has proper value semantics, so that
   it can be safely copied and assigned to.
 */
struct PasswdValue {
  std::string pw_name;
  std::string pw_passwd;
  uid_t pw_uid{0};
  gid_t pw_gid{0};
  std::string pw_gecos;
  std::string pw_dir;
  std::string pw_shell;

  /** Constructs from a passwd instance. */
  PasswdValue(const passwd &p)
      : pw_name{p.pw_name},
        pw_passwd{p.pw_passwd},
        pw_uid{p.pw_uid},
        pw_gid{p.pw_gid},
        pw_gecos{p.pw_gecos},
        pw_dir{p.pw_dir},
        pw_shell{p.pw_shell} {}

  /** Default constructor creates a void value. */
  PasswdValue() = default;

  /**
     Returns true if this PasswdValue instance does not represent a
     real passwd entry.
  */
  bool IsVoid() const { return pw_name.empty(); }
};

PasswdValue my_getpwnam(const char *);
PasswdValue my_getpwuid(uid_t);

#endif /* HAVE_GETPWNAM */

#endif  // MY_GETPWNAM_INCLUDED
