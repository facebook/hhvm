/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql_string.h"

#include <algorithm>

#include "my_dbug.h"
#include "my_macros.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "mysql_com.h"  // MAX_BIGINT_WIDTH

using std::max;
using std::min;

#ifdef MYSQL_SERVER
PSI_memory_key key_memory_String_value;
#endif

/*****************************************************************************
** String functions
*****************************************************************************/

bool String::real_alloc(size_t length) {
  size_t arg_length = ALIGN_SIZE(length + 1);
  DBUG_ASSERT(arg_length > length);
  if (arg_length <= length) return true; /* Overflow */
  m_length = 0;
  if (m_alloced_length < arg_length) {
    mem_free();
    if (!(m_ptr = static_cast<char *>(
              my_malloc(STRING_PSI_MEMORY_KEY, arg_length, MYF(MY_WME)))))
      return true;
    m_alloced_length = static_cast<uint32>(arg_length);
    m_is_alloced = true;
  }
  m_ptr[0] = 0;
  return false;
}

/**
   Allocates a new buffer on the heap for this String.

   - If the String's internal buffer is privately owned and heap allocated,
     one of the following is performed.

     - If the requested length is greater than what fits in the buffer, a new
       buffer is allocated, data moved and the old buffer freed.

     - If the requested length is less or equal to what fits in the buffer, a
       null character is inserted at the appropriate position.

   - If the String does not keep a private buffer on the heap:

      - If the requested length is greater than what fits in the buffer, or
        force_on_heap is true, a new buffer is allocated, data is copied.
      - If the requested length is less or equal to what fits in the buffer,
        and force_on_heap is false, a null character is inserted at the
        appropriate position.

   For C compatibility, the new string buffer is null terminated.

   @param alloc_length The requested string size in characters, excluding any
   null terminator.
   @param force_on_heap If the caller wants String's 'str' buffer to be on the
   heap in all cases.

   @retval false Either the copy operation is complete or, if the size of the
   new buffer is smaller than the currently allocated buffer (if one exists),
   no allocation occurred.

   @retval true An error occurred when attempting to allocate memory or memory
   allocation length exceeded allowed limit (4GB) for String Class.
*/
bool String::mem_realloc(size_t alloc_length, bool force_on_heap) {
  size_t len = ALIGN_SIZE(alloc_length + 1);
  DBUG_ASSERT(len > alloc_length);
  if (len <= alloc_length) return true; /* Overflow */

  if (force_on_heap && !m_is_alloced) {
    /*
      Caller wants bytes on the heap, and the currently available bytes are
      not; they are thus irrelevant:
      */
    m_alloced_length = 0;
  }

  if (m_alloced_length < len) {  // Available bytes are not enough.
    // Signal an error if len exceeds uint32 max on 64-bit word platform.
#if defined(__WORDSIZE) && (__WORDSIZE == 64)
    if (len > std::numeric_limits<uint32>::max()) return true;
#endif
    char *new_ptr;
    if (m_is_alloced) {
      if (!(new_ptr = static_cast<char *>(
                my_realloc(STRING_PSI_MEMORY_KEY, m_ptr, len, MYF(MY_WME)))))
        return true;  // Signal error
    } else if ((new_ptr = static_cast<char *>(
                    my_malloc(STRING_PSI_MEMORY_KEY, len, MYF(MY_WME))))) {
      if (m_length > len - 1) m_length = 0;
      if (m_length > 0) memcpy(new_ptr, m_ptr, m_length);
      new_ptr[m_length] = 0;
      m_is_alloced = true;
    } else
      return true;  // Signal error
    m_ptr = new_ptr;
    m_alloced_length = static_cast<uint32>(len);
  }
  m_ptr[alloc_length] = 0;  // This make other funcs shorter
  return false;
}

/*
  Helper function for @see mem_realloc_exp.
 */
inline size_t String::next_realloc_exp_size(size_t sz) {
  const size_t len = ALIGN_SIZE(sz + 1);
  const size_t ret =
      (m_is_alloced && m_alloced_length < len) ? sz + (m_length / 4) : sz;
  return ret;
}

/**
  This function is used by the various append() and replace() member functions,
  to ensure that functions have amortized constant cost.
  Once we have started to allocate buffer on the heap, we increase the buffer
  size exponentially, rather than linearly.

  @param alloc_length The requested string size in characters, excluding any
                      null terminator.

  @retval false Either the copy operation is complete or, if the size of the
  new buffer is smaller than the currently allocated buffer (if one exists),
  no allocation occurred.

  @retval true An error occurred when attempting to allocate memory.

  @see mem_realloc.
 */
bool String::mem_realloc_exp(size_t alloc_length) {
  if (mem_realloc(next_realloc_exp_size(alloc_length))) return true;
  m_ptr[alloc_length] = '\0';
  return false;
}

