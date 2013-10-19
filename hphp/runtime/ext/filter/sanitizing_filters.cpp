/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/filter/sanitizing_filters.h"
#include "hphp/runtime/ext/ext_filter.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {

typedef unsigned long filter_map[256];

static String php_filter_encode_html(const String& value,
                                     const unsigned char *chars) {
  int len = value.length();
  unsigned char *s = (unsigned char *)value.data();
  unsigned char *e = s + len;

  if (len == 0) {
    return empty_string;
  }
  StringBuffer str(len);

  while (s < e) {
    if (chars[*s]) {
      str.append("&#");
      str.append(static_cast<int64_t>(s[0]));
      str.append(';');
    } else {
      /* XXX: this needs to be optimized to work with blocks of 'safe' chars */
      str.append(s[0]);
    }
    s++;
  }

  return str.detach();
}

static const unsigned char hexchars[] = "0123456789ABCDEF";

#define LOWALPHA    "abcdefghijklmnopqrstuvwxyz"
#define HIALPHA     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT       "0123456789"

#define DEFAULT_URL_ENCODE    LOWALPHA HIALPHA DIGIT "-._"

static Variant php_filter_encode_url(const String& value, const unsigned char* chars,
                                     const int char_len, int high, int low,
                                     int encode_nul) {
  unsigned char tmp[256];
  unsigned char *s = (unsigned char *)chars;
  unsigned char *e = s + char_len;
  int len = value.length();
  if (len == 0) {
    return empty_string;
  }

  memset(tmp, 1, sizeof(tmp)-1);

  while (s < e) {
    tmp[*s++] = 0;
  }
  StringBuffer str(len);
  s = (unsigned char *)value.data();
  e = s + len;

  while (s < e) {
    if (tmp[*s]) {
      str.append('%');
      str.append((char) hexchars[(unsigned char) *s >> 4]);
      str.append((char) hexchars[(unsigned char) *s & 15]);
    } else {
      str.append((char) *s);
    }
    s++;
  }
  return str.detach();
}

static Variant php_filter_strip(const String& value, long flags) {
  unsigned char *str;
  int i;
  int len = value.length();
  if (len == 0) {
    return empty_string;
  }

  /* Optimization for if no strip flags are set */
  if (! ((flags & k_FILTER_FLAG_STRIP_LOW) ||
         (flags & k_FILTER_FLAG_STRIP_HIGH)) ) {
    return value;
  }

  str = (unsigned char *)value.data();
  StringBuffer buf(len);
  for (i = 0; i < len; i++) {
    if ((str[i] > 127) && (flags & k_FILTER_FLAG_STRIP_HIGH)) {
    } else if ((str[i] < 32) && (flags & k_FILTER_FLAG_STRIP_LOW)) {
    } else if ((str[i] == '`') && (flags & k_FILTER_FLAG_STRIP_BACKTICK)) {
    } else {
      buf.append((char) str[i]);
    }
  }
  return buf.detach();
}

static void filter_map_init(filter_map *map) {
  memset(map, 0, sizeof(filter_map));
}

static void filter_map_update(filter_map *map, int flag,
                              const unsigned char *allowed_list) {
  int l, i;

  l = strlen((const char*)allowed_list);
  for (i = 0; i < l; ++i) {
    (*map)[allowed_list[i]] = flag;
  }
}

static Variant filter_map_apply(const String& value, filter_map *map) {
  unsigned char *str;
  int i;
  int len = value.length();
  if (len == 0) {
    return empty_string;
  }

  str = (unsigned char *)value.data();
  StringBuffer buf(len);
  for (i = 0; i < len; i++) {
    if ((*map)[str[i]]) {
      buf.append((char) str[i]);
    }
  }
  return buf.detach();
}

template <typename T>
unsigned char uc(T c) { return (unsigned char)c; }

Variant php_filter_string(PHP_INPUT_FILTER_PARAM_DECL) {
  unsigned char enc[256] = {0};

  /* strip high/strip low ( see flags )*/
  String stripped(php_filter_strip(value, flags));

  if (!(flags & k_FILTER_FLAG_NO_ENCODE_QUOTES)) {
    enc[uc('\'')] = enc[uc('"')] = 1;
  }
  if (flags & k_FILTER_FLAG_ENCODE_AMP) {
    enc[uc('&')] = 1;
  }
  if (flags & k_FILTER_FLAG_ENCODE_LOW) {
    memset(enc, 1, 32);
  }
  if (flags & k_FILTER_FLAG_ENCODE_HIGH) {
    memset(enc + 127, 1, sizeof(enc) - 127);
  }

  String encoded(php_filter_encode_html(stripped, enc));
  int len = encoded.length();
  char *ret = string_strip_tags(
    encoded.data(), len, empty_string.data(), empty_string.length(), true
  );

  if (len == 0) {
    if (flags & k_FILTER_FLAG_EMPTY_STRING_NULL) {
      free(ret);
      return uninit_null();
    }
    free(ret);
    return empty_string;
  }
  // string_strip_tags mallocs this string
  return String(ret, AttachString);
}

