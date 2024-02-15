/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/util/rds-local.h"

#include <folly/lang/Assume.h>
#include <boost/algorithm/string/predicate.hpp>

#define ICONV_SUPPORTS_ERRNO 1
#include <iconv.h>

/**
 * libiconv sometimes defines the second parameter of its
 * main function as (char**), and sometimes as (const char**) but
 * provides no means to detect this. Let build system determine
 * which is appropriate and (optionally) define ICONV_CONST=const
 * if such is needed in the cast.
 */
#ifndef ICONV_CONST
# define ICONV_CONST
#endif

#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static char _generic_superset_name[] = "UCS-4LE";
#define GENERIC_SUPERSET_NAME _generic_superset_name
#define GENERIC_SUPERSET_NBYTES 4

#define PHP_ICONV_MIME_DECODE_STRICT            (1<<0)
#define PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR (1<<1)

typedef enum _php_iconv_enc_scheme_t {
  PHP_ICONV_ENC_SCHEME_BASE64,
  PHP_ICONV_ENC_SCHEME_QPRINT
} php_iconv_enc_scheme_t;

typedef enum _php_iconv_err_t {
  PHP_ICONV_ERR_SUCCESS           = 0,
  PHP_ICONV_ERR_CONVERTER         = 1,
  PHP_ICONV_ERR_WRONG_CHARSET     = 2,
  PHP_ICONV_ERR_TOO_BIG           = 3,
  PHP_ICONV_ERR_ILLEGAL_SEQ       = 4,
  PHP_ICONV_ERR_ILLEGAL_CHAR      = 5,
  PHP_ICONV_ERR_UNKNOWN           = 6,
  PHP_ICONV_ERR_MALFORMED         = 7,
  PHP_ICONV_ERR_ALLOC             = 8
} php_iconv_err_t;

static void _php_iconv_show_error(const char *func, php_iconv_err_t &err,
                                  const char *out_charset,
                                  const char *in_charset) {
  if (!strncmp(func, "libiconv", 8)) func += 3;
  switch (err) {
  case PHP_ICONV_ERR_SUCCESS:
    break;
  case PHP_ICONV_ERR_CONVERTER:
    raise_notice("%s(): Cannot open converter", func);
    break;
  case PHP_ICONV_ERR_WRONG_CHARSET:
    raise_notice("%s(): Wrong charset, "
                  "conversion from `%s' to `%s' is not allowed",
                  func, in_charset, out_charset);
    break;
  case PHP_ICONV_ERR_ILLEGAL_CHAR:
    raise_notice("%s(): Detected an incomplete multibyte character "
                  "in input string", func);
    break;
  case PHP_ICONV_ERR_ILLEGAL_SEQ:
    if (boost::ends_with(out_charset, "//IGNORE")) {
      err = PHP_ICONV_ERR_SUCCESS;
      break;
    }
    raise_notice("%s(): Detected an illegal character in input string",
                  func);
    break;
  case PHP_ICONV_ERR_TOO_BIG:
    // should not happen
    raise_warning("%s(): Buffer length exceeded", func);
    break;
  case PHP_ICONV_ERR_MALFORMED:
    raise_warning("%s(): Malformed string", func);
    break;
  default:
    // other error
    raise_notice("%s(): Unknown error (%d)", func, errno);
    break;
  }
}

const StaticString s_ISO_8859_1("ISO-8859-1");

struct ICONVGlobals final : RequestEventHandler {
  String input_encoding;
  String output_encoding;
  String internal_encoding;

  ICONVGlobals() {}

  void requestInit() override {
    input_encoding = s_ISO_8859_1;
    output_encoding = s_ISO_8859_1;
    internal_encoding = s_ISO_8859_1;
  }

  void requestShutdown() override {
    input_encoding.reset();
    output_encoding.reset();
    internal_encoding.reset();
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(ICONVGlobals, s_iconv_globals);
#define ICONVG(name) s_iconv_globals->name

///////////////////////////////////////////////////////////////////////////////
// helpers

namespace {

/*
 * input string is in the form
 *    charset//options
 * where the //options part is optional.
 *
 * we've already checked that charset needs to be replaced by rep,
 * so now check if we can just return rep as is, or if we need to
 * graft the options back onto it.
 */
const char* munge_one(const char* chs, char** tofree,
                      const char* rep, size_t replen, size_t len) {
  if (!chs[len]) return rep;
  if (chs[len] != '/' || chs[len + 1] != '/') return chs;
  auto chslen = strlen(chs);
  auto charset_len = chslen - len;
  /* Avoid warnings about reading beyond array bounds, by indicating
   * to the compiler the relative sizes of the passed in strings.  */
  folly::assume(len + 2 <= chslen);
  *tofree = static_cast<char*>(req::malloc_noptrs(replen + charset_len + 1));
  memcpy(*tofree, rep, replen);
  memcpy(*tofree + replen, chs + len, charset_len + 1);
  return *tofree;
}

/*
 * libiconv disagrees with glibc iconv on some charsets. Performing
 * these substitutions gets libiconv closer to the glibc behavior.
 * Its not clear that glibc's behavior is better (in fact for uCs-2
 * its definitely worse); but it avoids unexpected changes in
 * behavior.
 */
const char* munge_charset(const char* chs, char** tofree) {
#define MUNGE_ONE(name, repl) do {                                      \
  if (!strncasecmp(name, chs, strlen(name))) {                          \
    return munge_one(chs, tofree, repl, strlen(repl), strlen(name));    \
  } } while (0)

  MUNGE_ONE("BIG5", "CP950");
  MUNGE_ONE("uCs-2", "uCs-2le");
  MUNGE_ONE("Unicode", "UTF-16");
#undef MUNGE_ONE
  return chs;
}

iconv_t iconv_open_helper(const char* out, const char* in) {
  char* tofree1 = nullptr;
  in = munge_charset(in, &tofree1);
  char* tofree2 = nullptr;
  out = munge_charset(out, &tofree2);
  auto ret = iconv_open(out, in);
  req::free(tofree1);
  req::free(tofree2);
  return ret;
}

}

#ifndef ICONV_CSNMAXLEN
#define ICONV_CSNMAXLEN 64
#endif
static bool validate_charset(const String& charset) {
  if (charset.size() >= ICONV_CSNMAXLEN) {
    raise_invalid_argument_warning
      ("Charset parameter exceeds the maximum allowed "
       "length of %d characters", ICONV_CSNMAXLEN);
    return false;
  }
  return true;
}

static Variant check_charset(const Variant& charset) {
  String charset_str = charset.isNull() ? null_string : charset.toString();
  if (charset_str.empty()) {
    return ICONVG(internal_encoding);
  }
  if (!validate_charset(charset_str)) return false;
  return charset;
}

static php_iconv_err_t _php_iconv_appendl(StringBuffer &d, const char *s,
                                          size_t l, iconv_t cd) {
  const char *in_p = s;
  size_t in_left = l;
  size_t out_left = 0;
  size_t buf_growth = 128;
  char *out_p;
#if !ICONV_SUPPORTS_ERRNO
  size_t prev_in_left = in_left;
#endif

  if (in_p != NULL) {
    while (in_left > 0) {
      out_left = buf_growth;
      out_p = d.appendCursor(out_left);

      if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **)&out_p, &out_left) ==
          (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
        switch (errno) {
        case EINVAL: return PHP_ICONV_ERR_ILLEGAL_CHAR;
        case EILSEQ: return PHP_ICONV_ERR_ILLEGAL_SEQ;
        case E2BIG:  break;
        default:
          return PHP_ICONV_ERR_UNKNOWN;
        }
#else
        if (prev_in_left == in_left) {
          return PHP_ICONV_ERR_UNKNOWN;
        }
#endif
      }
#if !ICONV_SUPPORTS_ERRNO
      prev_in_left = in_left;
#endif
      d.resize(d.size() + buf_growth - out_left);
      buf_growth <<= 1;
    }
  } else {
    for (;;) {
      out_left = buf_growth;
      out_p = d.appendCursor(out_left);

      if (iconv(cd, NULL, NULL, (char **)&out_p, &out_left) == (size_t)0) {
        d.resize(d.size() + buf_growth - out_left);
        break;
      } else {
#if ICONV_SUPPORTS_ERRNO
        if (errno != E2BIG) {
          return PHP_ICONV_ERR_UNKNOWN;
        }
#else
        if (out_left != 0) {
          return PHP_ICONV_ERR_UNKNOWN;
        }
#endif
      }
      d.resize(d.size() + buf_growth - out_left);
      buf_growth <<= 1;
    }
  }
  return PHP_ICONV_ERR_SUCCESS;
}

