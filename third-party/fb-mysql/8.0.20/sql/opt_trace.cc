/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA  */

/**
   @file
   Implementation of the Optimizer trace API (WL#5257)
*/

#include "sql/opt_trace.h"

#include <float.h>
#include <stdio.h>
#include <algorithm>  // std::min
#include <new>

#include "lex_string.h"
#include "m_string.h"  // _dig_vec_lower
#include "my_dbug.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/current_thd.h"
#include "sql/enum_query_type.h"
#include "sql/handler.h"
#include "sql/item.h"  // Item
#include "sql/table.h"
#include "sql_string.h"  // String

namespace {
/**
  A wrapper of class String, for storing query or trace.
  Any memory allocation error in this class is reported by my_error(), see
  OOM_HANDLING in opt_trace.h.
*/
class Buffer {
 private:
  size_t allowed_mem_size;  ///< allowed memory size for this String
  size_t missing_bytes;     ///< how many bytes could not be added
  String string_buf;

 public:
  Buffer() : allowed_mem_size(0), missing_bytes(0) {}

  size_t alloced_length() const { return string_buf.alloced_length(); }
  size_t length() const { return string_buf.length(); }
  void prealloc();  ///< pro-actively extend buffer if soon short of space
  char *c_ptr_safe() { return string_buf.c_ptr_safe(); }
  const char *ptr() const { return string_buf.ptr(); }

  const CHARSET_INFO *charset() const { return string_buf.charset(); }
  void set_charset(const CHARSET_INFO *charset) {
    string_buf.set_charset(charset);
  }

  /**
    Like @c String::append()
    @param  str     String, in this instance's charset
    @param  length  length of string
  */
  void append(const char *str, size_t length);
  void append(const char *str) { return append(str, strlen(str)); }
  /**
    Like @c append() but escapes certain characters for string values to
    be JSON-compliant.
    @param  str     String in UTF8
    @param  length  length of string
  */
  void append_escaped(const char *str, size_t length);
  void append(char chr);

  size_t get_allowed_mem_size() const { return allowed_mem_size; }
  size_t get_missing_bytes() const { return missing_bytes; }

  void set_allowed_mem_size(size_t a) { allowed_mem_size = a; }
};

}  // namespace

/**
  @class Opt_trace_stmt

  The trace of one statement. For example, executing a stored procedure
  containing 3 sub-statements will produce 4 traces (one for the CALL
  statement, one for each sub-statement), so 4 Opt_trace_stmt linked together
  into Opt_trace_context's lists.
*/
class Opt_trace_stmt {
 public:
  /**
     Constructor, starts a trace for information_schema and dbug.
     @param  ctx_arg          context
  */
  Opt_trace_stmt(Opt_trace_context *ctx_arg);

  /**
     Ends a trace; destruction may not be possible immediately as we may have
     to keep the trace in case the user later reads it from I_S.
  */
  void end();

  /// @returns whether @c end() has been called on this instance.
  bool has_ended() const { return ended; }

  /// Sets the quantity of allowed memory for this trace.
  void set_allowed_mem_size(size_t size);

  /// @sa Opt_trace_context::set_query()
  void set_query(const char *query, size_t length, const CHARSET_INFO *charset);

  /* Below, functions for filling the statement's trace */

  /**
     When creating an Opt_trace_struct: adds a key and the opening bracket to
     the trace buffer, updates current_struct.
     @param  key              key or NULL
     @param  ots              structure being created
     @param  wants_disable_I_S whether structure wants to disable I_S output
     @param  opening_bracket  opening bracket to use
     @retval false ok
     @retval true  error, Opt_trace_struct must set itself to dummy; trace
     may have been written to, will likely be invalid JSON.
  */
  bool open_struct(const char *key, Opt_trace_struct *ots,
                   bool wants_disable_I_S, char opening_bracket);
  /**
     When closing an Opt_trace_struct:
     - adds the closing bracket and optionally the key to the trace buffer
     - re-enables I_S output if the dying structure had disabled it
     - updates current_struct.
     @param  saved_key        key or NULL
     @param  has_disabled_I_S whether structure had disabled I_S output
     @param  closing_bracket  closing bracket to use
  */
  void close_struct(const char *saved_key, bool has_disabled_I_S,
                    char closing_bracket);

  /// Put optional comma, newline and indentation
  void separator();
  /// Put newline and indentation
  void next_line();

  /**
     Adds a key/value pair to the trace buffer.
     @param  key  key or NULL
     @param  val  representation of value as string
     @param  val_length  length of value
     @param  quotes  should value be delimited with '"' (false when the value
     is the representation of a number, boolean or null)
     @param  escape  does value need escaping (has special characters)

     @note Structures prepare a string representation of their value-to-add
     and call this function.
  */
  void add(const char *key, const char *val, size_t val_length, bool quotes,
           bool escape);

  /* Below, functions to request information from this instance */

  /// Fills user-level information @sa Opt_trace_iterator
  void fill_info(Opt_trace_info *info) const;

  /// @returns 'size' last bytes of the trace buffer
  const char *trace_buffer_tail(size_t size);

  /// @returns total memory used by this trace
  size_t alloced_length() const {
    return trace_buffer.alloced_length() + query_buffer.alloced_length();
  }

  void assert_current_struct(
      const Opt_trace_struct *s MY_ATTRIBUTE((unused))) const {
    DBUG_ASSERT(current_struct == s);
  }

