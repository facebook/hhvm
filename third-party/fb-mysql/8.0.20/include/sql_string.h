#ifndef SQL_STRING_INCLUDED
#define SQL_STRING_INCLUDED

/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file include/sql_string.h
  Our own string classes, used pervasively throughout the executor.
  See in particular the comment on String before you use anything from here.
*/

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <new>
#include <string>

#include "lex_string.h"
#include "m_ctype.h"   // my_convert
#include "m_string.h"  // LEX_CSTRING
#include "memory_debugging.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/mysql_lex_string.h"  // LEX_STRING
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"  // my_free

struct MEM_ROOT;

#ifdef MYSQL_SERVER
extern PSI_memory_key key_memory_String_value;
#define STRING_PSI_MEMORY_KEY key_memory_String_value
#else
#define STRING_PSI_MEMORY_KEY PSI_NOT_INSTRUMENTED
#endif

/**
  A wrapper class for null-terminated constant strings.
  Constructors make sure that the position of the '\0' terminating byte
  in m_str is always in sync with m_length.

  This class must stay as small as possible as we often
  pass it and its descendants (such as Name_string) into functions
  using call-by-value evaluation.

  Don't add new members or virual methods into this class!
*/
class Simple_cstring {
 private:
  const char *m_str;
  size_t m_length;

 public:
  /**
    Initialize from a C string whose length is already known.
  */
  void set(const char *str_arg, size_t length_arg) {
    // NULL is allowed only with length==0
    DBUG_ASSERT(str_arg || length_arg == 0);
    // For non-NULL, make sure length_arg is in sync with '\0' terminator.
    DBUG_ASSERT(!str_arg || str_arg[length_arg] == '\0');
    m_str = str_arg;
    m_length = length_arg;
  }
  Simple_cstring() { set(nullptr, 0); }
  Simple_cstring(const char *str_arg, size_t length_arg) {
    set(str_arg, length_arg);
  }
  Simple_cstring(const LEX_STRING arg) { set(arg.str, arg.length); }
  Simple_cstring(const LEX_CSTRING arg) { set(arg.str, arg.length); }
  void reset() { set(nullptr, 0); }
  /**
    Set to a null-terminated string.
  */
  void set(const char *str) { set(str, str ? strlen(str) : 0); }
  /**
    Return string buffer.
  */
  const char *ptr() const { return m_str; }
  /**
    Check if m_ptr is set.
  */
  bool is_set() const { return m_str != nullptr; }
  /**
    Return name length.
  */
  size_t length() const { return m_length; }
  /**
    Compare to another Simple_cstring.
  */
  bool eq_bin(const Simple_cstring other) const {
    return m_length == other.m_length &&
           memcmp(m_str, other.m_str, m_length) == 0;
  }
  /**
    Copy to the given buffer
  */
  void strcpy(char *buff) const {
    memcpy(buff, m_str, m_length);
    buff[m_length] = '\0';
  }
};

class String;

struct CHARSET_INFO;
struct IO_CACHE;

bool validate_string(const CHARSET_INFO *cs, const char *str, size_t length,
                     size_t *valid_length, bool *length_error);
int sortcmp(const String *a, const String *b, const CHARSET_INFO *cs);
String *copy_if_not_alloced(String *to, String *from, size_t from_length);
inline size_t copy_and_convert(char *to, size_t to_length,
                               const CHARSET_INFO *to_cs, const char *from,
                               size_t from_length, const CHARSET_INFO *from_cs,
                               uint *errors) {
  return my_convert(to, to_length, to_cs, from, from_length, from_cs, errors);
}
size_t well_formed_copy_nchars(const CHARSET_INFO *to_cs, char *to,
                               size_t to_length, const CHARSET_INFO *from_cs,
                               const char *from, size_t from_length,
                               size_t nchars,
                               const char **well_formed_error_pos,
                               const char **cannot_convert_error_pos,
                               const char **from_end_pos);
size_t convert_to_printable(char *to, size_t to_len, const char *from,
                            size_t from_len, const CHARSET_INFO *from_cs,
                            size_t nbytes = 0);

size_t bin_to_hex_str(char *to, size_t to_len, const char *from,
                      size_t from_len);