static php_iconv_err_t _php_iconv_appendc(StringBuffer &d, const char c,
                                          iconv_t cd) {
  return _php_iconv_appendl(d, &c, 1, cd);
}

static php_iconv_err_t php_iconv_string(const char *in_p, size_t in_len,
                                        char **out, size_t *out_len,
                                        const char *out_charset,
                                        const char *in_charset) {
#if !ICONV_SUPPORTS_ERRNO
  size_t in_size, out_size, out_left;
  char *out_buffer, *out_p;
  iconv_t cd;
  size_t result;

  *out = NULL;
  *out_len = 0;

  /**
   * This is not the right way to get output size...
   * This is not space efficient for large text.
   * This is also problem for encoding like UTF-7/UTF-8/ISO-2022 which
   * a single char can be more than 4 bytes.
   * I added 15 extra bytes for safety. <yohgaki@php.net>
   */
  out_size = in_len * sizeof(int) + 15;
  out_left = out_size;

  in_size = in_len;

  cd = iconv_open_helper(out_charset, in_charset);

  if (cd == (iconv_t)(-1)) {
    return PHP_ICONV_ERR_UNKNOWN;
  }

  out_buffer = (char*)req::malloc_noptrs(out_size + 1);
  out_p = out_buffer;
  SCOPE_EXIT {
    if (*out == nullptr) req::free(out_buffer);
  };

  result = iconv(cd, (char **)&in_p, &in_size, (char **)&out_p, &out_left);
  if (result == (size_t)(-1)) {
    return PHP_ICONV_ERR_UNKNOWN;
  }

  if (out_left < 8) {
    out_buffer = (char*)req::realloc_noptrs(out_buffer, out_size + 8);
  }

  // flush the shift-out sequences
  result = iconv(cd, NULL, NULL, &out_p, &out_left);

  if (result == (size_t)(-1)) {
    return PHP_ICONV_ERR_UNKNOWN;
  }

  *out_len = out_size - out_left;
  out_buffer[*out_len] = '\0';
  *out = out_buffer;

  iconv_close(cd);
  return PHP_ICONV_ERR_SUCCESS;

#else // iconv supports errno. Handle it better way.

  iconv_t cd;
  size_t in_left, out_size, out_left;
  char *out_p, *out_buf;
  size_t bsz, result = 0;
  php_iconv_err_t retval = PHP_ICONV_ERR_SUCCESS;
  int ignore_ilseq = boost::ends_with(out_charset, "//IGNORE") ||
                     boost::ends_with(out_charset, "//IGNORE//TRANSLIT");

  *out = NULL;
  *out_len = 0;

  cd = iconv_open_helper(out_charset, in_charset);

  if (cd == (iconv_t)(-1)) {
    if (errno == EINVAL) {
      return PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      return PHP_ICONV_ERR_CONVERTER;
    }
  }
  in_left= in_len;
  out_left = in_len + 32; // Avoid realloc() most cases
  out_size = 0;
  bsz = out_left;
  out_buf = (char *)req::malloc_noptrs(bsz + 1);
  out_p = out_buf;
  SCOPE_EXIT {
    if (*out == nullptr) req::free(out_buf);
  };

  while (in_left > 0) {
    result = iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **)&out_p, &out_left);
    out_size = bsz - out_left;
    if (result == (size_t)(-1)) {
      if (ignore_ilseq && errno == EILSEQ) {
        if (in_left <= 1) {
          result = 0;
        } else {
          errno = 0;
          in_p++;
          in_left--;
          continue;
        }
      }
      if (errno == E2BIG && in_left > 0) {
        // converted string is longer than out buffer
        bsz += in_len;

        out_buf = (char*)req::realloc_noptrs(out_buf, bsz + 1);
        out_p = out_buf;
        out_p += out_size;
        out_left = bsz - out_size;
        continue;
      }
    }
    break;
  }

  if (result != (size_t)(-1)) {
    // flush the shift-out sequences
    for (;;) {
      result = iconv(cd, NULL, NULL, (char **)&out_p, &out_left);
      out_size = bsz - out_left;

      if (result != (size_t)(-1)) {
        break;
      }

      if (errno == E2BIG) {
        bsz += 16;
        out_buf = (char *)req::realloc_noptrs(out_buf, bsz);
        out_p = out_buf;
        out_p += out_size;
        out_left = bsz - out_size;
      } else {
        break;
      }
    }
  }

  iconv_close(cd);

  if (result == (size_t)(-1)) {
    switch (errno) {
    case EINVAL: retval = PHP_ICONV_ERR_ILLEGAL_CHAR; break;
    case EILSEQ: retval = PHP_ICONV_ERR_ILLEGAL_SEQ;  break;
    case E2BIG:
      // should not happen
      retval = PHP_ICONV_ERR_TOO_BIG;
      break;
    default:
      // other error
      retval = PHP_ICONV_ERR_UNKNOWN;
      return PHP_ICONV_ERR_UNKNOWN;
    }
  }
  *out_p = '\0';
  *out = out_buf;
  *out_len = out_size;
  return retval;
#endif
}

