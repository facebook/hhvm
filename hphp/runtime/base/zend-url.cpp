/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2018 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE.PHP, and is    |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/string-util.h"

#include <folly/portability/String.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_file("file");

static void replace_controlchars(String& output, const char *str, int len) {
  if (!str) return;

  unsigned char *s = (unsigned char *)str;
  unsigned char *e = (unsigned char *)str + len;
  output = String(str, len, CopyString);
  char *outbuf = output.mutableData();
  while (s < e) {
    if (iscntrl(*s)) {
      *outbuf='_';
    }
    s++;
    outbuf++;
  }
  output.setSize(len);
}

bool url_parse(Url &output, const char *str, size_t length) {
  char port_buf[6];
  // s: full string
  // ue: end of full string
  // p: start of string slice we're looking at
  // e: index of something we searched for, e.g. ':'. usually end of string
  //    slice, but not always
  // pp: start of string sub-slice
  const char *s, *e, *p, *pp, *ue;

  s = str;
  ue = s + length;

  /* parse scheme */
  if ((e = (const char *)memchr((const void *)s, ':', length)) && e != s) {
    /* validate scheme */
    p = s;
    while (p < e) {
      /* scheme = 1*[ lowalpha | digit | "+" | "-" | "." ] */
      if (!isalpha(*p) && !isdigit(*p) &&
          *p != '+' && *p != '.' && *p != '-') {
        if (e + 1 < ue && e < s + strcspn(s, "?#")) {
          goto parse_port;
        } else if (s + 1 < ue && *s == '/' && *(s + 1) == '/') {
          /* relative-scheme URL */
          s += 2;
          e = nullptr;
          goto parse_host;
        } else {
          goto just_path;
        }
      }
      p++;
    }

    if ((e + 1) == ue) { /* only scheme is available */
      replace_controlchars(output.scheme, s, (e - s));
      return true;
    }

    /*
     * certain schemas like mailto: and zlib: may not have any / after them
     * this check ensures we support those.
     */
    if (*(e+1) != '/') {
      /* check if the data we get is a port this allows us to
       * correctly parse things like a.com:80
       */
      p = e + 1;
      while (p < ue && isdigit(*p)) {
        p++;
      }

      if ((p == ue || *p == '/') && (p - e) < 7) {
        goto parse_port;
      }

      replace_controlchars(output.scheme, s, (e - s));

      s = e + 1;
      goto just_path;
    } else {
      replace_controlchars(output.scheme, s, (e - s));

      if (e + 2 < ue && *(e+2) == '/') {
        s = e + 3;
        if (output.scheme.get()->isame(s_file.get())) {
          if (e + 3 < ue && *(e + 3) == '/') {
            /* support windows drive letters as in:
               file:///c:/somedir/file.txt
            */
            if (e + 5 < ue && e[4] != '\0' && e[5] == ':') {
              s = e + 4;
            }
            goto just_path;
          }
        }
      } else {
        s = e + 1;
        goto just_path;
      }
    }
  } else if (e) { /* no scheme; starts with colon: look for port */
    parse_port:
    p = e + 1;
    pp = p;

    while (pp < ue && pp - p < 6 && isdigit(*pp)) {
      pp++;
    }

    if (pp - p > 0 && pp - p < 6 && (pp == ue || *pp == '/')) {
      memcpy(port_buf, p, (pp-p));
      port_buf[pp-p] = '\0';
      auto port = atoi(port_buf);
      if (port > 0 && port <= 65535) {
        output.port = port;
        if (s + 1 < ue && *s == '/' && *(s+1) == '/') {
          /* relative-scheme URL */
          s += 2;
        }
      } else {
        return false;
      }
    } else if (p == pp && pp == ue) {
      return false;
    } else if (s + 1 < ue && *s == '/' && *(s+1) == '/') {
      /* relative-scheme URL */
      s += 2;
    } else {
      goto just_path;
    }
  } else if (s + 1 < ue && *s == '/' && *(s +1 ) == '/') {
    /* relative-scheme URL */
    s += 2;
  } else {
    goto just_path;
  }

  parse_host:
  /* Binary-safe strcspn(s, "/?#") */
  e = ue;
  if ((p = (const char*)memchr(s, '/', e - s))) {
    e = p;
  }
  if ((p = (const char*)memchr(s, '?', e - s))) {
    e = p;
  }
  if ((p = (const char*)memchr(s, '#', e - s))) {
    e = p;
  }

  /* check for login and password */
  if ((p = (const char*)memrchr(s, '@', (e-s)))) {
    if ((pp = (const char*)memchr(s, ':', (p-s)))) {
      replace_controlchars(output.user, s, (pp - s));

      pp++;
      replace_controlchars(output.pass, pp, (p-pp));
    } else {
      replace_controlchars(output.user, s, (p-s));
    }

    s = p + 1;
  }

  /* check for port */
  if (s < ue && *s == '[' && *(e-1) == ']') {
    /* Short circuit portscan,
       we're dealing with an
       IPv6 embedded address */
    p = nullptr;
  } else {
    p = (const char*)memrchr(s, ':', e - s);
  }

  if (p) {
    if (!output.port) {
      p++;
      if (e-p > 5) { /* port cannot be longer then 5 characters */
        return false;
      } else if (e - p > 0) {
        memcpy(port_buf, p, (e-p));
        port_buf[e-p] = '\0';
        auto port = atoi(port_buf);
        if (port > 0 && port <= 65535) {
          output.port = port;
        } else {
          return false;
        }
      }
      p--;
    }
  } else {
    p = e;
  }

  /* check if we have a valid host, if we don't reject the string as url */
  if ((p-s) < 1) {
    return false;
  }

  replace_controlchars(output.host, s, (p - s));

  if (e == ue) {
    return true;
  }

  s = e;

  just_path:

  e = ue;
  p = (const char*)memchr(s, '#', (e - s));

  if (p) {
    p++;
    if (p < e) {
      replace_controlchars(output.fragment, p, e - p);
    }
    e = p - 1;
  }

  p = (const char*)memchr(s, '?', (e - s));
  if (p) {
    p++;
    if (p < e) {
      replace_controlchars(output.query, p, e - p);
    }
    e = p - 1;
  }

  if (s < e || s == ue) {
    replace_controlchars(output.path, s, e - s);
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

static int php_htoi(char *s) {
  int value;
  int c;

  c = ((unsigned char *)s)[0];
  if (isupper(c))
    c = tolower(c);
  value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

  c = ((unsigned char *)s)[1];
  if (isupper(c))
    c = tolower(c);
  value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

  return (value);
}

/* rfc1738:

   ...The characters ";",
   "/", "?", ":", "@", "=" and "&" are the characters which may be
   reserved for special meaning within a scheme...

   ...Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
   reserved characters used for their reserved purposes may be used
   unencoded within a URL...

   For added safety, we only leave -_. unencoded.
 */

static unsigned char hexchars[] = "0123456789ABCDEF";

String url_encode(const char *s, size_t len) {
  String retString(safe_address(len, 3, 1), ReserveString);
  register unsigned char c;
  unsigned char *to, *start;
  unsigned char const *from, *end;

  from = (unsigned char const *)s;
  end = (unsigned char const *)s + len;
  start = to = (unsigned char *)retString.mutableData();

  while (from < end) {
    c = *from++;

    if (c == ' ') {
      *to++ = '+';
    } else if ((c < '0' && c != '-' && c != '.') ||
           (c < 'A' && c > '9') ||
           (c > 'Z' && c < 'a' && c != '_') ||
           (c > 'z')) {
      to[0] = '%';
      to[1] = hexchars[c >> 4];
      to[2] = hexchars[c & 15];
      to += 3;
    } else {
      *to++ = c;
    }
  }
  retString.setSize(to - start);
  return retString;
}

String url_decode(const char *s, size_t len) {
  String retString(s, len, CopyString);
  char *str = retString.mutableData();
  char *dest = str;
  char *data = str;

  while (len--) {
    if (*data == '+') {
      *dest = ' ';
    }
    else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
         && isxdigit((int) *(data + 2))) {
      *dest = (char) php_htoi(data + 1);
      data += 2;
      len -= 2;
    } else {
      *dest = *data;
    }
    data++;
    dest++;
  }
  retString.setSize(dest - str);
  return retString;
}

size_t url_decode_ex(char *value, size_t len) {
  assertx(value && *value); // check before calling this function
  if (len == 0) return 0;

  size_t i = 0, o = 0;
  unsigned char *s = (unsigned char *)value;
  unsigned char *end = s + len;
  while (s + i < end) {
    if (s[i] == '+') {
      s[o++] = ' ';
      i++;
    } else if (s[i] == '%' && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
      char num;
      num = (s[i+1] >= 'A') ? ((s[i+1] & 0xdf) - 'A') + 10 : (s[i+1] - '0');
      num *= 16;
      num += (s[i+2] >= 'A') ? ((s[i+2] & 0xdf) - 'A') + 10 : (s[i+2] - '0');
      s[o++] = num;
      i+=3;
    } else {
      s[o++] = s[i++];
    }
  }
  if (i && o) s[o] = '\0';
  return o;
}

String url_raw_encode(const char *s, size_t len) {
  String retString(safe_address(len, 3, 1), ReserveString);
  size_t x, y;
  unsigned char *str = (unsigned char *)retString.mutableData();

  for (x = 0, y = 0; len--; x++, y++) {
    str[y] = (unsigned char) s[x];
    if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
      (str[y] < 'A' && str[y] > '9') ||
        (str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
      (str[y] > 'z' && str[y] != '~')) {
      str[y++] = '%';
      str[y++] = hexchars[(unsigned char) s[x] >> 4];
      str[y] = hexchars[(unsigned char) s[x] & 15];
    }
  }
  retString.setSize(y);
  return retString;
}

String url_raw_decode(const char *s, size_t len) {
  String retString(s, len, CopyString);
  char *str = retString.mutableData();
  char *dest = str;
  char *data = str;

  while (len--) {
    if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1))
        && isxdigit((int) *(data + 2))) {
      *dest = (char) php_htoi(data + 1);
      data += 2;
      len -= 2;
    } else {
      *dest = *data;
    }
    data++;
    dest++;
  }
  retString.setSize(dest - str);
  return retString;
}

///////////////////////////////////////////////////////////////////////////////
}
