#ifndef SQL_EXCEPTION_HANDLER_H_INCLUDED
#define SQL_EXCEPTION_HANDLER_H_INCLUDED

/*
  Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @brief This file declares functions to convert exceptions to MySQL
  error messages.

  The pattern for use in other functions is:

  @code
  try
  {
    something_that_throws();
  }
  catch (...)
  {
    handle_foo_exception("function_name");
  }
  @endcode

  There are different handlers for different use cases.
*/

/**
  Handle an exception of any type.

  Code that could throw exceptions should be wrapped in try/catch, and
  the catch block should raise a corresponding MySQL error. If this
  function is called from the catch block, it will raise a specialized
  error message for many of the std::exception subclasses, or a more
  generic error message if it is not a std::exception.

  @param funcname the name of the function that caught an exception

  @see handle_gis_exception
*/
void handle_std_exception(const char *funcname);

/**
  Handle a GIS exception of any type.

  This function constitutes the exception handling barrier between
  Boost.Geometry and MySQL code. It handles all exceptions thrown in
  GIS code and raises the corresponding error in MySQL.

  Pattern for use in other functions:

  @code
  try
  {
    something_that_throws();
  }
  catch (...)
  {
    handle_gis_exception("st_foo");
  }
  @endcode

  Other exception handling code put into the catch block, before or
  after the call to handle_gis_exception(), must not throw exceptions.

  @param funcname Function name for use in error message

  @see handle_std_exception
 */
void handle_gis_exception(const char *funcname);

#endif  // SQL_EXCEPTION_HANDLER_H_INCLUDED