/**
  Using this class is fraught with peril, and you need to be very careful
  when doing so. In particular, copy construction and assignment does not
  do a deep _nor_ a shallow copy; instead, it makes a _reference_ to the
  original string that will be invalid as soon as that string goes out of scope.
  (Move constructiong and assignment is safe, though.) In general, it is
  probably better not to use this class at all if you can avoid it.
*/
class String {
  char *m_ptr;
  size_t m_length;
  const CHARSET_INFO *m_charset;
  uint32
      m_alloced_length;  // should be size_t, but kept uint32 for size reasons
  bool m_is_alloced;

 public:
  String()
      : m_ptr(nullptr),
        m_length(0),
        m_charset(&my_charset_bin),
        m_alloced_length(0),
        m_is_alloced(false) {}
  explicit String(size_t length_arg)
      : m_ptr(nullptr),
        m_length(0),
        m_charset(&my_charset_bin),
        m_alloced_length(0),
        m_is_alloced(false) {
    (void)real_alloc(length_arg);
  }
  String(const char *str, const CHARSET_INFO *cs)
      : m_ptr(const_cast<char *>(str)),
        m_length(strlen(str)),
        m_charset(cs),
        m_alloced_length(0),
        m_is_alloced(false) {}
  String(const char *str, size_t len, const CHARSET_INFO *cs)
      : m_ptr(const_cast<char *>(str)),
        m_length(len),
        m_charset(cs),
        m_alloced_length(0),
        m_is_alloced(false) {}
  String(char *str, size_t len, const CHARSET_INFO *cs)
      : m_ptr(str),
        m_length(len),
        m_charset(cs),
        m_alloced_length(static_cast<uint32>(len)),
        m_is_alloced(false) {}
  String(const String &str)
      : m_ptr(str.m_ptr),
        m_length(str.m_length),
        m_charset(str.m_charset),
        m_alloced_length(static_cast<uint32>(str.m_alloced_length)),
        m_is_alloced(false) {}
  String(String &&str) noexcept
      : m_ptr(str.m_ptr),
        m_length(str.m_length),
        m_charset(str.m_charset),
        m_alloced_length(str.m_alloced_length),
        m_is_alloced(str.m_is_alloced) {
    str.m_is_alloced = false;
  }
  static void *operator new(size_t size, MEM_ROOT *mem_root,
                            const std::nothrow_t &arg MY_ATTRIBUTE((unused)) =
                                std::nothrow) noexcept {
    return mem_root->Alloc(size);
  }
  static void operator delete(void *ptr_arg, size_t size) {
    (void)ptr_arg;
    (void)size;
    TRASH(ptr_arg, size);
  }

  static void operator delete(
      void *, MEM_ROOT *, const std::nothrow_t &)noexcept { /* never called */
  }

  ~String() { mem_free(); }

  void set_charset(const CHARSET_INFO *charset_arg) { m_charset = charset_arg; }
  const CHARSET_INFO *charset() const { return m_charset; }
  size_t length() const { return m_length; }
  size_t alloced_length() const { return m_alloced_length; }
  const char &operator[](size_t i) const { return m_ptr[i]; }
  char &operator[](size_t i) { return m_ptr[i]; }
  void length(size_t len) { m_length = len; }
  bool is_empty() const { return (m_length == 0); }
  void mark_as_const() { m_alloced_length = 0; }
  /* Returns a pointer to data, may not include NULL terminating character. */
  const char *ptr() const { return m_ptr; }
  char *ptr() { return m_ptr; }
  char *c_ptr() {
    DBUG_ASSERT(!m_is_alloced || !m_ptr || !m_alloced_length ||
                (m_alloced_length >= (m_length + 1)));

    /*
      Should be safe, but in case valgrind complains on this line, it means
      there is a misuse of c_ptr(). Please prefer <ptr(), length()> instead.
    */
    if (!m_ptr || m_ptr[m_length]) (void)mem_realloc(m_length);
    return m_ptr;
  }
  char *c_ptr_quick() {
    if (m_ptr && m_length < m_alloced_length) m_ptr[m_length] = 0;
    return m_ptr;
  }
  char *c_ptr_safe() {
    if (m_ptr && m_length < m_alloced_length)
      m_ptr[m_length] = 0;
    else
      (void)mem_realloc(m_length);
    return m_ptr;
  }
  LEX_STRING lex_string() { return {m_ptr, length()}; }

  LEX_CSTRING lex_cstring() const {
    LEX_CSTRING lex_cstring = {ptr(), length()};
    return lex_cstring;
  }