static php_iconv_err_t _php_iconv_strlen(unsigned int *pretval,
                                         const char *str, size_t nbytes,
                                         const char *enc) {
  char buf[GENERIC_SUPERSET_NBYTES*2];
  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;
  iconv_t cd;
  const char *in_p;
  size_t in_left;
  char *out_p;
  size_t out_left;
  unsigned int cnt;

  *pretval = (unsigned int)-1;

  cd = iconv_open_helper(GENERIC_SUPERSET_NAME, enc);
  if (cd == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      return PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      return PHP_ICONV_ERR_CONVERTER;
    }
#else
    return PHP_ICONV_ERR_UNKNOWN;
#endif
  }

  errno = out_left = 0;

  for (in_p = str, in_left = nbytes, cnt = 0; in_left > 0; cnt+=2) {
    size_t prev_in_left;
    out_p = buf;
    out_left = sizeof(buf);

    prev_in_left = in_left;

    if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left)
        == (size_t)-1) {
      if (prev_in_left == in_left) {
        break;
      }
    }
  }

  if (out_left > 0) {
    cnt -= out_left / GENERIC_SUPERSET_NBYTES;
  }

#if ICONV_SUPPORTS_ERRNO
  switch (errno) {
  case EINVAL: err = PHP_ICONV_ERR_ILLEGAL_CHAR; break;
  case EILSEQ: err = PHP_ICONV_ERR_ILLEGAL_SEQ;  break;
  case E2BIG:
  case 0:
    *pretval = cnt;
    break;
  default:
    err = PHP_ICONV_ERR_UNKNOWN;
    break;
  }
#else
  *pretval = cnt;
#endif

  iconv_close(cd);
  return err;
}

static php_iconv_err_t _php_iconv_substr(StringBuffer &pretval,
                                         const char *str, size_t nbytes,
                                         int offset, int len, const char *enc) {
  char buf[GENERIC_SUPERSET_NBYTES];
  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;
  iconv_t cd1, cd2;
  const char *in_p;
  size_t in_left;
  char *out_p;
  size_t out_left;
  unsigned int cnt;
  unsigned int total_len;

  err = _php_iconv_strlen(&total_len, str, nbytes, enc);
  if (err != PHP_ICONV_ERR_SUCCESS) {
    return err;
  }

  if (len < 0) {
    if ((len += (total_len - offset)) < 0) {
      return PHP_ICONV_ERR_SUCCESS;
    }
  }

  if (offset < 0) {
    if ((offset += total_len) < 0) {
      return PHP_ICONV_ERR_SUCCESS;
    }
  }

  if (len > (int)total_len) {
    len = total_len;
  }


  if (offset >= (int)total_len) {
    return PHP_ICONV_ERR_SUCCESS;
  }

  if ((offset + len) > (int)total_len ) {
    /* trying to compute the length */
    len = total_len - offset;
  }

  if (len == 0) {
    return PHP_ICONV_ERR_SUCCESS;
  }

  cd1 = iconv_open_helper(GENERIC_SUPERSET_NAME, enc);

  if (cd1 == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      return PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      return PHP_ICONV_ERR_CONVERTER;
    }
#else
    return PHP_ICONV_ERR_UNKNOWN;
#endif
  }

  cd2 = (iconv_t)NULL;
  errno = 0;

  for (in_p = str, in_left = nbytes, cnt = 0; in_left > 0 && len > 0; ++cnt) {
    size_t prev_in_left;
    out_p = buf;
    out_left = sizeof(buf);

    prev_in_left = in_left;

    if (iconv(cd1, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left) ==
        (size_t)-1) {
      if (prev_in_left == in_left) {
        break;
      }
    }

    if (cnt >= (unsigned int)offset) {
      if (cd2 == (iconv_t)NULL) {
        cd2 = iconv_open_helper(enc, GENERIC_SUPERSET_NAME);

        if (cd2 == (iconv_t)(-1)) {
          cd2 = (iconv_t)NULL;
#if ICONV_SUPPORTS_ERRNO
          if (errno == EINVAL) {
            err = PHP_ICONV_ERR_WRONG_CHARSET;
          } else {
            err = PHP_ICONV_ERR_CONVERTER;
          }
#else
          err = PHP_ICONV_ERR_UNKNOWN;
#endif
          break;
        }
      }

      if (_php_iconv_appendl(pretval, buf, sizeof(buf), cd2) !=
          PHP_ICONV_ERR_SUCCESS) {
        break;
      }
      --len;
    }
  }

#if ICONV_SUPPORTS_ERRNO
  switch (errno) {
  case EINVAL:
    err = PHP_ICONV_ERR_ILLEGAL_CHAR;
    break;

  case EILSEQ:
    err = PHP_ICONV_ERR_ILLEGAL_SEQ;
    break;

  case E2BIG:
    break;
  }
#endif
  if (err == PHP_ICONV_ERR_SUCCESS) {
    if (cd2 != (iconv_t)NULL) {
      _php_iconv_appendl(pretval, NULL, 0, cd2);
    }
  }

  if (cd1 != (iconv_t)NULL) {
    iconv_close(cd1);
  }

  if (cd2 != (iconv_t)NULL) {
    iconv_close(cd2);
  }
  return err;
}