  /// @see Opt_trace_context::missing_privilege()
  void missing_privilege();

  bool support_I_S() const { return I_S_disabled == 0; }

  /// Temporarily disables I_S output for this statement.
  void disable_I_S() { ++I_S_disabled; }

  /**
     Restores I_S support to what it was before the previous call
     to disable_I_S().
  */
  void restore_I_S() { --I_S_disabled; }

  /**
     Generate a dummy unique key, and return pointer to it. The pointed data
     has the lifetime of Opt_trace_stmt, and is overwritten by the next call
     to this function.
  */
  const char *make_unknown_key();

 private:
  bool ended;  ///< Whether @c end() has been called on this instance

  /**
    0 <=> this trace should be in information_schema.
    In the life of an Opt_trace_stmt, support for I_S may be temporarily
    disabled.
    Once disabled, it must stay disabled until re-enabled at the same stack
    frame. This:
    Opt_trace_object1 // disables I_S
       Opt_trace_object2 // re-enables I_S
    is impossible (the top object wins).
    So it is sufficient, to keep track of the current state, to have a counter
    incremented each time we get a request to disable I_S.
  */
  int I_S_disabled;

  bool missing_priv;  ///< whether user lacks privilege to see this trace

  Opt_trace_context *ctx;            ///< context
  Opt_trace_struct *current_struct;  ///< current open structure

  /// Same logic as Opt_trace_context::stack_of_current_stmts.
  Prealloced_array<Opt_trace_struct *, 16> stack_of_current_structs;

  Buffer trace_buffer;  ///< Where the trace is accumulated
  Buffer query_buffer;  ///< Where the original query is put

  /**
    Counter which serves to have unique autogenerated keys, needed if we
    autogenerate more than one key in a single object.
    @see Opt_trace_struct::check_key() and @see Opt_trace_stmt::add() .
  */
  uint unknown_key_count;
  /// Space for last autogenerated key
  char unknown_key[24];
};

// implementation of class Opt_trace_struct

namespace {
/// opening and closing symbols for arrays ([])and objects ({})
const char brackets[] = {'[', '{', ']', '}'};
inline char opening_bracket(bool requires_key) {
  return brackets[requires_key];
}
inline char closing_bracket(bool requires_key) {
  return brackets[requires_key + 2];
}
}  // namespace

void Opt_trace_struct::do_construct(Opt_trace_context *ctx,
                                    bool requires_key_arg, const char *key,
                                    Opt_trace_context::feature_value feature) {
  saved_key = key;
  requires_key = requires_key_arg;

  DBUG_PRINT("opt", ("%s: starting struct", key));
  stmt = ctx->get_current_stmt_in_gen();
#ifndef DBUG_OFF
  previous_key[0] = 0;
#endif
  has_disabled_I_S = !ctx->feature_enabled(feature);
  empty = true;
  if (likely(!stmt->open_struct(key, this, has_disabled_I_S,
                                opening_bracket(requires_key))))
    started = true;
}

void Opt_trace_struct::do_destruct() {
  DBUG_PRINT("opt", ("%s: ending struct", saved_key));
  DBUG_ASSERT(started);
  stmt->close_struct(saved_key, has_disabled_I_S,
                     closing_bracket(requires_key));
  started = false;
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, const char *val,
                                           size_t val_length, bool escape) {
  DBUG_ASSERT(started);
  DBUG_PRINT("opt", ("%s: \"%.*s\"", key, (int)val_length, val));
  stmt->add(key, val, val_length, true, escape);
  return *this;
}

