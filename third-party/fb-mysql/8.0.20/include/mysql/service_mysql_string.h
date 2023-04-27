/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

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
  @file include/mysql/service_mysql_string.h
  This service provides functions to parse mysql String.
*/

#ifndef MYSQL_SERVICE_MYSQL_STRING_INCLUDED
#define MYSQL_SERVICE_MYSQL_STRING_INCLUDED

#ifndef MYSQL_ABI_CHECK
#include <stdlib.h>
#endif

typedef void *mysql_string_iterator_handle;
typedef void *mysql_string_handle;

extern "C" struct mysql_string_service_st {
  int (*mysql_string_convert_to_char_ptr_type)(mysql_string_handle,
                                               const char *, char *,
                                               unsigned int, int *);
  mysql_string_iterator_handle (*mysql_string_get_iterator_type)(
      mysql_string_handle);
  int (*mysql_string_iterator_next_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_isupper_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_islower_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_isdigit_type)(mysql_string_iterator_handle);
  mysql_string_handle (*mysql_string_to_lowercase_type)(mysql_string_handle);
  void (*mysql_string_free_type)(mysql_string_handle);
  void (*mysql_string_iterator_free_type)(mysql_string_iterator_handle);
} * mysql_string_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define mysql_string_convert_to_char_ptr(string_handle, charset_name, buffer, \
                                         buffer_size, error)                  \
  mysql_string_service->mysql_string_convert_to_char_ptr_type(                \
      string_handle, charset_name, buffer, buffer_size, error)

#define mysql_string_get_iterator(string_handle) \
  mysql_string_service->mysql_string_get_iterator_type(string_handle)

#define mysql_string_iterator_next(iterator_handle) \
  mysql_string_service->mysql_string_iterator_next_type(iterator_handle)

#define mysql_string_iterator_isupper(iterator_handle) \
  mysql_string_service->mysql_string_iterator_isupper_type(iterator_handle)

#define mysql_string_iterator_islower(iterator_handle) \
  mysql_string_service->mysql_string_iterator_islower_type(iterator_handle)

#define mysql_string_iterator_isdigit(iterator_handle) \
  mysql_string_service->mysql_string_iterator_isdigit_type(iterator_handle)

#define mysql_string_to_lowercase(string_handle) \
  mysql_string_service->mysql_string_to_lowercase_type(string_handle)

#define mysql_string_free(mysql_string_handle) \
  mysql_string_service->mysql_string_free_type(mysql_string_handle)

#define mysql_string_iterator_free(mysql_string_iterator_handle) \
  mysql_string_service->mysql_string_iterator_free_type(         \
      mysql_string_iterator_handle)
#else

/* This service function convert string into given character set */
int mysql_string_convert_to_char_ptr(mysql_string_handle string_handle,
                                     const char *charset_name, char *buffer,
                                     unsigned int buffer_size, int *error);

/* This service function returns the beginning of the iterator handle */
mysql_string_iterator_handle mysql_string_get_iterator(
    mysql_string_handle string_handle);
/*
  This service function gets the next iterator handle
  returns 0 if reached the end else return 1
*/
int mysql_string_iterator_next(mysql_string_iterator_handle iterator_handle);

/*
  This service function return 1 if current iterator handle points to a
  uppercase character else return 0 for client character set.
*/
int mysql_string_iterator_isupper(mysql_string_iterator_handle iterator_handle);

/*
  This service function return 1 if current iterator handle points to a
  lowercase character else return 0 for client character set.
*/
int mysql_string_iterator_islower(mysql_string_iterator_handle iterator_handle);

/*
  This service function return 1 if current iterator handle points to a digit
  else return 0 for client character sets.
*/
int mysql_string_iterator_isdigit(mysql_string_iterator_handle iterator_handle);

/* convert string_handle into lowercase */
mysql_string_handle mysql_string_to_lowercase(
    mysql_string_handle string_handle);

/* It deallocates the string created on server side during plugin operations */
void mysql_string_free(mysql_string_handle);

/*
  It deallocates the string iterator created on server side
  during plugin operations
*/
void mysql_string_iterator_free(mysql_string_iterator_handle);

#endif

#endif
