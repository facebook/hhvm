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

#include "hphp/runtime/ext/ext_mb.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/ext/php_unicode.h"
#include "hphp/runtime/ext/unicode_data.h"
#include "hphp/runtime/ext/ext_process.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-event-handler.h"

extern "C" {
#include <mbfl/mbfl_convert.h>
#include <mbfl/mbfilter.h>
#include <oniguruma.h>
#include <map>
}

#define php_mb_re_pattern_buffer   re_pattern_buffer
#define php_mb_regex_t             regex_t
#define php_mb_re_registers        re_registers

extern void mbfl_memory_device_unput(mbfl_memory_device *device);

#define PARSE_POST 0
#define PARSE_GET 1
#define PARSE_COOKIE 2
#define PARSE_STRING 3
#define PARSE_ENV 4
#define PARSE_SERVER 5
#define PARSE_SESSION 6

namespace HPHP {

static class mbstringExtension : public Extension {
  public:
  mbstringExtension() : Extension("mbstring", NO_EXTENSION_VERSION_YET) {}

  virtual void moduleInit() {
    // TODO make these PHP_INI_ALL and thread local once we use them
    IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "mbstring.http_input",
                     &http_input);
    IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "mbstring.http_output",
                     &http_output);
  }

  static std::string http_input;
  static std::string http_output;
} s_mbstring_extension;

std::string mbstringExtension::http_input = "pass";
std::string mbstringExtension::http_output = "pass";

///////////////////////////////////////////////////////////////////////////////
// statics

#define PHP_MBSTR_STACK_BLOCK_SIZE 32

typedef struct _php_mb_nls_ident_list {
  mbfl_no_language lang;
  mbfl_no_encoding* list;
  int list_size;
} php_mb_nls_ident_list;

static mbfl_no_encoding php_mb_default_identify_list_ja[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_jis,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_euc_jp,
  mbfl_no_encoding_sjis
};

static mbfl_no_encoding php_mb_default_identify_list_cn[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_euc_cn,
  mbfl_no_encoding_cp936
};

static mbfl_no_encoding php_mb_default_identify_list_tw_hk[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_euc_tw,
  mbfl_no_encoding_big5
};

static mbfl_no_encoding php_mb_default_identify_list_kr[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_euc_kr,
  mbfl_no_encoding_uhc
};

static mbfl_no_encoding php_mb_default_identify_list_ru[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_koi8r,
  mbfl_no_encoding_cp1251,
  mbfl_no_encoding_cp866
};

static mbfl_no_encoding php_mb_default_identify_list_hy[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_armscii8
};

static mbfl_no_encoding php_mb_default_identify_list_tr[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8,
  mbfl_no_encoding_8859_9
};

static mbfl_no_encoding php_mb_default_identify_list_neut[] = {
  mbfl_no_encoding_ascii,
  mbfl_no_encoding_utf8
};

static php_mb_nls_ident_list php_mb_default_identify_list[] = {
  { mbfl_no_language_japanese, php_mb_default_identify_list_ja,
    sizeof(php_mb_default_identify_list_ja) /
    sizeof(php_mb_default_identify_list_ja[0]) },
  { mbfl_no_language_korean, php_mb_default_identify_list_kr,
    sizeof(php_mb_default_identify_list_kr) /
    sizeof(php_mb_default_identify_list_kr[0]) },
  { mbfl_no_language_traditional_chinese, php_mb_default_identify_list_tw_hk,
    sizeof(php_mb_default_identify_list_tw_hk) /
    sizeof(php_mb_default_identify_list_tw_hk[0]) },
  { mbfl_no_language_simplified_chinese, php_mb_default_identify_list_cn,
    sizeof(php_mb_default_identify_list_cn) /
    sizeof(php_mb_default_identify_list_cn[0]) },
  { mbfl_no_language_russian, php_mb_default_identify_list_ru,
    sizeof(php_mb_default_identify_list_ru) /
    sizeof(php_mb_default_identify_list_ru[0]) },
  { mbfl_no_language_armenian, php_mb_default_identify_list_hy,
    sizeof(php_mb_default_identify_list_hy) /
    sizeof(php_mb_default_identify_list_hy[0]) },
  { mbfl_no_language_turkish, php_mb_default_identify_list_tr,
    sizeof(php_mb_default_identify_list_tr) /
    sizeof(php_mb_default_identify_list_tr[0]) },
  { mbfl_no_language_neutral, php_mb_default_identify_list_neut,
    sizeof(php_mb_default_identify_list_neut) /
    sizeof(php_mb_default_identify_list_neut[0]) }
};

///////////////////////////////////////////////////////////////////////////////
// globals
typedef std::map<std::string, php_mb_regex_t *> RegexCache;

struct MBGlobals final : RequestEventHandler {
  mbfl_no_language language;
  mbfl_no_language current_language;
  mbfl_no_encoding internal_encoding;
  mbfl_no_encoding current_internal_encoding;
  mbfl_no_encoding http_output_encoding;
  mbfl_no_encoding current_http_output_encoding;
  mbfl_no_encoding http_input_identify;
  mbfl_no_encoding http_input_identify_get;
  mbfl_no_encoding http_input_identify_post;
  mbfl_no_encoding http_input_identify_cookie;
  mbfl_no_encoding http_input_identify_string;
  mbfl_no_encoding *http_input_list;
  int http_input_list_size;
  mbfl_no_encoding *detect_order_list;
  int detect_order_list_size;
  mbfl_no_encoding *current_detect_order_list;
  int current_detect_order_list_size;
  mbfl_no_encoding *default_detect_order_list;
  int default_detect_order_list_size;
  int filter_illegal_mode;
  int filter_illegal_substchar;
  int current_filter_illegal_mode;
  int current_filter_illegal_substchar;
  bool encoding_translation;
  long strict_detection;
  long illegalchars;
  mbfl_buffer_converter *outconv;

  OnigEncoding default_mbctype;
  OnigEncoding current_mbctype;
  RegexCache ht_rc;
  std::string search_str;
  unsigned int search_pos;
  php_mb_regex_t *search_re;
  OnigRegion *search_regs;
  OnigOptionType regex_default_options;
  OnigSyntaxType *regex_default_syntax;

  MBGlobals() :
    language(mbfl_no_language_uni),
    current_language(mbfl_no_language_uni),
    internal_encoding(mbfl_no_encoding_utf8),
    current_internal_encoding(mbfl_no_encoding_utf8),
    http_output_encoding(mbfl_no_encoding_pass),
    current_http_output_encoding(mbfl_no_encoding_pass),
    http_input_identify(mbfl_no_encoding_invalid),
    http_input_identify_get(mbfl_no_encoding_invalid),
    http_input_identify_post(mbfl_no_encoding_invalid),
    http_input_identify_cookie(mbfl_no_encoding_invalid),
    http_input_identify_string(mbfl_no_encoding_invalid),
    http_input_list(NULL),
    http_input_list_size(0),
    detect_order_list(NULL),
    detect_order_list_size(0),
    current_detect_order_list(NULL),
    current_detect_order_list_size(0),
    default_detect_order_list
    ((mbfl_no_encoding *)php_mb_default_identify_list_neut),
    default_detect_order_list_size
    (sizeof(php_mb_default_identify_list_neut) /
     sizeof(php_mb_default_identify_list_neut[0])),
    filter_illegal_mode(MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR),
    filter_illegal_substchar(0x3f), /* '?' */
    current_filter_illegal_mode(MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR),
    current_filter_illegal_substchar(0x3f), /* '?' */
    encoding_translation(0),
    strict_detection(0),
    illegalchars(0),
    outconv(NULL),
    default_mbctype(ONIG_ENCODING_EUC_JP),
    current_mbctype(ONIG_ENCODING_EUC_JP),
    search_pos(0),
    search_re((php_mb_regex_t*)NULL),
    search_regs((OnigRegion*)NULL),
    regex_default_options(ONIG_OPTION_MULTILINE | ONIG_OPTION_SINGLELINE),
    regex_default_syntax(ONIG_SYNTAX_RUBY) {
  }

  void requestInit() override {
    current_language = language;
    current_internal_encoding = internal_encoding;
    current_http_output_encoding = http_output_encoding;
    current_filter_illegal_mode = filter_illegal_mode;
    current_filter_illegal_substchar = filter_illegal_substchar;
    if (!encoding_translation) {
      illegalchars = 0;
    }

    mbfl_no_encoding *list=NULL, *entry;
    int n = 0;
    if (detect_order_list) {
      list = detect_order_list;
      n = detect_order_list_size;
    }
    if (n <= 0) {
      list = default_detect_order_list;
      n = default_detect_order_list_size;
    }
    entry = (mbfl_no_encoding *)malloc(n * sizeof(int));
    current_detect_order_list = entry;
    current_detect_order_list_size = n;
    while (n > 0) {
      *entry++ = *list++;
      n--;
    }
  }

  void requestShutdown() override {
    if (current_detect_order_list != NULL) {
      free(current_detect_order_list);
      current_detect_order_list = NULL;
      current_detect_order_list_size = 0;
    }
    if (outconv != NULL) {
      illegalchars += mbfl_buffer_illegalchars(outconv);
      mbfl_buffer_converter_delete(outconv);
      outconv = NULL;
    }

    /* clear http input identification. */
    http_input_identify = mbfl_no_encoding_invalid;
    http_input_identify_post = mbfl_no_encoding_invalid;
    http_input_identify_get = mbfl_no_encoding_invalid;
    http_input_identify_cookie = mbfl_no_encoding_invalid;
    http_input_identify_string = mbfl_no_encoding_invalid;

    current_mbctype = default_mbctype;

    search_str.clear();
    search_pos = 0;

    if (search_regs != NULL) {
      onig_region_free(search_regs, 1);
      search_regs = (OnigRegion *)NULL;
    }
    for (RegexCache::const_iterator it = ht_rc.begin(); it != ht_rc.end();
         ++it) {
      onig_free(it->second);
    }
    ht_rc.clear();
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(MBGlobals, s_mb_globals);
#define MBSTRG(name) s_mb_globals->name

///////////////////////////////////////////////////////////////////////////////
// unicode functions

/*
 * A simple array of 32-bit masks for lookup.
 */
static unsigned long masks32[32] = {
  0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020,
  0x00000040, 0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800,
  0x00001000, 0x00002000, 0x00004000, 0x00008000, 0x00010000, 0x00020000,
  0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000,
  0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000,
  0x40000000, 0x80000000
};

static int prop_lookup(unsigned long code, unsigned long n) {
  long l, r, m;

  /*
   * There is an extra node on the end of the offsets to allow this routine
   * to work right.  If the index is 0xffff, then there are no nodes for the
   * property.
   */
  if ((l = _ucprop_offsets[n]) == 0xffff)
    return 0;

  /*
   * Locate the next offset that is not 0xffff.  The sentinel at the end of
   * the array is the max index value.
   */
  for (m = 1; n + m < _ucprop_size && _ucprop_offsets[n + m] == 0xffff; m++)
    ;

  r = _ucprop_offsets[n + m] - 1;

  while (l <= r) {
    /*
     * Determine a "mid" point and adjust to make sure the mid point is at
     * the beginning of a range pair.
     */
    m = (l + r) >> 1;
    m -= (m & 1);
    if (code > _ucprop_ranges[m + 1])
      l = m + 2;
    else if (code < _ucprop_ranges[m])
      r = m - 2;
    else if (code >= _ucprop_ranges[m] && code <= _ucprop_ranges[m + 1])
      return 1;
  }
  return 0;

}

static int php_unicode_is_prop(unsigned long code, unsigned long mask1,
                               unsigned long mask2) {
  unsigned long i;

  if (mask1 == 0 && mask2 == 0)
    return 0;

  for (i = 0; mask1 && i < 32; i++) {
    if ((mask1 & masks32[i]) && prop_lookup(code, i))
      return 1;
  }

  for (i = 32; mask2 && i < _ucprop_size; i++) {
    if ((mask2 & masks32[i & 31]) && prop_lookup(code, i))
      return 1;
  }

  return 0;
}

static unsigned long case_lookup(unsigned long code, long l, long r,
                                 int field) {
  long m;

  /*
   * Do the binary search.
   */
  while (l <= r) {
    /*
     * Determine a "mid" point and adjust to make sure the mid point is at
     * the beginning of a case mapping triple.
     */
    m = (l + r) >> 1;
    m -= (m % 3);
    if (code > _uccase_map[m])
      l = m + 3;
    else if (code < _uccase_map[m])
      r = m - 3;
    else if (code == _uccase_map[m])
      return _uccase_map[m + field];
  }

  return code;
}

static unsigned long php_turkish_toupper(unsigned long code, long l, long r,
                                         int field) {
  if (code == 0x0069L) {
    return 0x0130L;
  }
  return case_lookup(code, l, r, field);
}

static unsigned long php_turkish_tolower(unsigned long code, long l, long r,
                                         int field) {
  if (code == 0x0049L) {
    return 0x0131L;
  }
  return case_lookup(code, l, r, field);
}

static unsigned long php_unicode_toupper(unsigned long code,
                                         enum mbfl_no_encoding enc) {
  int field;
  long l, r;

  if (php_unicode_is_upper(code))
    return code;

  if (php_unicode_is_lower(code)) {
    /*
     * The character is lower case.
     */
    field = 2;
    l = _uccase_len[0];
    r = (l + _uccase_len[1]) - 3;

    if (enc == mbfl_no_encoding_8859_9) {
      return php_turkish_toupper(code, l, r, field);
    }

  } else {
    /*
     * The character is title case.
     */
    field = 1;
    l = _uccase_len[0] + _uccase_len[1];
    r = _uccase_size - 3;
  }
  return case_lookup(code, l, r, field);
}

static unsigned long php_unicode_tolower(unsigned long code,
                                         enum mbfl_no_encoding enc) {
  int field;
  long l, r;

  if (php_unicode_is_lower(code))
    return code;

  if (php_unicode_is_upper(code)) {
    /*
     * The character is upper case.
     */
    field = 1;
    l = 0;
    r = _uccase_len[0] - 3;

    if (enc == mbfl_no_encoding_8859_9) {
      return php_turkish_tolower(code, l, r, field);
    }

  } else {
    /*
     * The character is title case.
     */
    field = 2;
    l = _uccase_len[0] + _uccase_len[1];
    r = _uccase_size - 3;
  }
  return case_lookup(code, l, r, field);
}

static unsigned long php_unicode_totitle(unsigned long code,
                                         enum mbfl_no_encoding enc) {
  int field;
  long l, r;

  if (php_unicode_is_title(code))
    return code;

  /*
   * The offset will always be the same for converting to title case.
   */
  field = 2;

  if (php_unicode_is_upper(code)) {
    /*
     * The character is upper case.
     */
    l = 0;
    r = _uccase_len[0] - 3;
  } else {
    /*
     * The character is lower case.
     */
    l = _uccase_len[0];
    r = (l + _uccase_len[1]) - 3;
  }
  return case_lookup(code, l, r, field);

}

#define BE_ARY_TO_UINT32(ptr) (\
  ((unsigned char*)(ptr))[0]<<24 |\
  ((unsigned char*)(ptr))[1]<<16 |\
  ((unsigned char*)(ptr))[2]<< 8 |\
  ((unsigned char*)(ptr))[3] )

#define UINT32_TO_BE_ARY(ptr,val) { \
  unsigned int v = val; \
  ((unsigned char*)(ptr))[0] = (v>>24) & 0xff,\
  ((unsigned char*)(ptr))[1] = (v>>16) & 0xff,\
  ((unsigned char*)(ptr))[2] = (v>> 8) & 0xff,\
  ((unsigned char*)(ptr))[3] = (v    ) & 0xff;\
}

