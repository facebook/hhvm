/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Stig Sæther Bakken <ssb@php.net>                            |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

/* Synced with php 3.0 revision 1.193 1999-06-16 [ssb] */

#include <stdio.h>
#ifdef PHP_WIN32
# include "win32/php_stdint.h"
#else
# include <stdint.h>
#endif
#include "php.h"
#include "php_rand.h"
#include "php_string.h"
#include "php_variables.h"
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#ifdef HAVE_MONETARY_H
# include <monetary.h>
#endif
/*
 * This define is here because some versions of libintl redefine setlocale
 * to point to libintl_setlocale.  That's a ridiculous thing to do as far
 * as I am concerned, but with this define and the subsequent undef we
 * limit the damage to just the actual setlocale() call in this file
 * without turning zif_setlocale into zif_libintl_setlocale.  -Rasmus
 */
#define php_my_setlocale setlocale
#ifdef HAVE_LIBINTL
# include <libintl.h> /* For LC_MESSAGES */
 #ifdef setlocale
 # undef setlocale
 #endif
#endif

#include "zend_API.h"
#include "zend_execute.h"
#include "php_globals.h"
#include "php_smart_str.h"
#include <Zend/zend_exceptions.h>
#ifdef ZTS
#include "TSRM.h"
#endif

/* For str_getcsv() support */
#include "ext/standard/file.h"

#define STR_PAD_LEFT      0
#define STR_PAD_RIGHT      1
#define STR_PAD_BOTH      2
#define PHP_PATHINFO_DIRNAME   1
#define PHP_PATHINFO_BASENAME   2
#define PHP_PATHINFO_EXTENSION   4
#define PHP_PATHINFO_FILENAME   8
#define PHP_PATHINFO_ALL  (PHP_PATHINFO_DIRNAME | PHP_PATHINFO_BASENAME | PHP_PATHINFO_EXTENSION | PHP_PATHINFO_FILENAME)

#define STR_STRSPN        0
#define STR_STRCSPN        1

/* {{{ php_charmask
 * Fills a 256-byte bytemask with input. You can specify a range like 'a..z',
 * it needs to be incrementing.
 * Returns: FAILURE/SUCCESS whether the input was correct (i.e. no range errors)
 */
static inline int php_charmask(unsigned char *input, int len, char *mask TSRMLS_DC)
{
  unsigned char *end;
  unsigned char c;
  int result = SUCCESS;

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
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '..'-range, no character to the left of '..'");
        result = FAILURE;
        continue;
      }
      if (input+2 >= end) { /* there is no 'right' char */
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '..'-range, no character to the right of '..'");
        result = FAILURE;
        continue;
      }
      if (input[-1] > input[2]) { /* wrong order */
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '..'-range, '..'-range needs to be incrementing");
        result = FAILURE;
        continue;
      }
      /* FIXME: better error (a..b..c is the only left possibility?) */
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid '..'-range");
      result = FAILURE;
      continue;
    } else {
      mask[c]=1;
    }
  }
  return result;
}
/* }}} */

/* {{{ php_trim()
 * mode 1 : trim left
 * mode 2 : trim right
 * mode 3 : trim left and right
 * what indicates which chars are to be trimmed. NULL->default (' \t\n\r\v\0')
 */
PHPAPI char *php_trim(char *c, int len, char *what, int what_len, zval *return_value, int mode TSRMLS_DC)
{
  register int i;
  int trimmed = 0;
  char mask[256];

  if (what) {
    php_charmask((unsigned char*)what, what_len, mask TSRMLS_CC);
  } else {
    php_charmask((unsigned char*)" \n\r\t\v\0", 6, mask TSRMLS_CC);
  }

  if (mode & 1) {
    for (i = 0; i < len; i++) {
      if (mask[(unsigned char)c[i]]) {
        trimmed++;
      } else {
        break;
      }
    }
    len -= trimmed;
    c += trimmed;
  }
  if (mode & 2) {
    for (i = len - 1; i >= 0; i--) {
      if (mask[(unsigned char)c[i]]) {
        len--;
      } else {
        break;
      }
    }
  }

  if (return_value) {
    RETVAL_STRINGL(c, len, 1);
  } else {
    return estrndup(c, len);
  }
  return "";
}

/* {{{ php_str_to_str_ex
 */
