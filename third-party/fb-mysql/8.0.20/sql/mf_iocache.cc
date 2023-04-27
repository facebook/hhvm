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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file

  @details
  Caching of files with only does (sequential) read or writes of fixed-
  length records. A read isn't allowed to go over file-length. A read is ok
  if it ends at file-length and next read can try to read after file-length
  (and get a EOF-error).
  Used instead of FILE when reading or writing whole files.
  One can change info->pos_in_file to a higher value to skip bytes in file if
  also info->rc_pos is set to info->rc_end.
  If called through open_cached_file(), then the temporary file will
  only be created if a write exeeds the file buffer or if one calls
  flush_io_cache().
*/

#include <stddef.h>
#include <sys/types.h>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "sql/current_thd.h"
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"  // THD

/**
  Read buffered from the net.

  @retval
    1   if can't read requested characters
  @retval
    0   if record read
*/

int _my_b_net_read(IO_CACHE *info, uchar *Buffer,
                   size_t Count MY_ATTRIBUTE((unused))) {
  ulong read_length;
  NET *net = current_thd->get_protocol_classic()->get_net();
  DBUG_TRACE;

  if (!info->end_of_file)
    return 1; /* because my_b_get (no _) takes 1 byte at a time */
  read_length = my_net_read(net);
  if (read_length == packet_error) {
    info->error = -1;
    return 1;
  }
  if (read_length == 0) {
    info->end_of_file = 0; /* End of file from client */
    return 1;
  }
  /* to set up stuff for my_b_get (no _) */
  info->read_end = (info->read_pos = net->read_pos) + read_length;
  Buffer[0] = info->read_pos[0]; /* length is always 1 */

  /*
    info->request_pos is used by log_loaded_block() to know the size
    of the current block.
    info->pos_in_file is used by log_loaded_block() too.
  */
  info->pos_in_file += read_length;
  info->request_pos = info->read_pos;

  info->read_pos++;

  return 0;
}
