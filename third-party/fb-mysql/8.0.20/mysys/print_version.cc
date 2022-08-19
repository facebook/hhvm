/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

  @file mysys/print_version.cc
*/

#include "print_version.h"

#include "my_config.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include "m_string.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysql_version.h"

#ifdef MYSQL_SERVER_SUFFIX
#define MYSQL_SERVER_SUFFIX_STR STRINGIFY_ARG(MYSQL_SERVER_SUFFIX)
#else
#define MYSQL_SERVER_SUFFIX_STR MYSQL_SERVER_SUFFIX_DEF
#endif

void print_version() {
  char version_buffer[SERVER_VERSION_LENGTH];
  strxmov(version_buffer, MYSQL_SERVER_VERSION, MYSQL_SERVER_SUFFIX_STR, NullS);
  printf("%s  Ver %s for %s on %s (%s)\n", my_progname, version_buffer,
         SYSTEM_TYPE, MACHINE_TYPE, MYSQL_COMPILATION_COMMENT_SERVER);
}

void print_version_debug() {
  char version_buffer[SERVER_VERSION_LENGTH];
  strxmov(version_buffer, MYSQL_SERVER_VERSION, MYSQL_SERVER_SUFFIX_STR, NullS);
  printf("%s  Ver %s-debug for %s on %s (%s)\n", my_progname, version_buffer,
         SYSTEM_TYPE, MACHINE_TYPE, MYSQL_COMPILATION_COMMENT_SERVER);
}

void print_explicit_version(const char *version) {
  printf("%s  Ver %s for %s on %s (%s)\n", my_progname, version, SYSTEM_TYPE,
         MACHINE_TYPE, MYSQL_COMPILATION_COMMENT_SERVER);
}

void build_version(const std::string &progname, std::string *destination) {
  std::ostringstream output_buffer;
  output_buffer << progname << "  Ver " << MYSQL_SERVER_VERSION
                << MYSQL_SERVER_SUFFIX_STR << " for " << SYSTEM_TYPE << " on "
                << MACHINE_TYPE << " (" << MYSQL_COMPILATION_COMMENT << ")";
  *destination = output_buffer.str();
}