namespace {
/// human-readable names for boolean values
LEX_CSTRING bool_as_text[] = {{STRING_WITH_LEN("false")},
                              {STRING_WITH_LEN("true")}};
}  // namespace

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, bool val) {
  DBUG_ASSERT(started);
  DBUG_PRINT("opt", ("%s: %d", key, (int)val));
  const LEX_CSTRING *text = &bool_as_text[val];
  stmt->add(key, text->str, text->length, false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, longlong val) {
  DBUG_ASSERT(started);
  char buf[22];  // 22 is enough for digits of a 64-bit int
  llstr(val, buf);
  DBUG_PRINT("opt", ("%s: %s", key, buf));
  stmt->add(key, buf, strlen(buf), false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, ulonglong val) {
  DBUG_ASSERT(started);
  char buf[22];
  ullstr(val, buf);
  DBUG_PRINT("opt", ("%s: %s", key, buf));
  stmt->add(key, buf, strlen(buf), false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, double val) {
  DBUG_ASSERT(started);
  char buf[32];  // 32 is enough for digits of a double
  /*
    To fit in FLT_DIG digits, my_gcvt rounds DBL_MAX (1.7976931...e308), or
    anything >=1.5e308, to 2e308. But JSON parsers refuse to read 2e308. So,
    lower the number.
  */
  my_gcvt(std::min(1e308, val), MY_GCVT_ARG_DOUBLE, FLT_DIG, buf, nullptr);
  DBUG_PRINT("opt", ("%s: %s", key, buf));
  stmt->add(key, buf, strlen(buf), false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add_null(const char *key) {
  DBUG_ASSERT(started);
  DBUG_PRINT("opt", ("%s: null", key));
  stmt->add(key, STRING_WITH_LEN("null"), false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key, Item *item) {
  char buff[256];
  String str(buff, sizeof(buff), system_charset_info);
  str.length(0);
  if (item != nullptr) {
    // QT_TO_SYSTEM_CHARSET because trace must be in UTF8
    item->print(current_thd, &str,
                enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                                QT_NO_DEFAULT_DB));
    /* needs escaping */
    return do_add(key, str.ptr(), str.length(), true);
  } else
    return do_add_null(key);
}

Opt_trace_struct &Opt_trace_struct::do_add(const char *key,
                                           const Cost_estimate &value) {
  return do_add(key, value.total_cost());
}

Opt_trace_struct &Opt_trace_struct::do_add_hex(const char *key, uint64 val) {
  DBUG_ASSERT(started);
  char buf[2 + 16], *p_end = buf + sizeof(buf) - 1, *p = p_end;
  for (;;) {
    *p-- = _dig_vec_lower[val & 15];
    *p-- = _dig_vec_lower[(val & 240) >> 4];
    val >>= 8;
    if (val == 0) break;
  }
  *p-- = 'x';
  *p = '0';
  const size_t len = p_end + 1 - p;
  DBUG_PRINT("opt", ("%s: %.*s", key, static_cast<int>(len), p));
  stmt->add(check_key(key), p, len, false, false);
  return *this;
}

Opt_trace_struct &Opt_trace_struct::do_add_utf8_table(const TABLE_LIST *tl) {
  if (tl != nullptr) {
    StringBuffer<32> str;
    tl->print(current_thd, &str,
              enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                              QT_NO_DEFAULT_DB | QT_DERIVED_TABLE_ONLY_ALIAS));
    return do_add("table", str.ptr(), str.length(), true);
  }
  return *this;
}

const char *Opt_trace_struct::check_key(const char *key) {
  DBUG_ASSERT(started);
  //  User should always add to the innermost open object, not outside.
  stmt->assert_current_struct(this);
  bool has_key = key != nullptr;
  if (unlikely(has_key != requires_key)) {
    // fix the key to produce correct JSON syntax:
    key = has_key ? nullptr : stmt->make_unknown_key();
    has_key = !has_key;
  }
  if (has_key) {
#ifndef DBUG_OFF
    /*
      Check that we're not having two identical consecutive keys in one
      object; though the real restriction should not have 'consecutive'.
    */
    DBUG_ASSERT(strncmp(previous_key, key, sizeof(previous_key) - 1) != 0);
    strncpy(previous_key, key, sizeof(previous_key) - 1);
    previous_key[sizeof(previous_key) - 1] = 0;
#endif
  }
  return key;
}

// Implementation of Opt_trace_stmt class

Opt_trace_stmt::Opt_trace_stmt(Opt_trace_context *ctx_arg)
    : ended(false),
      I_S_disabled(0),
      missing_priv(false),
      ctx(ctx_arg),
      current_struct(nullptr),
      stack_of_current_structs(PSI_INSTRUMENT_ME),
      unknown_key_count(0) {
  // Trace is always in UTF8. This is the only charset which JSON accepts.
  trace_buffer.set_charset(system_charset_info);
  DBUG_ASSERT(system_charset_info == &my_charset_utf8_general_ci);
}

void Opt_trace_stmt::end() {
  DBUG_ASSERT(stack_of_current_structs.size() == 0);
  DBUG_ASSERT(I_S_disabled >= 0);
  ended = true;
  /*
    Because allocation is done in big chunks, buffer->Ptr[str_length]
    may be uninitialized while buffer->Ptr[allocated length] is 0, so we
    must use c_ptr_safe() as we want a 0-terminated string (which is easier
    to manipulate in a debugger, or to compare in unit tests with
    EXPECT_STREQ).
    c_ptr_safe() may realloc an empty String from 0 bytes to 8 bytes,
    when it adds the closing \0.
  */
  trace_buffer.c_ptr_safe();
  // Send the full nice trace to DBUG.
  DBUG_EXECUTE("opt", {
    const char *trace = trace_buffer.c_ptr_safe();
    DBUG_LOCK_FILE;
    fputs("Complete optimizer trace:", DBUG_FILE);
    fputs(trace, DBUG_FILE);
    fputs("\n", DBUG_FILE);
    DBUG_UNLOCK_FILE;
  });
  if (unlikely(missing_priv)) ctx->restore_I_S();
}

void Opt_trace_stmt::set_allowed_mem_size(size_t size) {
  trace_buffer.set_allowed_mem_size(size);
}

void Opt_trace_stmt::set_query(const char *query, size_t length,
                               const CHARSET_INFO *charset) {
  // Should be called only once per statement.
  DBUG_ASSERT(query_buffer.ptr() == nullptr);
  query_buffer.set_charset(charset);
  if (!support_I_S()) {
    /*
      Query won't be read, don't waste resources storing it. Still we have set
      the charset, which is necessary.
    */
    return;
  }
  // We are taking a bit of space from 'trace_buffer'.
  size_t available =
      (trace_buffer.alloced_length() >= trace_buffer.get_allowed_mem_size())
          ? 0
          : (trace_buffer.get_allowed_mem_size() -
             trace_buffer.alloced_length());
  query_buffer.set_allowed_mem_size(available);
  // No need to escape query, this is not for JSON.
  query_buffer.append(query, length);
  // Space which query took is taken out of the trace:
  const size_t new_allowed_mem_size =
      (query_buffer.alloced_length() >= trace_buffer.get_allowed_mem_size())
          ? 0
          : (trace_buffer.get_allowed_mem_size() -
             query_buffer.alloced_length());
  trace_buffer.set_allowed_mem_size(new_allowed_mem_size);
}

bool Opt_trace_stmt::open_struct(const char *key, Opt_trace_struct *ots,
                                 bool wants_disable_I_S, char opening_bracket) {
  if (support_I_S()) {
    if (wants_disable_I_S) {
      /*
        User requested no tracing for this structure's feature. We are
        entering a disabled portion; put an ellipsis "..." to alert the user.
        Disabling applies to all the structure's children.
        It is possible that inside this struct, a new statement is created
        (range optimizer can evaluate stored functions...): its tracing is
        disabled too.
        When the structure is destroyed, the initial setting is restored.
      */
      if (current_struct != nullptr) {
        if (key != nullptr)
          current_struct->add_alnum(key, "...");
        else
          current_struct->add_alnum("...");
      }
    } else {
      trace_buffer.prealloc();
      add(key, &opening_bracket, 1, false, false);
    }
  }
  if (wants_disable_I_S) ctx->disable_I_S_for_this_and_children();
  {
    DBUG_EXECUTE_IF("opt_trace_oom_in_open_struct",
                    DBUG_SET("+d,simulate_out_of_memory"););
    const bool rc = stack_of_current_structs.push_back(current_struct);
    /*
      If the append() above didn't trigger reallocation, we need to turn the
      symbol off by ourselves, or it could make an unrelated allocation
      fail.
    */
    DBUG_EXECUTE_IF("opt_trace_oom_in_open_struct",
                    DBUG_SET("-d,simulate_out_of_memory"););
    if (unlikely(rc)) return true;
  }
  current_struct = ots;
  return false;
}

void Opt_trace_stmt::close_struct(const char *saved_key, bool has_disabled_I_S,
                                  char closing_bracket) {
  /*
    This was constructed with current_stmt_in_gen=NULL which was pushed in
    'open_struct()'. So this NULL is in the array, back() is safe.
  */
  current_struct = stack_of_current_structs.back();
  stack_of_current_structs.pop_back();
  if (support_I_S()) {
    next_line();
    trace_buffer.append(closing_bracket);
    if (ctx->get_end_marker() && saved_key != nullptr) {
      trace_buffer.append(STRING_WITH_LEN(" /* "));
      trace_buffer.append(saved_key);
      trace_buffer.append(STRING_WITH_LEN(" */"));
    }
  }
  if (has_disabled_I_S) ctx->restore_I_S();
}

void Opt_trace_stmt::separator() {
  DBUG_ASSERT(support_I_S());
  // Put a comma first, if we have already written an object at this level.
  if (current_struct != nullptr) {
    if (!current_struct->set_not_empty()) trace_buffer.append(',');
    next_line();
  }
}

namespace {
const char my_spaces[] =
    "                                                                "
    "                                                                "
    "                                                                ";
}

void Opt_trace_stmt::next_line() {
  if (ctx->get_one_line()) return;
  trace_buffer.append('\n');

  size_t to_be_printed = 2 * stack_of_current_structs.size();
  const size_t spaces_len = sizeof(my_spaces) - 1;
  while (to_be_printed > spaces_len) {
    trace_buffer.append(my_spaces, spaces_len);
    to_be_printed -= spaces_len;
  }
  trace_buffer.append(my_spaces, to_be_printed);
}

const char *Opt_trace_stmt::make_unknown_key() {
  snprintf(unknown_key, sizeof(unknown_key), "unknown_key_%u",
           ++unknown_key_count);
  return unknown_key;
}

void Opt_trace_stmt::add(const char *key, const char *val, size_t val_length,
                         bool quotes, bool escape) {
  if (!support_I_S()) return;
  separator();
  if (current_struct != nullptr) key = current_struct->check_key(key);
  if (key != nullptr) {
    trace_buffer.append('"');
    trace_buffer.append(key);
    trace_buffer.append(STRING_WITH_LEN("\": "));
  }
  if (quotes) trace_buffer.append('"');
  /*
    Objects' keys use "normal" characters (A-Za-z0-9_), no escaping
    needed. Same for numeric/bool values. Only string values may need
    escaping.
  */
  if (escape)
    trace_buffer.append_escaped(val, val_length);
  else
    trace_buffer.append(val, val_length);
  if (quotes) trace_buffer.append('"');
}

void Opt_trace_stmt::fill_info(Opt_trace_info *info) const {
  if (unlikely(info->missing_priv = missing_priv)) {
    info->trace_ptr = info->query_ptr = "";
    info->trace_length = info->query_length = 0;
    info->query_charset = &my_charset_bin;
    info->missing_bytes = 0;
  } else {
    info->trace_ptr = trace_buffer.ptr();
    info->trace_length = trace_buffer.length();
    info->query_ptr = query_buffer.ptr();
    info->query_length = query_buffer.length();
    info->query_charset = query_buffer.charset();
    info->missing_bytes =
        trace_buffer.get_missing_bytes() + query_buffer.get_missing_bytes();
  }
}

const char *Opt_trace_stmt::trace_buffer_tail(size_t size) {
  size_t buffer_len = trace_buffer.length();
  const char *ptr = trace_buffer.c_ptr_safe();
  if (buffer_len > size) ptr += buffer_len - size;
  return ptr;
}

void Opt_trace_stmt::missing_privilege() {
  if (!missing_priv) {
    DBUG_PRINT("opt", ("trace denied"));
    // This mark will make the trace appear empty in OPTIMIZER_TRACE table.
    missing_priv = true;
    // And all substatements will not be traced.
    ctx->disable_I_S_for_this_and_children();
  }
}

// Implementation of class Buffer

namespace {

void Buffer::append_escaped(const char *str, size_t length) {
  if (alloced_length() >= allowed_mem_size) {
    missing_bytes += length;
    return;
  }
  const char *pstr, *pstr_end;
  char buf[128];  // Temporary output buffer.
  char *pbuf = buf;
  for (pstr = str, pstr_end = (str + length); pstr < pstr_end; pstr++) {
    char esc;
    const char c = *pstr;
    /*
      JSON syntax says that control characters must be escaped. Experience
      confirms that this means ASCII 0->31 and " and \ . A few of
      them are accepted with a short escaping syntax (using \ : like \n)
      but for most of them, only \uXXXX works, where XXXX is a
      hexadecimal value for the code point.
      Rules also mention escaping / , but Python's and Perl's json modules
      do not require it, and somewhere on Internet someone said JSON
      allows escaping of / but does not require it.

      Because UTF8 has the same characters in range 0-127 as ASCII does, and
      other UTF8 characters don't contain 0-127 bytes, if we see a byte
      equal to 0 it is really the UTF8 u0000 character (a.k.a. ASCII NUL)
      and not a part of a longer character; if we see a newline, same,
      etc. That wouldn't necessarily be true with another character set.
    */
    switch (c) {
        // Don't use \u when possible for common chars, \ is easier to read:
      case '\\':
        esc = '\\';
        break;
      case '"':
        esc = '\"';
        break;
      case '\n':
        esc = 'n';
        break;
      case '\r':
        esc = 'r';
        break;
      case '\t':
        esc = 't';
        break;
      default:
        esc = 0;
        break;
    }
    if (esc != 0)  // Escaping with backslash.
    {
      *pbuf++ = '\\';
      *pbuf++ = esc;
    } else {
      uint ascii_code = (uint)c;
      if (ascii_code < 32)  // Escaping with \u
      {
        *pbuf++ = '\\';
        *pbuf++ = 'u';
        *pbuf++ = '0';
        *pbuf++ = '0';
        if (ascii_code < 16) {
          *pbuf++ = '0';
        } else {
          *pbuf++ = '1';
          ascii_code -= 16;
        }
        *pbuf++ = _dig_vec_lower[ascii_code];
      } else
        *pbuf++ = c;  // Normal character, no escaping needed.
    }
    /*
      To fit a next character, we need at most 6 bytes (happens when using
      \uXXXX syntax) before the buffer's end:
    */
    if (pbuf > buf + (sizeof(buf) - 6)) {
      // Possibly no room in 'buf' for next char, so flush buf.
      string_buf.append(buf, pbuf - buf);
      pbuf = buf;  // back to buf's start
    }
  }
  // Flush any chars left in 'buf'.
  string_buf.append(buf, pbuf - buf);
}

void Buffer::append(const char *str, size_t length) {
  if (alloced_length() >= allowed_mem_size) {
    missing_bytes += length;
    return;
  }
  DBUG_EXECUTE_IF("opt_trace_oom_in_buffers",
                  DBUG_SET("+d,simulate_out_of_memory"););
  string_buf.append(str, length);
  DBUG_EXECUTE_IF("opt_trace_oom_in_buffers",
                  DBUG_SET("-d,simulate_out_of_memory"););
}

void Buffer::append(char chr) {
  if (alloced_length() >= allowed_mem_size) {
    missing_bytes++;
    return;
  }
  // No need for escaping chr, given how this function is used.
  string_buf.append(chr);
}

void Buffer::prealloc() {
  const size_t alloced = alloced_length();
  const size_t first_increment = 1024;
  if ((alloced - length()) < (first_increment / 3)) {
    /*
      Support for I_S will produce long strings, and there is little free
      space left in the allocated buffer, so it looks like
      realloc is soon unavoidable; so let's get many bytes at a time.
      Note that if this re-allocation fails, or any String::append(), we
      will get a weird trace; either truncated if the server stops, or maybe
      with a hole if there is later memory again for the trace's
      continuation. The statement will fail anyway due to my_error(), in the
      server.
      We jump from 0 to first_increment and then multiply by 1.5. Unlike
      addition of a constant length, multiplying is expected to give amortized
      constant reallocation time; 1.5 is a commonly seen factor in the
      litterature.
    */
    size_t new_size = (alloced == 0) ? first_increment : (alloced * 15 / 10);
    size_t max_size = allowed_mem_size;
    /*
      Determine a safety margin:
      (A) String::realloc() adds at most ALIGN_SIZE(1) bytes to requested
      length, so we need to decrement max_size by this amount, to be sure that
      we don't allocate more than max_size
      (B) We need to stay at least one byte under that max_size, or the next
      append() would trigger up-front truncation, which is potentially wrong
      for a "pre-emptive allocation" as we do here.
    */
    const size_t safety_margin = ALIGN_SIZE(1) /* (A) */ + 1 /* (B) */;
    if (max_size >= safety_margin) {
      max_size -= safety_margin;
      if (new_size > max_size)  // Don't pre-allocate more than the limit.
        new_size = max_size;
      if (new_size >= alloced)  // Never shrink string.
        string_buf.mem_realloc(new_size);
    }
  }
}

}  // namespace

// Implementation of Opt_trace_context class

const char *Opt_trace_context::flag_names[] = {"enabled", "one_line", "default",
                                               NullS};

const char *Opt_trace_context::feature_names[] = {
    "greedy_search",      "range_optimizer", "dynamic_range",
    "repeated_subselect", "default",         NullS};

const Opt_trace_context::feature_value Opt_trace_context::default_features =
    Opt_trace_context::feature_value(Opt_trace_context::GREEDY_SEARCH |
                                     Opt_trace_context::RANGE_OPTIMIZER |
                                     Opt_trace_context::DYNAMIC_RANGE |
                                     Opt_trace_context::REPEATED_SUBSELECT);

Opt_trace_context::~Opt_trace_context() {
  if (unlikely(pimpl != nullptr)) {
    /* There may well be some few ended traces left: */
    purge_stmts(true);
    /* All should have moved to 'del' list: */
    DBUG_ASSERT(pimpl->all_stmts_for_I_S.size() == 0);
    /* All of 'del' list should have been deleted: */
    DBUG_ASSERT(pimpl->all_stmts_to_del.size() == 0);
    delete pimpl;
  }
}

template <class T>
T *new_nothrow_w_my_error() {
  T *const t = new (std::nothrow) T();
  if (unlikely(t == nullptr))
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), static_cast<int>(sizeof(T)));
  return t;
}
template <class T, class Arg>
T *new_nothrow_w_my_error(Arg a) {
  T *const t = new (std::nothrow) T(a);
  if (unlikely(t == nullptr))
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), static_cast<int>(sizeof(T)));
  return t;
}