Variant php_filter_encoded(PHP_INPUT_FILTER_PARAM_DECL) {
  /* apply strip_high and strip_low filters */
  php_filter_strip(value, flags);
  /* urlencode */
  return php_filter_encode_url(
    value,
    (unsigned char *)DEFAULT_URL_ENCODE,
    sizeof(DEFAULT_URL_ENCODE)-1,
    flags & k_FILTER_FLAG_ENCODE_HIGH,
    flags & k_FILTER_FLAG_ENCODE_LOW,
    1
  );
}

Variant php_filter_special_chars(PHP_INPUT_FILTER_PARAM_DECL) {
  unsigned char enc[256] = {0};

  php_filter_strip(value, flags);

  /* encodes ' " < > & \0 to numerical entities */
  enc[uc('\'')] = enc[uc('"')] = enc[uc('<')] = enc[uc('>')] = enc[uc('&')] = enc[0] = 1;

  /* if strip low is not set, then we encode them as &#xx; */
  memset(enc, 1, 32);

  if (flags & k_FILTER_FLAG_ENCODE_HIGH) {
    memset(enc + 127, 1, sizeof(enc) - 127);
  }

  return php_filter_encode_html(value, enc);
}

Variant php_filter_full_special_chars(PHP_INPUT_FILTER_PARAM_DECL) {
  int quotes;
  if (!(flags & k_FILTER_FLAG_NO_ENCODE_QUOTES)) {
    quotes = k_ENT_QUOTES;
  } else {
    quotes = k_ENT_NOQUOTES;
  }
  return f_htmlentities(value, quotes);
}

Variant php_filter_unsafe_raw(PHP_INPUT_FILTER_PARAM_DECL) {
  /* Only if no flags are set (optimization) */
  if (flags != 0 && value.length() > 0) {
    unsigned char enc[256] = {0};

    php_filter_strip(value, flags);

    if (flags & k_FILTER_FLAG_ENCODE_AMP) {
      enc[uc('&')] = 1;
    }
    if (flags & k_FILTER_FLAG_ENCODE_LOW) {
      memset(enc, 1, 32);
    }
    if (flags & k_FILTER_FLAG_ENCODE_HIGH) {
      memset(enc + 127, 1, sizeof(enc) - 127);
    }

    return php_filter_encode_html(value, enc);
  } else if (flags & k_FILTER_FLAG_EMPTY_STRING_NULL && value.length() == 0) {
    return uninit_null();
  }
  return value;
}



#define SAFE        "$-_.+"
#define EXTRA       "!*'(),"
#define NATIONAL    "{}|\\^~[]`"
#define PUNCTUATION "<>#%\""
#define RESERVED    ";/?:@&="

Variant php_filter_email(PHP_INPUT_FILTER_PARAM_DECL) {
  /* Check section 6 of rfc 822 http://www.faqs.org/rfcs/rfc822.html */
  const unsigned char allowed_list[] = LOWALPHA HIALPHA DIGIT \
                                       "!#$%&'*+-=?^_`{|}~@.[]";
  filter_map     map;

  filter_map_init(&map);
  filter_map_update(&map, 1, allowed_list);
  return filter_map_apply(value, &map);
}

Variant php_filter_url(PHP_INPUT_FILTER_PARAM_DECL) {
  /* Strip all chars not part of section 5 of
   * http://www.faqs.org/rfcs/rfc1738.html */
  const unsigned char allowed_list[] = LOWALPHA HIALPHA DIGIT SAFE EXTRA \
                                       NATIONAL PUNCTUATION RESERVED;
  filter_map     map;

  filter_map_init(&map);
  filter_map_update(&map, 1, allowed_list);
  return filter_map_apply(value, &map);
}

Variant php_filter_number_int(PHP_INPUT_FILTER_PARAM_DECL) {
  /* strip everything [^0-9+-] */
  const unsigned char allowed_list[] = "+-" DIGIT;
  filter_map     map;

  filter_map_init(&map);
  filter_map_update(&map, 1, allowed_list);
  return filter_map_apply(value, &map);
}

Variant php_filter_number_float(PHP_INPUT_FILTER_PARAM_DECL) {
  /* strip everything [^0-9+-] */
  const unsigned char allowed_list[] = "+-" DIGIT;
  filter_map     map;

  filter_map_init(&map);
  filter_map_update(&map, 1, allowed_list);

  /* depending on flags, strip '.', 'e', ",", "'" */
  if (flags & k_FILTER_FLAG_ALLOW_FRACTION) {
    filter_map_update(&map, 2, (const unsigned char *) ".");
  }
  if (flags & k_FILTER_FLAG_ALLOW_THOUSAND) {
    filter_map_update(&map, 3,  (const unsigned char *) ",");
  }
  if (flags & k_FILTER_FLAG_ALLOW_SCIENTIFIC) {
    filter_map_update(&map, 4,  (const unsigned char *) "eE");
  }
  return filter_map_apply(value, &map);
}

Variant php_filter_magic_quotes(PHP_INPUT_FILTER_PARAM_DECL) {
  /* just call addslashes quotes */
  return f_addslashes(value);
}

}
