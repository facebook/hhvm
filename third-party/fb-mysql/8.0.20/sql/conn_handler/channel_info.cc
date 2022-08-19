/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/conn_handler/channel_info.h"

#include <stddef.h>
#include <stdio.h>
#include <new>

#include "my_dbug.h"
#include "my_stacktrace.h"  // my_safe_snprintf
#include "mysql_com.h"
#include "sql/derror.h"  // ER_DEFAULT
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"  // THD
#include "violite.h"

THD *Channel_info::create_thd() {
  DBUG_EXECUTE_IF("simulate_resource_failure", return nullptr;);

  Vio *vio_tmp = create_and_init_vio();
  if (vio_tmp == nullptr) return nullptr;

  THD *thd = new (std::nothrow) THD;
  if (thd == nullptr) {
    vio_delete(vio_tmp);
    return nullptr;
  }

  thd->get_protocol_classic()->init_net(vio_tmp);

  return thd;
}

void Channel_info::send_error_and_close_channel(uint errorcode, int error,
                                                bool senderror) {
  DBUG_ASSERT(errorcode != 0);
  if (!errorcode) return;

  char error_message_buff[MYSQL_ERRMSG_SIZE];

  if (senderror) {
    NET net_tmp;
    Vio *vio_tmp = create_and_init_vio();

    if (vio_tmp && !my_net_init(&net_tmp, vio_tmp)) {
      if (error)
        snprintf(error_message_buff, sizeof(error_message_buff),
                 ER_DEFAULT_NONCONST(errorcode), error);
      net_send_error(
          &net_tmp, errorcode,
          error ? error_message_buff : ER_DEFAULT_NONCONST(errorcode));
      net_end(&net_tmp);
    }
    if (vio_tmp != nullptr) {
      vio_tmp->inactive = true;  // channel is already closed.
      vio_delete(vio_tmp);
    }
  } else  // fatal error like out of memory.
  {
    if (error)
      my_safe_snprintf(error_message_buff, sizeof(error_message_buff),
                       ER_DEFAULT_NONCONST(errorcode), error);
    my_safe_printf_stderr(
        "[Warning] %s\n",
        error ? error_message_buff : ER_DEFAULT_NONCONST(errorcode));
  }
}
