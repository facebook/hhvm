/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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
  @file json_path.cc

  This file contains implementation support for the JSON path abstraction.
  The path abstraction is described by the functional spec
  attached to WL#7909.
*/

#include "sql/json_path.h"

#include "my_rapidjson_size_t.h"  // IWYU pragma: keep

#include <rapidjson/encodings.h>
#include <rapidjson/memorystream.h>  // rapidjson::MemoryStream
#include <stddef.h>
#include <algorithm>  // any_of
#include <memory>     // unique_ptr
#include <string>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/json_dom.h"
#include "sql/psi_memory_key.h"  // key_memory_JSON
#include "sql/sql_const.h"       // STRING_BUFFER_USUAL_SIZE
#include "sql_string.h"          // String
#include "template_utils.h"      // down_cast

namespace {

constexpr char SCOPE = '$';
constexpr char BEGIN_MEMBER = '.';
constexpr char BEGIN_ARRAY = '[';
constexpr char END_ARRAY = ']';
constexpr char DOUBLE_QUOTE = '"';
constexpr char WILDCARD = '*';
constexpr char MINUS = '-';
constexpr char LAST[] = "last";

class Stream;

}  // namespace

static bool is_ecmascript_identifier(const std::string &name);
static bool is_digit(unsigned codepoint);
static bool is_whitespace(char);

static bool parse_path(Stream *, Json_path *);
static bool parse_path_leg(Stream *, Json_path *);
static bool parse_ellipsis_leg(Stream *, Json_path *);
static bool parse_array_leg(Stream *, Json_path *);
static bool parse_member_leg(Stream *, Json_path *);

static bool append_array_index(String *buf, size_t index, bool from_end) {
  if (!from_end) return buf->append_ulonglong(index);

  bool ret = buf->append(STRING_WITH_LEN(LAST));
  if (index > 0) ret |= buf->append(MINUS) || buf->append_ulonglong(index);
  return ret;
}

// Json_path_leg

bool Json_path_leg::to_string(String *buf) const {
  switch (m_leg_type) {
    case jpl_member:
      return buf->append(BEGIN_MEMBER) ||
             (is_ecmascript_identifier(m_member_name)
                  ? buf->append(m_member_name.data(), m_member_name.length())
                  : double_quote(m_member_name.data(), m_member_name.length(),
                                 buf));
    case jpl_array_cell:
      return buf->append(BEGIN_ARRAY) ||
             append_array_index(buf, m_first_array_index,
                                m_first_array_index_from_end) ||
             buf->append(END_ARRAY);
    case jpl_array_range:
      return buf->append(BEGIN_ARRAY) ||
             append_array_index(buf, m_first_array_index,
                                m_first_array_index_from_end) ||
             buf->append(STRING_WITH_LEN(" to ")) ||
             append_array_index(buf, m_last_array_index,
                                m_last_array_index_from_end) ||
             buf->append(END_ARRAY);
    case jpl_member_wildcard:
      return buf->append(BEGIN_MEMBER) || buf->append(WILDCARD);
    case jpl_array_cell_wildcard:
      return buf->append(BEGIN_ARRAY) || buf->append(WILDCARD) ||
             buf->append(END_ARRAY);
    case jpl_ellipsis:
      return buf->append(WILDCARD) || buf->append(WILDCARD);
  }

  // Unknown leg type.
  DBUG_ASSERT(false); /* purecov: inspected */
  return true;        /* purecov: inspected */
}

bool Json_path_leg::is_autowrap() const {
  switch (m_leg_type) {
    case jpl_array_cell:
      /*
        If the array cell index matches an element in a single-element
        array (`0` or `last`), it will also match a non-array value
        which is auto-wrapped in a single-element array.
      */
      return first_array_index(1).within_bounds();
    case jpl_array_range: {
      /*
        If the range matches an element in a single-element array, it
        will also match a non-array which is auto-wrapped in a
        single-element array.
      */
      Array_range range = get_array_range(1);
      return range.m_begin < range.m_end;
    }
    default:
      return false;
  }
}