bool String::set_int(longlong num, bool unsigned_flag, const CHARSET_INFO *cs) {
  uint l = 20 * cs->mbmaxlen + 1;
  int base = unsigned_flag ? 10 : -10;

  if (alloc(l)) return true;
  m_length = (uint32)(cs->cset->longlong10_to_str)(cs, m_ptr, l, base, num);
  m_charset = cs;
  return false;
}

bool String::set_real(double num, uint decimals, const CHARSET_INFO *cs) {
  char buff[FLOATING_POINT_BUFFER];
  uint dummy_errors;

  if (decimals >= DECIMAL_NOT_SPECIFIED) {
    size_t len = my_gcvt(num, MY_GCVT_ARG_DOUBLE,
                         static_cast<int>(sizeof(buff)) - 1, buff, nullptr);
    return copy(buff, len, &my_charset_latin1, cs, &dummy_errors);
  }
  size_t len = my_fcvt(num, decimals, buff, nullptr);
  return copy(buff, len, &my_charset_latin1, cs, &dummy_errors);
}

bool String::copy() {
  if (!m_is_alloced) {
    m_alloced_length = 0;  // Force realloc
    return mem_realloc(m_length);
  }
  return false;
}

/**
   Copies the internal buffer from str. If this String has a private heap
   allocated buffer where new data does not fit, a new buffer is allocated
   before copying and the old buffer freed. Character set information is also
   copied.

   If str is the same as this and str doesn't own its buffer, a
   new buffer is allocated and it's owned by str.

   @param str The string whose internal buffer is to be copied.

   @retval false Success.
   @retval true Memory allocation failed.
*/
bool String::copy(const String &str) {
  /*
    If &str == this and it owns the buffer, this operation is a no-op, so skip
    the meaningless copy. Otherwise if we do, we will read freed memory at
    the memmove call below.
  */
  if (&str == this && str.is_alloced()) return false;

  /*
    If a String s doesn't own its buffer, here we should allocate
    a new buffer owned by s and copy the contents there. But alloc()
    will change this->m_ptr and this->m_length, and if this == &str, this
    will also change str->m_ptr and str->m_length, so we need to save
    these values first.
  */
  const size_t str_length = str.m_length;
  const char *str_ptr = str.m_ptr;
  if (alloc(str.m_length)) return true;
  m_length = str_length;
  if (m_length > 0) memmove(m_ptr, str_ptr, m_length);  // May be overlapping
  m_ptr[m_length] = 0;
  m_charset = str.m_charset;
  return false;
}

bool String::copy(const char *str, size_t arg_length, const CHARSET_INFO *cs) {
  if (alloc(arg_length)) return true;
  if ((m_length = arg_length)) memcpy(m_ptr, str, arg_length);
  m_ptr[arg_length] = 0;
  m_charset = cs;
  return false;
}

/*
  Checks that the source string can be just copied to the destination string
  without conversion.

  SYNPOSIS

  needs_conversion()
  arg_length		Length of string to copy.
  from_cs		Character set to copy from
  to_cs			Character set to copy to
  uint32 *offset	Returns number of unaligned characters.

  RETURN
   0  No conversion needed
   1  Either character set conversion or adding leading  zeros
      (e.g. for UCS-2) must be done

  NOTE
  to_cs may be NULL for "no conversion" if the system variable
  character_set_results is NULL.
*/

bool String::needs_conversion(size_t arg_length, const CHARSET_INFO *from_cs,
                              const CHARSET_INFO *to_cs, size_t *offset) {
  *offset = 0;
  if (!to_cs || (to_cs == &my_charset_bin) || (to_cs == from_cs) ||
      my_charset_same(from_cs, to_cs) ||
      ((from_cs == &my_charset_bin) &&
       (!(*offset = (arg_length % to_cs->mbminlen)))))
    return false;
  return true;
}

/*
  Checks that the source string can just be copied to the destination string
  without conversion.
  Unlike needs_conversion it will require conversion on incoming binary data
  to ensure the data are verified for vailidity first.

  @param arg_length   Length of string to copy.
  @param from_cs      Character set to copy from
  @param to_cs        Character set to copy to

  @return conversion needed
*/
bool String::needs_conversion_on_storage(size_t arg_length,
                                         const CHARSET_INFO *cs_from,
                                         const CHARSET_INFO *cs_to) {
  size_t offset;
  return (needs_conversion(arg_length, cs_from, cs_to, &offset) ||
          /* force conversion when storing a binary string */
          (cs_from == &my_charset_bin &&
           /* into a non-binary destination */
           cs_to != &my_charset_bin &&
           /* and any of the following is true :*/
           (
               /* it's a variable length encoding */
               cs_to->mbminlen != cs_to->mbmaxlen ||
               /* longer than 2 bytes : neither 1 byte nor ucs2 */
               cs_to->mbminlen > 2 ||
               /* and is not a multiple of the char byte size */
               0 != (arg_length % cs_to->mbmaxlen))));
}