/**
 *  Return 0 if input contains any illegal encoding, otherwise 1.
 *  Even if any illegal encoding is detected the result may contain a list
 *  of parsed encodings.
 */
static int php_mb_parse_encoding_list(const char *value, int value_length,
                                      mbfl_no_encoding **return_list,
                                      int *return_size, int persistent) {
  int n, l, size, bauto, ret = 1;
  char *p, *p1, *p2, *endp, *tmpstr;
  mbfl_no_encoding no_encoding;
  mbfl_no_encoding *src, *entry, *list;

  list = NULL;
  if (value == NULL || value_length <= 0) {
    if (return_list) {
      *return_list = NULL;
    }
    if (return_size) {
      *return_size = 0;
    }
    return 0;
  } else {
    mbfl_no_encoding *identify_list;
    int identify_list_size;

    identify_list = MBSTRG(default_detect_order_list);
    identify_list_size = MBSTRG(default_detect_order_list_size);

    /* copy the value string for work */
    if (value[0]=='"' && value[value_length-1]=='"' && value_length>2) {
      tmpstr = (char *)strndup(value+1, value_length-2);
      value_length -= 2;
    }
    else
      tmpstr = (char *)strndup(value, value_length);
    if (tmpstr == NULL) {
      return 0;
    }
    /* count the number of listed encoding names */
    endp = tmpstr + value_length;
    n = 1;
    p1 = tmpstr;
    while ((p2 = (char*)string_memnstr(p1, ",", 1, endp)) != NULL) {
      p1 = p2 + 1;
      n++;
    }
    size = n + identify_list_size;
    /* make list */
    list = (mbfl_no_encoding *)calloc(size, sizeof(int));
    if (list != NULL) {
      entry = list;
      n = 0;
      bauto = 0;
      p1 = tmpstr;
      do {
        p2 = p = (char*)string_memnstr(p1, ",", 1, endp);
        if (p == NULL) {
          p = endp;
        }
        *p = '\0';
        /* trim spaces */
        while (p1 < p && (*p1 == ' ' || *p1 == '\t')) {
          p1++;
        }
        p--;
        while (p > p1 && (*p == ' ' || *p == '\t')) {
          *p = '\0';
          p--;
        }
        /* convert to the encoding number and check encoding */
        if (strcasecmp(p1, "auto") == 0) {
          if (!bauto) {
            bauto = 1;
            l = identify_list_size;
            src = identify_list;
            while (l > 0) {
              *entry++ = *src++;
              l--;
              n++;
            }
          }
        } else {
          no_encoding = mbfl_name2no_encoding(p1);
          if (no_encoding != mbfl_no_encoding_invalid) {
            *entry++ = no_encoding;
            n++;
          } else {
            ret = 0;
          }
        }
        p1 = p2 + 1;
      } while (n < size && p2 != NULL);
      if (n > 0) {
        if (return_list) {
          *return_list = list;
        } else {
          free(list);
        }
      } else {
        free(list);
        if (return_list) {
          *return_list = NULL;
        }
        ret = 0;
      }
      if (return_size) {
        *return_size = n;
      }
    } else {
      if (return_list) {
        *return_list = NULL;
      }
      if (return_size) {
        *return_size = 0;
      }
      ret = 0;
    }
    free(tmpstr);
  }

  return ret;
}

static char *php_mb_convert_encoding(const char *input, size_t length,
                                     const char *_to_encoding,
                                     const char *_from_encodings,
                                     unsigned int *output_len) {
  mbfl_string string, result, *ret;
  mbfl_no_encoding from_encoding, to_encoding;
  mbfl_buffer_converter *convd;
  int size;
  mbfl_no_encoding *list;
  char *output = NULL;

  if (output_len) {
    *output_len = 0;
  }
  if (!input) {
    return NULL;
  }
  /* new encoding */
  if (_to_encoding && strlen(_to_encoding)) {
    to_encoding = mbfl_name2no_encoding(_to_encoding);
    if (to_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", _to_encoding);
      return NULL;
    }
  } else {
    to_encoding = MBSTRG(current_internal_encoding);
  }

  /* initialize string */
  mbfl_string_init(&string);
  mbfl_string_init(&result);
  from_encoding = MBSTRG(current_internal_encoding);
  string.no_encoding = from_encoding;
  string.no_language = MBSTRG(current_language);
  string.val = (unsigned char *)input;
  string.len = length;

  /* pre-conversion encoding */
  if (_from_encodings) {
    list = NULL;
    size = 0;
    php_mb_parse_encoding_list(_from_encodings, strlen(_from_encodings),
                               &list, &size, 0);
    if (size == 1) {
      from_encoding = *list;
      string.no_encoding = from_encoding;
    } else if (size > 1) {
      /* auto detect */
      from_encoding = mbfl_identify_encoding_no(&string, list, size,
                                                MBSTRG(strict_detection));
      if (from_encoding != mbfl_no_encoding_invalid) {
        string.no_encoding = from_encoding;
      } else {
        raise_warning("Unable to detect character encoding");
        from_encoding = mbfl_no_encoding_pass;
        to_encoding = from_encoding;
        string.no_encoding = from_encoding;
      }
    } else {
      raise_warning("Illegal character encoding specified");
    }
    if (list != NULL) {
      free((void *)list);
    }
  }

  /* initialize converter */
  convd = mbfl_buffer_converter_new(from_encoding, to_encoding, string.len);
  if (convd == NULL) {
    raise_warning("Unable to create character encoding converter");
    return NULL;
  }
  mbfl_buffer_converter_illegal_mode
    (convd, MBSTRG(current_filter_illegal_mode));
  mbfl_buffer_converter_illegal_substchar
    (convd, MBSTRG(current_filter_illegal_substchar));

  /* do it */
  ret = mbfl_buffer_converter_feed_result(convd, &string, &result);
  if (ret) {
    if (output_len) {
      *output_len = ret->len;
    }
    output = (char *)ret->val;
  }

  MBSTRG(illegalchars) += mbfl_buffer_illegalchars(convd);
  mbfl_buffer_converter_delete(convd);
  return output;
}

static char *php_unicode_convert_case(int case_mode, const char *srcstr,
                                      size_t srclen, unsigned int *ret_len,
                                      const char *src_encoding) {
  char *unicode, *newstr;
  unsigned int unicode_len;
  unsigned char *unicode_ptr;
  size_t i;
  enum mbfl_no_encoding _src_encoding = mbfl_name2no_encoding(src_encoding);

  unicode = php_mb_convert_encoding(srcstr, srclen, "UCS-4BE", src_encoding,
                                    &unicode_len);
  if (unicode == NULL)
    return NULL;

  unicode_ptr = (unsigned char *)unicode;

  switch(case_mode) {
  case PHP_UNICODE_CASE_UPPER:
    for (i = 0; i < unicode_len; i+=4) {
      UINT32_TO_BE_ARY(&unicode_ptr[i],
                       php_unicode_toupper(BE_ARY_TO_UINT32(&unicode_ptr[i]),
                                           _src_encoding));
    }
    break;

  case PHP_UNICODE_CASE_LOWER:
    for (i = 0; i < unicode_len; i+=4) {
      UINT32_TO_BE_ARY(&unicode_ptr[i],
                       php_unicode_tolower(BE_ARY_TO_UINT32(&unicode_ptr[i]),
                                           _src_encoding));
    }
    break;

  case PHP_UNICODE_CASE_TITLE:
    {
      int mode = 0;

      for (i = 0; i < unicode_len; i+=4) {
        int res = php_unicode_is_prop
          (BE_ARY_TO_UINT32(&unicode_ptr[i]),
           UC_MN|UC_ME|UC_CF|UC_LM|UC_SK|UC_LU|UC_LL|UC_LT, 0);
        if (mode) {
          if (res) {
            UINT32_TO_BE_ARY
              (&unicode_ptr[i],
               php_unicode_tolower(BE_ARY_TO_UINT32(&unicode_ptr[i]),
                                   _src_encoding));
          } else {
            mode = 0;
          }
        } else {
          if (res) {
            mode = 1;
            UINT32_TO_BE_ARY
              (&unicode_ptr[i],
               php_unicode_totitle(BE_ARY_TO_UINT32(&unicode_ptr[i]),
                                   _src_encoding));
          }
        }
      }
    }
    break;
  }

  newstr = php_mb_convert_encoding(unicode, unicode_len, src_encoding,
                                   "UCS-4BE", ret_len);
  free(unicode);
  return newstr;
}

///////////////////////////////////////////////////////////////////////////////
// helpers

/**
 *  Return 0 if input contains any illegal encoding, otherwise 1.
 *  Even if any illegal encoding is detected the result may contain a list
 *  of parsed encodings.
 */
static int php_mb_parse_encoding_array(const Array& array,
                                       mbfl_no_encoding **return_list,
                                       int *return_size, int persistent) {
  int n, l, size, bauto,ret = 1;
  mbfl_no_encoding no_encoding;
  mbfl_no_encoding *src, *list, *entry;

  list = NULL;
  mbfl_no_encoding *identify_list = MBSTRG(default_detect_order_list);
  int identify_list_size = MBSTRG(default_detect_order_list_size);

  size = array.size() + identify_list_size;
  list = (mbfl_no_encoding *)calloc(size, sizeof(int));
  if (list != NULL) {
    entry = list;
    bauto = 0;
    n = 0;
    for (ArrayIter iter(array); iter; ++iter) {
      String hash_entry = iter.second();
      if (strcasecmp(hash_entry.data(), "auto") == 0) {
        if (!bauto) {
          bauto = 1;
          l = identify_list_size;
          src = identify_list;
          while (l > 0) {
            *entry++ = *src++;
            l--;
            n++;
          }
        }
      } else {
        no_encoding = mbfl_name2no_encoding(hash_entry.data());
        if (no_encoding != mbfl_no_encoding_invalid) {
          *entry++ = no_encoding;
          n++;
        } else {
          ret = 0;
        }
      }
    }
    if (n > 0) {
      if (return_list) {
        *return_list = list;
      } else {
        free(list);
      }
    } else {
      free(list);
      if (return_list) {
        *return_list = NULL;
      }
      ret = 0;
    }
    if (return_size) {
      *return_size = n;
    }
  } else {
    if (return_list) {
      *return_list = NULL;
    }
    if (return_size) {
      *return_size = 0;
    }
    ret = 0;
  }
  return ret;
}

static bool php_mb_parse_encoding(const Variant& encoding,
                                  mbfl_no_encoding **return_list,
                                  int *return_size, bool persistent) {
  bool ret;
  if (encoding.is(KindOfArray)) {
    ret = php_mb_parse_encoding_array(encoding.toArray(),
                                      return_list, return_size,
                                      persistent ? 1 : 0);
  } else {
    String enc = encoding.toString();
    ret = php_mb_parse_encoding_list(enc.data(), enc.size(),
                                     return_list, return_size,
                                     persistent ? 1 : 0);
  }
  if (!ret) {
    if (return_list && *return_list) {
      free(*return_list);
      *return_list = NULL;
    }
    return_size = 0;
  }
  return ret;
}

static int php_mb_nls_get_default_detect_order_list(mbfl_no_language lang,
                                                    mbfl_no_encoding **plist,
                                                    int* plist_size) {
  size_t i;
  *plist = (mbfl_no_encoding *) php_mb_default_identify_list_neut;
  *plist_size = sizeof(php_mb_default_identify_list_neut) /
    sizeof(php_mb_default_identify_list_neut[0]);

  for (i = 0; i < sizeof(php_mb_default_identify_list) /
         sizeof(php_mb_default_identify_list[0]); i++) {
    if (php_mb_default_identify_list[i].lang == lang) {
      *plist = php_mb_default_identify_list[i].list;
      *plist_size = php_mb_default_identify_list[i].list_size;
      return 1;
    }
  }
  return 0;
}

static size_t php_mb_mbchar_bytes_ex(const char *s, const mbfl_encoding *enc) {
  if (enc != NULL) {
    if (enc->flag & MBFL_ENCTYPE_MBCS) {
      if (enc->mblen_table != NULL) {
        if (s != NULL) return enc->mblen_table[*(unsigned char *)s];
      }
    } else if (enc->flag & (MBFL_ENCTYPE_WCS2BE | MBFL_ENCTYPE_WCS2LE)) {
      return 2;
    } else if (enc->flag & (MBFL_ENCTYPE_WCS4BE | MBFL_ENCTYPE_WCS4LE)) {
      return 4;
    }
  }
  return 1;
}

static int php_mb_stripos(int mode,
                          const char *old_haystack, int old_haystack_len,
                          const char *old_needle, int old_needle_len,
                          long offset, const char *from_encoding) {
  int n;
  mbfl_string haystack, needle;
  n = -1;

  mbfl_string_init(&haystack);
  mbfl_string_init(&needle);
  haystack.no_language = MBSTRG(current_language);
  haystack.no_encoding = MBSTRG(current_internal_encoding);
  needle.no_language = MBSTRG(current_language);
  needle.no_encoding = MBSTRG(current_internal_encoding);

  do {
    haystack.val = (unsigned char *)php_unicode_convert_case
      (PHP_UNICODE_CASE_UPPER, old_haystack, (size_t)old_haystack_len,
       &haystack.len, from_encoding);
    if (!haystack.val) {
      break;
    }
    if (haystack.len <= 0) {
      break;
    }

    needle.val = (unsigned char *)php_unicode_convert_case
      (PHP_UNICODE_CASE_UPPER, old_needle, (size_t)old_needle_len,
       &needle.len, from_encoding);
    if (!needle.val) {
      break;
    }
    if (needle.len <= 0) {
      break;
    }

    haystack.no_encoding = needle.no_encoding =
      mbfl_name2no_encoding(from_encoding);
    if (haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", from_encoding);
      break;
    }

    int haystack_char_len = mbfl_strlen(&haystack);
    if (mode) {
      if ((offset > 0 && offset > haystack_char_len) ||
          (offset < 0 && -offset > haystack_char_len)) {
        raise_warning("Offset is greater than the length of haystack string");
        break;
      }
    } else {
      if (offset < 0 || offset > haystack_char_len) {
        raise_warning("Offset not contained in string.");
        break;
      }
    }

    n = mbfl_strpos(&haystack, &needle, offset, mode);
  } while(0);

  if (haystack.val) {
    free(haystack.val);
  }
  if (needle.val) {
    free(needle.val);
  }
  return n;
}

///////////////////////////////////////////////////////////////////////////////