Json_path_leg::Array_range Json_path_leg::get_array_range(
    size_t array_length) const {
  if (m_leg_type == jpl_array_cell_wildcard) return {0, array_length};

  DBUG_ASSERT(m_leg_type == jpl_array_range);

  // Get the beginning of the range.
  size_t begin = first_array_index(array_length).position();

  // Get the (exclusive) end of the range.
  Json_array_index last = last_array_index(array_length);
  size_t end = last.within_bounds() ? last.position() + 1 : last.position();

  return {begin, end};
}

Json_seekable_path::Json_seekable_path() : m_path_legs(key_memory_JSON) {}

// Json_path

Json_path::Json_path() : m_mem_root(key_memory_JSON, 256) {}

bool Json_path::to_string(String *buf) const {
  if (buf->append(SCOPE)) return true;

  for (const Json_path_leg *leg : *this) {
    if (leg->to_string(buf)) return true;
  }

  return false;
}

bool Json_path::can_match_many() const {
  return std::any_of(begin(), end(), [](const Json_path_leg *leg) -> bool {
    switch (leg->get_type()) {
      case jpl_member_wildcard:
      case jpl_array_cell_wildcard:
      case jpl_ellipsis:
      case jpl_array_range:
        return true;
      default:
        return false;
    }
  });
}

// Json_path parsing

namespace {

/// A simple input stream class for the JSON path parser.
class Stream {
 public:
  /// Creates an input stream reading from a character string.
  /// @param string  the input string
  /// @param length  the length of the input string
  Stream(const char *string, size_t length)
      : m_position(string), m_end(string + length) {}

  /// Returns a pointer to the current position in the stream.
  const char *position() const { return m_position; }

  /// Returns a pointer to the position just after the end of the stream.
  const char *end() const { return m_end; }

  /// Returns the number of bytes remaining in the stream.
  size_t remaining() const {
    DBUG_ASSERT(m_position <= m_end);
    return m_end - m_position;
  }

  /// Tells if the stream has been exhausted.
  bool exhausted() const { return remaining() == 0; }

  /// Reads the next byte from the stream and moves the position forward.
  char read() {
    DBUG_ASSERT(!exhausted());
    return *m_position++;
  }

  /// Reads the next byte from the stream without moving the position forward.
  char peek() const {
    DBUG_ASSERT(!exhausted());
    return *m_position;
  }

  /// Moves the position to the next non-whitespace character.
  void skip_whitespace() {
    m_position = std::find_if_not(m_position, m_end,
                                  [](char c) { return is_whitespace(c); });
  }

  /// Moves the position n bytes forward.
  void skip(size_t n) {
    DBUG_ASSERT(remaining() >= n);
    m_position += n;
  }

 private:
  /// The current position in the stream.
  const char *m_position;

  /// The end of the stream.
  const char *const m_end;
};

}  // namespace

/** Top level parsing factory method */
bool parse_path(size_t path_length, const char *path_expression,
                Json_path *path, size_t *bad_index) {
  Stream stream(path_expression, path_length);
  if (parse_path(&stream, path)) {
    *bad_index = stream.position() - path_expression;
    return true;
  }

  *bad_index = 0;
  return false;
}

/// Is this a whitespace character?
static inline bool is_whitespace(char ch) {
  return my_isspace(&my_charset_utf8mb4_bin, ch);
}

/**
   Fills in a Json_path from a path expression.

   @param[in,out] stream The stream to read the path expression from.
   @param[in,out] path The Json_path object to fill.

   @return true on error, false on success
*/
static bool parse_path(Stream *stream, Json_path *path) {
  path->clear();

  // the first non-whitespace character must be $
  stream->skip_whitespace();
  if (stream->exhausted() || stream->read() != SCOPE) return true;

  // now add the legs
  stream->skip_whitespace();
  while (!stream->exhausted()) {
    if (parse_path_leg(stream, path)) return true;
    stream->skip_whitespace();
  }

  // a path may not end with an ellipsis
  if (path->leg_count() > 0 && path->last_leg()->get_type() == jpl_ellipsis) {
    return true;
  }

  return false;
}

/**
   Parses a single path leg and appends it to a Json_path object.

   @param[in,out] stream The stream to read the path expression from.
   @param[in,out] path The Json_path object to fill.

   @return true on error, false on success
*/
static bool parse_path_leg(Stream *stream, Json_path *path) {
  switch (stream->peek()) {
    case BEGIN_ARRAY:
      return parse_array_leg(stream, path);
    case BEGIN_MEMBER:
      return parse_member_leg(stream, path);
    case WILDCARD:
      return parse_ellipsis_leg(stream, path);
    default:
      return true;
  }
}

