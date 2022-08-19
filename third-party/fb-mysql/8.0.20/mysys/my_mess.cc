/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_mess.cc
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"

/**
  Print an error message on stderr.
  Prefixed with the binary's name (sans .exe, where applicable,
  and without path, both to keep our test cases sane).
  The name is intended to aid debugging by clarifying which
  binary reported an error, especially in cases like mysql_upgrade
  which calls several other tools whose messages should be
  distinguishable from each other's, and from mysql_upgrade's.

  This is low-level; in most cases, you should use my_message_local()
  instead (which by default goes through my_message_local_stderr(),
  which is a wrapper around this function that adds a severity level).

  @param error    The error number. Currently unused.
  @param str      The message to print. Not trailing \n needed.
  @param MyFlags  ME_BELL to beep, or 0.
*/
void my_message_stderr(uint error MY_ATTRIBUTE((unused)), const char *str,
                       myf MyFlags) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("message: %s", str));
  (void)fflush(stdout);
  if (MyFlags & ME_BELL) (void)fputc('\007', stderr);
  if (my_progname) {
    size_t l;
    const char *r;

    if ((r = strrchr(my_progname, FN_LIBCHAR)))
      r++;
    else
      r = my_progname;

    l = strlen(r);
#ifdef _WIN32
    if ((l > 4) && !strcmp(&r[l - 4], ".exe"))
      l -= 4; /* purecov: inspected */ /* Windows-only */
#endif
    fprintf(stderr, "%.*s: ", (int)l, r);
  }
  (void)fputs(str, stderr);
  (void)fputc('\n', stderr);
  (void)fflush(stderr);
}