Array f_mb_list_encodings() {
  Array ret;
  int i = 0;
  const mbfl_encoding **encodings = mbfl_get_supported_encodings();
  const mbfl_encoding *encoding;
  while ((encoding = encodings[i++]) != NULL) {
    ret.append(String(encoding->name, CopyString));
  }
  return ret;
}

Variant f_mb_list_encodings_alias_names(const String& name /*= null_string*/) {
  const mbfl_encoding **encodings;
  const mbfl_encoding *encoding;
  mbfl_no_encoding no_encoding;
  int i, j;

  Array ret;
  if (name.isNull()) {
    i = 0;
    encodings = mbfl_get_supported_encodings();
    while ((encoding = encodings[i++]) != NULL) {
      Array row;
      if (encoding->aliases != NULL) {
        j = 0;
        while ((*encoding->aliases)[j] != NULL) {
          row.append(String((*encoding->aliases)[j], CopyString));
          j++;
        }
      }
      ret.set(String(encoding->name, CopyString), row);
    }
  } else {
    no_encoding = mbfl_name2no_encoding(name.data());
    if (no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", name.data());
      return false;
    }

    char *name = (char *)mbfl_no_encoding2name(no_encoding);
    if (name != NULL) {
      i = 0;
      encodings = mbfl_get_supported_encodings();
      while ((encoding = encodings[i++]) != NULL) {
        if (strcmp(encoding->name, name) != 0) continue;

        if (encoding->aliases != NULL) {
          j = 0;
          while ((*encoding->aliases)[j] != NULL) {
            ret.append(String((*encoding->aliases)[j], CopyString));
            j++;
          }
        }

        break;
      }
    } else {
      return false;
    }
  }
  return ret;
}

Variant f_mb_list_mime_names(const String& name /* = null_string */) {
  const mbfl_encoding **encodings;
  const mbfl_encoding *encoding;
  mbfl_no_encoding no_encoding;
  int i;

  Array ret;
  if (name.isNull()) {
    i = 0;
    encodings = mbfl_get_supported_encodings();
    while ((encoding = encodings[i++]) != NULL) {
      if (encoding->mime_name != NULL) {
        ret.set(String(encoding->name, CopyString),
                String(encoding->mime_name, CopyString));
      } else{
        ret.set(String(encoding->name, CopyString), "");
      }
    }
  } else {
    no_encoding = mbfl_name2no_encoding(name.data());
    if (no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", name.data());
      return false;
    }

    char *name = (char *)mbfl_no_encoding2name(no_encoding);
    if (name != NULL) {
      i = 0;
      encodings = mbfl_get_supported_encodings();
      while ((encoding = encodings[i++]) != NULL) {
        if (strcmp(encoding->name, name) != 0) continue;
        if (encoding->mime_name != NULL) {
          return String(encoding->mime_name, CopyString);
        }
        break;
      }
      return "";
    } else {
      return false;
    }
  }
  return ret;
}

bool f_mb_check_encoding(const String& var /* = null_string */,
                         const String& encoding /* = null_string */) {
  mbfl_buffer_converter *convd;
  mbfl_no_encoding no_encoding = MBSTRG(current_internal_encoding);
  mbfl_string string, result, *ret = NULL;
  long illegalchars = 0;

  if (var.isNull()) {
    return MBSTRG(illegalchars) == 0;
  }

  if (!encoding.isNull()) {
    no_encoding = mbfl_name2no_encoding(encoding.data());
    if (no_encoding == mbfl_no_encoding_invalid ||
        no_encoding == mbfl_no_encoding_pass) {
      raise_warning("Invalid encoding \"%s\"", encoding.data());
      return false;
    }
  }

  convd = mbfl_buffer_converter_new(no_encoding, no_encoding, 0);
  if (convd == NULL) {
    raise_warning("Unable to create converter");
    return false;
  }
  mbfl_buffer_converter_illegal_mode
    (convd, MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE);
  mbfl_buffer_converter_illegal_substchar
    (convd, 0);

  /* initialize string */
  mbfl_string_init_set(&string, mbfl_no_language_neutral, no_encoding);
  mbfl_string_init(&result);

  string.val = (unsigned char *)var.data();
  string.len = var.size();
  ret = mbfl_buffer_converter_feed_result(convd, &string, &result);
  illegalchars = mbfl_buffer_illegalchars(convd);
  mbfl_buffer_converter_delete(convd);

  if (ret != NULL) {
    MBSTRG(illegalchars) += illegalchars;
    if (illegalchars == 0 && string.len == ret->len &&
        memcmp((const char *)string.val, (const char *)ret->val,
                string.len) == 0) {
      mbfl_string_clear(&result);
      return true;
    } else {
      mbfl_string_clear(&result);
      return false;
    }
  } else {
    return false;
  }
}

Variant f_mb_convert_case(const String& str, int mode,
                         const String& encoding /* = null_string */) {
  const char *enc = NULL;
  if (encoding.empty()) {
    enc = mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  }

  unsigned int ret_len;
  char *newstr = php_unicode_convert_case(mode, str.data(), str.size(),
                                          &ret_len, enc);
  if (newstr) {
    return String(newstr, ret_len, AttachString);
  }
  return false;
}

Variant f_mb_convert_encoding(const String& str, const String& to_encoding,
                              const Variant& from_encoding /* = null_variant */) {
  String encoding = from_encoding.toString();
  if (from_encoding.is(KindOfArray)) {
    StringBuffer _from_encodings;
    Array encs = from_encoding.toArray();
    for (ArrayIter iter(encs); iter; ++iter) {
      if (!_from_encodings.empty()) {
        _from_encodings.append(",");
      }
      _from_encodings.append(iter.second().toString());
    }
    encoding = _from_encodings.detach();
  }

  unsigned int size;
  char *ret = php_mb_convert_encoding(str.data(), str.size(),
                                      to_encoding.data(),
                                      (!encoding.empty() ?
                                       encoding.data() : NULL),
                                      &size);
  if (ret != NULL) {
    return String(ret, size, AttachString);
  }
  return false;
}

Variant f_mb_convert_kana(const String& str,
                          const String& option /* = null_string */,
                          const String& encoding /* = null_string */) {
  mbfl_string string, result, *ret;
  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  int opt = 0x900;
  if (!option.empty()) {
    const char *p = option.data();
    int n = option.size();
    int i = 0;
    opt = 0;
    while (i < n) {
      i++;
      switch (*p++) {
      case 'A': opt |= 0x1;      break;
      case 'a': opt |= 0x10;     break;
      case 'R': opt |= 0x2;      break;
      case 'r': opt |= 0x20;     break;
      case 'N': opt |= 0x4;      break;
      case 'n': opt |= 0x40;     break;
      case 'S': opt |= 0x8;      break;
      case 's': opt |= 0x80;     break;
      case 'K': opt |= 0x100;    break;
      case 'k': opt |= 0x1000;   break;
      case 'H': opt |= 0x200;    break;
      case 'h': opt |= 0x2000;   break;
      case 'V': opt |= 0x800;    break;
      case 'C': opt |= 0x10000;  break;
      case 'c': opt |= 0x20000;  break;
      case 'M': opt |= 0x100000; break;
      case 'm': opt |= 0x200000; break;
      }
    }
  }

  /* encoding */
  if (!encoding.empty()) {
    string.no_encoding = mbfl_name2no_encoding(encoding.data());
    if (string.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  ret = mbfl_ja_jp_hantozen(&string, &result, opt);
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

static bool php_mbfl_encoding_detect(const Variant& var,
                                     mbfl_encoding_detector *identd,
                                     mbfl_string *string) {
  if (var.is(KindOfArray) || var.is(KindOfObject)) {
    Array items = var.toArray();
    for (ArrayIter iter(items); iter; ++iter) {
      if (php_mbfl_encoding_detect(iter.second(), identd, string)) {
        return true;
      }
    }
  } else if (var.isString()) {
    String svar = var.toString();
    string->val = (unsigned char *)svar.data();
    string->len = svar.size();
    if (mbfl_encoding_detector_feed(identd, string)) {
      return true;
    }
  }
  return false;
}

static Variant php_mbfl_convert(const Variant& var,
                                mbfl_buffer_converter *convd,
                                mbfl_string *string,
                                mbfl_string *result) {
  if (var.is(KindOfArray)) {
    Array ret;
    Array items = var.toArray();
    for (ArrayIter iter(items); iter; ++iter) {
      ret.set(iter.first(),
              php_mbfl_convert(iter.second(), convd, string, result));
    }
    return ret;
  }

  if (var.is(KindOfObject)) {
    Object obj = var.toObject();
    Array items = var.toArray();
    for (ArrayIter iter(items); iter; ++iter) {
      obj->o_set(iter.first().toString(),
                 php_mbfl_convert(iter.second().toString().data(), convd,
                                  string, result));
    }
    return var; // which still has obj
  }

  if (var.isString()) {
    String svar = var.toString();
    string->val = (unsigned char *)svar.data();
    string->len = svar.size();
    mbfl_string *ret =
      mbfl_buffer_converter_feed_result(convd, string, result);
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }

  return var;
}

Variant f_mb_convert_variables(int _argc, const String& to_encoding,
                               const Variant& from_encoding, VRefParam vars,
                               const Array& _argv /* = null_array */) {
  mbfl_string string, result;
  mbfl_no_encoding _from_encoding, _to_encoding;
  mbfl_encoding_detector *identd;
  mbfl_buffer_converter *convd;
  int elistsz;
  mbfl_no_encoding *elist;
  char *name;

  /* new encoding */
  _to_encoding = mbfl_name2no_encoding(to_encoding.data());
  if (_to_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", to_encoding.data());
    return false;
  }

  /* initialize string */
  mbfl_string_init(&string);
  mbfl_string_init(&result);
  _from_encoding = MBSTRG(current_internal_encoding);
  string.no_encoding = _from_encoding;
  string.no_language = MBSTRG(current_language);

  /* pre-conversion encoding */
  elist = NULL;
  elistsz = 0;
  php_mb_parse_encoding(from_encoding, &elist, &elistsz, false);
  if (elistsz <= 0) {
    _from_encoding = mbfl_no_encoding_pass;
  } else if (elistsz == 1) {
    _from_encoding = *elist;
  } else {
    /* auto detect */
    _from_encoding = mbfl_no_encoding_invalid;
    identd = mbfl_encoding_detector_new(elist, elistsz,
                                        MBSTRG(strict_detection));
    if (identd != NULL) {
      for (int n = -1; n < _argv.size(); n++) {
        if (php_mbfl_encoding_detect(n < 0 ? (Variant&)vars : _argv[n],
                                     identd, &string)) {
          break;
        }
      }
      _from_encoding = mbfl_encoding_detector_judge(identd);
      mbfl_encoding_detector_delete(identd);
    }

    if (_from_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unable to detect encoding");
      _from_encoding = mbfl_no_encoding_pass;
    }
  }
  if (elist != NULL) {
    free((void *)elist);
  }

  /* create converter */
  convd = NULL;
  if (_from_encoding != mbfl_no_encoding_pass) {
    convd = mbfl_buffer_converter_new(_from_encoding, _to_encoding, 0);
    if (convd == NULL) {
      raise_warning("Unable to create converter");
      return false;
    }
    mbfl_buffer_converter_illegal_mode
      (convd, MBSTRG(current_filter_illegal_mode));
    mbfl_buffer_converter_illegal_substchar
      (convd, MBSTRG(current_filter_illegal_substchar));
  }

  /* convert */
  if (convd != NULL) {
    vars = php_mbfl_convert(vars, convd, &string, &result);
    for (int n = 0; n < _argv.size(); n++) {
      const_cast<Array&>(_argv).set(n, php_mbfl_convert(_argv[n], convd,
                                                        &string, &result));
    }
    MBSTRG(illegalchars) += mbfl_buffer_illegalchars(convd);
    mbfl_buffer_converter_delete(convd);
  }

  name = (char *)mbfl_no_encoding2name(_from_encoding);
  if (name != NULL) {
    return String(name, CopyString);
  }
  return false;
}

Variant f_mb_decode_mimeheader(const String& str) {
  mbfl_string string, result, *ret;

  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  mbfl_string_init(&result);
  ret = mbfl_mime_header_decode(&string, &result,
                                MBSTRG(current_internal_encoding));
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

static Variant php_mb_numericentity_exec(const String& str, const Variant& convmap,
                                         const String& encoding, int type) {
  int mapsize=0;
  mbfl_string string, result, *ret;
  mbfl_no_encoding no_encoding;

  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  /* encoding */
  if (!encoding.empty()) {
    no_encoding = mbfl_name2no_encoding(encoding.data());
    if (no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    } else {
      string.no_encoding = no_encoding;
    }
  }

  /* conversion map */
  int *iconvmap = NULL;
  if (convmap.is(KindOfArray)) {
    Array convs = convmap.toArray();
    mapsize = convs.size();
    if (mapsize > 0) {
      iconvmap = (int*)malloc(mapsize * sizeof(int));
      int *mapelm = iconvmap;
      for (ArrayIter iter(convs); iter; ++iter) {
        *mapelm++ = iter.second().toInt32();
      }
    }
  }
  if (iconvmap == NULL) {
    return false;
  }
  mapsize /= 4;

  ret = mbfl_html_numeric_entity(&string, &result, iconvmap, mapsize, type);
  free(iconvmap);
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_decode_numericentity(const String& str, const Variant& convmap,
                                 const String& encoding /* = null_string */) {
  return php_mb_numericentity_exec(str, convmap, encoding, 1);
}

Variant f_mb_detect_encoding(const String& str,
                             const Variant& encoding_list /* = null_variant */,
                             const Variant& strict /* = null_variant */) {
  mbfl_string string;
  const char *ret;
  mbfl_no_encoding *elist;
  int size;
  mbfl_no_encoding *list = 0;

  /* make encoding list */
  list = NULL;
  size = 0;
  php_mb_parse_encoding(encoding_list, &list, &size, false);
  if (size > 0 && list != NULL) {
    elist = list;
  } else {
    elist = MBSTRG(current_detect_order_list);
    size = MBSTRG(current_detect_order_list_size);
  }

  long nstrict = 0;
  if (!strict.isNull()) {
    nstrict = strict.toInt64();
  } else {
    nstrict = MBSTRG(strict_detection);
  }

  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.val = (unsigned char *)str.data();
  string.len = str.size();
  ret = mbfl_identify_encoding_name(&string, elist, size, nstrict);
  if (list != NULL) {
    free(list);
  }
  if (ret != NULL) {
    return String(ret, CopyString);
  }
  return false;
}

Variant f_mb_detect_order(const Variant& encoding_list /* = null_variant */) {
  int n, size;
  mbfl_no_encoding *list, *entry;

  if (encoding_list.isNull()) {
    Array ret;
    entry = MBSTRG(current_detect_order_list);
    n = MBSTRG(current_detect_order_list_size);
    while (n > 0) {
      char *name = (char *)mbfl_no_encoding2name(*entry);
      if (name) {
        ret.append(String(name, CopyString));
      }
      entry++;
      n--;
    }
    return ret;
  }

  list = NULL;
  size = 0;
  if (!php_mb_parse_encoding(encoding_list, &list, &size, false) ||
      list == NULL) {
    return false;
  }
  if (MBSTRG(current_detect_order_list)) {
    free(MBSTRG(current_detect_order_list));
  }
  MBSTRG(current_detect_order_list) = list;
  MBSTRG(current_detect_order_list_size) = size;
  return true;
}

Variant f_mb_encode_mimeheader(const String& str,
                               const String& charset /* = null_string */,
                               const String& transfer_encoding/*= null_string*/,
                               const String& linefeed /* = "\r\n" */,
                               int indent /* = 0 */) {
  mbfl_no_encoding charsetenc, transenc;
  mbfl_string  string, result, *ret;

  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  charsetenc = mbfl_no_encoding_pass;
  transenc = mbfl_no_encoding_base64;

  if (!charset.empty()) {
    charsetenc = mbfl_name2no_encoding(charset.data());
    if (charsetenc == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", charset.data());
      return false;
    }
  } else {
    const mbfl_language *lang = mbfl_no2language(MBSTRG(current_language));
    if (lang != NULL) {
      charsetenc = lang->mail_charset;
      transenc = lang->mail_header_encoding;
    }
  }

  if (!transfer_encoding.empty()) {
    char ch = *transfer_encoding.data();
    if (ch == 'B' || ch == 'b') {
      transenc = mbfl_no_encoding_base64;
    } else if (ch == 'Q' || ch == 'q') {
      transenc = mbfl_no_encoding_qprint;
    }
  }

  mbfl_string_init(&result);
  ret = mbfl_mime_header_encode(&string, &result, charsetenc, transenc,
                                linefeed.data(), indent);
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_encode_numericentity(const String& str, const Variant& convmap,
                                 const String& encoding /* = null_string */) {
  return php_mb_numericentity_exec(str, convmap, encoding, 0);
}

const StaticString
  s_internal_encoding("internal_encoding"),
  s_http_input("http_input"),
  s_http_output("http_output"),
  s_mail_charset("mail_charset"),
  s_mail_header_encoding("mail_header_encoding"),
  s_mail_body_encoding("mail_body_encoding"),
  s_illegal_chars("illegal_chars"),
  s_encoding_translation("encoding_translation"),
  s_On("On"),
  s_Off("Off"),
  s_language("language"),
  s_detect_order("detect_order"),
  s_substitute_character("substitute_character"),
  s_strict_detection("strict_detection"),
  s_none("none"),
  s_long("long"),
  s_entity("entity");

Variant f_mb_get_info(const String& type /* = null_string */) {
  const mbfl_language *lang = mbfl_no2language(MBSTRG(current_language));
  mbfl_no_encoding *entry;
  int n;

  char *name;
  if (type.empty() || strcasecmp(type.data(), "all") == 0) {
    Array ret;
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(current_internal_encoding))) != NULL) {
      ret.set(s_internal_encoding, String(name, CopyString));
    }
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(http_input_identify))) != NULL) {
      ret.set(s_http_input, String(name, CopyString));
    }
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(current_http_output_encoding))) != NULL) {
      ret.set(s_http_output, String(name, CopyString));
    }
    if (lang != NULL) {
      if ((name = (char *)mbfl_no_encoding2name
           (lang->mail_charset)) != NULL) {
        ret.set(s_mail_charset, String(name, CopyString));
      }
      if ((name = (char *)mbfl_no_encoding2name
           (lang->mail_header_encoding)) != NULL) {
        ret.set(s_mail_header_encoding, String(name, CopyString));
      }
      if ((name = (char *)mbfl_no_encoding2name
           (lang->mail_body_encoding)) != NULL) {
        ret.set(s_mail_body_encoding, String(name, CopyString));
      }
    }
    ret.set(s_illegal_chars, MBSTRG(illegalchars));
    ret.set(s_encoding_translation,
            MBSTRG(encoding_translation) ? s_On : s_Off);
    if ((name = (char *)mbfl_no_language2name
         (MBSTRG(current_language))) != NULL) {
      ret.set(s_language, String(name, CopyString));
    }
    n = MBSTRG(current_detect_order_list_size);
    entry = MBSTRG(current_detect_order_list);
    if (n > 0) {
      Array row;
      while (n > 0) {
        if ((name = (char *)mbfl_no_encoding2name(*entry)) != NULL) {
          row.append(String(name, CopyString));
        }
        entry++;
        n--;
      }
      ret.set(s_detect_order, row);
    }
    switch (MBSTRG(current_filter_illegal_mode)) {
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE:
      ret.set(s_substitute_character, s_none);
      break;
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG:
      ret.set(s_substitute_character, s_long);
      break;
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY:
      ret.set(s_substitute_character, s_entity);
      break;
    default:
      ret.set(s_substitute_character,
              MBSTRG(current_filter_illegal_substchar));
    }
    ret.set(s_strict_detection, MBSTRG(strict_detection) ? s_On : s_Off);
    return ret;
  } else if (strcasecmp(type.data(), "internal_encoding") == 0) {
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(current_internal_encoding))) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "http_input") == 0) {
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(http_input_identify))) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "http_output") == 0) {
    if ((name = (char *)mbfl_no_encoding2name
         (MBSTRG(current_http_output_encoding))) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "mail_charset") == 0) {
    if (lang != NULL &&
        (name = (char *)mbfl_no_encoding2name
         (lang->mail_charset)) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "mail_header_encoding") == 0) {
    if (lang != NULL &&
        (name = (char *)mbfl_no_encoding2name
         (lang->mail_header_encoding)) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "mail_body_encoding") == 0) {
    if (lang != NULL &&
        (name = (char *)mbfl_no_encoding2name
         (lang->mail_body_encoding)) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "illegal_chars") == 0) {
    return MBSTRG(illegalchars);
  } else if (strcasecmp(type.data(), "encoding_translation") == 0) {
    return MBSTRG(encoding_translation) ? "On" : "Off";
  } else if (strcasecmp(type.data(), "language") == 0) {
    if ((name = (char *)mbfl_no_language2name
         (MBSTRG(current_language))) != NULL) {
      return String(name, CopyString);
    }
  } else if (strcasecmp(type.data(), "detect_order") == 0) {
    n = MBSTRG(current_detect_order_list_size);
    entry = MBSTRG(current_detect_order_list);
    if (n > 0) {
      Array ret;
      while (n > 0) {
        name = (char *)mbfl_no_encoding2name(*entry);
        if (name) {
          ret.append(String(name, CopyString));
        }
        entry++;
        n--;
      }
    }
  } else if (strcasecmp(type.data(), "substitute_character") == 0) {
    if (MBSTRG(current_filter_illegal_mode) ==
        MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
      return s_none;
    } else if (MBSTRG(current_filter_illegal_mode) ==
               MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG) {
      return s_long;
    } else if (MBSTRG(current_filter_illegal_mode) ==
               MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY) {
      return s_entity;
    } else {
      return MBSTRG(current_filter_illegal_substchar);
    }
  } else if (strcasecmp(type.data(), "strict_detection") == 0) {
    return MBSTRG(strict_detection) ? s_On : s_Off;
  }
  return false;
}

