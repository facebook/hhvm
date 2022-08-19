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

#ifndef _mysql_time_h_
#define _mysql_time_h_

/**
  @file include/mysql_time.h
  Time declarations shared between the server and client API:
  you should not add anything to this header unless it's used
  (and hence should be visible) in mysql.h.
  If you're looking for a place to add new time-related declaration,
  it's most likely my_time.h. See also "C API Handling of Date
  and Time Values" chapter in documentation.
*/

// Do not not pull in the server header "my_inttypes.h" from client code.
// IWYU pragma: no_include "my_inttypes.h"

enum enum_mysql_timestamp_type {
  MYSQL_TIMESTAMP_NONE = -2,
  MYSQL_TIMESTAMP_ERROR = -1,

  /// Stores year, month and day components.
  MYSQL_TIMESTAMP_DATE = 0,

  /**
    Stores all date and time components.
    Value is in UTC for `TIMESTAMP` type.
    Value is in local time zone for `DATETIME` type.
  */
  MYSQL_TIMESTAMP_DATETIME = 1,

  /// Stores hour, minute, second and microsecond.
  MYSQL_TIMESTAMP_TIME = 2,

  /**
    A temporary type for `DATETIME` or `TIMESTAMP` types equipped with time
    zone information. After the time zone information is reconciled, the type is
    converted to MYSQL_TIMESTAMP_DATETIME.
  */
  MYSQL_TIMESTAMP_DATETIME_TZ = 3
};

/*
  Structure which is used to represent datetime values inside MySQL.

  We assume that values in this structure are normalized, i.e. year <= 9999,
  month <= 12, day <= 31, hour <= 23, hour <= 59, hour <= 59. Many functions
  in server such as my_system_gmt_sec() or make_time() family of functions
  rely on this (actually now usage of make_*() family relies on a bit weaker
  restriction). Also functions that produce MYSQL_TIME as result ensure this.
  There is one exception to this rule though if this structure holds time
  value (time_type == MYSQL_TIMESTAMP_TIME) days and hour member can hold
  bigger values.
*/
typedef struct MYSQL_TIME {
  unsigned int year, month, day, hour, minute, second;
  unsigned long second_part; /**< microseconds */
  bool neg;
  enum enum_mysql_timestamp_type time_type;
  /// The time zone displacement, specified in seconds.
  int time_zone_displacement;
} MYSQL_TIME;

#endif /* _mysql_time_h_ */