  void set(String &str, size_t offset, size_t arg_length) {
    DBUG_ASSERT(&str != this);
    mem_free();
    m_ptr = str.ptr() + offset;
    m_length = arg_length;
    m_is_alloced = false;
    if (str.m_alloced_length)
      m_alloced_length = str.m_alloced_length - static_cast<uint32>(offset);
    else
      m_alloced_length = 0;
    m_charset = str.m_charset;
  }

  /**
     Points the internal buffer to the supplied one. The old buffer is freed.
     @param str Pointer to the new buffer.
     @param arg_length Length of the new buffer in characters, excluding any
            null character.
     @param cs Character set to use for interpreting string data.
     @note The new buffer will not be null terminated.
  */
  void set(char *str, size_t arg_length, const CHARSET_INFO *cs) {
    mem_free();
    m_ptr = str;
    m_length = m_alloced_length = static_cast<uint32>(arg_length);
    m_is_alloced = false;
    m_charset = cs;
  }
  void set(const char *str, size_t arg_length, const CHARSET_INFO *cs) {
    mem_free();
    m_ptr = const_cast<char *>(str);
    m_length = arg_length;
    m_alloced_length = 0;
    m_is_alloced = false;
    m_charset = cs;
  }
  bool set_ascii(const char *str, size_t arg_length);
  void set_quick(char *str, size_t arg_length, const CHARSET_INFO *cs) {
    if (!m_is_alloced) {
      m_ptr = str;
      m_length = arg_length;
      m_alloced_length = static_cast<uint32>(arg_length);
    }
    m_charset = cs;
  }
  bool set_int(longlong num, bool unsigned_flag, const CHARSET_INFO *cs);
  bool set(longlong num, const CHARSET_INFO *cs) {
    return set_int(num, false, cs);
  }
  bool set(ulonglong num, const CHARSET_INFO *cs) {
    return set_int((longlong)num, true, cs);
  }

  /**
    Sets the contents of this string to the string representation of the given
    double value.

    @param num the double value
    @param decimals the number of decimals
    @param cs the character set of the string
    @return false on success, true on error
  */
  bool set_real(double num, uint decimals, const CHARSET_INFO *cs);

  /*
    PMG 2004.11.12
    This is a method that works the same as perl's "chop". It simply
    drops the last byte of a string. This is useful in the case
    of the federated storage handler where I'm building a unknown
    number, list of values and fields to be used in a sql insert
    statement to be run on the remote server, and have a comma after each.
    When the list is complete, I "chop" off the trailing comma

    ex.
      String stringobj;
      stringobj.append("VALUES ('foo', 'fi', 'fo',");
      stringobj.chop();
      stringobj.append(")");

    In this case, the value of string was:

    VALUES ('foo', 'fi', 'fo',
    VALUES ('foo', 'fi', 'fo'
    VALUES ('foo', 'fi', 'fo')

    This is not safe to call when the string ends in a multi-byte character!
  */
  void chop() {
    m_length--;
    m_ptr[m_length] = '\0';
  }

  void mem_claim() {
    if (m_is_alloced) {
      my_claim(m_ptr);
    }
  }

  void mem_free() {
    if (m_is_alloced) {
      m_is_alloced = false;
      m_alloced_length = 0;
      my_free(m_ptr);
      m_ptr = nullptr;
      m_length = 0; /* Safety */
    }
  }

  bool alloc(size_t arg_length) {
    if (arg_length < m_alloced_length) return false;
    return real_alloc(arg_length);
  }
  bool real_alloc(size_t arg_length);  // Empties old string
  bool mem_realloc(size_t arg_length, bool force_on_heap = false);

 private:
  size_t next_realloc_exp_size(size_t sz);
  bool mem_realloc_exp(size_t arg_length);