/*
  Copy a multi-byte character sets with adding leading zeros.

  SYNOPSIS

  copy_aligned()
  str			String to copy
  arg_length		Length of string. This should NOT be dividable with
                        cs->mbminlen.
  offset		arg_length % cs->mb_minlength
  cs			Character set for 'str'

  NOTES
    For real multi-byte, ascii incompatible charactser sets,
    like UCS-2, add leading zeros if we have an incomplete character.
    Thus,
      SELECT _ucs2 0xAA
    will automatically be converted into
      SELECT _ucs2 0x00AA

  RETURN
    0  ok
    1  error
*/

bool String::copy_aligned(const char *str, size_t arg_length, size_t offset,
                          const CHARSET_INFO *cs) {
  /* How many bytes are in incomplete character */
  offset = cs->mbminlen - offset; /* How many zeros we should prepend */
  DBUG_ASSERT(offset && offset != cs->mbminlen);

  size_t aligned_length = arg_length + offset;
  if (alloc(aligned_length)) return true;

  /*
    Note, this is only safe for big-endian UCS-2.
    If we add little-endian UCS-2 sometimes, this code
    will be more complicated. But it's OK for now.
  */
  memset(m_ptr, 0, offset);
  memcpy(m_ptr + offset, str, arg_length);
  m_ptr[aligned_length] = 0;
  /* m_length is always >= 0 as arg_length is != 0 */
  m_length = aligned_length;
  m_charset = cs;
  return false;
}

bool String::set_or_copy_aligned(const char *str, size_t arg_length,
                                 const CHARSET_INFO *cs) {
  /* How many bytes are in incomplete character */
  size_t offset = (arg_length % cs->mbminlen);

  if (!offset) /* All characters are complete, just copy */
  {
    set(str, arg_length, cs);
    return false;
  }
  return copy_aligned(str, arg_length, offset, cs);
}

/**
   Copies the character data into this String, with optional character set
   conversion.

   @return
   false ok
   true  Could not allocate result buffer

*/

bool String::copy(const char *str, size_t arg_length,
                  const CHARSET_INFO *from_cs, const CHARSET_INFO *to_cs,
                  uint *errors) {
  size_t offset;

  DBUG_ASSERT(!str || str != m_ptr);

  if (!needs_conversion(arg_length, from_cs, to_cs, &offset)) {
    *errors = 0;
    return copy(str, arg_length, to_cs);
  }
  if ((from_cs == &my_charset_bin) && offset) {
    *errors = 0;
    return copy_aligned(str, arg_length, offset, to_cs);
  }
  size_t new_length = to_cs->mbmaxlen * arg_length;
  if (alloc(new_length)) return true;
  m_length = copy_and_convert(m_ptr, new_length, to_cs, str, arg_length,
                              from_cs, errors);
  m_charset = to_cs;
  return false;
}

/*
  Set a string to the value of a latin1-string, keeping the original charset

  SYNOPSIS
    copy_or_set()
    str			String of a simple charset (latin1)
    arg_length		Length of string

  IMPLEMENTATION
    If string object is of a simple character set, set it to point to the
    given string.
    If not, make a copy and convert it to the new character set.

  RETURN
    0	ok
    1	Could not allocate result buffer

*/

bool String::set_ascii(const char *str, size_t arg_length) {
  if (m_charset->mbminlen == 1) {
    set(str, arg_length, m_charset);
    return false;
  }
  uint dummy_errors;
  return copy(str, arg_length, &my_charset_latin1, m_charset, &dummy_errors);
}

/* This is used by mysql.cc */

bool String::fill(size_t max_length, char fill_char) {
  if (m_length > max_length)
    m_ptr[m_length = max_length] = 0;
  else {
    if (mem_realloc(max_length)) return true;
    memset(m_ptr + m_length, fill_char, max_length - m_length);
    m_length = max_length;
  }
  return false;
}

bool String::append(const String &s) {
  if (s.length()) {
    DBUG_ASSERT(!this->uses_buffer_owned_by(&s));
    DBUG_ASSERT(!s.uses_buffer_owned_by(this));

    if (mem_realloc_exp((m_length + s.length()))) return true;
    memcpy(m_ptr + m_length, s.ptr(), s.length());
    m_length += s.length();
  }
  return false;
}

/*
  Append an ASCII string to the a string of the current character set
*/

bool String::append(const char *s, size_t arg_length) {
  if (!arg_length) return false;

  /*
    For an ASCII incompatible string, e.g. UCS-2, we need to convert
  */
  if (m_charset->mbminlen > 1) {
    size_t add_length = arg_length * m_charset->mbmaxlen;
    uint dummy_errors;
    if (mem_realloc(m_length + add_length)) return true;
    m_length += copy_and_convert(m_ptr + m_length, add_length, m_charset, s,
                                 arg_length, &my_charset_latin1, &dummy_errors);
    return false;
  }

  /*
    For an ASCII compatinble string we can just append.
  */
  if (mem_realloc_exp(m_length + arg_length)) return true;
  memcpy(m_ptr + m_length, s, arg_length);
  m_length += arg_length;
  return false;
}