/**
   Parses a single ellipsis leg and appends it to a Json_path object.

   @param[in,out] stream The stream to read the path expression from.
   @param[in,out] path The Json_path object to fill.

   @return true on error, false on success
*/
static bool parse_ellipsis_leg(Stream *stream, Json_path *path) {
  // advance past the first *
  DBUG_ASSERT(stream->peek() == WILDCARD);
  stream->skip(1);

  // must be followed by a second *
  if (stream->exhausted() || stream->read() != WILDCARD) {
    return true;
  }

  // may not be the last leg
  if (stream->exhausted()) {
    return true;
  }

  // forbid the hard-to-read *** combination
  if (stream->peek() == WILDCARD) {
    return true;
  }

  return path->append(Json_path_leg(jpl_ellipsis));
}

/**
  Parse an array index in an array cell index or array range path leg.

  An array index is either a non-negative integer (a 0-based index relative to
  the beginning of the array), or the keyword "last" (which means the last
  element in the array), or the keyword "last" followed by a minus ("-") and a
  non-negative integer (which is the 0-based index relative to the end of the
  array).

  @param[in,out] stream    the stream to read the path expression from
  @param[out] array_index  gets set to the parsed array index
  @param[out] from_end     gets set to true if the array index is
                           relative to the end of the array

  @return true on error, false on success
*/
static bool parse_array_index(Stream *stream, uint32 *array_index,
                              bool *from_end) {
  *from_end = false;

  // Do we have the "last" token?
  if (stream->remaining() >= 4 &&
      std::equal(LAST, LAST + 4, stream->position())) {
    stream->skip(4);
    *from_end = true;

    stream->skip_whitespace();

    if (!stream->exhausted() && stream->peek() == MINUS) {
      // Found a minus sign, go on parsing to find the array index.
      stream->skip(1);
      stream->skip_whitespace();
    } else {
      // Didn't find any minus sign after "last", so we're done.
      *array_index = 0;
      return false;
    }
  }

  if (stream->exhausted() || !is_digit(stream->peek())) {
    return true;
  }

  const char *endp;
  int err;
  ulonglong idx = my_strntoull(&my_charset_utf8mb4_bin, stream->position(),
                               stream->remaining(), 10, &endp, &err);
  if (err != 0 || idx > UINT_MAX32) {
    return true;
  }

  stream->skip(endp - stream->position());
  *array_index = static_cast<uint32>(idx);
  return false;
}

/**
   Parses a single array leg and appends it to a Json_path object.

   @param[in,out] stream The stream to read the path expression from.
   @param[in,out] path The Json_path object to fill.

   @return true on error, false on success
*/
static bool parse_array_leg(Stream *stream, Json_path *path) {
  // advance past the [
  DBUG_ASSERT(stream->peek() == BEGIN_ARRAY);
  stream->skip(1);

  stream->skip_whitespace();
  if (stream->exhausted()) return true;

  if (stream->peek() == WILDCARD) {
    stream->skip(1);
    if (path->append(Json_path_leg(jpl_array_cell_wildcard)))
      return true; /* purecov: inspected */
  } else {
    /*
      Not a WILDCARD. The next token must be an array index (either
      the single index of a jpl_array_cell path leg, or the start
      index of a jpl_array_range path leg.
    */
    uint32 cell_index1;
    bool from_end1;
    if (parse_array_index(stream, &cell_index1, &from_end1)) return true;

    stream->skip_whitespace();
    if (stream->exhausted()) return true;

    // Is this a range, <arrayIndex> to <arrayIndex>?
    const char *const pos = stream->position();
    if (stream->remaining() > 3 && is_whitespace(pos[-1]) && pos[0] == 't' &&
        pos[1] == 'o' && is_whitespace(pos[2])) {
      // A range. Skip over the "to" token and any whitespace.
      stream->skip(3);
      stream->skip_whitespace();

      uint32 cell_index2;
      bool from_end2;
      if (parse_array_index(stream, &cell_index2, &from_end2)) return true;

      /*
        Reject pointless paths that can never return any matches, regardless of
        which array they are evaluated against. We know this if both indexes
        count from the same side of the array, and the start index is after the
        end index.
      */
      if (from_end1 == from_end2 && ((from_end1 && cell_index1 < cell_index2) ||
                                     (!from_end1 && cell_index2 < cell_index1)))
        return true;

      if (path->append(
              Json_path_leg(cell_index1, from_end1, cell_index2, from_end2)))
        return true; /* purecov: inspected */
    } else {
      // A single array cell.
      if (path->append(Json_path_leg(cell_index1, from_end1)))
        return true; /* purecov: inspected */
    }
  }

  // the next non-whitespace should be the closing ]
  stream->skip_whitespace();
  return stream->exhausted() || stream->read() != END_ARRAY;
}

