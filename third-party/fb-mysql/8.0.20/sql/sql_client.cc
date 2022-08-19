/* Copyright (c) 2003, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <sys/types.h>
#include <algorithm>

#include "mysql_com.h"
#include "sql/mysqld.h"  // global_system_variables
#include "sql/system_variables.h"

using std::max;
using std::min;

/*
  Function called by my_net_init() to set some check variables
*/

void my_net_local_init(NET *net) {
  net->max_packet = (uint)global_system_variables.net_buffer_length;

  my_net_set_read_timeout(
      net,
      timeout_from_seconds((uint)global_system_variables.net_read_timeout));
  my_net_set_write_timeout(
      net,
      timeout_from_seconds((uint)global_system_variables.net_write_timeout));

  net->retry_count = (uint)global_system_variables.net_retry_count;
  net->max_packet_size =
      max<size_t>(global_system_variables.net_buffer_length,
                  global_system_variables.max_allowed_packet);
}