/*
  Append a 0-terminated ASCII string
*/

bool String::append(const char *s) { return append(s, (uint)strlen(s)); }

/**
  Append an unsigned longlong to the string.
*/
bool String::append_ulonglong(ulonglong val) {
  if (mem_realloc_exp(m_length + MAX_BIGINT_WIDTH + 2)) return true;
  char *end = longlong10_to_str(val, m_ptr + m_length, 10);
  m_length = end - m_ptr;
  return false;
}

/**
  Append a signed longlong to the string.
*/
bool String::append_longlong(longlong val) {
  if (mem_realloc_exp(m_length + MAX_BIGINT_WIDTH + 2))
    return true; /* purecov: inspected */
  char *end = longlong10_to_str(val, m_ptr + m_length, -10);
  m_length = end - m_ptr;
  return false;
}

/*
  Append a string in the given charset to the string
  with character set recoding
*/

bool String::append(const char *s, size_t arg_length, const CHARSET_INFO *cs) {
  size_t offset;

  if (needs_conversion(arg_length, cs, m_charset, &offset)) {
    size_t add_length;
    if ((cs == &my_charset_bin) && offset) {
      DBUG_ASSERT(m_charset->mbminlen > offset);
      offset = m_charset->mbminlen - offset;  // How many characters to pad
      add_length = arg_length + offset;
      if (mem_realloc_exp(m_length + add_length)) return true;
      memset(m_ptr + m_length, 0, offset);
      memcpy(m_ptr + m_length + offset, s, arg_length);
      m_length += add_length;
      return false;
    }

    add_length = arg_length / cs->mbminlen * m_charset->mbmaxlen;
    uint dummy_errors;
    if (mem_realloc_exp(m_length + add_length)) return true;
    m_length += copy_and_convert(m_ptr + m_length, add_length, m_charset, s,
                                 arg_length, cs, &dummy_errors);
  } else {
    if (mem_realloc_exp(m_length + arg_length)) return true;
    memcpy(m_ptr + m_length, s, arg_length);
    m_length += arg_length;
  }
  return false;
}

/**
  Append a parenthesized number to String.
  Used in various pieces of SHOW related code.

  @param nr     Number
*/
bool String::append_parenthesized(int64_t nr) {
  return append('(') || append_longlong(nr) || append(')');
}

bool String::append_with_prefill(const char *s, size_t arg_length,
                                 size_t full_length, char fill_char) {
  size_t t_length = arg_length > full_length ? arg_length : full_length;

  if (mem_realloc(m_length + t_length)) return true;
  if (full_length > arg_length) {
    t_length = full_length - arg_length;
    memset(m_ptr + m_length, fill_char, t_length);
    m_length = m_length + t_length;
  }
  append(s, arg_length);
  return false;
}

size_t String::numchars() const {
  return m_charset->cset->numchars(m_charset, m_ptr, m_ptr + m_length);
}

size_t String::charpos(size_t i, size_t offset) const {
  if (i <= 0) return i;
  return m_charset->cset->charpos(m_charset, m_ptr + offset, m_ptr + m_length,
                                  i);
}

int String::strstr(const String &s, size_t offset) const {
  if (s.length() + offset <= m_length) {
    if (!s.length()) return ((int)offset);  // Empty string is always found

    const char *str = m_ptr + offset;
    const char *search = s.ptr();
    const char *end = m_ptr + m_length - s.length() + 1;
    const char *search_end = s.ptr() + s.length();
  skip:
    while (str != end) {
      if (*str++ == *search) {
        const char *i = str;
        const char *j = search + 1;
        while (j != search_end)
          if (*i++ != *j++) goto skip;
        return (int)(str - m_ptr) - 1;
      }
    }
  }
  return -1;
}

/*
** Search string from end. Offset is offset to the end of string
*/

int String::strrstr(const String &s, size_t offset) const {
  if (s.length() <= offset && offset <= m_length) {
    if (!s.length())
      return static_cast<int>(offset);  // Empty string is always found
    const char *str = m_ptr + offset - 1;
    const char *search = s.ptr() + s.length() - 1;

    const char *end = m_ptr + s.length() - 2;
    const char *search_end = s.ptr() - 1;
  skip:
    while (str != end) {
      if (*str-- == *search) {
        const char *i = str;
        const char *j = search - 1;
        while (j != search_end)
          if (*i-- != *j--) goto skip;
        return (int)(i - m_ptr) + 1;
      }
    }
  }
  return -1;
}