Variant f_mb_http_input(const String& type /* = null_string */) {
  int n;
  char *name;
  mbfl_no_encoding *entry;
  mbfl_no_encoding result = mbfl_no_encoding_invalid;

  if (type.empty()) {
    result = MBSTRG(http_input_identify);
  } else {
    switch (*type.data()) {
    case 'G': case 'g': result = MBSTRG(http_input_identify_get);    break;
    case 'P': case 'p': result = MBSTRG(http_input_identify_post);   break;
    case 'C': case 'c': result = MBSTRG(http_input_identify_cookie); break;
    case 'S': case 's': result = MBSTRG(http_input_identify_string); break;
    case 'I': case 'i':
      {
        Array ret;
        entry = MBSTRG(http_input_list);
        n = MBSTRG(http_input_list_size);
        while (n > 0) {
          name = (char *)mbfl_no_encoding2name(*entry);
          if (name) {
            ret.append(String(name, CopyString));
          }
          entry++;
          n--;
        }
        return ret;
      }
    case 'L': case 'l':
      {
        entry = MBSTRG(http_input_list);
        n = MBSTRG(http_input_list_size);
        StringBuffer list;
        while (n > 0) {
          name = (char *)mbfl_no_encoding2name(*entry);
          if (name) {
            if (list.empty()) {
              list.append(name);
            } else {
              list.append(',');
              list.append(name);
            }
          }
          entry++;
          n--;
        }
        if (list.empty()) {
          return false;
        }
        return list.detach();
      }
    default:
      result = MBSTRG(http_input_identify);
      break;
    }
  }

  if (result != mbfl_no_encoding_invalid &&
      (name = (char *)mbfl_no_encoding2name(result)) != NULL) {
    return String(name, CopyString);
  }
  return false;
}

Variant f_mb_http_output(const String& encoding /* = null_string */) {
  if (encoding.empty()) {
    char *name = (char *)mbfl_no_encoding2name
      (MBSTRG(current_http_output_encoding));
    if (name != NULL) {
      return String(name, CopyString);
    }
    return false;
  }

  mbfl_no_encoding no_encoding = mbfl_name2no_encoding(encoding.data());
  if (no_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", encoding.data());
    return false;
  }
  MBSTRG(current_http_output_encoding) = no_encoding;
  return true;
}

Variant f_mb_internal_encoding(const String& encoding /* = null_string */) {
  if (encoding.empty()) {
    char *name = (char *)mbfl_no_encoding2name
      (MBSTRG(current_internal_encoding));
    if (name != NULL) {
      return String(name, CopyString);
    }
    return false;
  }

  mbfl_no_encoding no_encoding = mbfl_name2no_encoding(encoding.data());
  if (no_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", encoding.data());
    return false;
  }

  MBSTRG(current_internal_encoding) = no_encoding;
  return true;
}

Variant f_mb_language(const String& language /* = null_string */) {
  if (language.empty()) {
    return String(mbfl_no_language2name(MBSTRG(current_language)), CopyString);
  }

  mbfl_no_language no_language = mbfl_name2no_language(language.data());
  if (no_language == mbfl_no_language_invalid) {
    raise_warning("Unknown language \"%s\"", language.data());
    return false;
  }

  php_mb_nls_get_default_detect_order_list
    (no_language, &MBSTRG(default_detect_order_list),
     &MBSTRG(default_detect_order_list_size));
  MBSTRG(current_language) = no_language;
  return true;
}

String f_mb_output_handler(const String& contents, int status) {
  mbfl_string string, result;
  int last_feed;

  mbfl_no_encoding encoding = MBSTRG(current_http_output_encoding);

  /* start phase only */
  if (status & PHP_OUTPUT_HANDLER_START) {
    /* delete the converter just in case. */
    if (MBSTRG(outconv)) {
      MBSTRG(illegalchars) += mbfl_buffer_illegalchars(MBSTRG(outconv));
      mbfl_buffer_converter_delete(MBSTRG(outconv));
      MBSTRG(outconv) = NULL;
    }
    if (encoding == mbfl_no_encoding_pass) {
      return contents;
    }

    /* analyze mime type */
    String mimetype = g_context->getMimeType();
    if (!mimetype.empty()) {
      const char *charset = mbfl_no2preferred_mime_name(encoding);
      if (charset) {
        g_context->setContentType(mimetype, charset);
      }
      /* activate the converter */
      MBSTRG(outconv) = mbfl_buffer_converter_new
        (MBSTRG(current_internal_encoding), encoding, 0);
    }
  }

  /* just return if the converter is not activated. */
  if (MBSTRG(outconv) == NULL) {
    return contents;
  }

  /* flag */
  last_feed = ((status & PHP_OUTPUT_HANDLER_END) != 0);
  /* mode */
  mbfl_buffer_converter_illegal_mode
    (MBSTRG(outconv), MBSTRG(current_filter_illegal_mode));
  mbfl_buffer_converter_illegal_substchar
    (MBSTRG(outconv), MBSTRG(current_filter_illegal_substchar));

  /* feed the string */
  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)contents.data();
  string.len = contents.size();
  mbfl_buffer_converter_feed(MBSTRG(outconv), &string);
  if (last_feed) {
    mbfl_buffer_converter_flush(MBSTRG(outconv));
  }
  /* get the converter output, and return it */
  mbfl_buffer_converter_result(MBSTRG(outconv), &result);

  /* delete the converter if it is the last feed. */
  if (last_feed) {
    MBSTRG(illegalchars) += mbfl_buffer_illegalchars(MBSTRG(outconv));
    mbfl_buffer_converter_delete(MBSTRG(outconv));
    MBSTRG(outconv) = NULL;
  }

  return String(reinterpret_cast<char*>(result.val), result.len, AttachString);
}

typedef struct _php_mb_encoding_handler_info_t {
  int data_type;
  const char *separator;
  unsigned int force_register_globals: 1;
  unsigned int report_errors: 1;
  enum mbfl_no_language to_language;
  enum mbfl_no_encoding to_encoding;
  enum mbfl_no_language from_language;
  int num_from_encodings;
  const enum mbfl_no_encoding *from_encodings;
} php_mb_encoding_handler_info_t;

