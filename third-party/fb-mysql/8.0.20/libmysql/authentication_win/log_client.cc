/* Copyright (c) 2011, 2016, Oracle and/or its affiliates. All rights reserved.

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

#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "my_loglevel.h"

// Client-side logging function

void error_log_vprint(error_log_level::type level, const char *fmt,
                      va_list args) {
  const char *level_string = "";
  int log_level = get_log_level();

  switch (level) {
    case error_log_level::INFO:
      if (3 > log_level) return;
      level_string = "Note";
      break;
    case error_log_level::WARNING:
      if (2 > log_level) return;
      level_string = "Warning";
      break;
    case error_log_level::ERROR:
      if (1 > log_level) return;
      level_string = "ERROR";
      break;
  }

  fprintf(stderr, "Windows Authentication Plugin %s: ", level_string);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  fflush(stderr);
}

// Trivial implementation of log-level setting storage.

void set_log_level(unsigned int level) { opt_auth_win_log_level = level; }

unsigned int get_log_level(void) { return opt_auth_win_log_level; }