String String::substr(int offset, int count) const {
  int original_count = this->numchars();
  if (offset > original_count) {
    offset = original_count;
  }
  if (offset + count > original_count) {
    count = original_count - offset;
  }
  size_t bytes_offset = this->charpos(offset);

  return String(this->m_ptr + bytes_offset,
                this->charpos(offset + count) - bytes_offset, this->m_charset);
}

/*
  Replace substring with string
  If wrong parameter or not enough memory, do nothing
*/

bool String::replace(size_t offset, size_t arg_length, const String &to) {
  return replace(offset, arg_length, to.ptr(), to.length());
}

bool String::replace(size_t offset, size_t arg_length, const char *to,
                     size_t to_length) {
  long diff = static_cast<long>(to_length) - static_cast<long>(arg_length);
  if (offset + arg_length <= m_length) {
    if (diff < 0) {
      if (to_length) memcpy(m_ptr + offset, to, to_length);
      memmove(m_ptr + offset + to_length, m_ptr + offset + arg_length,
              m_length - offset - arg_length);
    } else {
      if (diff) {
        if (mem_realloc_exp(m_length + diff)) return true;
        memmove(m_ptr + offset + to_length, m_ptr + offset + arg_length,
                m_length - offset - arg_length);
      }
      if (to_length) memcpy(m_ptr + offset, to, to_length);
    }
    m_length += diff;
  }
  return false;
}

// added by Holyfoot for "geometry" needs
bool String::reserve(size_t space_needed, size_t grow_by) {
  if (m_alloced_length < m_length + space_needed) {
    return mem_realloc(m_alloced_length + max(space_needed, grow_by) - 1);
  }
  return false;
}

void qs_append(const char *str_in, size_t len, String *str) {
  memcpy(&((*str)[str->length()]), str_in, len + 1);
  str->length(str->length() + len);
}

void qs_append(double d, size_t len, String *str) {
  char *buff = &((*str)[str->length()]);
  int written = my_gcvt(d, MY_GCVT_ARG_DOUBLE, len, buff, nullptr);
  str->length(str->length() + written);
}

void qs_append(int i, String *str) {
  char *end = longlong10_to_str(i, str->ptr() + str->length(), -10);
  str->length(end - str->ptr());
}

void qs_append(uint i, String *str) {
  char *end = longlong10_to_str(i, str->ptr() + str->length(), 10);
  str->length(end - str->ptr());
}

/*
  Compare strings according to collation, without end space.

  SYNOPSIS
    sortcmp()
    s		First string
    t		Second string
    cs		Collation

  NOTE:
    Normally this is case sensitive comparison

  RETURN
  < 0	s < t
  0	s == t
  > 0	s > t
*/

int sortcmp(const String *s, const String *t, const CHARSET_INFO *cs) {
  return cs->coll->strnncollsp(
      cs, pointer_cast<const uchar *>(s->ptr()), s->length(),
      pointer_cast<const uchar *>(t->ptr()), t->length());
}

/*
  Compare strings byte by byte. End spaces are also compared.

  SYNOPSIS
    stringcmp()
    s		First string
    t		Second string

  NOTE:
    Strings are compared as a stream of uchars

  RETURN
  < 0	s < t
  0	s == t
  > 0	s > t
*/

int stringcmp(const String *s, const String *t) {
  size_t s_len = s->length();
  size_t t_len = t->length();
  size_t len = min(s_len, t_len);
  int cmp = (len == 0) ? 0 : memcmp(s->ptr(), t->ptr(), len);
  return (cmp) ? cmp : static_cast<int>(s_len) - static_cast<int>(t_len);
}

/**
  Makes a copy of a String's buffer unless it's already heap-allocated.

  If the buffer ('str') of 'from' is on the heap, this function returns
  'from', possibly re-allocated to be at least from_length bytes long.
  It is also the case if from==to or to==NULL.
  Otherwise, this function makes and returns a copy of "from" into "to"; the
  buffer of "to" is heap-allocated; a pre-condition is that \c from->str and
  \c to->str must point to non-overlapping buffers.
  The logic behind this complex design, is that a caller, typically a
  val_str() function, sometimes has an input String ('from') which buffer it
  wants to modify; but this String's buffer may or not be heap-allocated; if
  it's not heap-allocated it is possibly in static storage or belongs to an
  outer context, and thus should not be modified; in that case the caller
  wants a heap-allocated copy which it can freely modify.

  @param  to    destination string
  @param  from  source string
  @param  from_length  destination string will hold at least from_length bytes.
 */

String *copy_if_not_alloced(String *to, String *from, size_t from_length) {
  if (from->m_is_alloced && from->m_alloced_length >= from_length) return from;
  if ((from->m_is_alloced && (from->m_alloced_length != 0)) || !to ||
      from == to) {
    (void)from->mem_realloc(from_length, true /* force heap allocation */);
    return from;
  }
  if (to->mem_realloc(from_length, true)) return from;  // Actually an error

  // from and to should not be overlapping
  DBUG_ASSERT(!to->uses_buffer_owned_by(from));
  DBUG_ASSERT(!from->uses_buffer_owned_by(to));

  if ((to->m_length = min(from->m_length, from_length)))
    memcpy(to->m_ptr, from->m_ptr, to->m_length);
  to->m_charset = from->m_charset;
  return to;
}

