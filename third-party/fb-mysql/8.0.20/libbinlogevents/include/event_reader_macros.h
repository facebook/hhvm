/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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
  @addtogroup Replication
  @{

  @file event_reader_macros.h

  @brief Contains macros used by libbinlogevents deserialization.
*/

#ifndef EVENT_READER_MACROS_INCLUDED
#define EVENT_READER_MACROS_INCLUDED

namespace binary_log {

/*
  Macro to be sourced by all event constructors to skip trying to deserialize a
  buffer that already found errors.
*/
#define READER_TRY_INITIALIZATION \
  if (!header()->get_is_valid()) BAPI_VOID_RETURN

/*
  Macro to be sourced by all event constructors to skip trying to deserialize a
  buffer once finding errors.
*/
#define READER_CATCH_ERROR \
  event_reader_footer:     \
  header()->set_is_valid(READER_CALL(has_error) == false)

/*
  Macro to be used to wrap calls to Event_reader functions that does not
  return values (void) or when the returned value is not needed.
*/
#define READER_CALL(func, ...) reader().func(__VA_ARGS__)

/*
  Same as READER_CALL, but checking if Event_reader entered error state.
*/
#define READER_TRY_CALL(func, ...) \
  READER_CALL(func, __VA_ARGS__);  \
  if (reader().get_error()) goto event_reader_footer

/*
  Macro to be used to wrap calls to Event_reader functions that return values.
*/
#define READER_SET(var, func, ...)                     \
  BAPI_PRINT("debug", ("Event_reader::SET %s", #var)); \
  var = reader().func(__VA_ARGS__)

/*
  Same as READER_SET, but checking if Event_reader entered error state.
*/
#define READER_TRY_SET(var, func, ...) \
  READER_SET(var, func, __VA_ARGS__);  \
  if (reader().get_error()) goto event_reader_footer

/*
  Macro to be used when event deserialization find illegal values or conditions.
*/
#define READER_THROW(message)    \
  {                              \
    reader().set_error(message); \
    goto event_reader_footer;    \
  }

/*
  Macro to assert that the cursor is in a specified position.
*/
#define READER_ASSERT_POSITION(pos) BAPI_ASSERT(READER_CALL(position) == pos)

}  // end namespace binary_log
/**
  @} (end of group Replication)
*/
#endif /* EVENT_READER_MACROS_INCLUDED */
