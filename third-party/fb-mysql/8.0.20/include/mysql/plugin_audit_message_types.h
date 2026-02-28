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

#ifndef PLUGIN_AUDIT_MESSAGE_TYPES_H
#define PLUGIN_AUDIT_MESSAGE_TYPES_H

#ifndef MYSQL_ABI_CHECK
#include <mysql/mysql_lex_string.h>
#endif

/**
  @enum mysql_event_message_subclass_t

  Events for MYSQL_AUDIT_MESSAGE_CLASS event class.
*/
typedef enum {
  /** Internally generated message. */
  MYSQL_AUDIT_MESSAGE_INTERNAL = 1 << 0,
  /** User generated message. */
  MYSQL_AUDIT_MESSAGE_USER = 1 << 1,
} mysql_event_message_subclass_t;

/**
  @enum mysql_event_message_value_type_t

  Type of the value element of the key-value pair.
*/
typedef enum {
  /** Value is of the string type. */
  MYSQL_AUDIT_MESSAGE_VALUE_TYPE_STR = 0,
  /** Value is of the numeric type. */
  MYSQL_AUDIT_MESSAGE_VALUE_TYPE_NUM = 1,
} mysql_event_message_value_type_t;

/**
  Structure that stores key-value pair of the MYSQL_AUDIT_MESSAGE_CLASS
  event class.
*/
typedef struct {
  /** Key element. */
  MYSQL_LEX_CSTRING key;
  /** Value element type. */
  mysql_event_message_value_type_t value_type;
  /** Value element. */
  union {
    /** String element. */
    MYSQL_LEX_CSTRING str;
    /** Numeric element. */
    long long num;
  } value;
} mysql_event_message_key_value_t;

#endif /* PLUGIN_AUDIT_MESSAGE_TYPES_H */