/****************************************************************************
  Help functions
****************************************************************************/

/*
  copy a string,
  with optional character set conversion,
  with optional left padding (for binary -> UCS2 conversion)

  SYNOPSIS
    well_formed_copy_nchars()
    to			     Store result here
    to_length                Maxinum length of "to" string
    to_cs		     Character set of "to" string
    from		     Copy from here
    from_length		     Length of from string
    from_cs		     From character set
    nchars                   Copy not more that nchars characters
    well_formed_error_pos    Return position when "from" is not well formed
                             or NULL otherwise.
    cannot_convert_error_pos Return position where a not convertable
                             character met, or NULL otherwise.
    from_end_pos             Return position where scanning of "from"
                             string stopped.
  NOTES

  RETURN
    length of bytes copied to 'to'
*/

size_t well_formed_copy_nchars(const CHARSET_INFO *to_cs, char *to,
                               size_t to_length, const CHARSET_INFO *from_cs,
                               const char *from, size_t from_length,
                               size_t nchars,
                               const char **well_formed_error_pos,
                               const char **cannot_convert_error_pos,
                               const char **from_end_pos) {
  size_t res;

  if ((to_cs == &my_charset_bin) || (from_cs == &my_charset_bin) ||
      (to_cs == from_cs) || my_charset_same(from_cs, to_cs)) {
    if (to_length < to_cs->mbminlen || !nchars) {
      *from_end_pos = from;
      *cannot_convert_error_pos = nullptr;
      *well_formed_error_pos = nullptr;
      return 0;
    }

    if (to_cs == &my_charset_bin) {
      res = min(min(nchars, to_length), from_length);
      memmove(to, from, res);
      *from_end_pos = from + res;
      *well_formed_error_pos = nullptr;
      *cannot_convert_error_pos = nullptr;
    } else {
      int well_formed_error;
      uint from_offset;

      if ((from_offset = (from_length % to_cs->mbminlen)) &&
          (from_cs == &my_charset_bin)) {
        /*
          Copying from BINARY to UCS2 needs to prepend zeros sometimes:
          INSERT INTO t1 (ucs2_column) VALUES (0x01);
          0x01 -> 0x0001
        */
        uint pad_length = to_cs->mbminlen - from_offset;
        memset(to, 0, pad_length);
        memmove(to + pad_length, from, from_offset);
        /*
          In some cases left zero-padding can create an incorrect character.
          For example:
            INSERT INTO t1 (utf32_column) VALUES (0x110000);
          We'll pad the value to 0x00110000, which is a wrong UTF32 sequence!
          The valid characters range is limited to 0x00000000..0x0010FFFF.

          Make sure we didn't pad to an incorrect character.
        */
        if (to_cs->cset->well_formed_len(to_cs, to, to + to_cs->mbminlen, 1,
                                         &well_formed_error) !=
            to_cs->mbminlen) {
          *from_end_pos = *well_formed_error_pos = from;
          *cannot_convert_error_pos = nullptr;
          return 0;
        }
        nchars--;
        from += from_offset;
        from_length -= from_offset;
        to += to_cs->mbminlen;
        to_length -= to_cs->mbminlen;
      }

      size_t min_length = min(from_length, to_length);
      /*
        If we operate on a multi-byte fixed-width character set, make
        sure the string wasn't truncated in the middle of a character.
        If so, truncate to a character boundary.
      */
      if (to_cs->mbmaxlen > 1 && to_cs->mbmaxlen == to_cs->mbminlen)
        min_length -= min_length % to_cs->mbmaxlen;

      res = to_cs->cset->well_formed_len(to_cs, from, from + min_length, nchars,
                                         &well_formed_error);
      if (res > 0) memmove(to, from, res);
      *from_end_pos = from + res;

      /*
        If we are operating on a multi-byte variable-width character set and
        "well_formed_error" is set to true, it means the string is not
        well-formed (either partially or completely). But, in case of a
        well-formed string whose string length is too long for the destination
        buffer (i.e to_length), there is a possibility that the string is
        truncated in the middle of a character, which would break its sequence.
        This could lead to mistakenly rejecting the string as malformed.

        To resolve this, we check if the full string contains a valid character
        immediately after the returned end point. If it does, the string was
        just truncated; if it doesn't, it was actually malformed.

        For example: Consider a well-formed string of 300 bytes, which contained
        a given three-byte code point at str[254..256]. Furthermore, suppose the
        destination buffer is 255 bytes long. In this case, well_formed_len()
        would return 254, so we would need to check the bytes str[254..257] to
        see if they contain a well-formed character. This second invocation of
        well_formed_len() would return 2, which takes us across the truncation
        point, confirming that the problem was indeed truncation and not a
        malformed code point.
      */
      if (well_formed_error && to_cs->mbmaxlen > 1 &&
          res > min_length - to_cs->mbmaxlen) {
        const char *from_end = from + min(res + to_cs->mbmaxlen, from_length);

        size_t extra = to_cs->cset->well_formed_len(
            to_cs, *from_end_pos, from_end, 1, &well_formed_error);

        well_formed_error = (res + extra < min_length);
      }

      *well_formed_error_pos = well_formed_error ? *from_end_pos : nullptr;
      *cannot_convert_error_pos = nullptr;
      if (from_offset) res += to_cs->mbminlen;
    }
  } else {
    int cnvres;
    my_wc_t wc;
    my_charset_conv_mb_wc mb_wc = from_cs->cset->mb_wc;
    my_charset_conv_wc_mb wc_mb = to_cs->cset->wc_mb;
    const uchar *from_end = (const uchar *)from + from_length;
    uchar *to_end = (uchar *)to + to_length;
    char *to_start = to;
    *well_formed_error_pos = nullptr;
    *cannot_convert_error_pos = nullptr;

    for (; nchars; nchars--) {
      const char *from_prev = from;
      if ((cnvres = (*mb_wc)(from_cs, &wc, pointer_cast<const uchar *>(from),
                             from_end)) > 0)
        from += cnvres;
      else if (cnvres == MY_CS_ILSEQ) {
        if (!*well_formed_error_pos) *well_formed_error_pos = from;
        from++;
        wc = '?';
      } else if (cnvres > MY_CS_TOOSMALL) {
        /*
          A correct multibyte sequence detected
          But it doesn't have Unicode mapping.
        */
        if (!*cannot_convert_error_pos) *cannot_convert_error_pos = from;
        from += (-cnvres);
        wc = '?';
      } else
        break;  // Not enough characters

    outp:
      if ((cnvres = (*wc_mb)(to_cs, wc, (uchar *)to, to_end)) > 0)
        to += cnvres;
      else if (cnvres == MY_CS_ILUNI && wc != '?') {
        if (!*cannot_convert_error_pos) *cannot_convert_error_pos = from_prev;
        wc = '?';
        goto outp;
      } else {
        from = from_prev;
        break;
      }
    }
    *from_end_pos = from;
    res = to - to_start;
  }
  return res;
}