/**
  Find the end of a member name in a JSON path. The name could be
  either a quoted or an unquoted identifier.

  @param start the start of the member name
  @param end the end of the JSON path expression
  @return pointer to the position right after the end of the name, or
  to the position right after the end of the string if the input
  string is an unterminated quoted identifier
*/
static const char *find_end_of_member_name(const char *start, const char *end) {
  const char *str = start;

  /*
    If we have a double-quoted name, the end of the name is the next
    unescaped double quote.
  */
  if (*str == DOUBLE_QUOTE) {
    str++;  // Advance past the opening double quote.
    while (str < end) {
      switch (*str++) {
        case '\\':
          /*
            Skip the next character after a backslash. It cannot mark
            the end of the quoted string.
          */
          str++;
          break;
        case DOUBLE_QUOTE:
          // An unescaped double quote marks the end of the quoted string.
          return str;
      }
    }

    /*
      Whoops. No terminating quote was found. Just return the end of
      the string. When we send the unterminated string through the
      JSON parser, it will detect and report the syntax error, so
      there is no need to handle the syntax error here.
    */
    return end;
  }

  /*
    If we have an unquoted name, the name is terminated by whitespace
    or [ or . or * or end-of-string.
  */
  const auto is_terminator = [](const char c) {
    return is_whitespace(c) || c == BEGIN_ARRAY || c == BEGIN_MEMBER ||
           c == WILDCARD;
  };
  return std::find_if(str, end, is_terminator);
}

/**
  Parse a quoted member name using the rapidjson parser, so that we
  get the name without the enclosing quotes and with any escape
  sequences replaced with the actual characters.

  It is the caller's responsibility to destroy the returned
  Json_string when it's done with it.

  @param str the input string
  @param len the length of the input string
  @return a Json_string that represents the member name, or NULL if
  the input string is not a valid name
*/
static std::unique_ptr<Json_string> parse_name_with_rapidjson(const char *str,
                                                              size_t len) {
  Json_dom_ptr dom = Json_dom::parse(str, len, false, nullptr, nullptr);

  if (dom == nullptr || dom->json_type() != enum_json_type::J_STRING)
    return nullptr;

  return std::unique_ptr<Json_string>(down_cast<Json_string *>(dom.release()));
}

/**
   Parses a single member leg and appends it to a Json_path object.

   @param[in,out] stream The stream to read the path expression from.
   @param[in,out] path The Json_path object to fill.

   @return true on error, false on success
*/
static bool parse_member_leg(Stream *stream, Json_path *path) {
  // advance past the .
  DBUG_ASSERT(stream->peek() == BEGIN_MEMBER);
  stream->skip(1);

  stream->skip_whitespace();
  if (stream->exhausted()) return true;

  if (stream->peek() == WILDCARD) {
    stream->skip(1);

    if (path->append(Json_path_leg(jpl_member_wildcard)))
      return true; /* purecov: inspected */
  } else {
    const char *const key_start = stream->position();
    const char *const key_end =
        find_end_of_member_name(key_start, stream->end());
    const bool was_quoted = (*key_start == DOUBLE_QUOTE);
    stream->skip(key_end - key_start);

    std::unique_ptr<Json_string> jstr;

    if (was_quoted) {
      /*
        Send the quoted name through the parser to unquote and
        unescape it.
      */
      jstr = parse_name_with_rapidjson(key_start, key_end - key_start);
    } else {
      /*
        An unquoted name may contain escape sequences. Wrap it in
        double quotes and send it through the JSON parser to unescape
        it.
      */
      StringBuffer<STRING_BUFFER_USUAL_SIZE> strbuff(&my_charset_utf8mb4_bin);
      if (strbuff.append(DOUBLE_QUOTE) ||
          strbuff.append(key_start, key_end - key_start) ||
          strbuff.append(DOUBLE_QUOTE))
        return true; /* purecov: inspected */
      jstr = parse_name_with_rapidjson(strbuff.ptr(), strbuff.length());
    }

    if (jstr == nullptr) return true;

    // unquoted names must be valid ECMAScript identifiers
    if (!was_quoted && !is_ecmascript_identifier(jstr->value())) return true;

    // Looking good.
    if (path->append(Json_path_leg(jstr->value())))
      return true; /* purecov: inspected */
  }

  return false;
}