bool Opt_trace_context::start(bool support_I_S_arg,
                              bool support_dbug_or_missing_priv,
                              bool end_marker_arg, bool one_line_arg,
                              long offset_arg, long limit_arg,
                              ulong max_mem_size_arg, ulonglong features_arg) {
  DBUG_TRACE;

  if (I_S_disabled != 0) {
    DBUG_PRINT("opt", ("opt_trace is already disabled"));
    support_I_S_arg = false;
  }

  /*
    Decide on optimizations possible to realize the requested support.
    If I_S or debug output is requested, we need to create an Opt_trace_stmt.
    Same if we should support calls to Opt_trace_context::missing_privilege(),
    because that function requires an Opt_trace_stmt.
  */
  if (!support_I_S_arg && !support_dbug_or_missing_priv) {
    // The statement will not do tracing.
    if (likely(pimpl == nullptr) || pimpl->current_stmt_in_gen == nullptr) {
      /*
        This should be the most commonly taken branch in a release binary,
        when the connection rarely has optimizer tracing runtime-enabled.
        It's thus important that it's optimized: we can short-cut the creation
        and starting of Opt_trace_stmt, unlike in the next "else" branch.
      */
      return false;
    }
    /*
      If we come here, there is a parent statement which has a trace.
      Imagine that we don't create a trace for the child statement
      here. Then trace structures of the child will be accidentally attached
      to the parent's trace (as it is still 'current_stmt_in_gen', which
      constructors of Opt_trace_struct will use); thus the child's trace
      will be visible (as a chunk of the parent's trace). That would be
      incorrect. To avoid this, we create a trace for the child but with I_S
      output disabled; this changes 'current_stmt_in_gen', thus this child's
      trace structures will be attached to the child's trace and thus not be
      visible.
    */
  }

  DBUG_EXECUTE_IF("no_new_opt_trace_stmt", DBUG_ASSERT(0););

  if (pimpl == nullptr &&
      ((pimpl = new_nothrow_w_my_error<Opt_trace_context_impl>()) == nullptr))
    return true;

  /*
    If tracing is disabled by some caller, then don't change settings (offset
    etc). Doing otherwise would surely bring a problem.
  */
  if (I_S_disabled == 0) {
    /*
      Here we allow a stored routine's sub-statement to enable/disable
      tracing, or change settings. Thus in a stored routine's body, there can
      be some 'SET OPTIMIZER_TRACE="enabled=[on|off]"' to trace only certain
      sub-statements.
    */
    pimpl->end_marker = end_marker_arg;
    pimpl->one_line = one_line_arg;
    pimpl->offset = offset_arg;
    pimpl->limit = limit_arg;
    pimpl->max_mem_size = max_mem_size_arg;
    // MISC always on
    pimpl->features = Opt_trace_context::feature_value(features_arg |
                                                       Opt_trace_context::MISC);
  }
  if (support_I_S_arg && pimpl->offset >= 0) {
    /* If outside the offset/limit window, no need to support I_S */
    if (pimpl->since_offset_0 < pimpl->offset) {
      DBUG_PRINT("opt", ("disabled: since_offset_0(%ld) < offset(%ld)",
                         pimpl->since_offset_0, pimpl->offset));
      support_I_S_arg = false;
    } else if (pimpl->since_offset_0 >= (pimpl->offset + pimpl->limit)) {
      DBUG_PRINT("opt", ("disabled: since_offset_0(%ld) >="
                         " offset(%ld) + limit(%ld)",
                         pimpl->since_offset_0, pimpl->offset, pimpl->limit));
      support_I_S_arg = false;
    }
    pimpl->since_offset_0++;
  }
  {
    /*
      We don't allocate it in THD's MEM_ROOT as it must survive until a next
      statement (SELECT) reads the trace.
    */
    Opt_trace_stmt *stmt = new_nothrow_w_my_error<Opt_trace_stmt>(this);

    DBUG_PRINT("opt", ("new stmt %p support_I_S %d", stmt, support_I_S_arg));

    if (unlikely(stmt == nullptr || pimpl->stack_of_current_stmts.push_back(
                                        pimpl->current_stmt_in_gen)))
      goto err;  // push_back() above called my_error()

    /*
      If sending only to DBUG, don't show to the user.
      Same if tracing was temporarily disabled at higher layers with
      Opt_trace_disable_I_S.
      So we just link it to the 'del' list for purging when ended.
    */
    Opt_trace_stmt_array *list;
    if (support_I_S_arg)
      list = &pimpl->all_stmts_for_I_S;
    else {
      stmt->disable_I_S();  // no need to fill a not-shown JSON trace
      list = &pimpl->all_stmts_to_del;
    }

    if (unlikely(list->push_back(stmt))) goto err;

    pimpl->current_stmt_in_gen = stmt;

    // As we just added one trace, maybe the previous ones are unneeded now
    purge_stmts(false);
    // This purge may have freed space, compute max allowed size:
    stmt->set_allowed_mem_size(allowed_mem_size_for_current_stmt());
    return false;
  err:
    delete stmt;
    DBUG_ASSERT(0);
    return true;
  }
}

