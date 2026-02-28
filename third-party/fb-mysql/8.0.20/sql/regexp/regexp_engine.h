#ifndef SQL_REGEXP_REGEXP_ENGINE_H_
#define SQL_REGEXP_REGEXP_ENGINE_H_

/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <unicode/uregex.h>

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <utility>

#include "m_ctype.h"    // CHARSET_INFO.
#include "my_config.h"  // WORDS_BIGENDIAN
#include "sql/current_thd.h"
#include "sql/regexp/errors.h"
#include "sql/sql_class.h"  // THD
#include "template_utils.h"

extern CHARSET_INFO my_charset_utf16le_general_ci;
extern CHARSET_INFO my_charset_utf16_general_ci;

namespace regexp_engine_unittest {
class Mock_regexp_engine;
}

namespace regexp {

static constexpr CHARSET_INFO *regexp_lib_charset =
#ifdef WORDS_BIGENDIAN
    &::my_charset_utf16_general_ci;
#else
    &::my_charset_utf16le_general_ci;
#endif

const char *icu_version_string();

/**
  Implements a match callback function for icu that aborts execution if the
  query was killed.

  @param context The session to check for killed query.
  @param steps Not used.

  @retval false Query was killed in the session and the match should abort.
  @retval true Query was not killed, matching should continue.
*/
UBool QueryNotKilled(const void *context, int32_t steps);

/**
  This class exposes high-level regular expression operations to the
  facade. It implements the algorithm for search-and-replace and the various
  matching options.

  A buffer is used for search-and-replace, whose initial size is that of the
  subject string. The buffer uses ICU preflight features to probe the required
  buffer size within each append operation, and the buffer can grow up until
  max_allowed_packet, at which case and error will be thrown.
*/
class Regexp_engine {
 public:
  /**
    Compiles the URegularExpression object. If compilation fails, my_error()
    is called and the IsError() returns true. In this case, all subsequent
    operations will be no-ops, reporting failure. This follows ICU's chaining
    conventions, see http://icu-project.org/apiref/icu4c/utypes_8h.html.

    @param pattern The pattern string in ICU's character set.

    @param flags ICU flags.

    @param stack_limit Sets the amount of heap storage, in bytes, that the
    match backtracking stack is allowed to allocate.

    @param time_limit Gets set on the URegularExpression. Please refer to the
    ICU API docs for the definition of time limit.
  */
  Regexp_engine(const std::u16string &pattern, uint flags, int stack_limit,
                int time_limit) {
    UParseError error;
    auto upattern = pattern.data();
    int length = pattern.size();
    m_re = uregex_open(pointer_cast<const UChar *>(upattern), length, flags,
                       &error, &m_error_code);
    uregex_setStackLimit(m_re, stack_limit, &m_error_code);
    uregex_setTimeLimit(m_re, time_limit, &m_error_code);
    uregex_setMatchCallback(m_re, QueryNotKilled, current_thd, &m_error_code);
    check_icu_status(m_error_code, &error);
  }

  uint flags() {
    uint flags = uregex_flags(m_re, &m_error_code);
    check_icu_status(m_error_code);
    return flags;
  }

  /**
    Resets the engine with a new subject string. This also clears the
    replacement buffer, see Replace().

    @param subject The new string to match the regular
    expression against.
  */
  void Reset(const std::u16string &subject);

  /**
    Tries to find match number `occurrence` in the string, starting on
    `start`.

    @param start Start position, 0-based.
    @param occurrence Which occurrence to replace. If zero, replace all
    occurrences.
  */
  bool Matches(int start, int occurrence);

  /**
    Returns the start position in the input string of the string where
    Matches() found a match.
  */
  int StartOfMatch() {
    /*
      The 0 is for capture group number, but we don't deal with those
      here. Zero means the start of the whole match, which is what's needed.
    */
    return uregex_start(m_re, 0, &m_error_code);
  }

  /**
    Returns the position in the input string right after the end of the text
    where Matches() found a match.
  */
  int EndOfMatch() {
    // The 0 means start of capture group 0, ie., the whole match.
    return uregex_end(m_re, 0, &m_error_code);
  }

  /**
    Iterates over the subject string, replacing matches.

    @param replacement The string to replace matches with.
    @param start Start position, 0-based.
    @param occurrence Which occurrence to replace. If zero, replace all
    occurrences.

    @return Reference to a the result of the operation. It is guaranteed to
    stay intact until a call is made to Reset().
  */
  const std::u16string &Replace(const std::u16string &replacement, int start,
                                int occurrence);

  /**
    The start of the match and its length.

    @return The index of the first code point of the match, and the length of
    the same.
  */
  std::pair<int, int> MatchedSubstring();

  bool IsError() const { return U_FAILURE(m_error_code); }
  bool CheckError() const { return check_icu_status(m_error_code); }

  virtual ~Regexp_engine() { uregex_close(m_re); }

  /**
    The hard limit for growing the replace buffer. The buffer cannot grow
    beyond this size, and an error will be thrown if the limit is reached.
  */
  size_t HardLimit() {
    return current_thd->variables.max_allowed_packet / sizeof(UChar);
  }

  /**
    Fills in the prefix in case we are doing a replace operation starting on a
    non-first occurrence of the pattern, or a non-first start
    position. AppendReplacement() will fill in the section starting after the
    previous match or start position, so a prefix must be appended first.

    The part we have to worry about here, the part that ICU doesn't add for
    us is, is if the search didn't start on the first character or first
    match for the regular expression. It's the longest such prefix that we
    have to copy ourselves.
  */
  void AppendHead(size_t size);

  /**
    Tries to write the replacement, growing the buffer if needed.

    @param replacement The replacement string.
  */
  void AppendReplacement(const std::u16string &replacement);

  /// Appends the trailing segment after the last match to the subject string,
  void AppendTail();

  /**
    The spare capacity in the replacement buffer, given in code points.

    ICU communicates via a `capacity` variable, but we like to use an absolute
    position instead, and we want to keep a single source of truth, so we
    calculate it when needed and assert that the number is correct.
  */
  int SpareCapacity() const {
    return m_replace_buffer.capacity() - m_replace_buffer.size();
  }

  friend class regexp_engine_unittest::Mock_regexp_engine;

 private:
  /**
    Preflight function: If the buffer capacity is adequate, the replacement is
    appended to the buffer, otherwise nothing is written. Either way, the
    replacement's full size is returned.
  */
  int TryToAppendReplacement(const std::u16string &replacement);

  /**
    Tries to append the part of the subject string after the last match to the
    buffer. This is a preflight function: If the buffer capacity is adequate,
    the tail is appended to the buffer, otherwise nothing is written. Either
    way, the tail's full size is returned.
  */
  int TryToAppendTail();

  /**
    Our handle to ICU's compiled regular expression, owned by instances of
    this class. URegularExpression is a C struct, but this class follows RAII
    and initializes this pointer in the constructor and cleans it up in the
    destructor.
  */
  URegularExpression *m_re;
  UErrorCode m_error_code = U_ZERO_ERROR;
  std::u16string m_current_subject;
  std::u16string m_replace_buffer;
  /**
    This is always the next index in m_replace_buffer where ICU can write
    data.
  */
  int m_replace_buffer_pos = 0;
};

}  // namespace regexp

#endif  // SQL_REGEXP_REGEXP_ENGINE_H_