void String::print(String *str) const {
  char *st = m_ptr;
  char *end = st + m_length;

  if (str->reserve(m_length)) return;

  for (; st < end; st++) {
    uchar c = *st;
    switch (c) {
      case '\\':
        str->append(STRING_WITH_LEN("\\\\"));
        break;
      case '\0':
        str->append(STRING_WITH_LEN("\\0"));
        break;
      case '\'':
        str->append(STRING_WITH_LEN("\\'"));
        break;
      case '\n':
        str->append(STRING_WITH_LEN("\\n"));
        break;
      case '\r':
        str->append(STRING_WITH_LEN("\\r"));
        break;
      case '\032':  // Ctrl-Z
        str->append(STRING_WITH_LEN("\\Z"));
        break;
      default:
        str->append(c);
    }
  }
}

/*
  Exchange state of this object and argument.

  SYNOPSIS
    String::swap()

  RETURN
    Target string will contain state of this object and vice versa.
*/

void String::swap(String &s) noexcept {
  using std::swap;
  swap(m_ptr, s.m_ptr);
  swap(m_length, s.m_length);
  swap(m_alloced_length, s.m_alloced_length);
  swap(m_is_alloced, s.m_is_alloced);
  swap(m_charset, s.m_charset);
}

char *String::dup(MEM_ROOT *root) const {
  return strmake_root(root, m_ptr, m_length);
}

/**
  Convert string to printable ASCII string

  @details This function converts input string "from" replacing non-ASCII bytes
  with hexadecimal sequences ("\xXX") optionally appending "..." to the end of
  the resulting string.
  This function used in the ER_TRUNCATED_WRONG_VALUE_FOR_FIELD error messages,
  e.g. when a string cannot be converted to a result charset.


  @param    to          output buffer
  @param    to_len      size of the output buffer (8 bytes or greater)
  @param    from        input string
  @param    from_len    size of the input string
  @param    from_cs     input charset
  @param    nbytes      maximal number of bytes to convert (from_len if 0)

  @return   number of bytes in the output string
*/