void Opt_trace_context::end() {
  DBUG_ASSERT(I_S_disabled >= 0);
  if (likely(pimpl == nullptr)) return;
  if (pimpl->current_stmt_in_gen != nullptr) {
    pimpl->current_stmt_in_gen->end();
    /*
      pimpl was constructed with current_stmt_in_gen=NULL which was pushed in
      'start()'. So this NULL is in the array, back() is safe.
    */
    Opt_trace_stmt *const parent = pimpl->stack_of_current_stmts.back();
    pimpl->stack_of_current_stmts.pop_back();
    pimpl->current_stmt_in_gen = parent;
    if (parent != nullptr) {
      /*
        Parent regains control, now it needs to be told that its child has
        used space, and thus parent's allowance has shrunk.
      */
      parent->set_allowed_mem_size(allowed_mem_size_for_current_stmt());
    }
    /*
      Purge again. Indeed when we are here, compared to the previous start()
      we have one more ended trace, so can potentially free more. Consider
      offset=-1 and:
         top_stmt, started
           sub_stmt, starts: can't free top_stmt as it is not ended yet
           sub_stmt, ends: won't free sub_stmt (as user will want to see it),
           can't free top_stmt as not ended yet
         top_stmt, continued
         top_stmt, ends: free top_stmt as it's not last and is ended, keep
         only sub_stmt.
      Still the purge is done in ::start() too, as an optimization, for this
      case:
         sub_stmt, started
         sub_stmt, ended
         sub_stmt, starts: can free above sub_stmt, will save memory compared
         to free-ing it only when the new sub_stmt ends.
    */
    purge_stmts(false);
  } else
    DBUG_ASSERT(pimpl->stack_of_current_stmts.size() == 0);
}