static mbfl_no_encoding _php_mb_encoding_handler_ex
(const php_mb_encoding_handler_info_t *info, Array& arg, char *res) {
  char *var, *val;
  const char *s1, *s2;
  char *strtok_buf = NULL, **val_list = NULL;
  int n, num, *len_list = NULL;
  unsigned int val_len;
  mbfl_string string, resvar, resval;
  enum mbfl_no_encoding from_encoding = mbfl_no_encoding_invalid;
  mbfl_encoding_detector *identd = NULL;
  mbfl_buffer_converter *convd = NULL;

  mbfl_string_init_set(&string, info->to_language, info->to_encoding);
  mbfl_string_init_set(&resvar, info->to_language, info->to_encoding);
  mbfl_string_init_set(&resval, info->to_language, info->to_encoding);

  if (!res || *res == '\0') {
    goto out;
  }

  /* count the variables(separators) contained in the "res".
   * separator may contain multiple separator chars.
   */
  num = 1;
  for (s1=res; *s1 != '\0'; s1++) {
    for (s2=info->separator; *s2 != '\0'; s2++) {
      if (*s1 == *s2) {
        num++;
      }
    }
  }
  num *= 2; /* need space for variable name and value */

  val_list = (char **)calloc(num, sizeof(char *));
  len_list = (int *)calloc(num, sizeof(int));

  /* split and decode the query */
  n = 0;
  strtok_buf = NULL;
  var = strtok_r(res, info->separator, &strtok_buf);
  while (var)  {
    val = strchr(var, '=');
    if (val) { /* have a value */
      len_list[n] = url_decode_ex(var, val-var);
      val_list[n] = var;
      n++;

      *val++ = '\0';
      val_list[n] = val;
      len_list[n] = url_decode_ex(val, strlen(val));
    } else {
      len_list[n] = url_decode_ex(var, strlen(var));
      val_list[n] = var;
      n++;

      val_list[n] = const_cast<char*>("");
      len_list[n] = 0;
    }
    n++;
    var = strtok_r(NULL, info->separator, &strtok_buf);
  }
  num = n; /* make sure to process initilized vars only */

  /* initialize converter */
  if (info->num_from_encodings <= 0) {
    from_encoding = mbfl_no_encoding_pass;
  } else if (info->num_from_encodings == 1) {
    from_encoding = info->from_encodings[0];
  } else {
    /* auto detect */
    from_encoding = mbfl_no_encoding_invalid;
    identd = mbfl_encoding_detector_new
      ((enum mbfl_no_encoding *)info->from_encodings,
       info->num_from_encodings, MBSTRG(strict_detection));
    if (identd) {
      n = 0;
      while (n < num) {
        string.val = (unsigned char *)val_list[n];
        string.len = len_list[n];
        if (mbfl_encoding_detector_feed(identd, &string)) {
          break;
        }
        n++;
      }
      from_encoding = mbfl_encoding_detector_judge(identd);
      mbfl_encoding_detector_delete(identd);
    }
    if (from_encoding == mbfl_no_encoding_invalid) {
      if (info->report_errors) {
        raise_warning("Unable to detect encoding");
      }
      from_encoding = mbfl_no_encoding_pass;
    }
  }

  convd = NULL;
  if (from_encoding != mbfl_no_encoding_pass) {
    convd = mbfl_buffer_converter_new(from_encoding, info->to_encoding, 0);
    if (convd != NULL) {
      mbfl_buffer_converter_illegal_mode
        (convd, MBSTRG(current_filter_illegal_mode));
      mbfl_buffer_converter_illegal_substchar
        (convd, MBSTRG(current_filter_illegal_substchar));
    } else {
      if (info->report_errors) {
        raise_warning("Unable to create converter");
      }
      goto out;
    }
  }

  /* convert encoding */
  string.no_encoding = from_encoding;

  n = 0;
  while (n < num) {
    string.val = (unsigned char *)val_list[n];
    string.len = len_list[n];
    if (convd != NULL &&
        mbfl_buffer_converter_feed_result(convd, &string, &resvar) != NULL) {
      var = (char *)resvar.val;
    } else {
      var = val_list[n];
    }
    n++;
    string.val = (unsigned char *)val_list[n];
    string.len = len_list[n];
    if (convd != NULL &&
        mbfl_buffer_converter_feed_result(convd, &string, &resval) != NULL) {
      val = (char *)resval.val;
      val_len = resval.len;
    } else {
      val = val_list[n];
      val_len = len_list[n];
    }
    n++;

    arg.set(String(var, CopyString), String(val, val_len, CopyString));

    if (convd != NULL) {
      mbfl_string_clear(&resvar);
      mbfl_string_clear(&resval);
    }
  }

out:
  if (convd != NULL) {
    MBSTRG(illegalchars) += mbfl_buffer_illegalchars(convd);
    mbfl_buffer_converter_delete(convd);
  }
  if (val_list != NULL) {
    free((void *)val_list);
  }
  if (len_list != NULL) {
    free((void *)len_list);
  }

  return from_encoding;
}

bool f_mb_parse_str(const String& encoded_string,
                    VRefParam result /* = null */) {
  php_mb_encoding_handler_info_t info;
  info.data_type              = PARSE_STRING;
  info.separator              = ";&";
  info.force_register_globals = false;
  info.report_errors          = 1;
  info.to_encoding            = MBSTRG(current_internal_encoding);
  info.to_language            = MBSTRG(current_language);
  info.from_encodings         = MBSTRG(http_input_list);
  info.num_from_encodings     = MBSTRG(http_input_list_size);
  info.from_language          = MBSTRG(current_language);

  char *encstr = strndup(encoded_string.data(), encoded_string.size());
  Array resultArr = Array::Create();
  mbfl_no_encoding detected =
    _php_mb_encoding_handler_ex(&info, resultArr, encstr);
  free(encstr);
  result = resultArr;

  MBSTRG(http_input_identify) = detected;
  return detected != mbfl_no_encoding_invalid;
}

Variant f_mb_preferred_mime_name(const String& encoding) {
  mbfl_no_encoding no_encoding = mbfl_name2no_encoding(encoding.data());
  if (no_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", encoding.data());
    return false;
  }

  const char *preferred_name = mbfl_no2preferred_mime_name(no_encoding);
  if (preferred_name == NULL || *preferred_name == '\0') {
    raise_warning("No MIME preferred name corresponding to \"%s\"",
                    encoding.data());
    return false;
  }

  return String(preferred_name, CopyString);
}