size_t convert_to_printable(char *to, size_t to_len, const char *from,
                            size_t from_len, const CHARSET_INFO *from_cs,
                            size_t nbytes /*= 0*/) {
  /* needs at least 8 bytes for '\xXX...' and zero byte */
  DBUG_ASSERT(to_len >= 8);

  char *t = to;
  char *t_end = to + to_len - 1;  // '- 1' is for the '\0' at the end
  const char *f = from;
  const char *f_end = from + (nbytes ? min(from_len, nbytes) : from_len);
  char *dots = to;  // last safe place to append '...'

  if (!f || t == t_end) return 0;

  for (; t < t_end && f < f_end; f++) {
    /*
      If the source string is ASCII compatible (mbminlen==1)
      and the source character is in ASCII printable range (0x20..0x7F),
      then display the character as is.

      Otherwise, if the source string is not ASCII compatible (e.g. UCS2),
      or the source character is not in the printable range,
      then print the character using HEX notation.
    */
    if (((unsigned char)*f) >= 0x20 && ((unsigned char)*f) <= 0x7F &&
        from_cs->mbminlen == 1) {
      *t++ = *f;
    } else {
      if (t_end - t < 4)  // \xXX
        break;
      *t++ = '\\';
      *t++ = 'x';
      *t++ = _dig_vec_upper[((unsigned char)*f) >> 4];
      *t++ = _dig_vec_upper[((unsigned char)*f) & 0x0F];
    }
    if (t_end - t >= 3)  // '...'
      dots = t;
  }
  if (f < from + from_len)
    memcpy(dots, STRING_WITH_LEN("...\0"));
  else
    *t = '\0';
  return t - to;
}

/**
  Convert a buffer to printable HEX encoded string
  For eg: ABCDEF1234


  @param    to          output buffer
  @param    to_len      size of the output buffer (from_len*2 + 1 or greater)
  @param    from        input buffer
  @param    from_len    size of the input buffer

  @return   number of bytes in the output string
*/
size_t bin_to_hex_str(char *to, size_t to_len, const char *from,
                      size_t from_len) {
  char *out;
  const char *in;
  size_t i;

  if (to_len < ((from_len * 2) + 1)) return 0;

  out = to;
  in = from;

  for (i = 0; i < from_len; i++, in++) {
    *out++ = _dig_vec_upper[((unsigned char)*in) >> 4];
    *out++ = _dig_vec_upper[((unsigned char)*in) & 0xF];
  }

  *out = '\0';

  return out - to;
}

/**
  Check if an input byte sequence is a valid character string of a given charset

  @param cs                     The input character set.
  @param str                    The input byte sequence to validate.
  @param length                 A byte length of the str.
  @param [out] valid_length     A byte length of a valid prefix of the str.
  @param [out] length_error     True in the case of a character length error:
                                some byte[s] in the input is not a valid
                                prefix for a character, i.e. the byte length
                                of that invalid character is undefined.

  @retval true if the whole input byte sequence is a valid character string.
               The length_error output parameter is undefined.

  @return
    if the whole input byte sequence is a valid character string
    then
        return false
    else
        if the length of some character in the input is undefined (MY_CS_ILSEQ)
           or the last character is truncated (MY_CS_TOOSMALL)
        then
            *length_error= true; // fatal error!
        else
            *length_error= false; // non-fatal error: there is no wide character
                                  // encoding for some input character
        return true
*/
bool validate_string(const CHARSET_INFO *cs, const char *str, size_t length,
                     size_t *valid_length, bool *length_error) {
  if (cs->mbmaxlen > 1) {
    int well_formed_error;
    *valid_length = cs->cset->well_formed_len(cs, str, str + length, length,
                                              &well_formed_error);
    *length_error = well_formed_error;
    return well_formed_error;
  }

  /*
    well_formed_len() is not functional on single-byte character sets,
    so use mb_wc() instead:
  */
  *length_error = false;

  const uchar *from = reinterpret_cast<const uchar *>(str);
  const uchar *from_end = from + length;
  my_charset_conv_mb_wc mb_wc = cs->cset->mb_wc;

  while (from < from_end) {
    my_wc_t wc;
    int cnvres = (*mb_wc)(cs, &wc, pointer_cast<const uchar *>(from), from_end);
    if (cnvres <= 0) {
      *valid_length = from - reinterpret_cast<const uchar *>(str);
      return true;
    }
    from += cnvres;
  }
  *valid_length = length;
  return false;
}

/**
  Appends from_str to to_str, escaping certain characters.

  @param [in,out] to_str   The destination string.
  @param [in]     from_str The source string.

  @return false on success, true on error.
*/

bool append_escaped(String *to_str, const String *from_str) {
  if (to_str->mem_realloc(to_str->length() + from_str->length()))
    return true;  // OOM

  const char *from = from_str->ptr();
  const char *end = from + from_str->length();
  for (; from < end; from++) {
    char c = *from;
    switch (c) {
      case '\0':
        c = '0';
        break;
      case '\032':
        c = 'Z';
        break;
      case '\\':
      case '\'':
        break;
      default:
        goto normal_character;
    }
    if (to_str->append('\\')) return true;  // OOM

  normal_character:
    if (to_str->append(c)) return true;  // OOM
  }
  return false;
}