PHPAPI char *php_str_to_str_ex(char *haystack, int length,
  char *needle, int needle_len, char *str, int str_len, int *_new_length, int case_sensitivity, int *replace_count)
{
  char *new_str;

  if (needle_len < length) {
    char *end, *haystack_dup = NULL, *needle_dup = NULL;
    char *e, *s, *p, *r;

    if (needle_len == str_len) {
      new_str = estrndup(haystack, length);
      *_new_length = length;

      if (case_sensitivity) {
        end = new_str + length;
        for (p = new_str; (r = php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
          memcpy(r, str, str_len);
          if (replace_count) {
            (*replace_count)++;
          }
        }
      } else {
        haystack_dup = estrndup(haystack, length);
        needle_dup = estrndup(needle, needle_len);
        php_strtolower(haystack_dup, length);
        php_strtolower(needle_dup, needle_len);
        end = haystack_dup + length;
        for (p = haystack_dup; (r = php_memnstr(p, needle_dup, needle_len, end)); p = r + needle_len) {
          memcpy(new_str + (r - haystack_dup), str, str_len);
          if (replace_count) {
            (*replace_count)++;
          }
        }
        efree(haystack_dup);
        efree(needle_dup);
      }
      return new_str;
    } else {
      if (!case_sensitivity) {
        haystack_dup = estrndup(haystack, length);
        needle_dup = estrndup(needle, needle_len);
        php_strtolower(haystack_dup, length);
        php_strtolower(needle_dup, needle_len);
      }

      if (str_len < needle_len) {
        new_str = (char*) emalloc(length + 1);
      } else {
        int count = 0;
        char *o, *n, *endp;

        if (case_sensitivity) {
          o = haystack;
          n = needle;
        } else {
          o = haystack_dup;
          n = needle_dup;
        }
        endp = o + length;

        while ((o = php_memnstr(o, n, needle_len, endp))) {
          o += needle_len;
          count++;
        }
        if (count == 0) {
          /* Needle doesn't occur, shortcircuit the actual replacement. */
          if (haystack_dup) {
            efree(haystack_dup);
          }
          if (needle_dup) {
            efree(needle_dup);
          }
          new_str = estrndup(haystack, length);
          if (_new_length) {
            *_new_length = length;
          }
          return new_str;
        } else {
          new_str = (char*) safe_emalloc(count, str_len - needle_len, length + 1);
        }
      }

      e = s = new_str;

      if (case_sensitivity) {
        end = haystack + length;
        for (p = haystack; (r = php_memnstr(p, needle, needle_len, end)); p = r + needle_len) {
          memcpy(e, p, r - p);
          e += r - p;
          memcpy(e, str, str_len);
          e += str_len;
          if (replace_count) {
            (*replace_count)++;
          }
        }

        if (p < end) {
          memcpy(e, p, end - p);
          e += end - p;
        }
      } else {
        end = haystack_dup + length;

        for (p = haystack_dup; (r = php_memnstr(p, needle_dup, needle_len, end)); p = r + needle_len) {
          memcpy(e, haystack + (p - haystack_dup), r - p);
          e += r - p;
          memcpy(e, str, str_len);
          e += str_len;
          if (replace_count) {
            (*replace_count)++;
          }
        }

        if (p < end) {
          memcpy(e, haystack + (p - haystack_dup), end - p);
          e += end - p;
        }
      }

      if (haystack_dup) {
        efree(haystack_dup);
      }
      if (needle_dup) {
        efree(needle_dup);
      }

      *e = '\0';
      *_new_length = e - s;

      new_str = (char*) erealloc(new_str, *_new_length + 1);
      return new_str;
    }
  } else if (needle_len > length) {
nothing_todo:
    *_new_length = length;
    new_str = estrndup(haystack, length);
    return new_str;
  } else {
    if (case_sensitivity && memcmp(haystack, needle, length)) {
      goto nothing_todo;
    } else if (!case_sensitivity) {
      char *l_haystack, *l_needle;

      l_haystack = estrndup(haystack, length);
      l_needle = estrndup(needle, length);

      php_strtolower(l_haystack, length);
      php_strtolower(l_needle, length);

      if (memcmp(l_haystack, l_needle, length)) {
        efree(l_haystack);
        efree(l_needle);
        goto nothing_todo;
      }
      efree(l_haystack);
      efree(l_needle);
    }

    *_new_length = str_len;
    new_str = estrndup(str, str_len);

    if (replace_count) {
      (*replace_count)++;
    }
    return new_str;
  }

}
/* }}} */

/* {{{ php_strtolower
 */
PHPAPI char *php_strtolower(char *s, size_t len)
{
  unsigned char *c, *e;

  c = (unsigned char *)s;
  e = c+len;

  while (c < e) {
    *c = tolower(*c);
    c++;
  }
  return s;
}
/* }}} */