static php_iconv_err_t _php_iconv_strpos(unsigned int *pretval,
                                         const char *haystk,
                                         size_t haystk_nbytes,
                                         const char *ndl, size_t ndl_nbytes,
                                         int offset, const char *enc) {

#define _php_iconv_memequal(a, b, c)            \
  ((c) == sizeof(uint64_t)                      \
   ? (a).buf_64 == *((uint64_t *)(b))           \
   : ((c) == sizeof(uint32_t)                   \
      ? (a).buf_32 == *((uint32_t *)(b))        \
      : memcmp((a).buf, b, c) == 0))

  union gsnb_t {
    char buf[GENERIC_SUPERSET_NBYTES];
    uint32_t buf_32;
    uint64_t buf_64;
  } x;
  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;
  iconv_t cd;
  const char *in_p;
  size_t in_left;
  char *out_p;
  size_t out_left;
  unsigned int cnt;
  char* ndl_buf = nullptr;
  const char *ndl_buf_p;
  size_t ndl_buf_len, ndl_buf_left;
  unsigned int match_ofs;

  *pretval = (unsigned int)-1;

  err = php_iconv_string(ndl, ndl_nbytes,
                         &ndl_buf, &ndl_buf_len, GENERIC_SUPERSET_NAME, enc);
  SCOPE_EXIT { req::free(ndl_buf); };

  if (err != PHP_ICONV_ERR_SUCCESS) {
    return err;
  }

  cd = iconv_open_helper(GENERIC_SUPERSET_NAME, enc);

  if (cd == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      return PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      return PHP_ICONV_ERR_CONVERTER;
    }
#else
    return PHP_ICONV_ERR_UNKNOWN;
#endif
  }

  ndl_buf_p = ndl_buf;
  ndl_buf_left = ndl_buf_len;
  match_ofs = (unsigned int)-1;

  for (in_p = haystk, in_left = haystk_nbytes, cnt = 0; in_left > 0; ++cnt) {
    size_t prev_in_left;
    out_p = x.buf;
    out_left = sizeof(x.buf);

    prev_in_left = in_left;

    if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left, (char **) &out_p, &out_left) ==
        (size_t)-1) {
      if (prev_in_left == in_left) {
#if ICONV_SUPPORTS_ERRNO
        switch (errno) {
        case EINVAL: err = PHP_ICONV_ERR_ILLEGAL_CHAR; break;
        case EILSEQ: err = PHP_ICONV_ERR_ILLEGAL_SEQ;  break;
        case E2BIG:
          break;
        default:
          err = PHP_ICONV_ERR_UNKNOWN;
          break;
        }
#endif
        break;
      }
    }
    if (offset >= 0) {
      if (cnt >= (unsigned int)offset) {
        if (_php_iconv_memequal(x, ndl_buf_p, sizeof(x.buf))) {
          if (match_ofs == (unsigned int)-1) {
            match_ofs = cnt;
          }
          ndl_buf_p += GENERIC_SUPERSET_NBYTES;
          ndl_buf_left -= GENERIC_SUPERSET_NBYTES;
          if (ndl_buf_left == 0) {
            *pretval = match_ofs;
            break;
          }
        } else {
          unsigned int i, j, lim;

          i = 0;
          j = GENERIC_SUPERSET_NBYTES;
          lim = (unsigned int)(ndl_buf_p - ndl_buf);

          while (j < lim) {
            if (_php_iconv_memequal(*(gsnb_t*)&ndl_buf[j], &ndl_buf[i],
                                    GENERIC_SUPERSET_NBYTES)) {
              i += GENERIC_SUPERSET_NBYTES;
            } else {
              j -= i;
              i = 0;
            }
            j += GENERIC_SUPERSET_NBYTES;
          }

          if (_php_iconv_memequal(x, &ndl_buf[i], sizeof(x.buf))) {
            match_ofs += (lim - i) / GENERIC_SUPERSET_NBYTES;
            i += GENERIC_SUPERSET_NBYTES;
            ndl_buf_p = &ndl_buf[i];
            ndl_buf_left = ndl_buf_len - i;
          } else {
            match_ofs = (unsigned int)-1;
            ndl_buf_p = ndl_buf;
            ndl_buf_left = ndl_buf_len;
          }
        }
      }
    } else {
      if (_php_iconv_memequal(x, ndl_buf_p, sizeof(x.buf))) {
        if (match_ofs == (unsigned int)-1) {
          match_ofs = cnt;
        }
        ndl_buf_p += GENERIC_SUPERSET_NBYTES;
        ndl_buf_left -= GENERIC_SUPERSET_NBYTES;
        if (ndl_buf_left == 0) {
          *pretval = match_ofs;
          ndl_buf_p = ndl_buf;
          ndl_buf_left = ndl_buf_len;
          match_ofs = (unsigned int)-1;
        }
      } else {
        unsigned int i, j, lim;

        i = 0;
        j = GENERIC_SUPERSET_NBYTES;
        lim = (unsigned int)(ndl_buf_p - ndl_buf);

        while (j < lim) {
          if (_php_iconv_memequal(*(gsnb_t*)&ndl_buf[j], &ndl_buf[i],
                                  GENERIC_SUPERSET_NBYTES)) {
            i += GENERIC_SUPERSET_NBYTES;
          } else {
            j -= i;
            i = 0;
          }
          j += GENERIC_SUPERSET_NBYTES;
        }

        if (_php_iconv_memequal(x, &ndl_buf[i], sizeof(x.buf))) {
          match_ofs += (lim - i) / GENERIC_SUPERSET_NBYTES;
          i += GENERIC_SUPERSET_NBYTES;
          ndl_buf_p = &ndl_buf[i];
          ndl_buf_left = ndl_buf_len - i;
        } else {
          match_ofs = (unsigned int)-1;
          ndl_buf_p = ndl_buf;
          ndl_buf_left = ndl_buf_len;
        }
      }
    }
  }

  iconv_close(cd);
  return err;
}