 public:
  // Shrink the buffer, but only if it is allocated on the heap.
  void shrink(size_t arg_length) {
    if (!is_alloced()) return;
    if (arg_length < m_alloced_length) {
      char *new_ptr;
      if (!(new_ptr = static_cast<char *>(my_realloc(
                STRING_PSI_MEMORY_KEY, m_ptr, arg_length, MYF(0))))) {
        m_alloced_length = 0;
        real_alloc(arg_length);
      } else {
        m_ptr = new_ptr;
        m_alloced_length = static_cast<uint32>(arg_length);
      }
    }
  }
  bool is_alloced() const { return m_is_alloced; }
  String &operator=(const String &s) {
    if (&s != this) {
      /*
        It is forbidden to do assignments like
        some_string = substring_of_that_string
       */
      DBUG_ASSERT(!s.uses_buffer_owned_by(this));
      mem_free();
      m_ptr = s.m_ptr;
      m_length = s.m_length;
      m_alloced_length = s.m_alloced_length;
      m_charset = s.m_charset;
      m_is_alloced = false;
    }
    return *this;
  }
  String &operator=(String &&s) noexcept {
    if (&s != this) {
      /*
        It is forbidden to do assignments like
        some_string = substring_of_that_string
       */
      DBUG_ASSERT(!s.uses_buffer_owned_by(this));
      mem_free();
      m_ptr = s.m_ptr;
      m_length = s.m_length;
      m_alloced_length = s.m_alloced_length;
      m_charset = s.m_charset;
      // This is the primary difference between move and copy.
      m_is_alloced = s.m_is_alloced;
      s.m_is_alloced = false;
    }
    return *this;
  }
  /**
    Takeover the buffer owned by another string.
    "this" becomes the owner of the buffer and
    is further responsible to free it.
    The string "s" is detached from the buffer (cleared).

    @param s - a String object to steal buffer from.
  */
  void takeover(String &s) {
    DBUG_ASSERT(this != &s);
    // Make sure buffers of the two Strings do not overlap
    DBUG_ASSERT(!s.uses_buffer_owned_by(this));
    mem_free();
    m_ptr = s.m_ptr;
    m_length = s.m_length;
    m_alloced_length = s.m_alloced_length;
    m_is_alloced = s.m_is_alloced;
    m_charset = s.m_charset;
    s.m_ptr = nullptr;
    s.m_alloced_length = 0;
    s.m_length = 0;
    s.m_is_alloced = false;
  }

  bool copy();                 // Alloc string if not alloced
  bool copy(const String &s);  // Allocate new string
  // Allocate new string
  bool copy(const char *s, size_t arg_length, const CHARSET_INFO *cs);
  static bool needs_conversion(size_t arg_length, const CHARSET_INFO *cs_from,
                               const CHARSET_INFO *cs_to, size_t *offset);
  bool needs_conversion(const CHARSET_INFO *cs_to) const {
    size_t offset;
    return needs_conversion(length(), charset(), cs_to, &offset);
  }
  bool is_valid_string(const CHARSET_INFO *cs_to) const {
    size_t valid_length;
    bool length_error;
    return !validate_string(cs_to, ptr(), length(), &valid_length,
                            &length_error);
  }
  static bool needs_conversion_on_storage(size_t arg_length,
                                          const CHARSET_INFO *cs_from,
                                          const CHARSET_INFO *cs_to);
  bool copy_aligned(const char *s, size_t arg_length, size_t offset,
                    const CHARSET_INFO *cs);
  bool set_or_copy_aligned(const char *s, size_t arg_length,
                           const CHARSET_INFO *cs);
  bool copy(const char *s, size_t arg_length, const CHARSET_INFO *csfrom,
            const CHARSET_INFO *csto, uint *errors);
  bool append(const String &s);
  bool append(const char *s);
  bool append(LEX_STRING *ls) { return append(ls->str, ls->length); }
  bool append(Simple_cstring str) { return append(str.ptr(), str.length()); }
  bool append(const char *s, size_t arg_length);
  bool append(const char *s, size_t arg_length, const CHARSET_INFO *cs);
  bool append_ulonglong(ulonglong val);
  bool append_longlong(longlong val);
  bool append_with_prefill(const char *s, size_t arg_length, size_t full_length,
                           char fill_char);
  bool append_parenthesized(int64_t nr);
  /**
    Search for a substring.

    @param search    substring to search for
    @param offset    starting point, bytes from the start of the string

    @return byte offset to the substring from the start of this string
    @retval -1 if the substring is not found starting from the offset
  */
  int strstr(const String &search, size_t offset = 0) const;
  /**
    Reverse search for a substring.

    @param search    substring to search for
    @param offset    starting point, bytes from the start of the string

    @return byte offset to the substring from the start of this string
    @retval -1 if the substring is not found starting from the offset
  */
  int strrstr(const String &search, size_t offset = 0) const;
  /**
   * Returns substring of given characters lenght, starting at given character
   * offset. Note that parameter indexes are character indexes and not byte
   * indexes.
   */
  String substr(int offset, int count) const;

