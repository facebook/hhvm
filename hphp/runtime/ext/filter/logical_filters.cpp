/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/filter/logical_filters.h"

#include <arpa/inet.h>
#include <pcre.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-php-config.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/ext/filter/ext_filter.h"
#include "hphp/runtime/ext/filter/sanitizing_filters.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/url/ext_url.h"

#define FORMAT_IPV4  4
#define FORMAT_IPV6  6
#define MAX_LENGTH_OF_LONG 20

namespace HPHP {

const StaticString
    s_min_range("min_range"),
    s_max_range("max_range"),
    s_decimal("decimal"),
    s_regexp("regexp"),
    s_separator("separator");

static int php_filter_parse_int(const char *str, unsigned int str_len,
                                long *ret) {
  int sign = 0;
  const char *end = str + str_len;

  switch (*str) {
    case '-':
      sign = 1;
      /* fallthrough */
    case '+':
      str++;
    default:
      break;
  }

  if (*str == '0' && str + 1 == end) {
    /* Special cases: +0 and -0 */
    return 1;
  }

  long ctx_value;
  /* must start with 1..9*/
  if (str < end && *str >= '1' && *str <= '9') {
    ctx_value = ((sign)?-1:1) * ((*(str++)) - '0');
  } else {
    return -1;
  }

  if ((end - str > MAX_LENGTH_OF_LONG - 1) /* number too long */
   || (SIZEOF_LONG == 4 && (end - str == MAX_LENGTH_OF_LONG - 1) &&
       *str > '2')) {
    /* overflow */
    return -1;
  }

  while (str < end) {
    if (*str >= '0' && *str <= '9') {
      int digit = (*(str++) - '0');
      if ( (!sign) && ctx_value <= (LONG_MAX-digit)/10 ) {
        ctx_value = (ctx_value * 10) + digit;
      } else if ( sign && ctx_value >= (LONG_MIN+digit)/10) {
        ctx_value = (ctx_value * 10) - digit;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  *ret = ctx_value;
  return 1;
}

static int php_filter_parse_octal(const char *str, unsigned int str_len,
                                  long *ret) {
  unsigned long ctx_value = 0;
  const char *end = str + str_len;

  while (str < end) {
    if (*str >= '0' && *str <= '7') {
      unsigned long n = ((*(str++)) - '0');

      if ((ctx_value > ((unsigned long)(~(long)0)) / 8) ||
        ((ctx_value = ctx_value * 8) > ((unsigned long)(~(long)0)) - n)) {
        return -1;
      }
      ctx_value += n;
    } else {
      return -1;
    }
  }

  *ret = (long)ctx_value;
  return 1;
}

static int php_filter_parse_hex(const char *str, unsigned int str_len,
                                long *ret) {
  unsigned long ctx_value = 0;
  const char *end = str + str_len;
  unsigned long n;

  while (str < end) {
    if (*str >= '0' && *str <= '9') {
      n = ((*(str++)) - '0');
    } else if (*str >= 'a' && *str <= 'f') {
      n = ((*(str++)) - ('a' - 10));
    } else if (*str >= 'A' && *str <= 'F') {
      n = ((*(str++)) - ('A' - 10));
    } else {
      return -1;
    }
    if ((ctx_value > ((unsigned long)(~(long)0)) / 16) ||
      ((ctx_value = ctx_value * 16) > ((unsigned long)(~(long)0)) - n)) {
      return -1;
    }
    ctx_value += n;
  }

  *ret = (long)ctx_value;
  return 1;
}

Variant php_filter_int(PHP_INPUT_FILTER_PARAM_DECL) {
  /* Parse options */
  long min_range, max_range;
  int min_range_set, max_range_set;
  FETCH_LONG_OPTION(min_range, s_min_range);
  FETCH_LONG_OPTION(max_range, s_max_range);
  long option_flags = flags;

  int len = value.length();

  if (len == 0) {
    RETURN_VALIDATION_FAILED
  }

  bool allow_octal = false, allow_hex = false;
  if (option_flags & k_FILTER_FLAG_ALLOW_OCTAL) {
    allow_octal = true;
  }
  if (option_flags & k_FILTER_FLAG_ALLOW_HEX) {
    allow_hex = true;
  }

  /* Start the validating loop */
  const char *p = value.data();
  long ctx_value = 0;

  PHP_FILTER_TRIM_DEFAULT(p, len);

  int error = 0;
  if (*p == '0') {
    p++; len--;
    if (allow_hex && (*p == 'x' || *p == 'X')) {
      p++; len--;
      if (php_filter_parse_hex(p, len, &ctx_value) < 0) {
        assert(ctx_value == 0);
        error = 1;
      }
    } else if (allow_octal) {
      if (php_filter_parse_octal(p, len, &ctx_value) < 0) {
        assert(ctx_value == 0);
        error = 1;
      }
    } else if (len != 0) {
      error = 1;
    }
  } else {
    if (php_filter_parse_int(p, len, &ctx_value) < 0) {
      assert(ctx_value == 0);
      error = 1;
    }
  }

  if (error > 0 || (min_range_set && (ctx_value < min_range)) ||
      (max_range_set && (ctx_value > max_range))) {
    RETURN_VALIDATION_FAILED
  } else {
    return ctx_value;
  }
}

Variant php_filter_boolean(PHP_INPUT_FILTER_PARAM_DECL) {
  const char *str = value.data();
  int len = value.length();

  PHP_FILTER_TRIM_DEFAULT_EX(str, len, 0);

  /* returns true for "1", "true", "on" and "yes"
   * returns false for "0", "false", "off", "no", and ""
   * null otherwise. */
  int ret;
  switch (len) {
    case 0:
      ret = 0;
      break;
    case 1:
      if (*str == '1') {
        ret = 1;
      } else if (*str == '0') {
        ret = 0;
      } else {
        ret = -1;
      }
      break;
    case 2:
      if (strncasecmp(str, "on", 2) == 0) {
        ret = 1;
      } else if (strncasecmp(str, "no", 2) == 0) {
        ret = 0;
      } else {
        ret = -1;
      }
      break;
    case 3:
      if (strncasecmp(str, "yes", 3) == 0) {
        ret = 1;
      } else if (strncasecmp(str, "off", 3) == 0) {
        ret = 0;
      } else {
        ret = -1;
      }
      break;
    case 4:
      if (strncasecmp(str, "true", 4) == 0) {
        ret = 1;
      } else {
        ret = -1;
      }
      break;
    case 5:
      if (strncasecmp(str, "false", 5) == 0) {
        ret = 0;
      } else {
        ret = -1;
      }
      break;
    default:
      ret = -1;
  }

  if (ret == -1) {
    RETURN_VALIDATION_FAILED
  } else {
    return (bool)ret;
  }
}

Variant php_filter_float(PHP_INPUT_FILTER_PARAM_DECL) {
  char dec_sep = '.';
  char tsd_sep[3] = {'\'', ',', '.'};

  int len = value.length();
  const char *str = value.data();
  PHP_FILTER_TRIM_DEFAULT(str, len);
  const char *end = str + len;

  const char *decimal;
  int decimal_set, decimal_len;
  FETCH_STRING_OPTION(decimal, s_decimal);

  if (decimal_set) {
    if (decimal_len != 1) {
      raise_warning("decimal separator must be one char");
      RETURN_VALIDATION_FAILED
    } else {
      dec_sep = *decimal;
    }
  }

  StringBuffer p(len);
  if (str < end && (*str == '+' || *str == '-')) {
    p.append(*str++);
  }
  int first = 1;
  while (1) {
    int n = 0;
    while (str < end && *str >= '0' && *str <= '9') {
      ++n;
      p.append(*str++);
    }
    if (str == end || *str == dec_sep || *str == 'e' || *str == 'E') {
      if (!first && n != 3) {
        goto error;
      }
      if (*str == dec_sep) {
        p.append('.');
        str++;
        while (str < end && *str >= '0' && *str <= '9') {
          p.append(*str++);
        }
      }
      if (*str == 'e' || *str == 'E') {
        p.append(*str++);
        if (str < end && (*str == '+' || *str == '-')) {
          p.append(*str++);
        }
        while (str < end && *str >= '0' && *str <= '9') {
          p.append(*str++);
        }
      }
      break;
    }
    if ((flags & k_FILTER_FLAG_ALLOW_THOUSAND) &&
        (*str == tsd_sep[0] || *str == tsd_sep[1] || *str == tsd_sep[2])) {
      if (first?(n < 1 || n > 3):(n != 3)) {
        goto error;
      }
      first = 0;
      str++;
    } else {
      goto error;
    }
  }
  if (str != end) {
    goto error;
  }

  int64_t lval;
  double dval;
  DataType dt;

  dt = is_numeric_string(p.data(), p.size(), &lval, &dval, 0);

  if (IS_INT_TYPE(dt)) {
    return (double)lval;
  } else if (IS_DOUBLE_TYPE(dt)) {
    if ((!dval && p.size() > 1 && strpbrk(p.data(), "123456789")) ||
         !zend_finite(dval)) {
      goto error;
    }
    return dval;
  } else {
error:
    RETURN_VALIDATION_FAILED
  }
  return value;
}

Variant php_filter_validate_regexp(PHP_INPUT_FILTER_PARAM_DECL) {
  /* Parse options */
  const char *regexp;
  UNUSED int regexp_len;
  int regexp_set;
  FETCH_STRING_OPTION(regexp, s_regexp);

  if (!regexp_set) {
    raise_warning("'regexp' option missing");
    RETURN_VALIDATION_FAILED
  }

  int matches = preg_match(regexp, value).toInt32();

  if (matches <= 0) {
    RETURN_VALIDATION_FAILED
  }
  return value;
}

const StaticString
  s_http("http"),
  s_https("https"),
  s_mailto("mailto"),
  s_news("news"),
  s_file("file");

Variant php_filter_validate_url(PHP_INPUT_FILTER_PARAM_DECL) {
  int old_len = value.length();

  Variant filter_result = php_filter_url(value, flags, option_array);

  if (!filter_result.isString() ||
      old_len != filter_result.toString().length()) {
    RETURN_VALIDATION_FAILED
  }

  /* Use parse_url - if it returns false, we return NULL */
  Url url;
  if (!url_parse(url, value.data(), value.length())) {
    RETURN_VALIDATION_FAILED
  }

  if (!url.scheme.isNull() && (url.scheme.get()->isame(s_http.get()) ||
                               url.scheme.get()->isame(s_https.get()))) {

    if (url.host.isNull()) {
      goto bad_url;
    }

    const char *s = url.host.data();
    const char *e = s + url.host.size();

    /* First char of hostname must be alphanumeric */
    if (!isalnum((int)*(unsigned char *)s)) {
      goto bad_url;
    }

    while (s < e) {
      if (!isalnum((int)*(unsigned char *)s) && *s != '-' && *s != '.') {
        goto bad_url;
      }
      s++;
    }

    if (*(e - 1) == '.') {
      goto bad_url;
    }
  }

  if (
    url.scheme.isNull() ||
    /* some schemas allow the host to be empty */
    (url.host.isNull() && (!url.scheme.same(s_mailto) &&
                           !url.scheme.same(s_news) &&
                           !url.scheme.same(s_file))) ||
    ((flags & k_FILTER_FLAG_PATH_REQUIRED) && url.path.isNull()) ||
    ((flags & k_FILTER_FLAG_QUERY_REQUIRED) && url.query.isNull())
  ) {
bad_url:
    RETURN_VALIDATION_FAILED
  }
  return value;
}

Variant php_filter_validate_email(PHP_INPUT_FILTER_PARAM_DECL) {
  /*
   * The regex below is based on a regex by Michael Rushton.
   * However, it is not identical.  I changed it to only consider routeable
   * addresses as valid.  Michael's regex considers a@b a valid address
   * which conflicts with section 2.3.5 of RFC 5321 which states that:
   *
   *   Only resolvable, fully-qualified domain names (FQDNs) are permitted
   *   when domain names are used in SMTP.  In other words, names that can
   *   be resolved to MX RRs or address (i.e., A or AAAA) RRs (as discussed
   *   in Section 5) are permitted, as are CNAME RRs whose targets can be
   *   resolved, in turn, to MX or address RRs.  Local nicknames or
   *   unqualified names MUST NOT be used.
   *
   * This regex does not handle comments and folding whitespace.  While
   * this is technically valid in an email address, these parts aren't
   * actually part of the address itself.
   *
   * Michael's regex carries this copyright:
   *
   * Copyright Michael Rushton 2009-10
   * http://squiloople.com/
   * Feel free to use and redistribute this code. But please keep this
   * copyright notice.
   *
   */
  const char regexp[] = "/^(?!(?:(?:\\x22?\\x5C[\\x00-\\x7E]\\x22?)|(?:\\x22?[^\\x5C\\x22]\\x22?)){255,})(?!(?:(?:\\x22?\\x5C[\\x00-\\x7E]\\x22?)|(?:\\x22?[^\\x5C\\x22]\\x22?)){65,}@)(?:(?:[\\x21\\x23-\\x27\\x2A\\x2B\\x2D\\x2F-\\x39\\x3D\\x3F\\x5E-\\x7E]+)|(?:\\x22(?:[\\x01-\\x08\\x0B\\x0C\\x0E-\\x1F\\x21\\x23-\\x5B\\x5D-\\x7F]|(?:\\x5C[\\x00-\\x7F]))*\\x22))(?:\\.(?:(?:[\\x21\\x23-\\x27\\x2A\\x2B\\x2D\\x2F-\\x39\\x3D\\x3F\\x5E-\\x7E]+)|(?:\\x22(?:[\\x01-\\x08\\x0B\\x0C\\x0E-\\x1F\\x21\\x23-\\x5B\\x5D-\\x7F]|(?:\\x5C[\\x00-\\x7F]))*\\x22)))*@(?:(?:(?!.*[^.]{64,})(?:(?:(?:xn--)?[a-z0-9]+(?:-+[a-z0-9]+)*\\.){1,126}){1,}(?:(?:[a-z][a-z0-9]*)|(?:(?:xn--)[a-z0-9]+))(?:-+[a-z0-9]+)*)|(?:\\[(?:(?:IPv6:(?:(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){7})|(?:(?!(?:.*[a-f0-9][:\\]]){7,})(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,5})?::(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,5})?)))|(?:(?:IPv6:(?:(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){5}:)|(?:(?!(?:.*[a-f0-9]:){5,})(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,3})?::(?:[a-f0-9]{1,4}(?::[a-f0-9]{1,4}){0,3}:)?)))?(?:(?:25[0-5])|(?:2[0-4][0-9])|(?:1[0-9]{2})|(?:[1-9]?[0-9]))(?:\\.(?:(?:25[0-5])|(?:2[0-4][0-9])|(?:1[0-9]{2})|(?:[1-9]?[0-9]))){3}))\\]))$/iD";

  /* The maximum length of an e-mail address is 320 octets, per RFC 2821. */
  if (value.length() > 320) {
    RETURN_VALIDATION_FAILED
  }

  int matches = preg_match(regexp, value).toInt32();

  if (matches <= 0) {
    RETURN_VALIDATION_FAILED
  }
  return value;
}

static int _php_filter_validate_ipv4(const char *str, int str_len, int *ip) {
  const char *end = str + str_len;
  int n = 0;

  while (str < end) {
    if (*str < '0' || *str > '9') {
      return 0;
    }
    int leading_zero = (*str == '0');
    int m = 1;
    int num = ((*(str++)) - '0');
    while (str < end && (*str >= '0' && *str <= '9')) {
      num = num * 10 + ((*(str++)) - '0');
      if (num > 255 || ++m > 3) {
        return 0;
      }
    }
    /* don't allow a leading 0; that introduces octal numbers,
     * which we don't support */
    if (leading_zero && (num != 0 || m > 1))
      return 0;
    ip[n++] = num;
    if (n == 4) {
      return str == end;
    } else if (str >= end || *(str++) != '.') {
      return 0;
    }
  }
  return 0;
}

static int _php_filter_validate_ipv6(const char *str, int str_len) {

  if (!memchr(str, ':', str_len)) {
    return 0;
  }

  /* check for bundled IPv4 */
  const char *ipv4 = (const char*) memchr(str, '.', str_len);
  int blocks = 0;
  if (ipv4) {
    while (ipv4 > str && *(ipv4-1) != ':') {
      ipv4--;
    }

    int ip4elm[4];
    if (!_php_filter_validate_ipv4(ipv4, (str_len - (ipv4 - str)), ip4elm)) {
      return 0;
    }

    str_len = ipv4 - str; /* length excluding ipv4 */
    if (str_len < 2) {
      return 0;
    }

    if (ipv4[-2] != ':') {
      /* don't include : before ipv4 unless it's a :: */
      str_len--;
    }

    blocks = 2;
  }

  const char *end = str + str_len;
  const char *s = str;

  int compressed = 0;
  int n;
  while (str < end) {
    if (*str == ':') {
      if (++str >= end) {
        /* cannot end in : without previous : */
        return 0;
      }
      if (*str == ':') {
        if (compressed) {
          return 0;
        }
        blocks++; /* :: means 1 or more 16-bit 0 blocks */
        compressed = 1;

        if (++str == end) {
          return (blocks <= 8);
        }
      } else if ((str - 1) == s) {
        /* dont allow leading : without another : following */
        return 0;
      }
    }
    n = 0;
    while ((str < end) &&
         ((*str >= '0' && *str <= '9') ||
        (*str >= 'a' && *str <= 'f') ||
        (*str >= 'A' && *str <= 'F'))) {
      n++;
      str++;
    }
    if (n < 1 || n > 4) {
      return 0;
    }
    if (++blocks > 8)
      return 0;
  }
  return ((compressed && blocks <= 8) || blocks == 8);
}

Variant php_filter_validate_ip(PHP_INPUT_FILTER_PARAM_DECL) {
  /* validates an ipv4 or ipv6 IP, based on the flag (4, 6, or both) add a
   * flag to throw out reserved ranges; multicast ranges... etc. If both
   * allow_ipv4 and allow_ipv6 flags flag are used, then the first dot or
   * colon determine the format */

  int      mode;

  if (memchr(value.data(), ':', value.length())) {
    mode = FORMAT_IPV6;
  } else if (memchr(value.data(), '.', value.length())) {
    mode = FORMAT_IPV4;
  } else {
    RETURN_VALIDATION_FAILED
  }

  if ((flags & k_FILTER_FLAG_IPV4) && (flags & k_FILTER_FLAG_IPV6)) {
    /* Both formats are cool */
  } else if ((flags & k_FILTER_FLAG_IPV4) && mode == FORMAT_IPV6) {
    RETURN_VALIDATION_FAILED
  } else if ((flags & k_FILTER_FLAG_IPV6) && mode == FORMAT_IPV4) {
    RETURN_VALIDATION_FAILED
  }

  switch (mode) {
    case FORMAT_IPV4:
      int ip[4];
      if (!_php_filter_validate_ipv4(value.data(), value.length(), ip)) {
        RETURN_VALIDATION_FAILED
      }

      /* Check flags */
      if (flags & k_FILTER_FLAG_NO_PRIV_RANGE) {
        if (
          (ip[0] == 10) ||
          (ip[0] == 172 && (ip[1] >= 16 && ip[1] <= 31)) ||
          (ip[0] == 192 && ip[1] == 168)
        ) {
          RETURN_VALIDATION_FAILED
        }
      }

      if (flags & k_FILTER_FLAG_NO_RES_RANGE) {
        if (
          (ip[0] == 0) ||
          (ip[0] == 128 && ip[1] == 0) ||
          (ip[0] == 191 && ip[1] == 255) ||
          (ip[0] == 169 && ip[1] == 254) ||
          (ip[0] == 192 && ip[1] == 0 && ip[2] == 2) ||
          (ip[0] == 127 && ip[1] == 0 && ip[2] == 0 && ip[3] == 1) ||
          (ip[0] >= 224 && ip[0] <= 255)
        ) {
          RETURN_VALIDATION_FAILED
        }
      }
      break;

    case FORMAT_IPV6:
      {
        int res = 0;
        res = _php_filter_validate_ipv6(value.data(), value.length());
        if (res < 1) {
          RETURN_VALIDATION_FAILED
        }
        /* Check flags */
        if (flags & k_FILTER_FLAG_NO_PRIV_RANGE) {
          if (value.length() >=2 &&
              (!strncasecmp("FC", value.data(), 2) ||
               !strncasecmp("FD", value.data(), 2))) {
            RETURN_VALIDATION_FAILED
          }
        }
        if (flags & k_FILTER_FLAG_NO_RES_RANGE) {
          switch (value.length()) {
            case 1: case 0:
              break;
            case 2:
              if (!strcmp("::", value.data())) {
                RETURN_VALIDATION_FAILED
              }
              break;
            case 3:
              if (!strcmp("::1", value.data()) ||
                  !strcmp("5f:", value.data())) {
                RETURN_VALIDATION_FAILED
              }
              break;
            default:
              if (value.length() >= 5) {
                if (
                  !strncasecmp("fe8", value.data(), 3) ||
                  !strncasecmp("fe9", value.data(), 3) ||
                  !strncasecmp("fea", value.data(), 3) ||
                  !strncasecmp("feb", value.data(), 3)
                ) {
                  RETURN_VALIDATION_FAILED
                }
              }
              if (
                (value.length() >= 9 &&
                 !strncasecmp("2001:0db8", value.data(), 9)) ||
                (value.length() >= 2 &&
                 !strncasecmp("5f", value.data(), 2)) ||
                (value.length() >= 4 &&
                 !strncasecmp("3ff3", value.data(), 4)) ||
                (value.length() >= 8 &&
                 !strncasecmp("2001:001", value.data(), 8))
              ) {
                RETURN_VALIDATION_FAILED
              }
          }
        }
      }
      break;
  }
  return value;
}

Variant php_filter_validate_mac(PHP_INPUT_FILTER_PARAM_DECL) {
  const char *input = value.data();
  int input_len = value.length();

  char separator;
  const char *exp_separator;
  int exp_separator_set, exp_separator_len;
  FETCH_STRING_OPTION(exp_separator, s_separator);

  if (exp_separator_set && exp_separator_len != 1) {
    raise_warning("Separator must be exactly one character long");
    RETURN_VALIDATION_FAILED;
  }

  int tokens, length;
  if (14 == input_len) {
    /* EUI-64 format: Four hexadecimal digits separated by dots. Less
     * commonly used but valid nonetheless.
     */
    tokens = 3;
    length = 4;
    separator = '.';
  } else if (17 == input_len && input[2] == '-') {
    /* IEEE 802 format: Six hexadecimal digits separated by hyphens. */
    tokens = 6;
    length = 2;
    separator = '-';
  } else if (17 == input_len && input[2] == ':') {
    /* IEEE 802 format: Six hexadecimal digits separated by colons. */
    tokens = 6;
    length = 2;
    separator = ':';
  } else {
    RETURN_VALIDATION_FAILED;
  }

  if (exp_separator_set && separator != exp_separator[0]) {
    RETURN_VALIDATION_FAILED;
  }

  /* Essentially what we now have is a set of tokens each consisting of
   * a hexadecimal number followed by a separator character. (With the
   * exception of the last token which does not have the separator.)
   */
  for (int i = 0; i < tokens; i++) {
    int offset = i * (length + 1);

    if (i < tokens - 1 && input[offset + length] != separator) {
      /* The current token did not end with e.g. a "." */
      RETURN_VALIDATION_FAILED
    }
    long ret = 0;
    if (php_filter_parse_hex(input + offset, length, &ret) < 0) {
      /* The current token is no valid hexadecimal digit */
      RETURN_VALIDATION_FAILED
    }
  }
  return value;
}

Variant php_filter_callback(PHP_INPUT_FILTER_PARAM_DECL) {
  if (!is_callable(option_array)) {
    raise_warning("First argument is expected to be a valid callback");
    return init_null();
  }
  Variant reffable = value;
  return vm_call_user_func(
    option_array,
    PackedArrayInit(1).appendRef(reffable).toArray()
  );
}

}