bool Opt_trace_context::support_I_S() const {
  return (pimpl != nullptr) && (pimpl->current_stmt_in_gen != nullptr) &&
         pimpl->current_stmt_in_gen->support_I_S();
}

void Opt_trace_context::purge_stmts(bool purge_all) {
  DBUG_TRACE;
  if (!purge_all && pimpl->offset >= 0) {
    /* This case is managed in @c Opt_trace_context::start() */
    return;
  }
  long idx;
  static_assert(
      static_cast<long>(static_cast<size_t>(LONG_MAX)) == LONG_MAX,
      "Every positive long must be able to round-trip through size_t.");
  /*
    Start from the newest traces (array's end), scroll back in time. This
    direction is necessary, as we may delete elements from the array (assume
    purge_all=true and array has 2 elements and we traverse starting from
    index 0: cell 0 is deleted, making cell 1 become cell 0; index is
    incremented to 1, which is past the array's end, so break out of the loop:
    cell 0 (old cell 1) was not deleted, wrong).
  */
  for (idx = (pimpl->all_stmts_for_I_S.size() - 1); idx >= 0; idx--) {
    // offset can be negative, so cast size() to signed!
    if (!purge_all && ((static_cast<long>(pimpl->all_stmts_for_I_S.size()) +
                        pimpl->offset) <= idx)) {
      /* OFFSET mandates that this trace should be kept; move to previous */
    } else {
      /*
        Remember to free it (as in @c free()) when possible. For now, make it
        invisible in OPTIMIZER_TRACE table.
      */
      DBUG_EXECUTE_IF("opt_trace_oom_in_purge",
                      DBUG_SET("+d,simulate_out_of_memory"););
      if (likely(!pimpl->all_stmts_to_del.push_back(
              pimpl->all_stmts_for_I_S.at(idx))))
        pimpl->all_stmts_for_I_S.erase(idx);
      else {
        /*
          OOM. Cannot purge. Which at worse should only break the
          offset/limit feature (the trace will accidentally still show up in
          the OPTIMIZER_TRACE table). append() above has called my_error().
        */
      }
      DBUG_EXECUTE_IF("opt_trace_oom_in_purge",
                      DBUG_SET("-d,simulate_out_of_memory"););
    }
  }
  /* Examine list of "to be freed" traces and free what can be */
  for (idx = (pimpl->all_stmts_to_del.size() - 1); idx >= 0; idx--) {
    Opt_trace_stmt *stmt = pimpl->all_stmts_to_del.at(idx);
#ifndef DBUG_OFF
    bool skip_del = false;
    DBUG_EXECUTE_IF("opt_trace_oom_in_purge", skip_del = true;);
#else
    const bool skip_del = false;
#endif
    if (!stmt->has_ended() || skip_del) {
      /*
        This trace is not finished, freeing it now would lead to use of
        freed memory if a structure is later added to it. This would be
        possible: assume OFFSET=-1 and we have
        CALL statement starts executing
          create its trace (call it "trace #1")
          add structure to trace #1
          add structure to trace #1
          First sub-statement executing
            create its trace (call it "trace #2")
            from then on, trace #1 is not needed, free() it
            add structure to trace #2
            add structure to trace #2
          First sub-statement ends
          add structure to trace #1 - oops, adding to a free()d trace!
        So if a trace is not finished, we will wait until it is and
        re-consider it then (which is why this function is called in @c
        Opt_trace_stmt::end() too).

        In unit testing, to simulate OOM, we let the list grow so
        that it consumes its pre-allocated cells and finally requires a
        (failing) allocation.
      */
    } else {
      pimpl->all_stmts_to_del.erase(idx);
      delete stmt;
    }
  }
}

