/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__UTILS_INCLUDED
#define DD__UTILS_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/tztime.h"          // my_time_t

struct CHARSET_INFO;

class THD;

namespace dd {
/**
  Create a lex string for the query from the string supplied
  and execute the query.

  @param thd     Thread handle.
  @param q_buf   String containing the query text.

  @retval false  Success.
  @retval true   Error.
*/
bool execute_query(THD *thd, const dd::String_type &q_buf);

///////////////////////////////////////////////////////////////////////////

class Properties;

/**
  Escaping of a String_type. Escapable characters are '\', '=' and
  ';'. Escape character is '\'. Iterate over all characters of src,
  precede all escapable characters by the escape character and append
  to dst. The source string is not modified.

  @param[out] dst string to which escaped result will be appended.
  @param[in] src source string for escaping
*/
void escape(String_type *dst, const String_type &src);

/**
  In place unescaping of String_type. Escapable characters are '\', '='
  and ';'. Escape character is '\'. Iterate over all characters, remove
  escape character if it precedes an escapable character.

  @param[in,out] dest source and destination string for escaping
  @return             Operation status
    @retval true      if an escapable character is not escaped
    @retval false     if success
*/
bool unescape(String_type &dest);

/**
  Start at it, iterate until we hit an unescaped c or the end
  of the string. The stop character c may be ';' or '='. The
  escape character is '\'. Escapable characters are '\', '='
  and ';'. Hitting an unescaped '=' while searching for ';' is
  an error, and also the opposite. Hitting end of string while
  searching for '=' is an error, but end of string while
  searching for ';' is ok.

  In the event of success, the iterator will be at the
  character to be searched for, or at the end of the string.

  @param[in,out] it  string iterator
  @param         end iterator pointing to string end
  @param         c   character to search for

  @return            Operation status
    @retval true     if an error occurred
    @retval false    if success
*/
bool eat_to(String_type::const_iterator &it, String_type::const_iterator end,
            char c);

/**
  Start at it, find first unescaped occorrence of c, create
  destination string and copy substring to destination. Unescape
  the destination string, advance the iterator to get to the
  next character after c, or to end of string.

  In the event of success, the iterator will be at the next
  character after the one that was to be searched for, or at the
  end of the string.

  @param[out]    dest destination string
  @param[in,out] it   string iterator
  @param         end  iterator pointing to string end
  @param         c    character to search for

  @return             Operation status
    @retval true      if an error occurred
    @retval false     if success
*/
bool eat_str(String_type &dest, String_type::const_iterator &it,
             String_type::const_iterator end, char c);

/**
  Start at it, find a key and value separated by an unescaped '='. Value
  is supposed to be terminated by an unescaped ';' or by the end of the
  string. Unescape the key and value and add them to the property
  object. Call recursively to find the remaining pairs.

  @param[in,out] props property object where key and value should be added
  @param[in,out] it    string iterator
  @param         end   iterator pointing to string end

  @return             Operation status
    @retval true      if an error occurred
    @retval false     if success
*/
bool eat_pairs(String_type::const_iterator &it, String_type::const_iterator end,
               dd::Properties *props);

///////////////////////////////////////////////////////////////////////////

/**
   Convert seconds since epoch, to a datetime ulonglong using my_tz_OFFSET0
   suitable for timestamp fields in the DD.

   @param seconds_since_epoch value to convert
   @return time value converted to datetime ulonglong
 */
ulonglong my_time_t_to_ull_datetime(my_time_t seconds_since_epoch);

///////////////////////////////////////////////////////////////////////////

/**
  Method to verify if string is in lowercase.

  @param   str       String to verify.
  @param   cs        Character set.

  @retval  true    If string is in lowercase.
  @retval  false   Otherwise.
*/
bool is_string_in_lowercase(const String_type &str, const CHARSET_INFO *cs);

///////////////////////////////////////////////////////////////////////////

/**
  Helper function to do rollback or commit, depending on
  error. Also closes tables and releases transactional
  locks, regardless of error.

  @param thd   Thread
  @param error If true, the transaction will be rolledback.
               otherwise, it is committed.

  @returns false on success, otherwise true.
*/
bool end_transaction(THD *thd, bool error);

}  // namespace dd

#endif  // DD__UTILS_INCLUDED
