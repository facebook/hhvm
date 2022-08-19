#ifndef UNITTEST_GUNIT_MYSYS_UTIL_H_
#define UNITTEST_GUNIT_MYSYS_UTIL_H_
/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys_util.h
  Utilities for working with mysys.
*/

#include "my_inttypes.h"
#include "mysql_time.h"

/// Adds sensible constructors to MYSQL_TIME.
struct MysqlTime : public MYSQL_TIME {
  MysqlTime() : MysqlTime(0, 0, 0) {}

  MysqlTime(uint year_arg, uint month_arg, uint day_arg)
      : MysqlTime(year_arg, month_arg, day_arg, 0, 0, 0, 0) {}

  MysqlTime(uint year_arg, uint month_arg, uint day_arg, uint hour_arg,
            uint minute_arg, uint second_arg, ulong microsecond_arg)
      : MysqlTime(year_arg, month_arg, day_arg, hour_arg, minute_arg,
                  second_arg, microsecond_arg, false, MYSQL_TIMESTAMP_DATE) {}

  MysqlTime(uint year_arg, uint month_arg, uint day_arg, uint hour_arg,
            uint minute_arg, uint second_arg, ulong microsecond_arg,
            int time_zone_displacement_arg)
      : MYSQL_TIME{year_arg,
                   month_arg,
                   day_arg,
                   hour_arg,
                   minute_arg,
                   second_arg,
                   microsecond_arg,
                   false,
                   MYSQL_TIMESTAMP_DATETIME_TZ,
                   time_zone_displacement_arg} {}

  MysqlTime(uint year_arg, uint month_arg, uint day_arg, uint hour_arg,
            uint minute_arg, uint second_arg, ulong microsecond_arg,
            bool isNegative, enum_mysql_timestamp_type type)
      : MYSQL_TIME{year_arg,
                   month_arg,
                   day_arg,
                   hour_arg,
                   minute_arg,
                   second_arg,
                   microsecond_arg /* second_arg_part */,
                   isNegative /* neg */,
                   type /* time_type */,
                   0 /* time_zone_displacement_arg */} {}
};

#endif  // UNITTEST_GUNIT_MYSYS_UTIL_H_