size_t Opt_trace_context::allowed_mem_size_for_current_stmt() const {
  size_t mem_size = 0;
  int idx;
  for (idx = (pimpl->all_stmts_for_I_S.size() - 1); idx >= 0; idx--) {
    const Opt_trace_stmt *stmt = pimpl->all_stmts_for_I_S.at(idx);
    mem_size += stmt->alloced_length();
  }
  // Even to-be-deleted traces use memory, so consider them in sum
  for (idx = (pimpl->all_stmts_to_del.size() - 1); idx >= 0; idx--) {
    const Opt_trace_stmt *stmt = pimpl->all_stmts_to_del.at(idx);
    mem_size += stmt->alloced_length();
  }
  /* The current statement is in exactly one of the two lists above */
  mem_size -= pimpl->current_stmt_in_gen->alloced_length();
  size_t rc =
      (mem_size <= pimpl->max_mem_size) ? (pimpl->max_mem_size - mem_size) : 0;
  DBUG_PRINT("opt", ("rc %llu max_mem_size %llu", (ulonglong)rc,
                     (ulonglong)pimpl->max_mem_size));
  return rc;
}

void Opt_trace_context::set_query(const char *query, size_t length,
                                  const CHARSET_INFO *charset) {
  pimpl->current_stmt_in_gen->set_query(query, length, charset);
}