static Variant php_mb_substr(const String& str, int from,
                             const Variant& vlen,
                             const String& encoding, bool substr) {
  mbfl_string string;
  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  if (!encoding.empty()) {
    string.no_encoding = mbfl_name2no_encoding(encoding.data());
    if (string.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  int size;
  if (substr) {
    size = mbfl_strlen(&string);
  } else {
    size = str.size();
  }
  int len = vlen.toInt64();
  if (vlen.isNull() || len == 0x7FFFFFFF) {
    len = size;
  }

  /* if "from" position is negative, count start position from the end
   * of the string
   */
  if (from < 0) {
    from = size + from;
    if (from < 0) {
      from = 0;
    }
  }

  /* if "length" position is negative, set it to the length
   * needed to stop that many chars from the end of the string
   */
  if (len < 0) {
    len = (size - from) + len;
    if (len < 0) {
      len = 0;
    }
  }

  if (from > size) {
    if (!substr) {
      return false;
    }
    from = size;
  }

  mbfl_string result;
  mbfl_string *ret;
  if (substr) {
    ret = mbfl_substr(&string, &result, from, len);
  } else {
    ret = mbfl_strcut(&string, &result, from, len);
  }
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_substr(const String& str, int start,
                    const Variant& length /*= uninit_null() */,
                    const String& encoding /* = null_string */) {
  return php_mb_substr(str, start, length, encoding, true);
}

Variant f_mb_strcut(const String& str, int start,
                    const Variant& length /*= uninit_null() */,
                    const String& encoding /* = null_string */) {
  return php_mb_substr(str, start, length, encoding, false);
}

Variant f_mb_strimwidth(const String& str, int start, int width,
                        const String& trimmarker /* = null_string */,
                        const String& encoding /* = null_string */) {
  mbfl_string string, result, marker, *ret;

  mbfl_string_init(&string);
  mbfl_string_init(&marker);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  marker.no_language = MBSTRG(current_language);
  marker.no_encoding = MBSTRG(current_internal_encoding);
  marker.val = NULL;
  marker.len = 0;

  if (!encoding.empty()) {
    string.no_encoding = marker.no_encoding =
      mbfl_name2no_encoding(encoding.data());
    if (string.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  string.val = (unsigned char *)str.data();
  string.len = str.size();

  if (start < 0 || start > str.size()) {
    raise_warning("Start position is out of reange");
    return false;
  }

  if (width < 0) {
    raise_warning("Width is negative value");
    return false;
  }

  marker.val = (unsigned char *)trimmarker.data();
  marker.len = trimmarker.size();

  ret = mbfl_strimwidth(&string, &marker, &result, start, width);
  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_stripos(const String& haystack, const String& needle,
                     int offset /* = 0 */,
                     const String& encoding /* = null_string */) {
  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }

  if (needle.empty()) {
    raise_warning("Empty delimiter");
    return false;
  }

  int n = php_mb_stripos(0, haystack.data(), haystack.size(),
                         needle.data(), needle.size(), offset, from_encoding);
  if (n >= 0) {
    return n;
  }
  return false;
}

Variant f_mb_strripos(const String& haystack, const String& needle,
                      int offset /* = 0 */,
                      const String& encoding /* = null_string */) {
  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }

  int n = php_mb_stripos(1, haystack.data(), haystack.size(),
                         needle.data(), needle.size(), offset, from_encoding);
  if (n >= 0) {
    return n;
  }
  return false;
}

Variant f_mb_stristr(const String& haystack, const String& needle,
                     bool part /* = false */,
                     const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();
  if (!mbs_needle.len) {
    raise_warning("Empty delimiter.");
    return false;
  }

  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }
  mbs_haystack.no_encoding = mbs_needle.no_encoding =
    mbfl_name2no_encoding(from_encoding);
  if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", from_encoding);
    return false;
  }

  int n = php_mb_stripos(0, (const char*)mbs_haystack.val, mbs_haystack.len,
                         (const char *)mbs_needle.val, mbs_needle.len,
                         0, from_encoding);
  if (n < 0) {
    return false;
  }

  int mblen = mbfl_strlen(&mbs_haystack);
  mbfl_string result, *ret = NULL;
  if (part) {
    ret = mbfl_substr(&mbs_haystack, &result, 0, n);
  } else {
    int len = (mblen - n);
    ret = mbfl_substr(&mbs_haystack, &result, n, len);
  }

  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_strlen(const String& str,
                    const String& encoding /* = null_string */) {
  mbfl_string string;
  mbfl_string_init(&string);
  string.val = (unsigned char *)str.data();
  string.len = str.size();
  string.no_language = MBSTRG(current_language);

  if (encoding.empty()) {
    string.no_encoding = MBSTRG(current_internal_encoding);
  } else {
    string.no_encoding = mbfl_name2no_encoding(encoding.data());
    if (string.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  int n = mbfl_strlen(&string);
  if (n >= 0) {
    return n;
  }
  return false;
}

Variant f_mb_strpos(const String& haystack, const String& needle,
                    int offset /* = 0 */,
                    const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  if (!encoding.empty()) {
    mbs_haystack.no_encoding = mbs_needle.no_encoding =
      mbfl_name2no_encoding(encoding.data());
    if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  if (offset < 0 || offset > mbfl_strlen(&mbs_haystack)) {
    raise_warning("Offset not contained in string.");
    return false;
  }
  if (mbs_needle.len == 0) {
    raise_warning("Empty delimiter.");
    return false;
  }

  int reverse = 0;
  int n = mbfl_strpos(&mbs_haystack, &mbs_needle, offset, reverse);
  if (n >= 0) {
    return n;
  }

  switch (-n) {
  case 1:
    break;
  case 2:
    raise_warning("Needle has not positive length.");
    break;
  case 4:
    raise_warning("Unknown encoding or conversion error.");
    break;
  case 8:
    raise_warning("Argument is empty.");
    break;
  default:
    raise_warning("Unknown error in mb_strpos.");
    break;
  }
  return false;
}

Variant f_mb_strrpos(const String& haystack, const String& needle,
                     const Variant& offset /* = 0LL */,
                     const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  const char *enc_name = encoding.data();
  long noffset = 0;
  String soffset = offset.toString();
  if (offset.isString()) {
    enc_name = soffset.data();

    int str_flg = 1;
    if (enc_name != NULL) {
      switch (*enc_name) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case ' ': case '-': case '.':
        break;
      default :
        str_flg = 0;
        break;
      }
    }
    if (str_flg) {
      noffset = offset.toInt32();
      enc_name = encoding.data();
    }
  } else {
    noffset = offset.toInt32();
  }

  if (!enc_name && !*enc_name) {
    mbs_haystack.no_encoding = mbs_needle.no_encoding =
      mbfl_name2no_encoding(enc_name);
    if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", enc_name);
      return false;
    }
  }

  if (mbs_haystack.len <= 0) {
    return false;
  }
  if (mbs_needle.len <= 0) {
    return false;
  }

  if ((noffset > 0 && noffset > mbfl_strlen(&mbs_haystack)) ||
      (noffset < 0 && -noffset > mbfl_strlen(&mbs_haystack))) {
    raise_notice("Offset is greater than the length of haystack string");
    return false;
  }

  int n = mbfl_strpos(&mbs_haystack, &mbs_needle, noffset, 1);
  if (n >= 0) {
    return n;
  }
  return false;
}

Variant f_mb_strrchr(const String& haystack, const String& needle,
                     bool part /* = false */,
                    const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  if (!encoding.empty()) {
    mbs_haystack.no_encoding = mbs_needle.no_encoding =
      mbfl_name2no_encoding(encoding.data());
    if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  if (mbs_haystack.len <= 0) {
    return false;
  }
  if (mbs_needle.len <= 0) {
    return false;
  }

  mbfl_string result, *ret = NULL;
  int n = mbfl_strpos(&mbs_haystack, &mbs_needle, 0, 1);
  if (n >= 0) {
    int mblen = mbfl_strlen(&mbs_haystack);
    if (part) {
      ret = mbfl_substr(&mbs_haystack, &result, 0, n);
    } else {
      int len = (mblen - n);
      ret = mbfl_substr(&mbs_haystack, &result, n, len);
    }
  }

  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_strrichr(const String& haystack, const String& needle,
                      bool part /* = false */,
                      const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }
  mbs_haystack.no_encoding = mbs_needle.no_encoding =
    mbfl_name2no_encoding(from_encoding);
  if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
    raise_warning("Unknown encoding \"%s\"", from_encoding);
    return false;
  }

  int n = php_mb_stripos(1, (const char*)mbs_haystack.val, mbs_haystack.len,
                         (const char*)mbs_needle.val, mbs_needle.len,
                         0, from_encoding);
  if (n < 0) {
    return false;
  }

  mbfl_string result, *ret = NULL;
  int mblen = mbfl_strlen(&mbs_haystack);
  if (part) {
    ret = mbfl_substr(&mbs_haystack, &result, 0, n);
  } else {
    int len = (mblen - n);
    ret = mbfl_substr(&mbs_haystack, &result, n, len);
  }

  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_strstr(const String& haystack, const String& needle,
                    bool part /* = false */,
                    const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  if (!encoding.empty()) {
    mbs_haystack.no_encoding = mbs_needle.no_encoding =
      mbfl_name2no_encoding(encoding.data());
    if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  if (mbs_needle.len <= 0) {
    raise_warning("Empty delimiter.");
    return false;
  }

  mbfl_string result, *ret = NULL;
  int n = mbfl_strpos(&mbs_haystack, &mbs_needle, 0, 0);
  if (n >= 0) {
    int mblen = mbfl_strlen(&mbs_haystack);
    if (part) {
      ret = mbfl_substr(&mbs_haystack, &result, 0, n);
    } else {
      int len = (mblen - n);
      ret = mbfl_substr(&mbs_haystack, &result, n, len);
    }
  }

  if (ret != NULL) {
    return String(reinterpret_cast<char*>(ret->val), ret->len, AttachString);
  }
  return false;
}

Variant f_mb_strtolower(const String& str,
                        const String& encoding /* = null_string */) {
  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }

  unsigned int ret_len;
  char *newstr = php_unicode_convert_case(PHP_UNICODE_CASE_LOWER,
                                          str.data(), str.size(),
                                          &ret_len, from_encoding);
  if (newstr) {
    return String(newstr, ret_len, AttachString);
  }
  return false;
}

Variant f_mb_strtoupper(const String& str,
                        const String& encoding /* = null_string */) {
  const char *from_encoding;
  if (encoding.empty()) {
    from_encoding =
      mbfl_no2preferred_mime_name(MBSTRG(current_internal_encoding));
  } else {
    from_encoding = encoding.data();
  }

  unsigned int ret_len;
  char *newstr = php_unicode_convert_case(PHP_UNICODE_CASE_UPPER,
                                          str.data(), str.size(),
                                          &ret_len, from_encoding);
  if (newstr) {
    return String(newstr, ret_len, AttachString);
  }
  return false;
}

Variant f_mb_strwidth(const String& str,
                      const String& encoding /* = null_string */) {
  mbfl_string string;
  mbfl_string_init(&string);
  string.no_language = MBSTRG(current_language);
  string.no_encoding = MBSTRG(current_internal_encoding);
  string.val = (unsigned char *)str.data();
  string.len = str.size();

  if (!encoding.empty()) {
    string.no_encoding = mbfl_name2no_encoding(encoding.data());
    if (string.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  int n = mbfl_strwidth(&string);
  if (n >= 0) {
    return n;
  }
  return false;
}

Variant f_mb_substitute_character(const Variant& substrchar /* = null_variant */) {
  if (substrchar.isNull()) {
    switch (MBSTRG(current_filter_illegal_mode)) {
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE:
      return "none";
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG:
      return "long";
    case MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY:
      return "entity";
    default:
      return MBSTRG(current_filter_illegal_substchar);
    }
  }

  if (substrchar.isString()) {
    String s = substrchar.toString();
    if (strcasecmp("none", s.data()) == 0) {
      MBSTRG(current_filter_illegal_mode) =
        MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE;
      return true;
    }
    if (strcasecmp("long", s.data()) == 0) {
      MBSTRG(current_filter_illegal_mode) =
        MBFL_OUTPUTFILTER_ILLEGAL_MODE_LONG;
      return true;
    }
    if (strcasecmp("entity", s.data()) == 0) {
      MBSTRG(current_filter_illegal_mode) =
        MBFL_OUTPUTFILTER_ILLEGAL_MODE_ENTITY;
      return true;
    }
  }

  int64_t n = substrchar.toInt64();
  if (n < 0xffff && n > 0) {
    MBSTRG(current_filter_illegal_mode) =
      MBFL_OUTPUTFILTER_ILLEGAL_MODE_CHAR;
    MBSTRG(current_filter_illegal_substchar) = n;
  } else {
    raise_warning("Unknown character.");
    return false;
  }
  return true;
}

Variant f_mb_substr_count(const String& haystack, const String& needle,
                      const String& encoding /* = null_string */) {
  mbfl_string mbs_haystack;
  mbfl_string_init(&mbs_haystack);
  mbs_haystack.no_language = MBSTRG(current_language);
  mbs_haystack.no_encoding = MBSTRG(current_internal_encoding);
  mbs_haystack.val = (unsigned char *)haystack.data();
  mbs_haystack.len = haystack.size();

  mbfl_string mbs_needle;
  mbfl_string_init(&mbs_needle);
  mbs_needle.no_language = MBSTRG(current_language);
  mbs_needle.no_encoding = MBSTRG(current_internal_encoding);
  mbs_needle.val = (unsigned char *)needle.data();
  mbs_needle.len = needle.size();

  if (!encoding.empty()) {
    mbs_haystack.no_encoding = mbs_needle.no_encoding =
      mbfl_name2no_encoding(encoding.data());
    if (mbs_haystack.no_encoding == mbfl_no_encoding_invalid) {
      raise_warning("Unknown encoding \"%s\"", encoding.data());
      return false;
    }
  }

  if (mbs_needle.len <= 0) {
    raise_warning("Empty substring.");
    return false;
  }

  int n = mbfl_substr_count(&mbs_haystack, &mbs_needle);
  if (n >= 0) {
    return n;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// regex helpers

typedef struct _php_mb_regex_enc_name_map_t {
  const char *names;
  OnigEncoding code;
} php_mb_regex_enc_name_map_t;

static php_mb_regex_enc_name_map_t enc_name_map[] ={
  {
    "EUC-JP\0EUCJP\0X-EUC-JP\0UJIS\0EUCJP\0EUCJP-WIN\0",
    ONIG_ENCODING_EUC_JP
  },
  {
    "UTF-8\0UTF8\0",
    ONIG_ENCODING_UTF8
  },
  {
    "UTF-16\0UTF-16BE\0",
    ONIG_ENCODING_UTF16_BE
  },
  {
   "UTF-16LE\0",
    ONIG_ENCODING_UTF16_LE
  },
  {
    "UCS-4\0UTF-32\0UTF-32BE\0",
    ONIG_ENCODING_UTF32_BE
  },
  {
    "UCS-4LE\0UTF-32LE\0",
    ONIG_ENCODING_UTF32_LE
  },
  {
    "SJIS\0CP932\0MS932\0SHIFT_JIS\0SJIS-WIN\0WINDOWS-31J\0",
    ONIG_ENCODING_SJIS
  },
  {
    "BIG5\0BIG-5\0BIGFIVE\0CN-BIG5\0BIG-FIVE\0",
    ONIG_ENCODING_BIG5
  },
  {
    "EUC-CN\0EUCCN\0EUC_CN\0GB-2312\0GB2312\0",
    ONIG_ENCODING_EUC_CN
  },
  {
    "EUC-TW\0EUCTW\0EUC_TW\0",
    ONIG_ENCODING_EUC_TW
  },
  {
    "EUC-KR\0EUCKR\0EUC_KR\0",
    ONIG_ENCODING_EUC_KR
  },
  {
    "KOI8R\0KOI8-R\0KOI-8R\0",
    ONIG_ENCODING_KOI8_R
  },
  {
    "ISO-8859-1\0ISO8859-1\0ISO_8859_1\0ISO8859_1\0",
    ONIG_ENCODING_ISO_8859_1
  },
  {
    "ISO-8859-2\0ISO8859-2\0ISO_8859_2\0ISO8859_2\0",
    ONIG_ENCODING_ISO_8859_2
  },
  {
    "ISO-8859-3\0ISO8859-3\0ISO_8859_3\0ISO8859_3\0",
    ONIG_ENCODING_ISO_8859_3
  },
  {
    "ISO-8859-4\0ISO8859-4\0ISO_8859_4\0ISO8859_4\0",
    ONIG_ENCODING_ISO_8859_4
  },
  {
    "ISO-8859-5\0ISO8859-5\0ISO_8859_5\0ISO8859_5\0",
    ONIG_ENCODING_ISO_8859_5
  },
  {
    "ISO-8859-6\0ISO8859-6\0ISO_8859_6\0ISO8859_6\0",
    ONIG_ENCODING_ISO_8859_6
  },
  {
    "ISO-8859-7\0ISO8859-7\0ISO_8859_7\0ISO8859_7\0",
    ONIG_ENCODING_ISO_8859_7
  },
  {
    "ISO-8859-8\0ISO8859-8\0ISO_8859_8\0ISO8859_8\0",
    ONIG_ENCODING_ISO_8859_8
  },
  {
    "ISO-8859-9\0ISO8859-9\0ISO_8859_9\0ISO8859_9\0",
    ONIG_ENCODING_ISO_8859_9
  },
  {
    "ISO-8859-10\0ISO8859-10\0ISO_8859_10\0ISO8859_10\0",
    ONIG_ENCODING_ISO_8859_10
  },
  {
    "ISO-8859-11\0ISO8859-11\0ISO_8859_11\0ISO8859_11\0",
    ONIG_ENCODING_ISO_8859_11
  },
  {
    "ISO-8859-13\0ISO8859-13\0ISO_8859_13\0ISO8859_13\0",
    ONIG_ENCODING_ISO_8859_13
  },
  {
    "ISO-8859-14\0ISO8859-14\0ISO_8859_14\0ISO8859_14\0",
    ONIG_ENCODING_ISO_8859_14
  },
  {
    "ISO-8859-15\0ISO8859-15\0ISO_8859_15\0ISO8859_15\0",
    ONIG_ENCODING_ISO_8859_15
  },
  {
    "ISO-8859-16\0ISO8859-16\0ISO_8859_16\0ISO8859_16\0",
    ONIG_ENCODING_ISO_8859_16
  },
  {
    "ASCII\0US-ASCII\0US_ASCII\0ISO646\0",
    ONIG_ENCODING_ASCII
  },
  { NULL, ONIG_ENCODING_UNDEF }
};

static OnigEncoding php_mb_regex_name2mbctype(const char *pname) {
  const char *p;
  php_mb_regex_enc_name_map_t *mapping;

  if (pname == NULL) {
    return ONIG_ENCODING_UNDEF;
  }

  for (mapping = enc_name_map; mapping->names != NULL; mapping++) {
    for (p = mapping->names; *p != '\0'; p += (strlen(p) + 1)) {
      if (strcasecmp(p, pname) == 0) {
        return mapping->code;
      }
    }
  }

  return ONIG_ENCODING_UNDEF;
}

static const char *php_mb_regex_mbctype2name(OnigEncoding mbctype) {
  php_mb_regex_enc_name_map_t *mapping;

  for (mapping = enc_name_map; mapping->names != NULL; mapping++) {
    if (mapping->code == mbctype) {
      return mapping->names;
    }
  }

  return NULL;
}

/*
 * regex cache
 */
static php_mb_regex_t *php_mbregex_compile_pattern(const String& pattern,
                                                   OnigOptionType options,
                                                   OnigEncoding enc,
                                                   OnigSyntaxType *syntax) {
  int err_code = 0;
  OnigErrorInfo err_info;
  OnigUChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
  php_mb_regex_t *rc = NULL;

  std::string spattern = std::string(pattern.data(), pattern.size());
  RegexCache &cache = MBSTRG(ht_rc);
  RegexCache::const_iterator it =
    cache.find(spattern);
  if (it != cache.end()) {
    rc = it->second;
  }

  if (!rc || rc->options != options || rc->enc != enc ||
      rc->syntax != syntax) {
    if (rc) {
      onig_free(rc);
      rc = NULL;
    }
    if ((err_code = onig_new(&rc, (OnigUChar *)pattern.data(),
                             (OnigUChar *)(pattern.data() + pattern.size()),
                             options,enc, syntax, &err_info)) != ONIG_NORMAL) {
      onig_error_code_to_str(err_str, err_code, err_info);
      raise_warning("mbregex compile err: %s", err_str);
      return NULL;
    }
    MBSTRG(ht_rc)[spattern] = rc;
  }
  return rc;
}

static size_t _php_mb_regex_get_option_string(char *str, size_t len,
                                              OnigOptionType option,
                                              OnigSyntaxType *syntax) {
  size_t len_left = len;
  size_t len_req = 0;
  char *p = str;
  char c;

  if ((option & ONIG_OPTION_IGNORECASE) != 0) {
    if (len_left > 0) {
      --len_left;
      *(p++) = 'i';
    }
    ++len_req;
  }

  if ((option & ONIG_OPTION_EXTEND) != 0) {
    if (len_left > 0) {
      --len_left;
      *(p++) = 'x';
    }
    ++len_req;
  }

  if ((option & (ONIG_OPTION_MULTILINE | ONIG_OPTION_SINGLELINE)) ==
      (ONIG_OPTION_MULTILINE | ONIG_OPTION_SINGLELINE)) {
    if (len_left > 0) {
      --len_left;
      *(p++) = 'p';
    }
    ++len_req;
  } else {
    if ((option & ONIG_OPTION_MULTILINE) != 0) {
      if (len_left > 0) {
        --len_left;
        *(p++) = 'm';
      }
      ++len_req;
    }

    if ((option & ONIG_OPTION_SINGLELINE) != 0) {
      if (len_left > 0) {
        --len_left;
        *(p++) = 's';
      }
      ++len_req;
    }
  }
  if ((option & ONIG_OPTION_FIND_LONGEST) != 0) {
    if (len_left > 0) {
      --len_left;
      *(p++) = 'l';
    }
    ++len_req;
  }
  if ((option & ONIG_OPTION_FIND_NOT_EMPTY) != 0) {
    if (len_left > 0) {
      --len_left;
      *(p++) = 'n';
    }
    ++len_req;
  }

  c = 0;

  if (syntax == ONIG_SYNTAX_JAVA) {
    c = 'j';
  } else if (syntax == ONIG_SYNTAX_GNU_REGEX) {
    c = 'u';
  } else if (syntax == ONIG_SYNTAX_GREP) {
    c = 'g';
  } else if (syntax == ONIG_SYNTAX_EMACS) {
    c = 'c';
  } else if (syntax == ONIG_SYNTAX_RUBY) {
    c = 'r';
  } else if (syntax == ONIG_SYNTAX_PERL) {
    c = 'z';
  } else if (syntax == ONIG_SYNTAX_POSIX_BASIC) {
    c = 'b';
  } else if (syntax == ONIG_SYNTAX_POSIX_EXTENDED) {
    c = 'd';
  }

  if (c != 0) {
    if (len_left > 0) {
      --len_left;
      *(p++) = c;
    }
    ++len_req;
  }

  if (len_left > 0) {
    --len_left;
    *(p++) = '\0';
  }
  ++len_req;
  if (len < len_req) {
    return len_req;
  }

  return 0;
}

static void _php_mb_regex_init_options(const char *parg, int narg,
                                       OnigOptionType *option,
                                       OnigSyntaxType **syntax, int *eval) {
  int n;
  char c;
  int optm = 0;

  *syntax = ONIG_SYNTAX_RUBY;
  if (parg != NULL) {
    n = 0;
    while (n < narg) {
      c = parg[n++];
      switch (c) {
      case 'i': optm |= ONIG_OPTION_IGNORECASE;                         break;
      case 'x': optm |= ONIG_OPTION_EXTEND;                             break;
      case 'm': optm |= ONIG_OPTION_MULTILINE;                          break;
      case 's': optm |= ONIG_OPTION_SINGLELINE;                         break;
      case 'p': optm |= ONIG_OPTION_MULTILINE | ONIG_OPTION_SINGLELINE; break;
      case 'l': optm |= ONIG_OPTION_FIND_LONGEST;                       break;
      case 'n': optm |= ONIG_OPTION_FIND_NOT_EMPTY;                     break;
      case 'j': *syntax = ONIG_SYNTAX_JAVA;                             break;
      case 'u': *syntax = ONIG_SYNTAX_GNU_REGEX;                        break;
      case 'g': *syntax = ONIG_SYNTAX_GREP;                             break;
      case 'c': *syntax = ONIG_SYNTAX_EMACS;                            break;
      case 'r': *syntax = ONIG_SYNTAX_RUBY;                             break;
      case 'z': *syntax = ONIG_SYNTAX_PERL;                             break;
      case 'b': *syntax = ONIG_SYNTAX_POSIX_BASIC;                      break;
      case 'd': *syntax = ONIG_SYNTAX_POSIX_EXTENDED;                   break;
      case 'e':
        if (eval != NULL) *eval = 1;
        break;
      default:
        break;
      }
    }
    if (option != NULL) *option|=optm;
  }
}

///////////////////////////////////////////////////////////////////////////////
// regex functions

bool f_mb_ereg_match(const String& pattern, const String& str,
                     const String& option /* = null_string */) {
  OnigSyntaxType *syntax;
  OnigOptionType noption = 0;
  if (!option.empty()) {
    _php_mb_regex_init_options(option.data(), option.size(), &noption,
                               &syntax, NULL);
  } else {
    noption |= MBSTRG(regex_default_options);
    syntax = MBSTRG(regex_default_syntax);
  }

  php_mb_regex_t *re;
  if ((re = php_mbregex_compile_pattern
       (pattern, noption, MBSTRG(current_mbctype), syntax)) == NULL) {
    return false;
  }

  /* match */
  int err = onig_match(re, (OnigUChar *)str.data(),
                       (OnigUChar *)(str.data() + str.size()),
                       (OnigUChar *)str.data(), NULL, 0);
  return err >= 0;
}

static Variant _php_mb_regex_ereg_replace_exec(const Variant& pattern,
                                               const String& replacement,
                                               const String& str,
                                               const String& option,
                                               OnigOptionType options) {
  const char *p;
  php_mb_regex_t *re;
  OnigSyntaxType *syntax;
  OnigRegion *regs = NULL;
  StringBuffer out_buf;
  int i, err, eval, n;
  OnigUChar *pos;
  OnigUChar *string_lim;
  char pat_buf[2];

  const mbfl_encoding *enc;

  {
    const char *current_enc_name;
    current_enc_name = php_mb_regex_mbctype2name(MBSTRG(current_mbctype));
    if (current_enc_name == NULL ||
      (enc = mbfl_name2encoding(current_enc_name)) == NULL) {
      raise_warning("Unknown error");
      return false;
    }
  }
  eval = 0;
  {
    if (!option.empty()) {
      _php_mb_regex_init_options(option.data(), option.size(),
                                 &options, &syntax, &eval);
    } else {
      options |= MBSTRG(regex_default_options);
      syntax = MBSTRG(regex_default_syntax);
    }
  }

  String spattern;
  if (pattern.isString()) {
    spattern = pattern.toString();
  } else {
    /* FIXME: this code is not multibyte aware! */
    pat_buf[0] = pattern.toByte();
    pat_buf[1] = '\0';
    spattern = String(pat_buf, 1, CopyString);
  }
  /* create regex pattern buffer */
  re = php_mbregex_compile_pattern(spattern, options,
                                   MBSTRG(current_mbctype), syntax);
  if (re == NULL) {
    return false;
  }

  if (eval) {
    throw NotSupportedException("ereg_replace", "dynamic coding");
  }

  /* do the actual work */
  err = 0;
  pos = (OnigUChar*)str.data();
  string_lim = (OnigUChar*)(str.data() + str.size());
  regs = onig_region_new();
  while (err >= 0) {
    err = onig_search(re, (OnigUChar *)str.data(), (OnigUChar *)string_lim,
                      pos, (OnigUChar *)string_lim, regs, 0);
    if (err <= -2) {
      OnigUChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
      onig_error_code_to_str(err_str, err);
      raise_warning("mbregex search failure: %s", err_str);
      break;
    }
    if (err >= 0) {
#if moriyoshi_0
      if (regs->beg[0] == regs->end[0]) {
        raise_warning("Empty regular expression");
        break;
      }
#endif
      /* copy the part of the string before the match */
      out_buf.append((const char *)pos,
                     (OnigUChar *)(str.data() + regs->beg[0]) - pos);
      /* copy replacement and backrefs */
      i = 0;
      p = replacement.data();
      while (i < replacement.size()) {
        int fwd = (int)php_mb_mbchar_bytes_ex(p, enc);
        n = -1;
        if ((replacement.size() - i) >= 2 && fwd == 1 &&
          p[0] == '\\' && p[1] >= '0' && p[1] <= '9') {
          n = p[1] - '0';
        }
        if (n >= 0 && n < regs->num_regs) {
          if (regs->beg[n] >= 0 && regs->beg[n] < regs->end[n] &&
              regs->end[n] <= str.size()) {
            out_buf.append(str.data() + regs->beg[n],
                           regs->end[n] - regs->beg[n]);
          }
          p += 2;
          i += 2;
        } else {
          out_buf.append(p, fwd);
          p += fwd;
          i += fwd;
        }
      }
      n = regs->end[0];
      if ((pos - (OnigUChar *)str.data()) < n) {
        pos = (OnigUChar *)(str.data() + n);
      } else {
        if (pos < string_lim) {
          out_buf.append((const char *)pos, 1);
        }
        pos++;
      }
    } else { /* nomatch */
      /* stick that last bit of string on our output */
      if (string_lim - pos > 0) {
        out_buf.append((const char *)pos, string_lim - pos);
      }
    }
    onig_region_free(regs, 0);
  }

  if (regs != NULL) {
    onig_region_free(regs, 1);
  }

  if (err <= -2) {
    return false;
  }
  return out_buf.detach();
}

Variant f_mb_ereg_replace(const Variant& pattern, const String& replacement,
                          const String& str,
                         const String& option /* = null_string */) {
  return _php_mb_regex_ereg_replace_exec(pattern, replacement,
                                         str, option, 0);
}

Variant f_mb_eregi_replace(const Variant& pattern, const String& replacement,
                           const String& str,
                           const String& option /* = null_string */) {
  return _php_mb_regex_ereg_replace_exec(pattern, replacement,
                                         str, option, ONIG_OPTION_IGNORECASE);
}

int64_t f_mb_ereg_search_getpos() {
  return MBSTRG(search_pos);
}

bool f_mb_ereg_search_setpos(int position) {
  if (position < 0 || position >= (int)MBSTRG(search_str).size()) {
    raise_warning("Position is out of range");
    MBSTRG(search_pos) = 0;
    return false;
  }
  MBSTRG(search_pos) = position;
  return true;
}

Variant f_mb_ereg_search_getregs() {
  OnigRegion *search_regs = MBSTRG(search_regs);
  if (search_regs && !MBSTRG(search_str).empty()) {
    Array ret;
    OnigUChar *str = (OnigUChar *)MBSTRG(search_str).data();
    int len = MBSTRG(search_str).size();
    int n = search_regs->num_regs;
    for (int i = 0; i < n; i++) {
      int beg = search_regs->beg[i];
      int end = search_regs->end[i];
      if (beg >= 0 && beg <= end && end <= len) {
        ret.append(String((const char *)(str + beg), end - beg, CopyString));
      } else {
        ret.append(false);
      }
    }
    return ret;
  }
  return false;
}

bool f_mb_ereg_search_init(const String& str,
                           const String& pattern /* = null_string */,
                           const String& option /* = null_string */) {
  OnigOptionType noption = MBSTRG(regex_default_options);
  OnigSyntaxType *syntax = MBSTRG(regex_default_syntax);
  if (!option.empty()) {
    noption = 0;
    _php_mb_regex_init_options(option.data(), option.size(),
                               &noption, &syntax, NULL);
  }
  if (!pattern.empty()) {
    if ((MBSTRG(search_re) = php_mbregex_compile_pattern
         (pattern, noption, MBSTRG(current_mbctype), syntax)) == NULL) {
      return false;
    }
  }

  MBSTRG(search_str) = std::string(str.data(), str.size());
  MBSTRG(search_pos) = 0;

  if (MBSTRG(search_regs) != NULL) {
    onig_region_free(MBSTRG(search_regs), 1);
    MBSTRG(search_regs) = (OnigRegion *)NULL;
  }
  return true;
}

/* regex search */
static Variant _php_mb_regex_ereg_search_exec(const String& pattern,
                                              const String& option,
                                              int mode) {
  int n, i, err, pos, len, beg, end;
  OnigUChar *str;
  OnigSyntaxType *syntax = NULL;
  OnigOptionType noption;

  noption = MBSTRG(regex_default_options);
  if (!option.empty()) {
    noption = 0;
    _php_mb_regex_init_options(option.data(), option.size(),
                               &noption, &syntax, NULL);
  }
  if (!pattern.empty()) {
    if ((MBSTRG(search_re) = php_mbregex_compile_pattern
         (pattern, noption, MBSTRG(current_mbctype), syntax)) == NULL) {
      return false;
    }
  }

  pos = MBSTRG(search_pos);
  str = NULL;
  len = 0;
  if (!MBSTRG(search_str).empty()) {
    str = (OnigUChar *)MBSTRG(search_str).data();
    len = MBSTRG(search_str).size();
  }

  if (MBSTRG(search_re) == NULL) {
    raise_warning("No regex given");
    return false;
  }

  if (str == NULL) {
    raise_warning("No string given");
    return false;
  }

  if (MBSTRG(search_regs)) {
    onig_region_free(MBSTRG(search_regs), 1);
  }
  MBSTRG(search_regs) = onig_region_new();

  err = onig_search(MBSTRG(search_re), str, str + len, str + pos, str  + len,
                    MBSTRG(search_regs), 0);
  Variant ret;
  if (err == ONIG_MISMATCH) {
    MBSTRG(search_pos) = len;
    ret = false;
  } else if (err <= -2) {
    OnigUChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(err_str, err);
    raise_warning("mbregex search failure in mbregex_search(): %s", err_str);
    ret = false;
  } else {
    if (MBSTRG(search_regs)->beg[0] == MBSTRG(search_regs)->end[0]) {
      raise_warning("Empty regular expression");
    }
    switch (mode) {
    case 1:
      {
        beg = MBSTRG(search_regs)->beg[0];
        end = MBSTRG(search_regs)->end[0];
        ret = make_packed_array(beg, end - beg);
      }
      break;
    case 2:
      n = MBSTRG(search_regs)->num_regs;
      ret = Variant(Array::Create());
      for (i = 0; i < n; i++) {
        beg = MBSTRG(search_regs)->beg[i];
        end = MBSTRG(search_regs)->end[i];
        if (beg >= 0 && beg <= end && end <= len) {
          ret.toArrRef().append(
            String((const char *)(str + beg), end - beg, CopyString));
        } else {
          ret.toArrRef().append(false);
        }
      }
      break;
    default:
      ret = true;
      break;
    }
    end = MBSTRG(search_regs)->end[0];
    if (pos < end) {
      MBSTRG(search_pos) = end;
    } else {
      MBSTRG(search_pos) = pos + 1;
    }
  }

  if (err < 0) {
    onig_region_free(MBSTRG(search_regs), 1);
    MBSTRG(search_regs) = (OnigRegion *)NULL;
  }
  return ret;
}

Variant f_mb_ereg_search(const String& pattern /* = null_string */,
                      const String& option /* = null_string */) {
  return _php_mb_regex_ereg_search_exec(pattern, option, 0);
}

Variant f_mb_ereg_search_pos(const String& pattern /* = null_string */,
                           const String& option /* = null_string */) {
  return _php_mb_regex_ereg_search_exec(pattern, option, 1);
}

Variant f_mb_ereg_search_regs(const String& pattern /* = null_string */,
                            const String& option /* = null_string */) {
  return _php_mb_regex_ereg_search_exec(pattern, option, 2);
}

static Variant _php_mb_regex_ereg_exec(const Variant& pattern, const String& str,
                                       Variant &regs, int icase) {
  php_mb_regex_t *re;
  OnigRegion *regions = NULL;
  int i, match_len, beg, end;
  OnigOptionType options;

  options = MBSTRG(regex_default_options);
  if (icase) {
    options |= ONIG_OPTION_IGNORECASE;
  }

  /* compile the regular expression from the supplied regex */
  String spattern;
  if (!pattern.isString()) {
    /* we convert numbers to integers and treat them as a string */
    if (pattern.is(KindOfDouble)) {
      spattern = String(pattern.toInt64()); /* get rid of decimal places */
    } else {
      spattern = pattern.toString();
    }
  } else {
    spattern = pattern.toString();
  }
  re = php_mbregex_compile_pattern(spattern, options, MBSTRG(current_mbctype),
                                   MBSTRG(regex_default_syntax));
  if (re == NULL) {
    return false;
  }

  regions = onig_region_new();

  /* actually execute the regular expression */
  if (onig_search(re, (OnigUChar *)str.data(),
                  (OnigUChar *)(str.data() + str.size()),
                  (OnigUChar *)str.data(),
                  (OnigUChar *)(str.data() + str.size()),
                  regions, 0) < 0) {
    onig_region_free(regions, 1);
    return false;
  }

  const char *s = str.data();
  int string_len = str.size();
  match_len = regions->end[0] - regions->beg[0];

  PackedArrayInit regsPai(regions->num_regs);
  for (i = 0; i < regions->num_regs; i++) {
    beg = regions->beg[i];
    end = regions->end[i];
    if (beg >= 0 && beg < end && end <= string_len) {
      regsPai.append(String(s + beg, end - beg, CopyString));
    } else {
      regsPai.append(false);
    }
  }
  regs = regsPai.toArray();

  if (match_len == 0) {
    match_len = 1;
  }
  if (regions != NULL) {
    onig_region_free(regions, 1);
  }
  return match_len;
}

Variant f_mb_ereg(const Variant& pattern, const String& str,
                  VRefParam regs /* = null */) {
  return _php_mb_regex_ereg_exec(pattern, str, regs, 0);
}

Variant f_mb_eregi(const Variant& pattern, const String& str,
                   VRefParam regs /* = null */) {
  return _php_mb_regex_ereg_exec(pattern, str, regs, 1);
}

Variant f_mb_regex_encoding(const String& encoding /* = null_string */) {
  if (encoding.empty()) {
    const char *retval = php_mb_regex_mbctype2name(MBSTRG(current_mbctype));
    if (retval != NULL) {
      return String(retval, CopyString);
    }
    return false;
  }

  OnigEncoding mbctype = php_mb_regex_name2mbctype(encoding.data());
  if (mbctype == ONIG_ENCODING_UNDEF) {
    raise_warning("Unknown encoding \"%s\"", encoding.data());
    return false;
  }

  MBSTRG(current_mbctype) = mbctype;
  return true;
}

static void php_mb_regex_set_options(OnigOptionType options,
                                     OnigSyntaxType *syntax,
                                     OnigOptionType *prev_options,
                                     OnigSyntaxType **prev_syntax) {
  if (prev_options != NULL) {
    *prev_options = MBSTRG(regex_default_options);
  }
  if (prev_syntax != NULL) {
    *prev_syntax = MBSTRG(regex_default_syntax);
  }
  MBSTRG(regex_default_options) = options;
  MBSTRG(regex_default_syntax) = syntax;
}

String f_mb_regex_set_options(const String& options /* = null_string */) {
  OnigOptionType opt;
  OnigSyntaxType *syntax;
  char buf[16];

  if (!options.empty()) {
    opt = 0;
    syntax = NULL;
    _php_mb_regex_init_options(options.data(), options.size(),
                               &opt, &syntax, NULL);
    php_mb_regex_set_options(opt, syntax, NULL, NULL);
  } else {
    opt = MBSTRG(regex_default_options);
    syntax = MBSTRG(regex_default_syntax);
  }
  _php_mb_regex_get_option_string(buf, sizeof(buf), opt, syntax);
  return String(buf, CopyString);
}

Variant f_mb_split(const String& pattern, const String& str,
                   int count /* = -1 */) {
  php_mb_regex_t *re;
  OnigRegion *regs = NULL;

  int n, err;
  if (count == 0) {
    count = 1;
  }

  /* create regex pattern buffer */
  if ((re = php_mbregex_compile_pattern(pattern,
                                        MBSTRG(regex_default_options),
                                        MBSTRG(current_mbctype),
                                        MBSTRG(regex_default_syntax)))
      == NULL) {
    return false;
  }

  Array ret;
  OnigUChar *pos0 = (OnigUChar *)str.data();
  OnigUChar *pos_end = (OnigUChar *)(str.data() + str.size());
  OnigUChar *pos = pos0;
  err = 0;
  regs = onig_region_new();
  /* churn through str, generating array entries as we go */
  while ((--count != 0) &&
         (err = onig_search(re, pos0, pos_end, pos, pos_end, regs, 0)) >= 0) {
    if (regs->beg[0] == regs->end[0]) {
      raise_warning("Empty regular expression");
      break;
    }

    /* add it to the array */
    if (regs->beg[0] < str.size() && regs->beg[0] >= (pos - pos0)) {
      ret.append(String((const char *)pos,
                        ((OnigUChar *)(str.data() + regs->beg[0]) - pos),
                        CopyString));
    } else {
      err = -2;
      break;
    }
    /* point at our new starting point */
    n = regs->end[0];
    if ((pos - pos0) < n) {
      pos = pos0 + n;
    }
    if (count < 0) {
      count = 0;
    }
    onig_region_free(regs, 0);
  }

  onig_region_free(regs, 1);

  /* see if we encountered an error */
  if (err <= -2) {
    OnigUChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(err_str, err);
    raise_warning("mbregex search failure in mbsplit(): %s", err_str);
    return false;
  }

  /* otherwise we just have one last element to add to the array */
  n = pos_end - pos;
  if (n > 0) {
    ret.append(String((const char *)pos, n, CopyString));
  } else {
    ret.append("");
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define SKIP_LONG_HEADER_SEP_MBSTRING(str, pos)                         \
  if (str[pos] == '\r' && str[pos + 1] == '\n' &&                       \
      (str[pos + 2] == ' ' || str[pos + 2] == '\t')) {                  \
    pos += 2;                                                           \
    while (str[pos + 1] == ' ' || str[pos + 1] == '\t') {               \
      pos++;                                                            \
    }                                                                   \
    continue;                                                           \
  }

static int _php_mbstr_parse_mail_headers(Array &ht, const char *str,
                                         size_t str_len) {
  const char *ps;
  size_t icnt;
  int state = 0;
  int crlf_state = -1;

  StringBuffer token;
  String fld_name, fld_val;

  ps = str;
  icnt = str_len;

  /*
   *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
   *             ^ ^^^^^^^^^^^^^^^^^^^^^ ^^^ ^^^^^^^^^^^^^^^^^ ^^^^
   *      state  0            1           2          3
   *
   *             C o n t e n t - T y p e :   t e x t / h t m l \r\n
   *             ^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^
   * crlf_state -1                       0                     1 -1
   *
   */

  while (icnt > 0) {
    switch (*ps) {
    case ':':
      if (crlf_state == 1) {
        token.append('\r');
      }

      if (state == 0 || state == 1) {
        fld_name = token.detach();

        state = 2;
      } else {
        token.append(*ps);
      }

      crlf_state = 0;
      break;

    case '\n':
      if (crlf_state == -1) {
        goto out;
      }
      crlf_state = -1;
      break;

    case '\r':
      if (crlf_state == 1) {
        token.append('\r');
      } else {
        crlf_state = 1;
      }
      break;

    case ' ': case '\t':
      if (crlf_state == -1) {
        if (state == 3) {
          /* continuing from the previous line */
          state = 4;
        } else {
          /* simply skipping this new line */
          state = 5;
        }
      } else {
        if (crlf_state == 1) {
          token.append('\r');
        }
        if (state == 1 || state == 3) {
          token.append(*ps);
        }
      }
      crlf_state = 0;
      break;

    default:
      switch (state) {
      case 0:
        token.clear();
        state = 1;
        break;

      case 2:
        if (crlf_state != -1) {
          token.clear();
          state = 3;
          break;
        }
        /* break is missing intentionally */

      case 3:
        if (crlf_state == -1) {
          fld_val = token.detach();
          if (!fld_name.empty() && !fld_val.empty()) {
            /* FIXME: some locale free implementation is
             * really required here,,, */
            ht.set(f_strtoupper(fld_name), fld_val);
          }
          state = 1;
        }
        break;

      case 4:
        token.append(' ');
        state = 3;
        break;
      }

      if (crlf_state == 1) {
        token.append('\r');
      }

      token.append(*ps);

      crlf_state = 0;
      break;
    }
    ps++, icnt--;
  }
out:
  if (state == 2) {
    token.clear();
    state = 3;
  }
  if (state == 3) {
    fld_val = token.detach();
    if (!fld_name.empty() && !fld_val.empty()) {
      /* FIXME: some locale free implementation is
       * really required here,,, */
      ht.set(f_strtoupper(fld_name), fld_val);
    }
  }
  return state;
}

static int php_mail(const char *to, const char *subject, const char *message,
                    const char *headers, const char *extra_cmd) {
  const char *sendmail_path = "/usr/sbin/sendmail -t -i";
  String sendmail_cmd = sendmail_path;
  if (extra_cmd != NULL) {
    sendmail_cmd += " ";
    sendmail_cmd += extra_cmd;
  }

  /* Since popen() doesn't indicate if the internal fork() doesn't work
   * (e.g. the shell can't be executed) we explicitely set it to 0 to be
   * sure we don't catch any older errno value. */
  errno = 0;
  FILE *sendmail = popen(sendmail_cmd.data(), "w");
  if (sendmail == NULL) {
    raise_warning("Could not execute mail delivery program '%s'",
                    sendmail_path);
    return 0;
  }

  if (EACCES == errno) {
    raise_warning("Permission denied: unable to execute shell to run "
                    "mail delivery binary '%s'", sendmail_path);
    pclose(sendmail);
    return 0;
  }

  fprintf(sendmail, "To: %s\n", to);
  fprintf(sendmail, "Subject: %s\n", subject);
  if (headers != NULL) {
    fprintf(sendmail, "%s\n", headers);
  }
  fprintf(sendmail, "\n%s\n", message);
  int ret = pclose(sendmail);
#if defined(EX_TEMPFAIL)
  if ((ret != EX_OK) && (ret != EX_TEMPFAIL)) return 0;
#elif defined(EX_OK)
  if (ret != EX_OK) return 0;
#else
  if (ret != 0) return 0;
#endif
  return 1;
}

bool f_mb_send_mail(const String& to, const String& subject, const String& message,
                    const String& headers /* = null_string */,
                    const String& extra_cmd /* = null_string */) {
  /* initialize */
  /* automatic allocateable buffer for additional header */
  mbfl_memory_device device;
  mbfl_memory_device_init(&device, 0, 0);
  mbfl_string orig_str, conv_str;
  mbfl_string_init(&orig_str);
  mbfl_string_init(&conv_str);

  /* character-set, transfer-encoding */
  mbfl_no_encoding
    tran_cs,  /* transfar text charset */
    head_enc,  /* header transfar encoding */
    body_enc;  /* body transfar encoding */
  tran_cs = mbfl_no_encoding_utf8;
  head_enc = mbfl_no_encoding_base64;
  body_enc = mbfl_no_encoding_base64;
  const mbfl_language *lang = mbfl_no2language(MBSTRG(current_language));
  if (lang != NULL) {
    tran_cs = lang->mail_charset;
    head_enc = lang->mail_header_encoding;
    body_enc = lang->mail_body_encoding;
  }

  Array ht_headers;
  if (!headers.empty()) {
    _php_mbstr_parse_mail_headers(ht_headers, headers.data(), headers.size());
  }

  struct {
    int cnt_type:1;
    int cnt_trans_enc:1;
  } suppressed_hdrs = { 0, 0 };

  static const StaticString s_CONTENT_TYPE("CONTENT-TYPE");
  String s = ht_headers[s_CONTENT_TYPE].toString();
  if (!s.isNull()) {
    char *tmp;
    char *param_name;
    char *charset = NULL;

    char *p = const_cast<char*>(strchr(s.data(), ';'));
    if (p != NULL) {
      /* skipping the padded spaces */
      do {
        ++p;
      } while (*p == ' ' || *p == '\t');

      if (*p != '\0') {
        if ((param_name = strtok_r(p, "= ", &tmp)) != NULL) {
          if (strcasecmp(param_name, "charset") == 0) {
            mbfl_no_encoding _tran_cs = tran_cs;

            charset = strtok_r(NULL, "= ", &tmp);
            if (charset != NULL) {
              _tran_cs = mbfl_name2no_encoding(charset);
            }

            if (_tran_cs == mbfl_no_encoding_invalid) {
              raise_warning("Unsupported charset \"%s\" - "
                              "will be regarded as ascii", charset);
              _tran_cs = mbfl_no_encoding_ascii;
            }
            tran_cs = _tran_cs;
          }
        }
      }
    }
    suppressed_hdrs.cnt_type = 1;
  }

  static const StaticString
         s_CONTENT_TRANSFER_ENCODING("CONTENT-TRANSFER-ENCODING");
  s = ht_headers[s_CONTENT_TRANSFER_ENCODING];
  if (!s.isNull()) {
    mbfl_no_encoding _body_enc = mbfl_name2no_encoding(s.data());
    switch (_body_enc) {
    case mbfl_no_encoding_base64:
    case mbfl_no_encoding_7bit:
    case mbfl_no_encoding_8bit:
      body_enc = _body_enc;
      break;

    default:
      raise_warning("Unsupported transfer encoding \"%s\" - "
                      "will be regarded as 8bit", s.data());
      body_enc =  mbfl_no_encoding_8bit;
      break;
    }
    suppressed_hdrs.cnt_trans_enc = 1;
  }

  /* To: */
  char *to_r = NULL;
  int err = 0;
  if (!to.empty()) {
    int to_len = to.size();
    if (to_len > 0) {
      to_r = strndup(to.data(), to_len);
      for (; to_len; to_len--) {
        if (!isspace((unsigned char)to_r[to_len - 1])) {
          break;
        }
        to_r[to_len - 1] = '\0';
      }
      for (int i = 0; to_r[i]; i++) {
        if (iscntrl((unsigned char)to_r[i])) {
          /**
           * According to RFC 822, section 3.1.1 long headers may be
           * separated into parts using CRLF followed at least one
           * linear-white-space character ('\t' or ' ').
           * To prevent these separators from being replaced with a space,
           * we use the SKIP_LONG_HEADER_SEP_MBSTRING to skip over them.
           */
          SKIP_LONG_HEADER_SEP_MBSTRING(to_r, i);
          to_r[i] = ' ';
        }
      }
    } else {
      to_r = (char*)to.data();
    }
  } else {
    raise_warning("Missing To: field");
    err = 1;
  }

  /* Subject: */
  String encoded_subject;
  if (!subject.isNull()) {
    orig_str.no_language = MBSTRG(current_language);
    orig_str.val = (unsigned char *)subject.data();
    orig_str.len = subject.size();
    orig_str.no_encoding = MBSTRG(current_internal_encoding);
    if (orig_str.no_encoding == mbfl_no_encoding_invalid
        || orig_str.no_encoding == mbfl_no_encoding_pass) {
      orig_str.no_encoding = mbfl_identify_encoding_no
        (&orig_str, MBSTRG(current_detect_order_list),
         MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
    }
    mbfl_string *pstr = mbfl_mime_header_encode
      (&orig_str, &conv_str, tran_cs, head_enc,
       "\n", sizeof("Subject: [PHP-jp nnnnnnnn]"));
    if (pstr != NULL) {
      encoded_subject = String(reinterpret_cast<char*>(pstr->val),
                               pstr->len,
                               AttachString);
    }
  } else {
    raise_warning("Missing Subject: field");
    err = 1;
  }

  /* message body */
  String encoded_message;
  if (!message.empty()) {
    orig_str.no_language = MBSTRG(current_language);
    orig_str.val = (unsigned char*)message.data();
    orig_str.len = message.size();
    orig_str.no_encoding = MBSTRG(current_internal_encoding);

    if (orig_str.no_encoding == mbfl_no_encoding_invalid
        || orig_str.no_encoding == mbfl_no_encoding_pass) {
      orig_str.no_encoding = mbfl_identify_encoding_no
        (&orig_str, MBSTRG(current_detect_order_list),
         MBSTRG(current_detect_order_list_size), MBSTRG(strict_detection));
    }

    mbfl_string *pstr = NULL;
    {
      mbfl_string tmpstr;
      if (mbfl_convert_encoding(&orig_str, &tmpstr, tran_cs) != NULL) {
        tmpstr.no_encoding = mbfl_no_encoding_8bit;
        pstr = mbfl_convert_encoding(&tmpstr, &conv_str, body_enc);
        free(tmpstr.val);
      }
    }
    if (pstr != NULL) {
      encoded_message = String(reinterpret_cast<char*>(pstr->val),
                               pstr->len,
                               AttachString);
    }
  } else {
    /* this is not really an error, so it is allowed. */
    raise_warning("Empty message body");
  }

  /* other headers */
#define PHP_MBSTR_MAIL_MIME_HEADER1 "Mime-Version: 1.0"
#define PHP_MBSTR_MAIL_MIME_HEADER2 "Content-Type: text/plain"
#define PHP_MBSTR_MAIL_MIME_HEADER3 "; charset="
#define PHP_MBSTR_MAIL_MIME_HEADER4 "Content-Transfer-Encoding: "
  if (!headers.empty()) {
    const char *p = headers.data();
    int n = headers.size();
    mbfl_memory_device_strncat(&device, p, n);
    if (n > 0 && p[n - 1] != '\n') {
      mbfl_memory_device_strncat(&device, "\n", 1);
    }
  }

  mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER1,
                             sizeof(PHP_MBSTR_MAIL_MIME_HEADER1) - 1);
  mbfl_memory_device_strncat(&device, "\n", 1);

  if (!suppressed_hdrs.cnt_type) {
    mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER2,
                               sizeof(PHP_MBSTR_MAIL_MIME_HEADER2) - 1);

    char *p = (char *)mbfl_no2preferred_mime_name(tran_cs);
    if (p != NULL) {
      mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER3,
                                 sizeof(PHP_MBSTR_MAIL_MIME_HEADER3) - 1);
      mbfl_memory_device_strcat(&device, p);
    }
    mbfl_memory_device_strncat(&device, "\n", 1);
  }
  if (!suppressed_hdrs.cnt_trans_enc) {
    mbfl_memory_device_strncat(&device, PHP_MBSTR_MAIL_MIME_HEADER4,
                               sizeof(PHP_MBSTR_MAIL_MIME_HEADER4) - 1);
    const char *p = (char *)mbfl_no2preferred_mime_name(body_enc);
    if (p == NULL) {
      p = "7bit";
    }
    mbfl_memory_device_strcat(&device, p);
    mbfl_memory_device_strncat(&device, "\n", 1);
  }

  mbfl_memory_device_unput(&device);
  mbfl_memory_device_output('\0', &device);

  char *all_headers = (char *)device.buffer;

  String cmd = f_escapeshellcmd(extra_cmd);
  bool ret = (!err && php_mail(to_r, encoded_subject.data(),
                               encoded_message.data(),
                               all_headers, cmd.data()));
  mbfl_memory_device_clear(&device);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
