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

#ifndef incl_HPHP_ZEND_HTML_H_
#define incl_HPHP_ZEND_HTML_H_

#include <cstdint>

// Avoid dragging in the icu namespace.
#ifndef U_USING_ICU_NAMESPACE
#define U_USING_ICU_NAMESPACE 0
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * Major departures from Zend:
 *
 * 1. We are only supporting UTF-8 and ISO-8859-1 encoding.
 *    Major reason for this is because the original get_next_char() bothers me,
 *    sacrificing performance for some character sets that people rarely used
 *    or that people shouldn't use. UTF-8 should really be the standard string
 *    format everywhere, and we ought to write coding specifilized for it to
 *    take full advantage of it: one example would be the new html encoding
 *    function that simply do *p one a time iterating through the strings to
 *    look for special characters for entity escaping.
 *
 * 2. HTML encoding function no longer encodes entities other than the basic
 *    ones. There is no need to encode them, since all browsers support UTF-8
 *    natively, and we are ok to send out UTF-8 encoding characters without
 *    turning them into printable ASCIIs. Basic entities are encoded for
 *    a different reason! In fact, I personally don't see why HTML spec has
 *    those extended list of entities, other than historical artifacts.
 *
 * 3. Double encoding parameter is not supported. That really sounds like
 *    a workaround of buggy coding. I don't find a legit use for that yet.
 */

struct AsciiMap {
  uint64_t map[2];
};

enum StringHtmlEncoding {
  STRING_HTML_ENCODE_UTF8 = 1,
  STRING_HTML_ENCODE_NBSP = 2,
  STRING_HTML_ENCODE_HIGH = 4,
  STRING_HTML_ENCODE_UTF8IZE_REPLACE = 8
};

enum class EntBitmask {
  ENT_BM_NOQUOTES = 0,   /* leave all quotes alone */
  ENT_BM_SINGLE = 1,     /* escape single quotes only */
  ENT_BM_DOUBLE = 2,     /* escape double quotes only */
  ENT_BM_IGNORE = 4,     /* silently discard invalid chars */
  ENT_BM_SUBSTITUTE = 8, /* replace invalid chars with U+FFFD */
  ENT_BM_XML1 = 16,      /* XML1 mode*/
  ENT_BM_XHTML = 32,     /* XHTML mode */
};

namespace entity_charset_enum {
enum entity_charset_impl {
  cs_terminator, cs_8859_1, cs_cp1252,
  cs_8859_15, cs_utf_8, cs_big5, cs_gb2312,
  cs_big5hkscs, cs_sjis, cs_eucjp, cs_koi8r,
  cs_cp1251, cs_8859_5, cs_cp866, cs_macroman,
  cs_unknown,
  cs_end
};
}
typedef entity_charset_enum::entity_charset_impl entity_charset;

struct HtmlBasicEntity {
  unsigned short charcode;
  const char *entity;
  int entitylen;
  int flags;
};

typedef const char *const entity_table_t;

struct html_entity_map {
  entity_charset charset; /* charset identifier */
  unsigned short basechar; /* char code at start of table */
  unsigned short endchar;  /* last char code in the table */
  entity_table_t *table;   /* the table of mappings */
};

const html_entity_map* html_get_entity_map();

/*
 * returns cs_unknown iff not found;
 * if input null, returns default charset of cs_utf_8
 */
entity_charset determine_charset(const char*);

char *string_html_encode(const char *input, int &len,
                         const int64_t qsBitmask, bool utf8,
                         bool dEncode, bool htmlEnt);
char *string_html_encode_extra(const char *input, int &len,
                               StringHtmlEncoding flags,
                               const AsciiMap *asciiMap);

/**
 * returns decoded string;
 * note, can return nullptr if the charset could not be detected
 * using the given charset_hint; can also pass in nullptr
 * for the charset_hint to use the default one (UTF-8).
 * (see determine_charset).
 */
char *string_html_decode(const char *input, int &len,
                         bool decode_double_quote, bool decode_single_quote,
                         const char *charset_hint,
                         bool all, bool xhp = false );

///////////////////////////////////////////////////////////////////////////////
}

#endif