void Opt_trace_context::reset() {
  if (pimpl == nullptr) return;
  purge_stmts(true);
  pimpl->since_offset_0 = 0;
}

void Opt_trace_context::Opt_trace_context_impl::
    disable_I_S_for_this_and_children() {
  if (current_stmt_in_gen != nullptr) current_stmt_in_gen->disable_I_S();
}

void Opt_trace_context::Opt_trace_context_impl::restore_I_S() {
  if (current_stmt_in_gen != nullptr) current_stmt_in_gen->restore_I_S();
}

void Opt_trace_context::missing_privilege() {
  /*
    By storing the 'missing_priv' mark in Opt_trace_stmt instead of in
    Opt_trace_context we get automatic re-enabling of I_S when the stmt ends,
    Opt_trace_stmt::missing_priv being the "memory" of where I_S has been
    disabled.
    Storing in Opt_trace_context would require an external memory (probably a
    RAII object), which would not be possible in
    TABLE_LIST::prepare_security(), where I_S must be disabled even after the
    end of that function - so RAII would not work.

    Which is why this function needs an existing current_stmt_in_gen.
  */
  pimpl->current_stmt_in_gen->missing_privilege();
}

const Opt_trace_stmt *Opt_trace_context::get_next_stmt_for_I_S(
    long *got_so_far) const {
  const Opt_trace_stmt *p;
  if ((pimpl == nullptr) || (*got_so_far >= pimpl->limit) ||
      (*got_so_far >= static_cast<long>(pimpl->all_stmts_for_I_S.size())))
    p = nullptr;
  else {
    p = pimpl->all_stmts_for_I_S.at(*got_so_far);
    DBUG_ASSERT(p != nullptr);
    (*got_so_far)++;
  }
  return p;
}

// Implementation of class Opt_trace_iterator

Opt_trace_iterator::Opt_trace_iterator(Opt_trace_context *ctx_arg)
    : ctx(ctx_arg), row_count(0) {
  next();
}

void Opt_trace_iterator::next() {
  cursor = ctx->get_next_stmt_for_I_S(&row_count);
}

void Opt_trace_iterator::get_value(Opt_trace_info *info) const {
  cursor->fill_info(info);
}
