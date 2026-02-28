/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MOCKLOGGER_H
#define MOCKLOGGER_H

#include <gmock/gmock.h>

#include <sql/derror.h>
#include "plugin/keyring/common/logger.h"

namespace keyring {
class Mock_logger : public ILogger {
 public:
  MOCK_METHOD2(log, void(longlong level, const char *msg));

  void log(longlong level, longlong errcode, ...) {
    char buf[LOG_BUFF_MAX];
    const char *fmt = error_message_for_error_log(errcode);

    va_list vl;
    va_start(vl, errcode);
    vsnprintf(buf, LOG_BUFF_MAX - 1, fmt, vl);
    va_end(vl);

    log(level, buf);
  }
};
}  // namespace keyring
#endif  // MOCKLOGGER_H