static php_iconv_err_t _php_iconv_mime_decode(StringBuffer &retval,
                                              const char *str,
                                              size_t str_nbytes,
                                              const char *enc,
                                              const char **next_pos,
                                              int mode) {
  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;

  iconv_t cd = (iconv_t)(-1), cd_pl = (iconv_t)(-1);

  const char *p1;
  size_t str_left;
  unsigned int scan_stat = 0;
  const char *csname = NULL;
  size_t csname_len;
  const char *encoded_text = NULL;
  size_t encoded_text_len = 0;
  const char *encoded_word = NULL;
  const char *spaces = NULL;

  php_iconv_enc_scheme_t enc_scheme = PHP_ICONV_ENC_SCHEME_BASE64;

  if (next_pos != NULL) {
    *next_pos = NULL;
  }
  cd_pl = iconv_open_helper(enc, "ASCII");

  if (cd_pl == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      err = PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      err = PHP_ICONV_ERR_CONVERTER;
    }
#else
    err = PHP_ICONV_ERR_UNKNOWN;
#endif
    goto out;
  }

  p1 = str;
  for (str_left = str_nbytes; str_left > 0; str_left--, p1++) {
    int eos = 0;

    switch (scan_stat) {
    case 0: /* expecting any character */
      switch (*p1) {
      case '\r': /* part of an EOL sequence? */
        scan_stat = 7;
        break;

      case '\n':
        scan_stat = 8;
        break;

      case '=': /* first letter of an encoded chunk */
        encoded_word = p1;
        scan_stat = 1;
        break;

      case ' ': case '\t': /* a chunk of whitespaces */
        spaces = p1;
        scan_stat = 11;
        break;

      default: /* first letter of a non-encoded word */
        _php_iconv_appendc(retval, *p1, cd_pl);
        encoded_word = NULL;
        if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
          scan_stat = 12;
        }
        break;
      }
      break;

    case 1: /* expecting a delimiter */
      if (*p1 != '?') {
        err = _php_iconv_appendl(retval, encoded_word,
                                 (size_t)((p1 + 1) - encoded_word), cd_pl);
        if (err != PHP_ICONV_ERR_SUCCESS) {
          goto out;
        }
        encoded_word = NULL;
        if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
          scan_stat = 12;
        } else {
          scan_stat = 0;
        }
        break;
      }
      csname = p1 + 1;
      scan_stat = 2;
      break;

    case 2: /* expecting a charset name */
      switch (*p1) {
      case '?': /* normal delimiter: encoding scheme follows */
        scan_stat = 3;
        break;

      case '*': /* new style delimiter: locale id follows */
        scan_stat = 10;
        break;
      }
      if (scan_stat != 2) {
        char tmpbuf[80];

        if (csname == NULL) {
          err = PHP_ICONV_ERR_MALFORMED;
          goto out;
        }

        csname_len = (size_t)(p1 - csname);

        if (csname_len > sizeof(tmpbuf) - 1) {
          if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
            err = _php_iconv_appendl(retval, encoded_word,
                                     (size_t)((p1 + 1) - encoded_word), cd_pl);
            if (err != PHP_ICONV_ERR_SUCCESS) {
              goto out;
            }
            encoded_word = NULL;
            if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
              scan_stat = 12;
            } else {
              scan_stat = 0;
            }
            break;
          } else {
            err = PHP_ICONV_ERR_MALFORMED;
            goto out;
          }
        }

        memcpy(tmpbuf, csname, csname_len);
        tmpbuf[csname_len] = '\0';

        if (cd != (iconv_t)(-1)) {
          iconv_close(cd);
        }

        cd = iconv_open_helper(enc, tmpbuf);

        if (cd == (iconv_t)(-1)) {
          if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
            auto qmarks = 2;
            while (qmarks > 0 && str_left > 1) {
              p1++;
              qmarks -= *p1 == '?' ? 1 : 0 ;
              str_left--;
            }

            if (*(p1+1) == '=') {
              ++p1;
              --str_left;
            }

            err = _php_iconv_appendl(retval, encoded_word,
                                     (size_t)((p1 + 1) - encoded_word), cd_pl);
            if (err != PHP_ICONV_ERR_SUCCESS) {
              goto out;
            }
            scan_stat = 12;
            break;
          } else {
#if ICONV_SUPPORTS_ERRNO
            if (errno == EINVAL) {
              err = PHP_ICONV_ERR_WRONG_CHARSET;
            } else {
              err = PHP_ICONV_ERR_CONVERTER;
            }
#else
            err = PHP_ICONV_ERR_UNKNOWN;
#endif
            goto out;
          }
        }
      }
      break;

    case 3: /* expecting a encoding scheme specifier */
      switch (*p1) {
      case 'b':
      case 'B':
        enc_scheme = PHP_ICONV_ENC_SCHEME_BASE64;
        scan_stat = 4;
        break;

      case 'q':
      case 'Q':
        enc_scheme = PHP_ICONV_ENC_SCHEME_QPRINT;
        scan_stat = 4;
        break;

      default:
        if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
          err = _php_iconv_appendl(retval, encoded_word,
                                   (size_t)((p1 + 1) - encoded_word), cd_pl);
          if (err != PHP_ICONV_ERR_SUCCESS) {
            goto out;
          }
          encoded_word = NULL;
          if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
            scan_stat = 12;
          } else {
            scan_stat = 0;
          }
          break;
        } else {
          err = PHP_ICONV_ERR_MALFORMED;
          goto out;
        }
      }
      break;

    case 4: /* expecting a delimiter */
      if (*p1 != '?') {
        if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
          /* pass the entire chunk through the converter */
          err = _php_iconv_appendl(retval, encoded_word,
                                   (size_t)((p1 + 1) - encoded_word), cd_pl);
          if (err != PHP_ICONV_ERR_SUCCESS) {
            goto out;
          }
          encoded_word = NULL;
          if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
            scan_stat = 12;
          } else {
            scan_stat = 0;
          }
          break;
        } else {
          err = PHP_ICONV_ERR_MALFORMED;
          goto out;
        }
      }
      encoded_text = p1 + 1;
      scan_stat = 5;
      break;

    case 5: /* expecting an encoded portion */
      if (*p1 == '?') {
        encoded_text_len = (size_t)(p1 - encoded_text);
        scan_stat = 6;
      }
      break;

    case 7: /* expecting a "\n" character */
      if (*p1 == '\n') {
        scan_stat = 8;
      } else {
        /* bare CR */
        _php_iconv_appendc(retval, '\r', cd_pl);
        _php_iconv_appendc(retval, *p1, cd_pl);
        scan_stat = 0;
      }
      break;

    case 8: /* checking whether the following line is part of a
               folded header */
      if (*p1 != ' ' && *p1 != '\t') {
        --p1;
        str_left = 1; /* quit_loop */
        break;
      }
      if (encoded_word == NULL) {
        _php_iconv_appendc(retval, ' ', cd_pl);
      }
      spaces = NULL;
      scan_stat = 11;
      break;

    case 6: /* expecting a End-Of-Chunk character "=" */
      if (*p1 != '=') {
        if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
          /* pass the entire chunk through the converter */
          err = _php_iconv_appendl(retval, encoded_word,
                                   (size_t)((p1 + 1) - encoded_word), cd_pl);
          if (err != PHP_ICONV_ERR_SUCCESS) {
            goto out;
          }
          encoded_word = NULL;
          if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
            scan_stat = 12;
          } else {
            scan_stat = 0;
          }
          break;
        } else {
          err = PHP_ICONV_ERR_MALFORMED;
          goto out;
        }
      }
      scan_stat = 9;
      if (str_left == 1) {
        eos = 1;
      } else {
        break;
      }

    case 9: /* choice point, seeing what to do next.*/
      switch (*p1) {
      default:
        /* Handle non-RFC-compliant formats
         *
         * RFC2047 requires the character that comes right
         * after an encoded word (chunk) to be a whitespace,
         * while there are lots of broken implementations that
         * generate such malformed headers that don't fulfill
         * that requirement.
         */
        if (!eos) {
          if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
            /* pass the entire chunk through the converter */
            err = _php_iconv_appendl(retval, encoded_word,
                                     (size_t)((p1 + 1) - encoded_word), cd_pl);
            if (err != PHP_ICONV_ERR_SUCCESS) {
              goto out;
            }
            scan_stat = 12;
            break;
          }
        }
        /* break is omitted intentionally */

      case '\r': case '\n': case ' ': case '\t': {
        String decoded;
        switch (enc_scheme) {
        case PHP_ICONV_ENC_SCHEME_BASE64:
          {
            int len = encoded_text_len;
            decoded = string_base64_decode(encoded_text, len, false);
          }
          break;
        case PHP_ICONV_ENC_SCHEME_QPRINT:
          {
            int len = encoded_text_len;
            decoded = string_quoted_printable_decode(encoded_text, len, true);
          }
          break;
        default:
          break;
        }

        if (decoded.isNull()) {
          if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
            /* pass the entire chunk through the converter */
            err = _php_iconv_appendl(retval, encoded_word,
                                     (size_t)((p1 + 1) - encoded_word), cd_pl);
            if (err != PHP_ICONV_ERR_SUCCESS) {
              goto out;
            }
            encoded_word = NULL;
            if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
              scan_stat = 12;
            } else {
              scan_stat = 0;
            }
            break;
          } else {
            err = PHP_ICONV_ERR_UNKNOWN;
            goto out;
          }
        }

        err = _php_iconv_appendl(retval, decoded.data(), decoded.size(), cd);
        if (err != PHP_ICONV_ERR_SUCCESS) {
          if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
            /* pass the entire chunk through the converter */
            err = _php_iconv_appendl(retval, encoded_word,
                                     (size_t)(p1 - encoded_word), cd_pl);
            encoded_word = nullptr;
            if (err != PHP_ICONV_ERR_SUCCESS) {
              break;
            }
          } else {
            goto out;
          }
        }

        if (eos) { /* reached end-of-string. done. */
          scan_stat = 0;
          break;
        }

        switch (*p1) {
        case '\r': /* part of an EOL sequence? */
          scan_stat = 7;
          break;

        case '\n':
          scan_stat = 8;
          break;

        case '=': /* first letter of an encoded chunk */
          scan_stat = 1;
          break;

        case ' ': case '\t': /* medial whitespaces */
          spaces = p1;
          scan_stat = 11;
          break;

        default: /* first letter of a non-encoded word */
          _php_iconv_appendc(retval, *p1, cd_pl);
          scan_stat = 12;
          break;
        }
      } break;
      }
      break;

    case 10: /* expects a language specifier. dismiss it for now */
      if (*p1 == '?') {
        scan_stat = 3;
      }
      break;

    case 11: /* expecting a chunk of whitespaces */
      switch (*p1) {
      case '\r': /* part of an EOL sequence? */
        scan_stat = 7;
        break;

      case '\n':
        scan_stat = 8;
        break;

      case '=': /* first letter of an encoded chunk */
        if (spaces != NULL && encoded_word == NULL) {
          _php_iconv_appendl(retval, spaces, (size_t)(p1 - spaces), cd_pl);
          spaces = NULL;
        }
        encoded_word = p1;
        scan_stat = 1;
        break;

      case ' ': case '\t':
        break;

      default: /* first letter of a non-encoded word */
        if (spaces != NULL) {
          _php_iconv_appendl(retval, spaces, (size_t)(p1 - spaces), cd_pl);
          spaces = NULL;
        }
        _php_iconv_appendc(retval, *p1, cd_pl);
        encoded_word = NULL;
        if ((mode & PHP_ICONV_MIME_DECODE_STRICT)) {
          scan_stat = 12;
        } else {
          scan_stat = 0;
        }
        break;
      }
      break;

    case 12: /* expecting a non-encoded word */
      switch (*p1) {
      case '\r': /* part of an EOL sequence? */
        scan_stat = 7;
        break;

      case '\n':
        scan_stat = 8;
        break;

      case ' ': case '\t':
        spaces = p1;
        scan_stat = 11;
        break;

      case '=': /* first letter of an encoded chunk */
        if (!(mode & PHP_ICONV_MIME_DECODE_STRICT)) {
          encoded_word = p1;
          scan_stat = 1;
          break;
        }
        /* break is omitted intentionally */

      default:
        _php_iconv_appendc(retval, *p1, cd_pl);
        break;
      }
      break;
    }
  }
  switch (scan_stat) {
  case 0: case 8: case 11: case 12:
    break;
  default:
    if ((mode & PHP_ICONV_MIME_DECODE_CONTINUE_ON_ERROR)) {
      if (scan_stat == 1) {
        _php_iconv_appendc(retval, '=', cd_pl);
      }
      err = PHP_ICONV_ERR_SUCCESS;
    } else {
      err = PHP_ICONV_ERR_MALFORMED;
      goto out;
    }
  }

  if (next_pos != NULL) {
    *next_pos = p1;
  }

 out:
  if (cd != (iconv_t)(-1)) {
    iconv_close(cd);
  }
  if (cd_pl != (iconv_t)(-1)) {
    iconv_close(cd_pl);
  }
  return err;
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_scheme("scheme"),
  s_input_charset("input-charset"),
  s_output_charset("output-charset"),
  s_line_length("line-length"),
  s_line_break_chars("line-break-chars");