  bool replace(size_t offset, size_t arg_length, const char *to, size_t length);
  bool replace(size_t offset, size_t arg_length, const String &to);
  bool append(char chr) {
    if (m_length < m_alloced_length) {
      m_ptr[m_length++] = chr;
    } else {
      if (mem_realloc_exp(m_length + 1)) return true;
      m_ptr[m_length++] = chr;
    }
    return false;
  }
  bool fill(size_t max_length, char fill);
  friend int sortcmp(const String *a, const String *b, const CHARSET_INFO *cs);
  friend int stringcmp(const String *a, const String *b);
  friend String *copy_if_not_alloced(String *to, String *from,
                                     size_t from_length);
  size_t numchars() const;
  size_t charpos(size_t i, size_t offset = 0) const;

  bool reserve(size_t space_needed) {
    return mem_realloc(m_length + space_needed);
  }
  bool reserve(size_t space_needed, size_t grow_by);

  /* Inline (general) functions used by the protocol functions */

  char *prep_append(size_t arg_length, size_t step_alloc) {
    size_t new_length = arg_length + m_length;
    if (new_length > m_alloced_length) {
      if (mem_realloc(new_length + step_alloc)) return nullptr;
    }
    size_t old_length = m_length;
    m_length += arg_length;
    return m_ptr + old_length; /* Area to use */
  }

  bool append(const char *s, size_t arg_length, size_t step_alloc) {
    size_t new_length = arg_length + m_length;
    if (new_length > m_alloced_length &&
        mem_realloc_exp(new_length + step_alloc))
      return true;
    memcpy(m_ptr + m_length, s, arg_length);
    m_length += arg_length;
    return false;
  }
  void print(String *print) const;

  /* Swap two string objects. Efficient way to exchange data without memcpy. */
  void swap(String &s) noexcept;

  bool uses_buffer_owned_by(const String *s) const {
    return (s->m_is_alloced && m_ptr >= s->m_ptr &&
            m_ptr < s->m_ptr + s->m_length);
  }
  bool is_ascii() const {
    if (length() == 0) return true;
    if (charset()->mbminlen > 1) return false;
    for (const char *c = ptr(), *end = c + length(); c < end; c++) {
      if (!my_isascii(*c)) return false;
    }
    return true;
  }
  /**
    Make a zero-terminated copy of our value,allocated in the specified MEM_ROOT

    @param root         MEM_ROOT to allocate the result

    @return allocated string or NULL
  */
  char *dup(MEM_ROOT *root) const;
};

static inline void swap(String &a, String &b) noexcept { a.swap(b); }

static inline std::string to_string(const String &str) {
  return std::string(str.ptr(), str.length());
}

/**
  String class wrapper with a preallocated buffer of size buff_sz

  This class allows to replace sequences of:
     char buff[12345];
     String str(buff, sizeof(buff));
     str.length(0);
  with a simple equivalent declaration:
     StringBuffer<12345> str;
*/

template <size_t buff_sz>
class StringBuffer : public String {
  char buff[buff_sz];

 public:
  StringBuffer() : String(buff, buff_sz, &my_charset_bin) { length(0); }
  explicit StringBuffer(const CHARSET_INFO *cs) : String(buff, buff_sz, cs) {
    length(0);
  }
  StringBuffer(const char *str, size_t length, const CHARSET_INFO *cs)
      : String(buff, buff_sz, cs) {
    set(str, length, cs);
  }
};

static inline bool check_if_only_end_space(const CHARSET_INFO *cs,
                                           const char *str, const char *end) {
  return str + cs->cset->scan(cs, str, end, MY_SEQ_SPACES) == end;
}

inline LEX_CSTRING to_lex_cstring(const LEX_STRING &s) {
  LEX_CSTRING cstr = {s.str, s.length};
  return cstr;
}

inline LEX_STRING to_lex_string(const LEX_CSTRING &s) {
  LEX_STRING str = {const_cast<char *>(s.str), s.length};
  return str;
}

inline LEX_CSTRING to_lex_cstring(const char *s) {
  LEX_CSTRING cstr = {s, s != nullptr ? strlen(s) : 0};
  return cstr;
}

bool append_escaped(String *to_str, const String *from_str);

#endif /* SQL_STRING_INCLUDED */
