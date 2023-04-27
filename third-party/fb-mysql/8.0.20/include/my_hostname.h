/*
 * Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

/**
  @file include/my_hostname.h
  Common definition used by mysys, performance schema and server & client.
*/

#ifndef MY_HOSTNAME_INCLUDED
#define MY_HOSTNAME_INCLUDED

/*
   Note that this is the actual length available, and that room
   for a trailing '\0' is not included in this size.

   The metadata columns storing hostname use single byte character
   as they are using ASCII character set.

   pre 8.0.17: 60 characters
*/
static constexpr int HOSTNAME_LENGTH = 255;

/*
  Number of characters that a port number can have.
  As per rfc6335 section 6, the range is 0 to 65535.
*/
static constexpr int PORTNUMBER_LENGTH = 5;

/**
  Number of bytes required to hold "hostname:portnumber"
*/
static constexpr int HOST_AND_PORT_LENGTH =
    (HOSTNAME_LENGTH + 1 + PORTNUMBER_LENGTH + 1);

#endif /* MY_HOSTNAME_INCLUDED */