static Variant HHVM_FUNCTION(iconv_mime_encode,
    const String& field_name, const String& field_value,
    const Variant& preferences /* = uninit_variant */) {
  php_iconv_enc_scheme_t scheme_id = PHP_ICONV_ENC_SCHEME_BASE64;
  String in_charset;
  String out_charset;
  long line_len = 76;
  String lfchars = "\r\n";
  StringBuffer ret;
  char *buf = NULL;

  if (!preferences.isNull()) {
    Variant scheme = preferences.toArray()[s_scheme];
    if (scheme.isString()) {
      String s = scheme.toString();
      switch (*s.data()) {
      case 'B': case 'b':
        scheme_id = PHP_ICONV_ENC_SCHEME_BASE64;
        break;
      case 'Q': case 'q':
        scheme_id = PHP_ICONV_ENC_SCHEME_QPRINT;
        break;
      }
    }

    Variant input_charset = preferences.toArray()[s_input_charset];
    if (input_charset.isString()) {
      in_charset = input_charset.toString();
      if (!validate_charset(in_charset)) return false;
    }

    Variant output_charset = preferences.toArray()[s_output_charset];
    if (output_charset.isString()) {
      out_charset = output_charset.toString();
      if (!validate_charset(out_charset)) return false;
    }

    Variant line_length = preferences.toArray()[s_line_length];
    if (!line_length.isNull()) {
      line_len = line_length.toInt64();
    }

    Variant line_break_chars = preferences.toArray()[s_line_break_chars];
    if (!line_break_chars.isNull()) {
      lfchars = line_break_chars.toString();
    }
  }

  static int qp_table[256] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x00 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x10 */
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x20 */
    1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 3, 1, 3, /* 0x30 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x40 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, /* 0x50 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x60 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, /* 0x70 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x80 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x90 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xA0 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xB0 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xC0 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xD0 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xE0 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3  /* 0xF0 */
  };

  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;
  iconv_t cd = (iconv_t)(-1), cd_pl = (iconv_t)(-1);

  if ((field_name.size() + 2) >= line_len ||
      (out_charset.size() + 12) >= line_len) {
    /* field name is too long */
    err = PHP_ICONV_ERR_TOO_BIG;
    goto out;
  }

  cd_pl = iconv_open_helper("ASCII", in_charset.data());
  if (cd_pl == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      err = PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      err = PHP_ICONV_ERR_CONVERTER;
    }
#else
    err = PHP_ICONV_ERR_UNKNOWN;
