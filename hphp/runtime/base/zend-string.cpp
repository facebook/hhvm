/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-math.h"

#include "hphp/util/lock.h"
#include "hphp/util/overflow.h"
#include <math.h>
#include <monetary.h>

#include "hphp/util/bstring.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/builtin-functions.h"

#ifdef __APPLE__
#ifndef isnan
#define isnan(x)  \
  ( sizeof (x) == sizeof(float )  ? __inline_isnanf((float)(x)) \
  : sizeof (x) == sizeof(double)  ? __inline_isnand((double)(x))  \
  : __inline_isnan ((long double)(x)))
#endif

#ifndef isinf
#define isinf(x)  \
  ( sizeof (x) == sizeof(float )  ? __inline_isinff((float)(x)) \
  : sizeof (x) == sizeof(double)  ? __inline_isinfd((double)(x))  \
  : __inline_isinf ((long double)(x)))
#endif
#endif


#define PHP_QPRINT_MAXL 75

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

bool string_substr_check(int len, int &f, int &l) {
  if (l < 0 && -l > len) {
    return false;
  } else if (l > len) {
    l = len;
  }

  if (f > len) {
    return false;
  } else if (f < 0 && -f > len) {
    f = 0;
  }

  if (l < 0 && (l + len - f) < 0) {
    return false;
  }

  // if "from" position is negative, count start position from the end
  if (f < 0) {
    f += len;
    if (f < 0) {
      f = 0;
    }
  }
  if (f >= len) {
    return false;
  }

  // if "length" position is negative, set it to the length
  // needed to stop that many chars from the end of the string
  if (l < 0) {
    l += len - f;
    if (l < 0) {
      l = 0;
    }
  }
  if ((unsigned int)f + (unsigned int)l > (unsigned int)len) {
    l = len - f;
  }
  return true;
}

void string_charmask(const char *sinput, int len, char *mask) {
  const unsigned char *input = (unsigned char *)sinput;
  const unsigned char *end;
  unsigned char c;

  memset(mask, 0, 256);
  for (end = input+len; input < end; input++) {
    c=*input;
    if ((input+3 < end) && input[1] == '.' && input[2] == '.'
        && input[3] >= c) {
      memset(mask+c, 1, input[3] - c + 1);
      input+=3;
    } else if ((input+1 < end) && input[0] == '.' && input[1] == '.') {
      /* Error, try to be as helpful as possible:
         (a range ending/starting with '.' won't be captured here) */
      if (end-len >= input) { /* there was no 'left' char */
        throw_invalid_argument
          ("charlist: Invalid '..'-range, missing left of '..'");
        continue;
      }
      if (input+2 >= end) { /* there is no 'right' char */
        throw_invalid_argument
          ("charlist: Invalid '..'-range, missing right of '..'");
        continue;
      }
      if (input[-1] > input[2]) { /* wrong order */
        throw_invalid_argument
          ("charlist: '..'-range needs to be incrementing");
        continue;
      }
      /* FIXME: better error (a..b..c is the only left possibility?) */
      throw_invalid_argument("charlist: Invalid '..'-range");
      continue;
    } else {
      mask[c]=1;
    }
  }
}

int string_copy(char *dst, const char *src, int siz) {
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0)
        break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0';    /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return(s - src - 1);  /* count does not include NUL */
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

int string_ncmp(const char *s1, const char *s2, int len) {
  for (int i = 0; i < len; i++) {
    char c1 = s1[i];
    char c2 = s2[i];
    if (c1 > c2) return 1;
    if (c1 < c2) return -1;
  }
  return 0;
}

static int compare_right(char const **a, char const *aend,
                         char const **b, char const *bend) {
  int bias = 0;

  /* The longest run of digits wins.  That aside, the greatest
     value wins, but we can't know that it will until we've scanned
     both numbers to know that they have the same magnitude, so we
     remember it in BIAS. */
  for(;; (*a)++, (*b)++) {
    if ((*a == aend || !isdigit((int)(unsigned char)**a)) &&
        (*b == bend || !isdigit((int)(unsigned char)**b)))
      return bias;
    else if (*a == aend || !isdigit((int)(unsigned char)**a))
      return -1;
    else if (*b == bend || !isdigit((int)(unsigned char)**b))
      return +1;
    else if (**a < **b) {
      if (!bias)
        bias = -1;
    } else if (**a > **b) {
      if (!bias)
        bias = +1;
    }
  }

  return 0;
}

static int compare_left(char const **a, char const *aend,
                        char const **b, char const *bend) {
  /* Compare two left-aligned numbers: the first to have a
     different value wins. */
  for(;; (*a)++, (*b)++) {
    if ((*a == aend || !isdigit((int)(unsigned char)**a)) &&
        (*b == bend || !isdigit((int)(unsigned char)**b)))
      return 0;
    else if (*a == aend || !isdigit((int)(unsigned char)**a))
      return -1;
    else if (*b == bend || !isdigit((int)(unsigned char)**b))
      return +1;
    else if (**a < **b)
      return -1;
    else if (**a > **b)
      return +1;
  }

  return 0;
}

int string_natural_cmp(char const *a, size_t a_len,
                       char const *b, size_t b_len, int fold_case) {
  char ca, cb;
  char const *ap, *bp;
  char const *aend = a + a_len, *bend = b + b_len;
  int fractional, result;

  if (a_len == 0 || b_len == 0)
    return a_len - b_len;

  ap = a;
  bp = b;
  while (1) {
    ca = *ap; cb = *bp;

    /* skip over leading spaces or zeros */
    while (isspace((int)(unsigned char)ca))
      ca = *++ap;

    while (isspace((int)(unsigned char)cb))
      cb = *++bp;

    /* process run of digits */
    if (isdigit((int)(unsigned char)ca)  &&  isdigit((int)(unsigned char)cb)) {
      fractional = (ca == '0' || cb == '0');

      if (fractional)
        result = compare_left(&ap, aend, &bp, bend);
      else
        result = compare_right(&ap, aend, &bp, bend);

      if (result != 0)
        return result;
      else if (ap == aend && bp == bend)
        /* End of the strings. Let caller sort them out. */
        return 0;
      else {
        /* Keep on comparing from the current point. */
        ca = *ap; cb = *bp;
      }
    }

    if (fold_case) {
      ca = toupper((int)(unsigned char)ca);
      cb = toupper((int)(unsigned char)cb);
    }

    if (ca < cb)
      return -1;
    else if (ca > cb)
      return +1;

    ++ap; ++bp;
    if (ap >= aend && bp >= bend)
      /* The strings compare the same.  Perhaps the caller
         will want to call strcmp to break the tie. */
      return 0;
    else if (ap >= aend)
      return -1;
    else if (bp >= bend)
      return 1;
  }
}

///////////////////////////////////////////////////////////////////////////////

