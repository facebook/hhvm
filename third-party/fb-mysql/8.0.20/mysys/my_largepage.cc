/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_largepage.cc
*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/psi/mysql_file.h"
#include "mysys/mysys_priv.h"
#include "mysys_err.h"

static uint my_get_large_page_size_int(void);

/* Gets the size of large pages from the OS */

uint my_get_large_page_size(void) {
  uint size;
  DBUG_TRACE;

  if (!(size = my_get_large_page_size_int()))
    my_message_local(
        WARNING_LEVEL,
        EE_FAILED_TO_DETERMINE_LARGE_PAGE_SIZE); /* purecov: inspected */

  return size;
}

/* Linux-specific function to determine the size of large pages */

uint my_get_large_page_size_int(void) {
  MYSQL_FILE *f;
  uint size = 0;
  char buf[256];
  DBUG_TRACE;

  if (!(f = mysql_file_fopen(key_file_proc_meminfo, "/proc/meminfo", O_RDONLY,
                             MYF(MY_WME))))
    goto finish;

  while (mysql_file_fgets(buf, sizeof(buf), f))
    if (sscanf(buf, "Hugepagesize: %u kB", &size)) break;

  mysql_file_fclose(f, MYF(MY_WME));

finish:
  return size * 1024;
}