#endif
    goto out;
  }

  cd = iconv_open_helper(out_charset.data(), in_charset.data());
  if (cd == (iconv_t)(-1)) {
#if ICONV_SUPPORTS_ERRNO
    if (errno == EINVAL) {
      err = PHP_ICONV_ERR_WRONG_CHARSET;
    } else {
      err = PHP_ICONV_ERR_CONVERTER;
    }
#else
    err = PHP_ICONV_ERR_UNKNOWN;
#endif
    goto out;
  }

  const char *in_p;
  size_t in_left;
  char *out_p;
  size_t out_left;

  buf = (char*)req::malloc_noptrs(line_len + 5);
  unsigned int char_cnt;
  char_cnt = line_len;

  _php_iconv_appendl(ret, field_name.data(), field_name.size(), cd_pl);
  char_cnt -= field_name.size();
  ret.append(": ");
  char_cnt -= 2;

  in_p = field_value.data();
  in_left = field_value.size();

  do {
    size_t prev_in_left;
    size_t out_size;

    if ((int)char_cnt < (out_charset.size() + 12)) {
      ret.append(lfchars); // lfchars must be encoded in ASCII here
      ret.append(' ');
      char_cnt = line_len - 1;
    }

    ret.append("=?");
    char_cnt -= 2;
    ret.append(out_charset);
    char_cnt -= out_charset.size();
    ret.append('?');
    char_cnt --;

    switch (scheme_id) {
    case PHP_ICONV_ENC_SCHEME_BASE64:
      {
        size_t ini_in_left;
        const char *ini_in_p;
        size_t out_reserved = 4;

        ret.append('B');
        char_cnt--;
        ret.append('?');
        char_cnt--;

        prev_in_left = ini_in_left = in_left;
        ini_in_p = in_p;

        out_size = (char_cnt - 2) / 4 * 3;

        for (;;) {
          out_p = buf;

          if (out_size <= out_reserved) {
            err = PHP_ICONV_ERR_TOO_BIG;
            goto out;
          }

          out_left = out_size - out_reserved;

          if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left,
                    (char **)&out_p, &out_left) == (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
            switch (errno) {
            case EINVAL: err = PHP_ICONV_ERR_ILLEGAL_CHAR; goto out;
            case EILSEQ: err = PHP_ICONV_ERR_ILLEGAL_SEQ;  goto out;
            case E2BIG:
              if (prev_in_left == in_left) {
                err = PHP_ICONV_ERR_TOO_BIG;
                goto out;
              }
              break;
            default:
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#else
            if (prev_in_left == in_left) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#endif
          }

          out_left += out_reserved;

          if (iconv(cd, NULL, NULL, (char **)&out_p, &out_left) ==
              (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
            if (errno != E2BIG) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#else
            if (out_left != 0) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#endif
          } else {
            break;
          }

          if (iconv(cd, NULL, NULL, NULL, NULL) == (size_t)-1) {
            err = PHP_ICONV_ERR_UNKNOWN;
            goto out;
          }

          out_reserved += 4;
          in_left = ini_in_left;
          in_p = ini_in_p;
        }

        prev_in_left = in_left;


        int encoded_len = out_size - out_left;
        String encoded = string_base64_encode(buf, encoded_len);
        if ((int)char_cnt < encoded.size()) {
          /* something went wrong! */
          err = PHP_ICONV_ERR_UNKNOWN;
          goto out;
        }

        ret.append(encoded);
        char_cnt -= encoded.size();
        ret.append("?=");
        char_cnt -= 2;
      }
      break; /* case PHP_ICONV_ENC_SCHEME_BASE64: */

    case PHP_ICONV_ENC_SCHEME_QPRINT:
      {
        size_t ini_in_left;
        const char *ini_in_p;
        const unsigned char *p;
        size_t nbytes_required;

        ret.append('Q');
        char_cnt--;
        ret.append('?');
        char_cnt--;

        prev_in_left = ini_in_left = in_left;
        ini_in_p = in_p;

        for (out_size = (char_cnt - 2) / 3; out_size > 0;) {
          size_t prev_out_left ATTRIBUTE_UNUSED;

          nbytes_required = 0;

          out_p = buf;
          out_left = out_size;

          if (iconv(cd, (ICONV_CONST char **)&in_p, &in_left,
                    (char **)&out_p, &out_left) == (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
            switch (errno) {
            case EINVAL: err = PHP_ICONV_ERR_ILLEGAL_CHAR; goto out;
            case EILSEQ: err = PHP_ICONV_ERR_ILLEGAL_SEQ;  goto out;
            case E2BIG:
              if (prev_in_left == in_left) {
                err = PHP_ICONV_ERR_UNKNOWN;
                goto out;
              }
              break;
            default:
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#else
            if (prev_in_left == in_left) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#endif
          }

          prev_out_left = out_left;
          if (iconv(cd, NULL, NULL, (char **)&out_p, &out_left) ==
              (size_t)-1) {
#if ICONV_SUPPORTS_ERRNO
            if (errno != E2BIG) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#else
            if (out_left == prev_out_left) {
              err = PHP_ICONV_ERR_UNKNOWN;
              goto out;
            }
#endif
          }

          for (p = (unsigned char *)buf; p < (unsigned char *)out_p; p++) {
            nbytes_required += qp_table[*p];
          }

          if (nbytes_required <= char_cnt - 2) {
            break;
          }

          out_size -= ((nbytes_required - (char_cnt - 2)) + 1) / (3 - 1);
          in_left = ini_in_left;
          in_p = ini_in_p;
        }

        for (p = (unsigned char *)buf; p < (unsigned char *)out_p; p++) {
          if (qp_table[*p] == 1) {
            ret.append(*(char*)p);
            char_cnt--;
          } else {
            static char qp_digits[] = "0123456789ABCDEF";
            ret.append('=');
            ret.append(qp_digits[(*p >> 4) & 0x0f]);
            ret.append(qp_digits[(*p & 0x0f)]);
            char_cnt -= 3;
          }
        }
        prev_in_left = in_left;

        ret.append("?=");
        char_cnt -= 2;

        if (iconv(cd, NULL, NULL, NULL, NULL) == (size_t)-1) {
          err = PHP_ICONV_ERR_UNKNOWN;
          goto out;
        }

      } break; /* case PHP_ICONV_ENC_SCHEME_QPRINT: */
    }
  } while (in_left > 0);

 out:
  if (cd != (iconv_t)(-1)) {
    iconv_close(cd);
  }
  if (cd_pl != (iconv_t)(-1)) {
    iconv_close(cd_pl);
  }
  if (buf != NULL) {
    req::free(buf);
  }

  if (err != PHP_ICONV_ERR_SUCCESS) {
    return false;
  }
  return ret.detach();
}

static Variant HHVM_FUNCTION(iconv_mime_decode,
    const String& encoded_string, int64_t mode /* = 0 */,
    const Variant& charset /* = null_string */) {
  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  StringBuffer retval;
  php_iconv_err_t err =
    _php_iconv_mime_decode(retval, encoded_string.data(),
                           encoded_string.size(), enc.data(), NULL, mode);
  _php_iconv_show_error(__FUNCTION__+2, err, enc.data(), "???");

  if (err == PHP_ICONV_ERR_SUCCESS) {
    return retval.detach();
  }
  return false;
}

static Variant HHVM_FUNCTION(iconv_mime_decode_headers,
    const String& encoded_headers,
    int64_t mode /* = 0 */,
    const Variant& charset /* = null_string */) {
  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  Array ret = Array::CreateDict();
  php_iconv_err_t err = PHP_ICONV_ERR_SUCCESS;
  const char *encoded_str = encoded_headers.data();
  int encoded_str_len = encoded_headers.size();
  while (encoded_str_len > 0) {
    StringBuffer decoded_header;

    const char *header_name = NULL;
    size_t header_name_len = 0;
    const char *header_value = NULL;
    size_t header_value_len = 0;
    const char *p, *limit;

    const char *next_pos;
    err = _php_iconv_mime_decode(decoded_header, encoded_str, encoded_str_len,
                                 enc.data(), &next_pos, mode);
    if (err != PHP_ICONV_ERR_SUCCESS || decoded_header.data() == NULL) {
      break;
    }

    limit = decoded_header.data() + decoded_header.size();
    for (p = decoded_header.data(); p < limit; p++) {
      if (*p == ':') {
        *((char*)p) = '\0';
        header_name = decoded_header.data();
        header_name_len = p - decoded_header.data();

        while (++p < limit) {
          if (*p != ' ' && *p != '\t') {
            break;
          }
        }

        header_value = p;
        header_value_len = limit - p;
        break;
      }
    }

    if (header_name != NULL) {
      String header(header_name, header_name_len, CopyString);
      String value(header_value, header_value_len, CopyString);
      if (ret.exists(header)) {
        Variant elem = ret[header];
        if (!elem.isArray()) {
          ret.set(header, make_vec_array(elem, value));
        } else {
          elem.asArrRef().append(value);
          ret.set(header, elem);
        }
      } else {
        ret.set(header, value);
      }
    }
    encoded_str_len -= next_pos - encoded_str;
    encoded_str = next_pos;
  }

  if (err != PHP_ICONV_ERR_SUCCESS) {
    _php_iconv_show_error(__FUNCTION__+2, err, enc.data(), "???");
    return false;
  }
  if (ret.empty()) {
    return init_null();
  }
  return ret;
}

const StaticString
  s_input_encoding("input_encoding"),
  s_output_encoding("output_encoding"),
  s_internal_encoding("internal_encoding"),
  s_all("all");


static Variant HHVM_FUNCTION(iconv_get_encoding,
    const String& type /* = "all" */) {
  if (type == s_all) {
    return make_dict_array(
      s_input_encoding,    ICONVG(input_encoding),
      s_output_encoding,   ICONVG(output_encoding),
      s_internal_encoding, ICONVG(internal_encoding)
    );
  }
  if (type == s_input_encoding)    return ICONVG(input_encoding);
  if (type == s_output_encoding)   return ICONVG(output_encoding);
  if (type == s_internal_encoding) return ICONVG(internal_encoding);
  return false;
}

static bool HHVM_FUNCTION(iconv_set_encoding,
    const String& type, const String& charset) {
  if (!validate_charset(charset)) return false;
  if (type == s_input_encoding) {
    ICONVG(input_encoding) = charset;
  } else if (type == s_output_encoding) {
    ICONVG(output_encoding) = charset;
  } else if (type == s_internal_encoding) {
    ICONVG(internal_encoding) = charset;
  } else {
    return false;
  }
  return true;
}

static Variant HHVM_FUNCTION(iconv, const String& in_charset,
    const String& out_charset, const String& str) {
  if (!validate_charset(in_charset)) return false;
  if (!validate_charset(out_charset)) return false;

  char* out_buffer = nullptr;
  size_t out_len;
  php_iconv_err_t err =
    php_iconv_string(str.data(), str.size(), &out_buffer, &out_len,
                     out_charset.data(), in_charset.data());
  SCOPE_EXIT { req::free(out_buffer); };
  _php_iconv_show_error(__FUNCTION__+2, err,
                        out_charset.data(), in_charset.data());
  if (err == PHP_ICONV_ERR_SUCCESS && out_buffer != nullptr) {
    return String(out_buffer, out_len, CopyString);
  }
  return false;
}

static Variant HHVM_FUNCTION(iconv_strlen,
    const String& str, const Variant& charset /* = null_string */) {
  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  unsigned int retval;
  php_iconv_err_t err = _php_iconv_strlen(&retval, str.data(), str.size(),
                                          enc.data());
  _php_iconv_show_error(__FUNCTION__+2, err, GENERIC_SUPERSET_NAME, enc.data());
  if (err == PHP_ICONV_ERR_SUCCESS) {
    return (int64_t)retval;
  }
  return false;
}

static Variant HHVM_FUNCTION(iconv_strpos,
    const String& haystack, const String& needle, int64_t offset /* = 0 */,
    const Variant& charset /* = null_string */) {
  if (offset < 0) {
    raise_warning("Offset not contained in string.");
    return false;
  }
  if (needle.size() < 1) {
    return false;
  }

  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  unsigned int retval;
  php_iconv_err_t err =
    _php_iconv_strpos(&retval, haystack.data(), haystack.size(),
                      needle.data(), needle.size(), offset, enc.data());
  _php_iconv_show_error(__FUNCTION__+2, err, GENERIC_SUPERSET_NAME, enc.data());
  if (err == PHP_ICONV_ERR_SUCCESS && retval != (unsigned int)-1) {
    return (long)retval;
  }
  return false;
}

static Variant HHVM_FUNCTION(iconv_strrpos,
    const String& haystack, const String& needle,
    const Variant& charset /* = null_string */) {
  if (needle.size() < 1) {
    return false;
  }

  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  unsigned int retval;
  php_iconv_err_t err =
    _php_iconv_strpos(&retval, haystack.data(), haystack.size(),
                      needle.data(), needle.size(), -1, enc.data());
  _php_iconv_show_error(__FUNCTION__+2, err, GENERIC_SUPERSET_NAME, enc.data());
  if (err == PHP_ICONV_ERR_SUCCESS && retval != (unsigned int)-1) {
    return (long)retval;
  }
  return false;
}

static Variant HHVM_FUNCTION(iconv_substr,
    const String& str, int64_t offset, int64_t length /* = INT_MAX */,
    const Variant& charset /* = null_string */) {
  Variant encoded = check_charset(charset);
  if (same(encoded, false)) return false;
  String enc = encoded.toString();
  length = length <= INT32_MAX ? length : INT32_MAX;
  StringBuffer retval;
  php_iconv_err_t err = _php_iconv_substr(retval, str.data(), str.size(),
                                          offset, length, enc.data());
  _php_iconv_show_error(__FUNCTION__+2, err, GENERIC_SUPERSET_NAME, enc.data());
  if (err == PHP_ICONV_ERR_SUCCESS && !str.empty() && retval.data()) {
    return retval.detach();
  }
  return false;
}

static String
HHVM_FUNCTION(ob_iconv_handler, const String& contents, int64_t /*status*/) {
  String mimetype = g_context->getMimeType();
  if (!mimetype.empty()) {
    char* out_buffer = nullptr;
    size_t out_len;
    php_iconv_err_t err =
      php_iconv_string(contents.data(), contents.size(), &out_buffer, &out_len,
                       ICONVG(output_encoding).c_str(),
                       ICONVG(internal_encoding).c_str());
    SCOPE_EXIT { req::free(out_buffer); };
    _php_iconv_show_error(__FUNCTION__+2, err, ICONVG(output_encoding).c_str(),
                          ICONVG(internal_encoding).c_str());
    if (out_buffer != NULL) {
      g_context->setContentType(mimetype, ICONVG(output_encoding));
      return String(out_buffer, out_len, CopyString);
    }
  }
  return contents;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _LIBICONV_VERSION
const char* iconv_impl() { return "libiconv"; };
std::string iconv_version() {
  return folly::sformat("{}.{}",
                        _LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 255);
}
#else
const char* iconv_impl() { return "glibc"; };
#ifdef __GLIBC__
std::string iconv_version() {
  return folly::sformat("{}.{}",
                        __GLIBC__, __GLIBC_MINOR__);
}
#else
const char* iconv_version() { return "2.5"; }
#endif
#endif

struct iconvExtension final : Extension {
  iconvExtension() : Extension("iconv", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleRegisterNative() override {
    HHVM_RC_STR(ICONV_IMPL, iconv_impl());
    HHVM_RC_STR(ICONV_VERSION, iconv_version());

    HHVM_FE(iconv_get_encoding);
    HHVM_FE(iconv_mime_decode_headers);
    HHVM_FE(iconv_mime_decode);
    HHVM_FE(iconv_mime_encode);
    HHVM_FE(iconv_set_encoding);
    HHVM_FE(iconv_strlen);
    HHVM_FE(iconv_strpos);
    HHVM_FE(iconv_strrpos);
    HHVM_FE(iconv_substr);
    HHVM_FE(iconv);
    HHVM_FE(ob_iconv_handler);
  }

} s_iconv_extension;

/////////////////////////////////////////////////////////////////////////////
}