void string_to_case(String& s, int (*tocase)(int)) {
  assert(!s.isNull());
  assert(tocase);
  auto data = s.mutableData();
  auto len = s.size();
  for (int i = 0; i < len; i++) {
    data[i] = tocase(data[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////

#define STR_PAD_LEFT            0
#define STR_PAD_RIGHT           1
#define STR_PAD_BOTH            2

String string_pad(const char *input, int len, int pad_length,
                  const char *pad_string, int pad_str_len,
                  int pad_type) {
  assert(input);
  int num_pad_chars = pad_length - len;

  /* If resulting string turns out to be shorter than input string,
     we simply copy the input and return. */
  if (pad_length < 0 || num_pad_chars < 0) {
    return String(input, len, CopyString);
  }

  /* Setup the padding string values if specified. */
  if (pad_str_len == 0) {
    throw_invalid_argument("pad_string: (empty)");
    return String();
  }

  String ret(pad_length, ReserveString);
  char *result = ret.mutableData();

  /* We need to figure out the left/right padding lengths. */
  int left_pad, right_pad;
  switch (pad_type) {
  case STR_PAD_RIGHT:
    left_pad = 0;
    right_pad = num_pad_chars;
    break;
  case STR_PAD_LEFT:
    left_pad = num_pad_chars;
    right_pad = 0;
    break;
  case STR_PAD_BOTH:
    left_pad = num_pad_chars / 2;
    right_pad = num_pad_chars - left_pad;
    break;
  default:
    throw_invalid_argument("pad_type: %d", pad_type);
    return String();
  }

  /* First we pad on the left. */
  int result_len = 0;
  for (int i = 0; i < left_pad; i++) {
    result[result_len++] = pad_string[i % pad_str_len];
  }

  /* Then we copy the input string. */
  memcpy(result + result_len, input, len);
  result_len += len;

  /* Finally, we pad on the right. */
  for (int i = 0; i < right_pad; i++) {
    result[result_len++] = pad_string[i % pad_str_len];
  }
  ret.setSize(result_len);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int string_find(const char *input, int len, char ch, int pos,
                bool case_sensitive) {
  assert(input);
  if (pos < 0 || pos > len) {
    return -1;
  }
  const void *ptr;
  if (case_sensitive) {
    ptr = memchr(input + pos, ch, len - pos);
  } else {
    ptr = bstrcasechr(input + pos, ch, len - pos);
  }
  if (ptr != nullptr) {
    return (int)((const char *)ptr - input);
  }
  return -1;
}

int string_rfind(const char *input, int len, char ch, int pos,
                 bool case_sensitive) {
  assert(input);
  if (pos < -len || pos > len) {
    return -1;
  }
  const void *ptr;
  if (case_sensitive) {
    if (pos >= 0) {
      ptr = memrchr(input + pos, ch, len - pos);
    } else {
      ptr = memrchr(input, ch, len + pos + 1);
    }
  } else {
    if (pos >= 0) {
      ptr = bstrrcasechr(input + pos, ch, len - pos);
    } else {
      ptr = bstrrcasechr(input, ch, len + pos + 1);
    }
  }
  if (ptr != nullptr) {
    return (int)((const char *)ptr - input);
  }
  return -1;
}

int string_find(const char *input, int len, const char *s, int s_len,
                int pos, bool case_sensitive) {
  assert(input);
  assert(s);
  if (!s_len || pos < 0 || pos > len) {
    return -1;
  }
  void *ptr;
  if (case_sensitive) {
    ptr = (void*)string_memnstr(input + pos, s, s_len, input + len);
  } else {
    ptr = bstrcasestr(input + pos, len - pos, s, s_len);
  }
  if (ptr != nullptr) {
    return (int)((const char *)ptr - input);
  }
  return -1;
}

int string_rfind(const char *input, int len, const char *s, int s_len,
                 int pos, bool case_sensitive) {
  assert(input);
  assert(s);
  if (!s_len || pos < -len || pos > len) {
    return -1;
  }
  void *ptr;
  if (case_sensitive) {
    if (pos >= 0) {
      ptr = bstrrstr(input + pos, len - pos, s, s_len);
    } else {
      ptr = bstrrstr(input, len + pos + s_len, s, s_len);
    }
  } else {
    if (pos >= 0) {
      ptr = bstrrcasestr(input + pos, len - pos, s, s_len);
    } else {
      ptr = bstrrcasestr(input, len + pos + s_len, s, s_len);
    }
  }
  if (ptr != nullptr) {
    return (int)((const char *)ptr - input);
  }
  return -1;
}

const char *string_memnstr(const char *haystack, const char *needle,
                           int needle_len, const char *end) {
  const char *p = haystack;
  char ne = needle[needle_len-1];

  end -= needle_len;
  while (p <= end) {
    if ((p = (char *)memchr(p, *needle, (end-p+1))) && ne == p[needle_len-1]) {
      if (!memcmp(needle, p, needle_len-1)) {
        return p;
      }
    }
    if (p == nullptr) {
      return nullptr;
    }
    p++;
  }
  return nullptr;
}

String string_replace(const char *s, int len, int start, int length,
                      const char *replacement, int len_repl) {
  assert(s);
  assert(replacement);
  assert(len >= 0);

  // if "start" position is negative, count start position from the end
  // of the string
  if (start < 0) {
    start = len + start;
    if (start < 0) {
      start = 0;
    }
  }
  if (start > len) {
    start = len;
  }
  // if "length" position is negative, set it to the length
  // needed to stop that many chars from the end of the string
  if (length < 0) {
    length = (len - start) + length;
    if (length < 0) {
      length = 0;
    }
  }
  // check if length is too large
  if (length > len) {
    length = len;
  }
  // check if the length is too large adjusting for non-zero start
  // Write this way instead of start + length > len to avoid overflow
  if (length > len - start) {
    length = len - start;
  }

  String retString(len + len_repl - length, ReserveString);
  char *ret = retString.mutableData();

  int ret_len = 0;
  if (start) {
    memcpy(ret, s, start);
    ret_len += start;
  }
  if (len_repl) {
    memcpy(ret + ret_len, replacement, len_repl);
    ret_len += len_repl;
  }
  len -= (start + length);
  if (len) {
    memcpy(ret + ret_len, s + start + length, len);
    ret_len += len;
  }
  retString.setSize(ret_len);
  return retString;
}

String string_replace(const char *input, int len,
                      const char *search, int len_search,
                      const char *replacement, int len_replace,
                      int &count, bool case_sensitive) {
  assert(input);
  assert(search && len_search);
  assert(len >= 0);
  assert(len_search >= 0);
  assert(len_replace >= 0);

  if (len == 0) {
    return String();
  }

  req::vector<int> founds;
  founds.reserve(16);
  if (len_search == 1) {
    for (int pos = string_find(input, len, *search, 0, case_sensitive);
         pos >= 0;
         pos = string_find(input, len, *search, pos + len_search,
                           case_sensitive)) {
      founds.push_back(pos);
    }
  } else {
    for (int pos = string_find(input, len, search, len_search, 0,
                               case_sensitive);
         pos >= 0;
         pos = string_find(input, len, search, len_search,
                           pos + len_search, case_sensitive)) {
      founds.push_back(pos);
    }
  }

  count = founds.size();
  if (count == 0) {
    return String(); // not found
  }

  int reserve;

  // Make sure the new size of the string wouldn't overflow int32_t. Don't
  // bother if the replacement wouldn't make the string longer.
  if (len_replace > len_search) {
    auto raise = [&] { raise_error("String too large"); };
    if (mul_overflow(len_replace - len_search, count)) {
      raise();
    }
    int diff = (len_replace - len_search) * count;
    if (add_overflow(len, diff)) {
      raise();
    }
    reserve = len + diff;
  } else {
    reserve = len + (len_replace - len_search) * count;
  }

  String retString(reserve, ReserveString);
  char *ret = retString.mutableData();
  char *p = ret;
  int pos = 0; // last position in input that hasn't been copied over yet
  int n;
  for (unsigned int i = 0; i < founds.size(); i++) {
    n = founds[i];
    if (n > pos) {
      n -= pos;
      memcpy(p, input, n);
      p += n;
      input += n;
      pos += n;
    }
    if (len_replace) {
      memcpy(p, replacement, len_replace);
      p += len_replace;
    }
    input += len_search;
    pos += len_search;
  }
  n = len;
  if (n > pos) {
    n -= pos;
    memcpy(p, input, n);
    p += n;
  }
  retString.setSize(p - ret);
  return retString;
}

///////////////////////////////////////////////////////////////////////////////

String string_chunk_split(const char *src, int srclen, const char *end,
                          int endlen, int chunklen) {
  int chunks = srclen / chunklen; // complete chunks!
  int restlen = srclen - chunks * chunklen; /* srclen % chunklen */

  String ret(
    safe_address(
      chunks + 1,
      endlen,
      srclen
    ),
    ReserveString
  );
  char *dest = ret.mutableData();

  const char *p; char *q;
  const char *pMax = src + srclen - chunklen + 1;
  for (p = src, q = dest; p < pMax; ) {
    memcpy(q, p, chunklen);
    q += chunklen;
    memcpy(q, end, endlen);
    q += endlen;
    p += chunklen;
  }

  if (restlen) {
    memcpy(q, p, restlen);
    q += restlen;
    memcpy(q, end, endlen);
    q += endlen;
  }

  ret.setSize(q - dest);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define PHP_TAG_BUF_SIZE 1023

/**
 * Check if tag is in a set of tags
 *
 * states:
 *
 * 0 start tag
 * 1 first non-whitespace char seen
 */
static int string_tag_find(const char *tag, int len, const char *set) {
  char c, *n;
  const char *t;
  int state=0, done=0;
  char *norm;

  if (len <= 0) {
    return 0;
  }

  norm = (char *)req::malloc(len+1);

  n = norm;
  t = tag;
  c = tolower(*t);
  /*
    normalize the tag removing leading and trailing whitespace
    and turn any <a whatever...> into just <a> and any </tag>
    into <tag>
  */
  while (!done) {
    switch (c) {
    case '<':
      *(n++) = c;
      break;
    case '>':
      done =1;
      break;
    default:
      if (!isspace((int)c)) {
        if (state == 0) {
          state=1;
        }
        if (c != '/') {
          *(n++) = c;
        }
      } else {
        if (state == 1)
          done=1;
      }
      break;
    }
    c = tolower(*(++t));
  }
  *(n++) = '>';
  *n = '\0';
  if (strstr(set, norm)) {
    done=1;
  } else {
    done=0;
  }
  req::free(norm);
  return done;
}

/**
 * A simple little state-machine to strip out html and php tags
 *
 * State 0 is the output state, State 1 means we are inside a
 * normal html tag and state 2 means we are inside a php tag.
 *
 * The state variable is passed in to allow a function like fgetss
 * to maintain state across calls to the function.
 *
 * lc holds the last significant character read and br is a bracket
 * counter.
 *
 * When an allow string is passed in we keep track of the string
 * in state 1 and when the tag is closed check it against the
 * allow string to see if we should allow it.

 * swm: Added ability to strip <?xml tags without assuming it PHP
 * code.
 */
String string_strip_tags(const char *s, const int len,
                         const char *allow, const int allow_len,
                         bool allow_tag_spaces) {
  const char *abuf, *p;
  char *rbuf, *tbuf, *tp, *rp, c, lc;

  int br, i=0, depth=0, in_q = 0;
  int state = 0, pos;

  assert(s);
  assert(allow);

  String retString(s, len, CopyString);
  rbuf = retString.mutableData();
  String allowString;

  c = *s;
  lc = '\0';
  p = s;
  rp = rbuf;
  br = 0;
  if (allow_len) {
    assert(allow);

    allowString = String(allow_len, ReserveString);
    char *atmp = allowString.mutableData();
    for (const char *tmp = allow; *tmp; tmp++, atmp++) {
      *atmp = tolower((int)*(const unsigned char *)tmp);
    }
    allowString.setSize(allow_len);
    abuf = allowString.data();

    tbuf = (char *)req::malloc(PHP_TAG_BUF_SIZE+1);
    tp = tbuf;
  } else {
    abuf = nullptr;
    tbuf = tp = nullptr;
  }

  auto move = [&pos, &tbuf, &tp]() {
    if (tp - tbuf >= PHP_TAG_BUF_SIZE) {
      pos = tp - tbuf;
      tbuf = (char*)req::realloc(tbuf, (tp - tbuf) + PHP_TAG_BUF_SIZE + 1);
      tp = tbuf + pos;
    }
  };

  while (i < len) {
    switch (c) {
    case '\0':
      break;
    case '<':
      if (isspace(*(p + 1)) && !allow_tag_spaces) {
        goto reg_char;
      }
      if (state == 0) {
        lc = '<';
        state = 1;
        if (allow_len) {
          move();
          *(tp++) = '<';
        }
      } else if (state == 1) {
        depth++;
      }
      break;

    case '(':
      if (state == 2) {
        if (lc != '"' && lc != '\'') {
          lc = '(';
          br++;
        }
      } else if (allow_len && state == 1) {
        move();
        *(tp++) = c;
      } else if (state == 0) {
        *(rp++) = c;
      }
      break;

    case ')':
      if (state == 2) {
        if (lc != '"' && lc != '\'') {
          lc = ')';
          br--;
        }
      } else if (allow_len && state == 1) {
        move();
        *(tp++) = c;
      } else if (state == 0) {
        *(rp++) = c;
      }
      break;

    case '>':
      if (depth) {
        depth--;
        break;
      }

      if (in_q) {
        break;
      }

      switch (state) {
      case 1: /* HTML/XML */
        lc = '>';
        in_q = state = 0;
        if (allow_len) {
          move();
          *(tp++) = '>';
          *tp='\0';
          if (string_tag_find(tbuf, tp-tbuf, abuf)) {
            memcpy(rp, tbuf, tp-tbuf);
            rp += tp-tbuf;
          }
          tp = tbuf;
        }
        break;

      case 2: /* PHP */
        if (!br && lc != '\"' && *(p-1) == '?') {
          in_q = state = 0;
          tp = tbuf;
        }
        break;

      case 3:
        in_q = state = 0;
        tp = tbuf;
        break;

      case 4: /* JavaScript/CSS/etc... */
        if (p >= s + 2 && *(p-1) == '-' && *(p-2) == '-') {
          in_q = state = 0;
          tp = tbuf;
        }
        break;

      default:
        *(rp++) = c;
        break;
      }
      break;

    case '"':
    case '\'':
      if (state == 4) {
        /* Inside <!-- comment --> */
        break;
      } else if (state == 2 && *(p-1) != '\\') {
        if (lc == c) {
          lc = '\0';
        } else if (lc != '\\') {
          lc = c;
        }
      } else if (state == 0) {
        *(rp++) = c;
      } else if (allow_len && state == 1) {
        move();
        *(tp++) = c;
      }
      if (state && p != s && *(p-1) != '\\' && (!in_q || *p == in_q)) {
        if (in_q) {
          in_q = 0;
        } else {
          in_q = *p;
        }
      }
      break;

    case '!':
      /* JavaScript & Other HTML scripting languages */
      if (state == 1 && *(p-1) == '<') {
        state = 3;
        lc = c;
      } else {
        if (state == 0) {
          *(rp++) = c;
        } else if (allow_len && state == 1) {
          move();
          *(tp++) = c;
        }
      }
      break;

    case '-':
      if (state == 3 && p >= s + 2 && *(p-1) == '-' && *(p-2) == '!') {
        state = 4;
      } else {
        goto reg_char;
      }
      break;

    case '?':

      if (state == 1 && *(p-1) == '<') {
        br=0;
        state=2;
        break;
      }

    case 'E':
    case 'e':
      /* !DOCTYPE exception */
      if (state==3 && p > s+6
          && tolower(*(p-1)) == 'p'
          && tolower(*(p-2)) == 'y'
          && tolower(*(p-3)) == 't'
          && tolower(*(p-4)) == 'c'
          && tolower(*(p-5)) == 'o'
          && tolower(*(p-6)) == 'd') {
        state = 1;
        break;
      }
      /* fall-through */

    case 'l':

      /* swm: If we encounter '<?xml' then we shouldn't be in
       * state == 2 (PHP). Switch back to HTML.
       */

      if (state == 2 && p > s+2 && *(p-1) == 'm' && *(p-2) == 'x') {
        state = 1;
        break;
      }

      /* fall-through */
    default:
    reg_char:
      if (state == 0) {
        *(rp++) = c;
      } else if (allow_len && state == 1) {
        move();
        *(tp++) = c;
      }
      break;
    }
    c = *(++p);
    i++;
  }
  if (rp < rbuf + len) {
    *rp = '\0';
  }
  if (allow_len) {
    req::free(tbuf);
  }

  retString.setSize(rp - rbuf);
  return retString;
}

///////////////////////////////////////////////////////////////////////////////

String string_addslashes(const char *str, int length) {
  assert(str);
  if (length == 0) {
    return String();
  }

  String retString((length << 1) + 1, ReserveString);
  char *new_str = retString.mutableData();
  const char *source = str;
  const char *end = source + length;
  char *target = new_str;

  while (source < end) {
    switch (*source) {
    case '\0':
      *target++ = '\\';
      *target++ = '0';
      break;
    case '\'':
    case '\"':
    case '\\':
      *target++ = '\\';
      /* break is missing *intentionally* */
    default:
      *target++ = *source;
      break;
    }

    source++;
  }

  retString.setSize(target - new_str);
  return retString;
}

///////////////////////////////////////////////////////////////////////////////

static char string_hex2int(int c) {
  if (isdigit(c)) {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return -1;
}

String string_quoted_printable_encode(const char *input, int len) {
  size_t length = len;
  const unsigned char *str = (unsigned char*)input;

  unsigned long lp = 0;
  unsigned char c;
  char *d, *buffer;
  char *hex = "0123456789ABCDEF";

  String ret(
    safe_address(
      3,
      length + ((safe_address(3, length, 0)/(PHP_QPRINT_MAXL-9)) + 1),
      1),
    ReserveString
  );
  d = buffer = ret.mutableData();

  while (length--) {
    if (((c = *str++) == '\015') && (*str == '\012') && length > 0) {
      *d++ = '\015';
      *d++ = *str++;
      length--;
      lp = 0;
    } else {
      if (iscntrl (c) || (c == 0x7f) || (c & 0x80) ||
          (c == '=') || ((c == ' ') && (*str == '\015'))) {
        if ((((lp+= 3) > PHP_QPRINT_MAXL) && (c <= 0x7f))
            || ((c > 0x7f) && (c <= 0xdf) && ((lp + 3) > PHP_QPRINT_MAXL))
            || ((c > 0xdf) && (c <= 0xef) && ((lp + 6) > PHP_QPRINT_MAXL))
            || ((c > 0xef) && (c <= 0xf4) && ((lp + 9) > PHP_QPRINT_MAXL))) {
          *d++ = '=';
          *d++ = '\015';
          *d++ = '\012';
          lp = 3;
        }
        *d++ = '=';
        *d++ = hex[c >> 4];
        *d++ = hex[c & 0xf];
      } else {
        if ((++lp) > PHP_QPRINT_MAXL) {
          *d++ = '=';
          *d++ = '\015';
          *d++ = '\012';
          lp = 1;
        }
        *d++ = c;
      }
    }
  }
  len = d - buffer;

  ret.setSize(len);
  return ret;
}

String string_quoted_printable_decode(const char *input, int len, bool is_q) {
  assert(input);
  if (len == 0) {
    return String();
  }

  int i = 0, j = 0, k;
  const char *str_in = input;
  String ret(len, ReserveString);
  char *str_out = ret.mutableData();
  while (i < len && str_in[i]) {
    switch (str_in[i]) {
    case '=':
      if (i + 2 < len && str_in[i + 1] && str_in[i + 2] &&
          isxdigit((int) str_in[i + 1]) && isxdigit((int) str_in[i + 2]))
        {
          str_out[j++] = (string_hex2int((int) str_in[i + 1]) << 4)
            + string_hex2int((int) str_in[i + 2]);
          i += 3;
        } else  /* check for soft line break according to RFC 2045*/ {
        k = 1;
        while (str_in[i + k] &&
               ((str_in[i + k] == 32) || (str_in[i + k] == 9))) {
          /* Possibly, skip spaces/tabs at the end of line */
          k++;
        }
        if (!str_in[i + k]) {
          /* End of line reached */
          i += k;
        }
        else if ((str_in[i + k] == 13) && (str_in[i + k + 1] == 10)) {
          /* CRLF */
          i += k + 2;
        }
        else if ((str_in[i + k] == 13) || (str_in[i + k] == 10)) {
          /* CR or LF */
          i += k + 1;
        }
        else {
          str_out[j++] = str_in[i++];
        }
      }
      break;
    case '_':
      if (is_q) {
        str_out[j++] = ' ';
        i++;
      } else {
        str_out[j++] = str_in[i++];
      }
      break;
    default:
      str_out[j++] = str_in[i++];
    }
  }
  ret.setSize(j);
  return ret;
}

Variant string_base_to_numeric(const char *s, int len, int base) {
  int64_t num = 0;
  double fnum = 0;
  int mode = 0;
  int64_t cutoff;
  int cutlim;

  assert(string_validate_base(base));

  cutoff = LONG_MAX / base;
  cutlim = LONG_MAX % base;

  for (int i = len; i > 0; i--) {
    char c = *s++;

    /* might not work for EBCDIC */
    if (c >= '0' && c <= '9')
      c -= '0';
    else if (c >= 'A' && c <= 'Z')
      c -= 'A' - 10;
    else if (c >= 'a' && c <= 'z')
      c -= 'a' - 10;
    else
      continue;

    if (c >= base)
      continue;

    switch (mode) {
    case 0: /* Integer */
      if (num < cutoff || (num == cutoff && c <= cutlim)) {
        num = num * base + c;
        break;
      } else {
        fnum = num;
        mode = 1;
      }
      /* fall-through */
    case 1: /* Float */
      fnum = fnum * base + c;
    }
  }

  if (mode == 1) {
    return fnum;
  }
  return num;
}

String string_long_to_base(unsigned long value, int base) {
  static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char buf[(sizeof(unsigned long) << 3) + 1];
  char *ptr, *end;

  assert(string_validate_base(base));

  end = ptr = buf + sizeof(buf) - 1;

  do {
    *--ptr = digits[value % base];
    value /= base;
  } while (ptr > buf && value);

  return String(ptr, end - ptr, CopyString);
}

String string_numeric_to_base(const Variant& value, int base) {
  static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  assert(string_validate_base(base));
  if ((!value.isInteger() && !value.isDouble())) {
    return empty_string();
  }

  if (value.isDouble()) {
    double fvalue = floor(value.toDouble()); /* floor it just in case */
    char *ptr, *end;
    char buf[(sizeof(double) << 3) + 1];

    /* Don't try to convert +/- infinity */
    if (fvalue == HUGE_VAL || fvalue == -HUGE_VAL) {
      raise_warning("Number too large");
      return empty_string();
    }

    end = ptr = buf + sizeof(buf) - 1;

    do {
      *--ptr = digits[(int) fmod(fvalue, base)];
      fvalue /= base;
    } while (ptr > buf && fabs(fvalue) >= 1);

    return String(ptr, end - ptr, CopyString);
  }

  return string_long_to_base(value.toInt64(), base);
}

///////////////////////////////////////////////////////////////////////////////
// uuencode

#define PHP_UU_ENC(c) \
  ((c) ? ((c) & 077) + ' ' : '`')
#define PHP_UU_ENC_C2(c) \
  PHP_UU_ENC(((*(c) << 4) & 060) | ((*((c) + 1) >> 4) & 017))
#define PHP_UU_ENC_C3(c) \
  PHP_UU_ENC(((*(c + 1) << 2) & 074) | ((*((c) + 2) >> 6) & 03))
#define PHP_UU_DEC(c) \
  (((c) - ' ') & 077)

String string_uuencode(const char *src, int src_len) {
  assert(src);
  assert(src_len);

  int len = 45;
  char *p;
  const char *s, *e, *ee;
  char *dest;

  /* encoded length is ~ 38% greater than the original */
  String ret((int)ceil(src_len * 1.38) + 45, ReserveString);
  p = dest = ret.mutableData();
  s = src;
  e = src + src_len;

  while ((s + 3) < e) {
    ee = s + len;
    if (ee > e) {
      ee = e;
      len = ee - s;
      if (len % 3) {
        ee = s + (int) (floor(len / 3) * 3);
      }
    }
    *p++ = PHP_UU_ENC(len);

    while (s < ee) {
      *p++ = PHP_UU_ENC(*s >> 2);
      *p++ = PHP_UU_ENC_C2(s);
      *p++ = PHP_UU_ENC_C3(s);
      *p++ = PHP_UU_ENC(*(s + 2) & 077);

      s += 3;
    }

    if (len == 45) {
      *p++ = '\n';
    }
  }

  if (s < e) {
    if (len == 45) {
      *p++ = PHP_UU_ENC(e - s);
      len = 0;
    }

    *p++ = PHP_UU_ENC(*s >> 2);
    *p++ = PHP_UU_ENC_C2(s);
    *p++ = ((e - s) > 1) ? PHP_UU_ENC_C3(s) : PHP_UU_ENC('\0');
    *p++ = ((e - s) > 2) ? PHP_UU_ENC(*(s + 2) & 077) : PHP_UU_ENC('\0');
  }

  if (len < 45) {
    *p++ = '\n';
  }

  *p++ = PHP_UU_ENC('\0');
  *p++ = '\n';
  *p = '\0';

  ret.setSize(p - dest);
  return ret;
}

String string_uudecode(const char *src, int src_len) {
  int total_len = 0;
  int len;
  const char *s, *e, *ee;
  char *p, *dest;

  String ret(ceil(src_len * 0.75), ReserveString);
  p = dest = ret.mutableData();
  s = src;
  e = src + src_len;

  while (s < e) {
    if ((len = PHP_UU_DEC(*s++)) <= 0) {
      break;
    }
    /* sanity check */
    if (len > src_len) {
      goto err;
    }

    total_len += len;

    ee = s + (len == 45 ? 60 : (int) floor(len * 1.33));
    /* sanity check */
    if (ee > e) {
      goto err;
    }

    while (s < ee) {
      if (s + 4 > e) goto err;

      *p++ = PHP_UU_DEC(*s) << 2 | PHP_UU_DEC(*(s + 1)) >> 4;
      *p++ = PHP_UU_DEC(*(s + 1)) << 4 | PHP_UU_DEC(*(s + 2)) >> 2;
      *p++ = PHP_UU_DEC(*(s + 2)) << 6 | PHP_UU_DEC(*(s + 3));
      s += 4;
    }

    if (len < 45) {
      break;
    }

    /* skip \n */
    s++;
  }

  if ((len = total_len > (p - dest))) {
    *p++ = PHP_UU_DEC(*s) << 2 | PHP_UU_DEC(*(s + 1)) >> 4;
    if (len > 1) {
      *p++ = PHP_UU_DEC(*(s + 1)) << 4 | PHP_UU_DEC(*(s + 2)) >> 2;
      if (len > 2) {
        *p++ = PHP_UU_DEC(*(s + 2)) << 6 | PHP_UU_DEC(*(s + 3));
      }
    }
  }

  ret.setSize(total_len);
  return ret;

 err:
  return String();
}

///////////////////////////////////////////////////////////////////////////////
// base64

static const char base64_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
  -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
  -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};

static String php_base64_encode(const unsigned char *str, int length) {
  const unsigned char *current = str;
  unsigned char *p;
  unsigned char *result;

  if ((length + 2) < 0 || ((length + 2) / 3) >= (1 << (sizeof(int) * 8 - 2))) {
    return String();
  }

  String ret(((length + 2) / 3) * 4, ReserveString);
  p = result = (unsigned char *)ret.mutableData();

  while (length > 2) { /* keep going until we have less than 24 bits */
    *p++ = base64_table[current[0] >> 2];
    *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
    *p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
    *p++ = base64_table[current[2] & 0x3f];

    current += 3;
    length -= 3; /* we just handle 3 octets of data */
  }

  /* now deal with the tail end of things */
  if (length != 0) {
    *p++ = base64_table[current[0] >> 2];
    if (length > 1) {
      *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
      *p++ = base64_table[(current[1] & 0x0f) << 2];
      *p++ = base64_pad;
    } else {
      *p++ = base64_table[(current[0] & 0x03) << 4];
      *p++ = base64_pad;
      *p++ = base64_pad;
    }
  }
  ret.setSize(p - result);
  return ret;
}

static String php_base64_decode(const char *str, int length, bool strict) {
  const unsigned char *current = (unsigned char*)str;
  int ch, i = 0, j = 0, k;
  /* this sucks for threaded environments */

  String retString(length, ReserveString);
  unsigned char* result = (unsigned char*)retString.mutableData();

  /* run through the whole string, converting as we go */
  while ((ch = *current++) != '\0' && length-- > 0) {
    if (ch == base64_pad) {
      if (*current != '=' && ((i % 4) == 1 || (strict && length > 0))) {
        if ((i % 4) != 1) {
          while (isspace(*(++current))) {
            continue;
          }
          if (*current == '\0') {
            continue;
          }
        }
        return String();
      }
      continue;
    }

    ch = base64_reverse_table[ch];
    if ((!strict && ch < 0) || ch == -1) {
      /* a space or some other separator character, we simply skip over */
      continue;
    } else if (ch == -2) {
      return String();
    }

    switch(i % 4) {
    case 0:
      result[j] = ch << 2;
      break;
    case 1:
      result[j++] |= ch >> 4;
      result[j] = (ch & 0x0f) << 4;
      break;
    case 2:
      result[j++] |= ch >>2;
      result[j] = (ch & 0x03) << 6;
      break;
    case 3:
      result[j++] |= ch;
      break;
    }
    i++;
  }

  k = j;
  /* mop things up if we ended on a boundary */
  if (ch == base64_pad) {
    switch(i % 4) {
    case 1:
      return String();
    case 2:
      k++;
    case 3:
      result[k] = 0;
    }
  }
  retString.setSize(j);
  return retString;
}

String string_base64_encode(const char *input, int len) {
  return php_base64_encode((unsigned char *)input, len);
}

String string_base64_decode(const char *input, int len, bool strict) {
  return php_base64_decode(input, len, strict);
}

///////////////////////////////////////////////////////////////////////////////

String string_escape_shell_arg(const char *str) {
  int x, y, l;
  char *cmd;

  y = 0;
  l = strlen(str);

  String ret(safe_address(l, 4, 3), ReserveString); /* worst case */
  cmd = ret.mutableData();

  cmd[y++] = '\'';

  for (x = 0; x < l; x++) {
    switch (str[x]) {
    case '\'':
      cmd[y++] = '\'';
      cmd[y++] = '\\';
      cmd[y++] = '\'';
      /* fall-through */
    default:
      cmd[y++] = str[x];
    }
  }
  cmd[y++] = '\'';
  ret.setSize(y);
  return ret;
}

String string_escape_shell_cmd(const char *str) {
  register int x, y, l;
  char *cmd;
  char *p = nullptr;

  l = strlen(str);
  String ret(safe_address(l, 2, 1), ReserveString);
  cmd = ret.mutableData();

  for (x = 0, y = 0; x < l; x++) {
    switch (str[x]) {
    case '"':
    case '\'':
      if (!p && (p = (char *)memchr(str + x + 1, str[x], l - x - 1))) {
        /* noop */
      } else if (p && *p == str[x]) {
        p = nullptr;
      } else {
        cmd[y++] = '\\';
      }
      cmd[y++] = str[x];
      break;
    case '#': /* This is character-set independent */
    case '&':
    case ';':
    case '`':
    case '|':
    case '*':
    case '?':
    case '~':
    case '<':
    case '>':
    case '^':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '$':
    case '\\':
    case '\x0A': /* excluding these two */
    case '\xFF':
      cmd[y++] = '\\';
      /* fall-through */
    default:
      cmd[y++] = str[x];
    }
  }
  ret.setSize(y);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static void string_similar_str(const char *txt1, int len1,
                               const char *txt2, int len2,
                               int *pos1, int *pos2, int *max) {
  const char *p, *q;
  const char *end1 = txt1 + len1;
  const char *end2 = txt2 + len2;
  int l;

  *max = 0;
  for (p = txt1; p < end1; p++) {
    for (q = txt2; q < end2; q++) {
      for (l = 0; (p + l < end1) && (q + l < end2) && (p[l] == q[l]); l++);
      if (l > *max) {
        *max = l;
        *pos1 = p - txt1;
        *pos2 = q - txt2;
      }
    }
  }
}

static int string_similar_char(const char *txt1, int len1,
                               const char *txt2, int len2) {
  int sum;
  int pos1 = 0, pos2 = 0, max;

  string_similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max);
  if ((sum = max)) {
    if (pos1 && pos2) {
      sum += string_similar_char(txt1, pos1, txt2, pos2);
    }
    if ((pos1 + max < len1) && (pos2 + max < len2)) {
      sum += string_similar_char(txt1 + pos1 + max, len1 - pos1 - max,
                                 txt2 + pos2 + max, len2 - pos2 - max);
    }
  }

  return sum;
}

int string_similar_text(const char *t1, int len1,
                        const char *t2, int len2, float *percent) {
  if (len1 == 0 && len2 == 0) {
    if (percent) *percent = 0.0;
    return 0;
  }

  int sim = string_similar_char(t1, len1, t2, len2);
  if (percent) *percent = sim * 200.0 / (len1 + len2);
  return sim;
}

///////////////////////////////////////////////////////////////////////////////

#define LEVENSHTEIN_MAX_LENTH 255

// reference implementation, only optimized for memory usage, not speed
int string_levenshtein(const char *s1, int l1, const char *s2, int l2,
                       int cost_ins, int cost_rep, int cost_del ) {
  int *p1, *p2, *tmp;
  int i1, i2, c0, c1, c2;

  if (l1==0) return l2*cost_ins;
  if (l2==0) return l1*cost_del;

  if ((l1>LEVENSHTEIN_MAX_LENTH)||(l2>LEVENSHTEIN_MAX_LENTH)) {
    raise_warning("levenshtein(): Argument string(s) too long");
    return -1;
  }

  p1 = (int*)req::malloc((l2+1) * sizeof(int));
  p2 = (int*)req::malloc((l2+1) * sizeof(int));

  for(i2=0;i2<=l2;i2++) {
    p1[i2] = i2*cost_ins;
  }

  for(i1=0;i1<l1;i1++) {
    p2[0]=p1[0]+cost_del;
    for(i2=0;i2<l2;i2++) {
      c0=p1[i2]+((s1[i1]==s2[i2])?0:cost_rep);
      c1=p1[i2+1]+cost_del; if (c1<c0) c0=c1;
      c2=p2[i2]+cost_ins; if (c2<c0) c0=c2;
      p2[i2+1]=c0;
    }
    tmp=p1; p1=p2; p2=tmp;
  }

  c0=p1[l2];
  req::free(p1);
  req::free(p2);
  return c0;
}

///////////////////////////////////////////////////////////////////////////////

String string_money_format(const char *format, double value) {
  bool check = false;
  const char *p = format;
  while ((p = strchr(p, '%'))) {
    if (*(p + 1) == '%') {
      p += 2;
    } else if (!check) {
      check = true;
      p++;
    } else {
      throw_invalid_argument
        ("format: Only a single %%i or %%n token can be used");
      return String();
    }
  }

  int format_len = strlen(format);
  int str_len = safe_address(format_len, 1, 1024);
  String ret(str_len, ReserveString);
  char *str = ret.mutableData();
  if ((str_len = strfmon(str, str_len, format, value)) < 0) {
    return String();
  }
  ret.setSize(str_len);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

String string_number_format(double d, int dec,
                            const String& dec_point,
                            const String& thousand_sep) {
  char *tmpbuf = nullptr, *resbuf;
  char *s, *t;  /* source, target */
  char *dp;
  int integral;
  int tmplen, reslen=0;
  int count=0;
  int is_negative=0;

  if (d < 0) {
    is_negative = 1;
    d = -d;
  }

  if (dec < 0) dec = 0;
  d = php_math_round(d, dec);

  // departure from PHP: we got rid of dependencies on spprintf() here.
  String tmpstr(63, ReserveString);
  tmpbuf = tmpstr.mutableData();
  tmplen = snprintf(tmpbuf, 64, "%.*F", dec, d);
  if (tmpbuf == nullptr || !isdigit((int)tmpbuf[0])) {
    tmpstr.setSize(tmplen);
    return tmpstr;
  }
  if (tmplen >= 64) {
    // Uncommon, asked for more than 64 chars worth of precision
    tmpstr = String(tmplen, ReserveString);
    tmpbuf = tmpstr.mutableData();
    tmplen = snprintf(tmpbuf, tmplen + 1, "%.*F", dec, d);
    if (tmpbuf == nullptr || !isdigit((int)tmpbuf[0])) {
      tmpstr.setSize(tmplen);
      return tmpstr;
    }
  }

  /* find decimal point, if expected */
  if (dec) {
    dp = strpbrk(tmpbuf, ".,");
  } else {
    dp = nullptr;
  }

  /* calculate the length of the return buffer */
  if (dp) {
    integral = dp - tmpbuf;
  } else {
    /* no decimal point was found */
    integral = tmplen;
  }

  /* allow for thousand separators */
  if (!thousand_sep.empty()) {
    integral += ((integral-1) / 3) * thousand_sep.size();
  }

  reslen = integral;

  if (dec) {
    reslen += dec;

    if (!dec_point.empty()) {
      reslen += dec_point.size();
    }
  }

  /* add a byte for minus sign */
  if (is_negative) {
    reslen++;
  }
  String resstr(reslen, ReserveString);
  resbuf = resstr.mutableData();

  s = tmpbuf+tmplen-1;
  t = resbuf+reslen-1;

  /* copy the decimal places.
   * Take care, as the sprintf implementation may return less places than
   * we requested due to internal buffer limitations */
  if (dec) {
    int declen = dp ? s - dp : 0;
    int topad = dec > declen ? dec - declen : 0;

    /* pad with '0's */
    while (topad--) {
      *t-- = '0';
    }

    if (dp) {
      s -= declen + 1; /* +1 to skip the point */
      t -= declen;

      /* now copy the chars after the point */
      memcpy(t + 1, dp + 1, declen);
    }

    /* add decimal point */
    if (!dec_point.empty()) {
      memcpy(t + (1 - dec_point.size()), dec_point.data(), dec_point.size());
      t -= dec_point.size();
    }
  }

  /* copy the numbers before the decimal point, adding thousand
   * separator every three digits */
  while(s >= tmpbuf) {
    *t-- = *s--;
    if (thousand_sep && (++count%3)==0 && s>=tmpbuf) {
      memcpy(t + (1 - thousand_sep.size()),
             thousand_sep.data(),
             thousand_sep.size());
      t -= thousand_sep.size();
    }
  }

  /* and a minus sign, if needed */
  if (is_negative) {
    *t-- = '-';
  }

  resstr.setSize(reslen);
  return resstr;
}

///////////////////////////////////////////////////////////////////////////////
// soundex

/* Simple soundex algorithm as described by Knuth in TAOCP, vol 3 */
String string_soundex(const String& str) {
  assert(!str.empty());
  int _small, code, last;
  String retString(4, ReserveString);
  char* soundex = retString.mutableData();

  static char soundex_table[26] = {
    0,              /* A */
    '1',            /* B */
    '2',            /* C */
    '3',            /* D */
    0,              /* E */
    '1',            /* F */
    '2',            /* G */
    0,              /* H */
    0,              /* I */
    '2',            /* J */
    '2',            /* K */
    '4',            /* L */
    '5',            /* M */
    '5',            /* N */
    0,              /* O */
    '1',            /* P */
    '2',            /* Q */
    '6',            /* R */
    '2',            /* S */
    '3',            /* T */
    0,              /* U */
    '1',            /* V */
    0,              /* W */
    '2',            /* X */
    0,              /* Y */
    '2'             /* Z */
  };

  /* build soundex string */
  last = -1;
  const char *p = str.slice().ptr;
  for (_small = 0; *p && _small < 4; p++) {
    /* convert chars to upper case and strip non-letter chars */
    /* BUG: should also map here accented letters used in non */
    /* English words or names (also found in English text!): */
    /* esstsett, thorn, n-tilde, c-cedilla, s-caron, ... */
    code = toupper((int)(unsigned char)(*p));
    if (code >= 'A' && code <= 'Z') {
      if (_small == 0) {
        /* remember first valid char */
        soundex[_small++] = code;
        last = soundex_table[code - 'A'];
      } else {
        /* ignore sequences of consonants with same soundex */
        /* code in trail, and vowels unless they separate */
        /* consonant letters */
        code = soundex_table[code - 'A'];
        if (code != last) {
          if (code != 0) {
            soundex[_small++] = code;
          }
          last = code;
        }
      }
    }
  }
  /* pad with '0' and terminate with 0 ;-) */
  while (_small < 4) {
    soundex[_small++] = '0';
  }
  retString.setSize(4);
  return retString;
}

///////////////////////////////////////////////////////////////////////////////
// metaphone

/**
 * this is now the original code by Michael G Schwern:
 * i've changed it just a slightly bit (use emalloc,
 * get rid of includes etc)
 * - thies - 13.09.1999
 */

/*-----------------------------  */
/* this used to be "metaphone.h" */
/*-----------------------------  */

/* Special encodings */
#define  SH   'X'
#define  TH   '0'

/*-----------------------------  */
/* end of "metaphone.h"          */
/*-----------------------------  */

/*----------------------------- */
/* this used to be "metachar.h" */
/*----------------------------- */

/* Metachar.h ... little bits about characters for metaphone */
/*-- Character encoding array & accessing macros --*/
/* Stolen directly out of the book... */
char _codes[26] = { 1,16,4,16,9,2,4,16,9,2,0,2,2,2,1,4,0,2,4,4,1,0,0,0,8,0};

#define ENCODE(c) (isalpha(c) ? _codes[((toupper(c)) - 'A')] : 0)

#define isvowel(c)  (ENCODE(c) & 1)    /* AEIOU */

/* These letters are passed through unchanged */
#define NOCHANGE(c) (ENCODE(c) & 2)    /* FJMNR */

/* These form dipthongs when preceding H */
#define AFFECTH(c)  (ENCODE(c) & 4)    /* CGPST */

/* These make C and G soft */
#define MAKESOFT(c) (ENCODE(c) & 8)    /* EIY */

/* These prevent GH from becoming F */
#define NOGHTOF(c)  (ENCODE(c) & 16)  /* BDH */

/*----------------------------- */
/* end of "metachar.h"          */
/*----------------------------- */

/* I suppose I could have been using a character pointer instead of
 * accesssing the array directly... */

/* Look at the next letter in the word */
#define Next_Letter ((char)toupper(word[w_idx+1]))
/* Look at the current letter in the word */
#define Curr_Letter ((char)toupper(word[w_idx]))
/* Go N letters back. */
#define Look_Back_Letter(n)  (w_idx >= n ? (char)toupper(word[w_idx-n]) : '\0')
/* Previous letter.  I dunno, should this return null on failure? */
#define Prev_Letter (Look_Back_Letter(1))
/* Look two letters down.  It makes sure you don't walk off the string. */
#define After_Next_Letter  (Next_Letter != '\0' ? (char)toupper(word[w_idx+2]) \
                           : '\0')
#define Look_Ahead_Letter(n) ((char)toupper(Lookahead(word+w_idx, n)))

/* Allows us to safely look ahead an arbitrary # of letters */
/* I probably could have just used strlen... */
static char Lookahead(unsigned char *word, int how_far) {
  char letter_ahead = '\0';  /* null by default */
  int idx;
  for (idx = 0; word[idx] != '\0' && idx < how_far; idx++);
  /* Edge forward in the string... */

  letter_ahead = (char)word[idx];  /* idx will be either == to how_far or
                                    * at the end of the string
                                    */
  return letter_ahead;
}

/* phonize one letter
 * We don't know the buffers size in advance. On way to solve this is to just
 * re-allocate the buffer size. We're using an extra of 2 characters (this
 * could be one though; or more too). */
#define Phonize(c)  { buffer.append(c); }
/* How long is the phoned word? */
#define Phone_Len  (buffer.size())

/* Note is a letter is a 'break' in the word */
#define Isbreak(c)  (!isalpha(c))

String string_metaphone(const char *input, int word_len, long max_phonemes,
                        int traditional) {
  unsigned char *word = (unsigned char *)input;

  int w_idx = 0;        /* point in the phonization we're at. */
  int max_buffer_len = 0;    /* maximum length of the destination buffer */

  /*-- Parameter checks --*/
  /* Negative phoneme length is meaningless */

  if (max_phonemes < 0)
    return String();

  /* Empty/null string is meaningless */
  /* Overly paranoid */
  /* always_assert(word != NULL && word[0] != '\0'); */

  if (word == nullptr)
    return String();

  /*-- Allocate memory for our phoned_phrase --*/
  if (max_phonemes == 0) {  /* Assume largest possible */
    max_buffer_len = word_len;
  } else {
    max_buffer_len = max_phonemes;
  }
  StringBuffer buffer(max_buffer_len);

  /*-- The first phoneme has to be processed specially. --*/
  /* Find our first letter */
  for (; !isalpha(Curr_Letter); w_idx++) {
    /* On the off chance we were given nothing but crap... */
    if (Curr_Letter == '\0') {
      return buffer.detach();  /* For testing */
    }
  }

  switch (Curr_Letter) {
    /* AE becomes E */
  case 'A':
    if (Next_Letter == 'E') {
      Phonize('E');
      w_idx += 2;
    }
    /* Remember, preserve vowels at the beginning */
    else {
      Phonize('A');
      w_idx++;
    }
    break;
    /* [GKP]N becomes N */
  case 'G':
  case 'K':
  case 'P':
    if (Next_Letter == 'N') {
      Phonize('N');
      w_idx += 2;
    }
    break;
    /* WH becomes H,
       WR becomes R
       W if followed by a vowel */
  case 'W':
    if (Next_Letter == 'H' ||
      Next_Letter == 'R') {
      Phonize(Next_Letter);
      w_idx += 2;
    } else if (isvowel(Next_Letter)) {
      Phonize('W');
      w_idx += 2;
    }
    /* else ignore */
    break;
    /* X becomes S */
  case 'X':
    Phonize('S');
    w_idx++;
    break;
    /* Vowels are kept */
    /* We did A already
       case 'A':
       case 'a':
     */
  case 'E':
  case 'I':
  case 'O':
  case 'U':
    Phonize(Curr_Letter);
    w_idx++;
    break;
  default:
    /* do nothing */
    break;
  }

  /* On to the metaphoning */
  for (; Curr_Letter != '\0' &&
         (max_phonemes == 0 || Phone_Len < max_phonemes);
       w_idx++) {
    /* How many letters to skip because an eariler encoding handled
     * multiple letters */
    unsigned short int skip_letter = 0;


    /* THOUGHT:  It would be nice if, rather than having things like...
     * well, SCI.  For SCI you encode the S, then have to remember
     * to skip the C.  So the phonome SCI invades both S and C.  It would
     * be better, IMHO, to skip the C from the S part of the encoding.
     * Hell, I'm trying it.
     */

    /* Ignore non-alphas */
    if (!isalpha(Curr_Letter))
      continue;

    /* Drop duplicates, except CC */
    if (Curr_Letter == Prev_Letter &&
      Curr_Letter != 'C')
      continue;

    switch (Curr_Letter) {
      /* B -> B unless in MB */
    case 'B':
      if (Prev_Letter != 'M')
        Phonize('B');
      break;
      /* 'sh' if -CIA- or -CH, but not SCH, except SCHW.
       * (SCHW is handled in S)
       *  S if -CI-, -CE- or -CY-
       *  dropped if -SCI-, SCE-, -SCY- (handed in S)
       *  else K
       */
    case 'C':
      if (MAKESOFT(Next_Letter)) {  /* C[IEY] */
        if (After_Next_Letter == 'A' &&
          Next_Letter == 'I') {  /* CIA */
          Phonize(SH);
        }
        /* SC[IEY] */
        else if (Prev_Letter == 'S') {
          /* Dropped */
        } else {
          Phonize('S');
        }
      } else if (Next_Letter == 'H') {
        if ((!traditional) && (After_Next_Letter == 'R' ||
                               Prev_Letter == 'S')) {  /* Christ, School */
          Phonize('K');
        } else {
          Phonize(SH);
        }
        skip_letter++;
      } else {
        Phonize('K');
      }
      break;
      /* J if in -DGE-, -DGI- or -DGY-
       * else T
       */
    case 'D':
      if (Next_Letter == 'G' && MAKESOFT(After_Next_Letter)) {
        Phonize('J');
        skip_letter++;
      } else
        Phonize('T');
      break;
      /* F if in -GH and not B--GH, D--GH, -H--GH, -H---GH
       * else dropped if -GNED, -GN,
       * else dropped if -DGE-, -DGI- or -DGY- (handled in D)
       * else J if in -GE-, -GI, -GY and not GG
       * else K
       */
    case 'G':
      if (Next_Letter == 'H') {
        if (!(NOGHTOF(Look_Back_Letter(3)) || Look_Back_Letter(4) == 'H')) {
          Phonize('F');
          skip_letter++;
        } else {
          /* silent */
        }
      } else if (Next_Letter == 'N') {
        if (Isbreak(After_Next_Letter) ||
            (After_Next_Letter == 'E' && Look_Ahead_Letter(3) == 'D')) {
          /* dropped */
        } else
          Phonize('K');
      } else if (MAKESOFT(Next_Letter) && Prev_Letter != 'G') {
        Phonize('J');
      } else {
        Phonize('K');
      }
      break;
      /* H if before a vowel and not after C,G,P,S,T */
    case 'H':
      if (isvowel(Next_Letter) && !AFFECTH(Prev_Letter))
        Phonize('H');
      break;
      /* dropped if after C
       * else K
       */
    case 'K':
      if (Prev_Letter != 'C')
        Phonize('K');
      break;
      /* F if before H
       * else P
       */
    case 'P':
      if (Next_Letter == 'H') {
        Phonize('F');
      } else {
        Phonize('P');
      }
      break;
      /* K
       */
    case 'Q':
      Phonize('K');
      break;
      /* 'sh' in -SH-, -SIO- or -SIA- or -SCHW-
       * else S
       */
    case 'S':
      if (Next_Letter == 'I' &&
          (After_Next_Letter == 'O' || After_Next_Letter == 'A')) {
        Phonize(SH);
      } else if (Next_Letter == 'H') {
        Phonize(SH);
        skip_letter++;
      } else if ((!traditional) &&
                 (Next_Letter == 'C' && Look_Ahead_Letter(2) == 'H' &&
                  Look_Ahead_Letter(3) == 'W')) {
        Phonize(SH);
        skip_letter += 2;
      } else {
        Phonize('S');
      }
      break;
      /* 'sh' in -TIA- or -TIO-
       * else 'th' before H
       * else T
       */
    case 'T':
      if (Next_Letter == 'I' &&
        (After_Next_Letter == 'O' || After_Next_Letter == 'A')) {
        Phonize(SH);
      } else if (Next_Letter == 'H') {
        Phonize(TH);
        skip_letter++;
      } else {
        Phonize('T');
      }
      break;
      /* F */
    case 'V':
      Phonize('F');
      break;
      /* W before a vowel, else dropped */
    case 'W':
      if (isvowel(Next_Letter))
        Phonize('W');
      break;
      /* KS */
    case 'X':
      Phonize('K');
      Phonize('S');
      break;
      /* Y if followed by a vowel */
    case 'Y':
      if (isvowel(Next_Letter))
        Phonize('Y');
      break;
      /* S */
    case 'Z':
      Phonize('S');
      break;
      /* No transformation */
    case 'F':
    case 'J':
    case 'L':
    case 'M':
    case 'N':
    case 'R':
      Phonize(Curr_Letter);
      break;
    default:
      /* nothing */
      break;
    } /* END SWITCH */

    w_idx += skip_letter;
  } /* END FOR */

  return buffer.detach();
}

///////////////////////////////////////////////////////////////////////////////
// Cyrillic

/**
 * This is codetables for different Cyrillic charsets (relative to koi8-r).
 * Each table contains data for 128-255 symbols from ASCII table.
 * First 256 symbols are for conversion from koi8-r to corresponding charset,
 * second 256 symbols are for reverse conversion, from charset to koi8-r.
 *
 * Here we have the following tables:
 * _cyr_win1251   - for windows-1251 charset
 * _cyr_iso88595  - for iso8859-5 charset
 * _cyr_cp866     - for x-cp866 charset
 * _cyr_mac       - for x-mac-cyrillic charset
 */
typedef unsigned char _cyr_charset_table[512];

static const _cyr_charset_table _cyr_win1251 = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
  46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,46,
  154,174,190,46,159,189,46,46,179,191,180,157,46,46,156,183,
  46,46,182,166,173,46,46,158,163,152,164,155,46,46,46,167,
  225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
  242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
  193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
  210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,184,186,32,179,191,32,32,32,32,32,180,162,32,
  32,32,32,168,170,32,178,175,32,32,32,32,32,165,161,169,
  254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
  239,255,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
  222,192,193,214,196,197,212,195,213,200,201,202,203,204,205,206,
  207,223,208,209,210,211,198,194,220,219,199,216,221,217,215,218,
};

static const _cyr_charset_table _cyr_cp866 = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
  242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
  193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
  35,35,35,124,124,124,124,43,43,124,124,43,43,43,43,43,
  43,45,45,124,45,43,124,124,43,43,45,45,124,45,43,45,
  45,45,45,43,43,43,43,43,43,43,43,35,35,124,124,35,
  210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
  179,163,180,164,183,167,190,174,32,149,158,32,152,159,148,154,
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  205,186,213,241,243,201,32,245,187,212,211,200,190,32,247,198,
  199,204,181,240,242,185,32,244,203,207,208,202,216,32,246,32,
  238,160,161,230,164,165,228,163,229,168,169,170,171,172,173,174,
  175,239,224,225,226,227,166,162,236,235,167,232,237,233,231,234,
  158,128,129,150,132,133,148,131,149,136,137,138,139,140,141,142,
  143,159,144,145,146,147,134,130,156,155,135,152,157,153,151,154,
};

static const _cyr_charset_table _cyr_iso88595 = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,179,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
  242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
  193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
  210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,
  32,163,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,241,32,32,32,32,32,32,32,32,32,32,32,32,
  32,32,32,161,32,32,32,32,32,32,32,32,32,32,32,32,
  238,208,209,230,212,213,228,211,229,216,217,218,219,220,221,222,
  223,239,224,225,226,227,214,210,236,235,215,232,237,233,231,234,
  206,176,177,198,180,181,196,179,197,184,185,186,187,188,189,190,
  191,207,192,193,194,195,182,178,204,203,183,200,205,201,199,202,
};

static const _cyr_charset_table _cyr_mac = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
  242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,179,163,209,
  193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
  210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,255,
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  160,161,162,222,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,221,180,181,182,183,184,185,186,187,188,189,190,191,
  254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
  239,223,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
  158,128,129,150,132,133,148,131,149,136,137,138,139,140,141,142,
  143,159,144,145,146,147,134,130,156,155,135,152,157,153,151,154,
};

/**
 * This is the function that performs real in-place conversion of the string
 * between charsets.
 * Parameters:
 *    str - string to be converted
 *    from,to - one-symbol label of source and destination charset
 * The following symbols are used as labels:
 *    k - koi8-r
 *    w - windows-1251
 *    i - iso8859-5
 *    a - x-cp866
 *    d - x-cp866
 *    m - x-mac-cyrillic
 */
String string_convert_cyrillic_string(const String& input, char from, char to) {
  const unsigned char *from_table, *to_table;
  unsigned char tmp;
  const unsigned char *uinput = (unsigned char *)input.slice().ptr;
  String retString(input.size(), ReserveString);
  unsigned char *str = (unsigned char *)retString.mutableData();

  from_table = nullptr;
  to_table   = nullptr;

  switch (toupper((int)(unsigned char)from)) {
  case 'W': from_table = _cyr_win1251;  break;
  case 'A':
  case 'D': from_table = _cyr_cp866;    break;
  case 'I': from_table = _cyr_iso88595; break;
  case 'M': from_table = _cyr_mac;      break;
  case 'K':
    break;
  default:
    throw_invalid_argument("Unknown source charset: %c", from);
    break;
  }

  switch (toupper((int)(unsigned char)to)) {
  case 'W': to_table = _cyr_win1251;    break;
  case 'A':
  case 'D': to_table = _cyr_cp866;      break;
  case 'I': to_table = _cyr_iso88595;   break;
  case 'M': to_table = _cyr_mac;        break;
  case 'K':
    break;
  default:
    throw_invalid_argument("Unknown destination charset: %c", to);
    break;
  }

  for (int i = 0; i < input.size(); i++) {
    tmp = from_table == nullptr ? uinput[i] : from_table[uinput[i]];
    str[i] = to_table == nullptr ? tmp : to_table[tmp + 256];
  }
  retString.setSize(input.size());
  return retString;
}

///////////////////////////////////////////////////////////////////////////////
// Hebrew

#define HEB_BLOCK_TYPE_ENG 1
#define HEB_BLOCK_TYPE_HEB 2

#define isheb(c)                                                        \
  (((((unsigned char) c) >= 224) && (((unsigned char) c) <= 250)) ? 1 : 0)
#define _isblank(c)                                                     \
  (((((unsigned char) c) == ' '  || ((unsigned char) c) == '\t')) ? 1 : 0)
#define _isnewline(c)                                                   \
  (((((unsigned char) c) == '\n' || ((unsigned char) c) == '\r')) ? 1 : 0)

/**
 * Converts Logical Hebrew text (Hebrew Windows style) to Visual text
 * Cheers/complaints/flames - Zeev Suraski <zeev@php.net>
 */
String string_convert_hebrew_string(const String& inStr,
                                    int max_chars_per_line,
                                    int convert_newlines) {
  assert(!inStr.empty());
  auto str = inStr.data();
  auto str_len = inStr.size();
  const char *tmp;
  char *heb_str, *broken_str;
  char *target;
  int block_start, block_end, block_type, block_length, i;
  long max_chars=0;
  int begin, end, char_count, orig_begin;

  tmp = str;
  block_start=block_end=0;

  heb_str = (char *) req::malloc(str_len + 1);
  SCOPE_EXIT { req::free(heb_str); };
  target = heb_str+str_len;
  *target = 0;
  target--;

  block_length=0;

  if (isheb(*tmp)) {
    block_type = HEB_BLOCK_TYPE_HEB;
  } else {
    block_type = HEB_BLOCK_TYPE_ENG;
  }

  do {
    if (block_type == HEB_BLOCK_TYPE_HEB) {
      while ((isheb((int)*(tmp+1)) ||
              _isblank((int)*(tmp+1)) ||
              ispunct((int)*(tmp+1)) ||
              (int)*(tmp+1)=='\n' ) && block_end<str_len-1) {
        tmp++;
        block_end++;
        block_length++;
      }
      for (i = block_start; i<= block_end; i++) {
        *target = str[i];
        switch (*target) {
        case '(':  *target = ')';  break;
        case ')':  *target = '(';  break;
        case '[':  *target = ']';  break;
        case ']':  *target = '[';  break;
        case '{':  *target = '}';  break;
        case '}':  *target = '{';  break;
        case '<':  *target = '>';  break;
        case '>':  *target = '<';  break;
        case '\\': *target = '/';  break;
        case '/':  *target = '\\'; break;
        default:
          break;
        }
        target--;
      }
      block_type = HEB_BLOCK_TYPE_ENG;
    } else {
      while (!isheb(*(tmp+1)) &&
             (int)*(tmp+1)!='\n' && block_end < str_len-1) {
        tmp++;
        block_end++;
        block_length++;
      }
      while ((_isblank((int)*tmp) ||
              ispunct((int)*tmp)) && *tmp!='/' &&
             *tmp!='-' && block_end > block_start) {
        tmp--;
        block_end--;
      }
      for (i = block_end; i >= block_start; i--) {
        *target = str[i];
        target--;
      }
      block_type = HEB_BLOCK_TYPE_HEB;
    }
    block_start=block_end+1;
  } while (block_end < str_len-1);

  String brokenStr(str_len, ReserveString);
  broken_str = brokenStr.mutableData();
  begin=end=str_len-1;
  target = broken_str;

  while (1) {
    char_count=0;
    while ((!max_chars || char_count < max_chars) && begin > 0) {
      char_count++;
      begin--;
      if (begin <= 0 || _isnewline(heb_str[begin])) {
        while (begin > 0 && _isnewline(heb_str[begin-1])) {
          begin--;
          char_count++;
        }
        break;
      }
    }
    if (char_count == max_chars) { /* try to avoid breaking words */
      int new_char_count=char_count, new_begin=begin;

      while (new_char_count > 0) {
        if (_isblank(heb_str[new_begin]) || _isnewline(heb_str[new_begin])) {
          break;
        }
        new_begin++;
        new_char_count--;
      }
      if (new_char_count > 0) {
        char_count=new_char_count;
        begin=new_begin;
      }
    }
    orig_begin=begin;

    if (_isblank(heb_str[begin])) {
      heb_str[begin]='\n';
    }
    while (begin <= end && _isnewline(heb_str[begin])) {
      /* skip leading newlines */
      begin++;
    }
    for (i = begin; i <= end; i++) { /* copy content */
      *target = heb_str[i];
      target++;
    }
    for (i = orig_begin; i <= end && _isnewline(heb_str[i]); i++) {
      *target = heb_str[i];
      target++;
    }
    begin=orig_begin;

    if (begin <= 0) {
      *target = 0;
      break;
    }
    begin--;
    end=begin;
  }

  if (convert_newlines) {
    int count;
    auto ret = string_replace(broken_str, str_len, "\n", strlen("\n"),
                              "<br />\n", strlen("<br />\n"), count, true);
    if (!ret.isNull()) {
      return ret;
    }
  }
  brokenStr.setSize(str_len);
  return brokenStr;
}

#if defined(__APPLE__)

  void *memrchr(const void *s, int c, size_t n) {
    for (const char *p = (const char *)s + n - 1; p >= s; p--) {
      if (*p == c) return (void *)p;
    }
    return nullptr;
  }

#endif

///////////////////////////////////////////////////////////////////////////////
}