/**
   Return true if the character is a unicode combining mark.

   @param codepoint A unicode codepoint.

   @return True if the codepoint is a unicode combining mark.
*/
static inline bool unicode_combining_mark(unsigned codepoint) {
  return ((0x300 <= codepoint) && (codepoint <= 0x36F));
}

/**
   Return true if the codepoint is a Unicode letter.

   This was the best
   recommendation from the old-timers about how to answer this question.
   But as you can see from the need to call unicode_combining_mark(),
   my_isalpha() isn't good enough. It probably has many other defects.

   FIXME
*/
static bool is_letter(unsigned codepoint) {
  /*
    The Unicode combining mark \u036F passes the my_isalpha() test.
    That doesn't inspire much confidence in the correctness
    of my_isalpha().
   */
  if (unicode_combining_mark(codepoint)) {
    return false;
  }
  return my_isalpha(&my_charset_utf8mb4_bin, codepoint);
}

/**
   Return true if the codepoint is a Unicode digit.

   This was the best
   recommendation from the old-times about how to answer this question.
*/
static bool is_digit(unsigned codepoint) {
  return my_isdigit(&my_charset_utf8mb4_bin, codepoint);
}

/**
   Return true if the codepoint is Unicode connector punctuation.
*/
static bool is_connector_punctuation(unsigned codepoint) {
  switch (codepoint) {
    case 0x5F:    // low line
    case 0x203F:  // undertie
    case 0x2040:  // character tie
    case 0x2054:  // inverted undertie
    case 0xFE33:  // presentation form for vertical low line
    case 0xFE34:  // presentation form for vertical wavy low line
    case 0xFE4D:  // dashed low line
    case 0xFE4E:  // centerline low line
    case 0xFE4F:  // wavy low line
    case 0xFF3F:  // fullwidth low line
    {
      return true;
    }
    default: {
      return false;
    }
  }
}

/**
   Returns true if the name is a valid ECMAScript identifier.

   The name
   must be a sequence of UTF8-encoded bytes. All escape sequences
   have been replaced with UTF8-encoded bytes.

   @param[in] name        name to check

   @return True if the name is a valid ECMAScript identifier. False otherwise.
*/
static bool is_ecmascript_identifier(const std::string &name) {
  // An empty string is not a valid identifier.
  if (name.empty()) return false;

  /*
    At this point, The unicode escape sequences have already
    been replaced with the corresponding UTF-8 bytes. Now we apply
    the rules here: https://es5.github.io/x7.html#x7.6
  */
  rapidjson::MemoryStream input_stream(name.data(), name.length());
  unsigned codepoint;

  while (input_stream.Tell() < name.length()) {
    bool first_codepoint = (input_stream.Tell() == 0);
    if (!rapidjson::UTF8<char>::Decode(input_stream, &codepoint)) return false;

    // a unicode letter
    if (is_letter(codepoint)) continue;
    // $ is ok
    if (codepoint == 0x24) continue;
    // _ is ok
    if (codepoint == 0x5F) continue;

    /*
      the first character must be one of the above.
      more possibilities are available for subsequent characters.
    */

    if (first_codepoint) {
      return false;
    } else {
      // unicode combining mark
      if (unicode_combining_mark(codepoint)) continue;

      // a unicode digit
      if (is_digit(codepoint)) continue;
      if (is_connector_punctuation(codepoint)) continue;
      // <ZWNJ>
      if (codepoint == 0x200C) continue;
      // <ZWJ>
      if (codepoint == 0x200D) continue;
    }

    // nope
    return false;
  }

  return true;
}
