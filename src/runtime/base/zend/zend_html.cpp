/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/zend/zend_html.h>
#include <runtime/base/complex_types.h>
#include <util/lock.h>

namespace HPHP {

#define HTML_SPECIALCHARS   0
#define HTML_ENTITIES       1

#define ENT_HTML_QUOTE_NONE     0
#define ENT_HTML_QUOTE_SINGLE   1
#define ENT_HTML_QUOTE_DOUBLE   2

#define ENT_COMPAT    ENT_HTML_QUOTE_DOUBLE
#define ENT_QUOTES    (ENT_HTML_QUOTE_DOUBLE | ENT_HTML_QUOTE_SINGLE)
#define ENT_NOQUOTES  ENT_HTML_QUOTE_NONE

///////////////////////////////////////////////////////////////////////////////
// UTF-8 entity tables

enum entity_charset {
  cs_terminator, cs_8859_1, cs_cp1252,
  cs_8859_15, cs_utf_8, cs_big5, cs_gb2312,
  cs_big5hkscs, cs_sjis, cs_eucjp, cs_koi8r,
  cs_cp1251, cs_8859_5, cs_cp866, cs_macroman,
  cs_end
};

typedef const char *const entity_table_t;

/* codepage 1252 is a Windows extension to iso-8859-1. */
static entity_table_t ent_cp_1252[] = {
  "euro", NULL, "sbquo", "fnof", "bdquo", "hellip", "dagger",
  "Dagger", "circ", "permil", "Scaron", "lsaquo", "OElig",
  NULL, NULL, NULL, NULL, "lsquo", "rsquo", "ldquo", "rdquo",
  "bull", "ndash", "mdash", "tilde", "trade", "scaron", "rsaquo",
  "oelig", NULL, NULL, "Yuml"
};

static entity_table_t ent_iso_8859_1[] = {
  "nbsp", "iexcl", "cent", "pound", "curren", "yen", "brvbar",
  "sect", "uml", "copy", "ordf", "laquo", "not", "shy", "reg",
  "macr", "deg", "plusmn", "sup2", "sup3", "acute", "micro",
  "para", "middot", "cedil", "sup1", "ordm", "raquo", "frac14",
  "frac12", "frac34", "iquest", "Agrave", "Aacute", "Acirc",
  "Atilde", "Auml", "Aring", "AElig", "Ccedil", "Egrave",
  "Eacute", "Ecirc", "Euml", "Igrave", "Iacute", "Icirc",
  "Iuml", "ETH", "Ntilde", "Ograve", "Oacute", "Ocirc", "Otilde",
  "Ouml", "times", "Oslash", "Ugrave", "Uacute", "Ucirc", "Uuml",
  "Yacute", "THORN", "szlig", "agrave", "aacute", "acirc",
  "atilde", "auml", "aring", "aelig", "ccedil", "egrave",
  "eacute", "ecirc", "euml", "igrave", "iacute", "icirc",
  "iuml", "eth", "ntilde", "ograve", "oacute", "ocirc", "otilde",
  "ouml", "divide", "oslash", "ugrave", "uacute", "ucirc",
  "uuml", "yacute", "thorn", "yuml"
};

static entity_table_t ent_iso_8859_15[] = {
  "nbsp", "iexcl", "cent", "pound", "euro", "yen", "Scaron",
  "sect", "scaron", "copy", "ordf", "laquo", "not", "shy", "reg",
  "macr", "deg", "plusmn", "sup2", "sup3", NULL, /* Zcaron */
  "micro", "para", "middot", NULL, /* zcaron */ "sup1", "ordm",
  "raquo", "OElig", "oelig", "Yuml", "iquest", "Agrave", "Aacute",
  "Acirc", "Atilde", "Auml", "Aring", "AElig", "Ccedil", "Egrave",
  "Eacute", "Ecirc", "Euml", "Igrave", "Iacute", "Icirc",
  "Iuml", "ETH", "Ntilde", "Ograve", "Oacute", "Ocirc", "Otilde",
  "Ouml", "times", "Oslash", "Ugrave", "Uacute", "Ucirc", "Uuml",
  "Yacute", "THORN", "szlig", "agrave", "aacute", "acirc",
  "atilde", "auml", "aring", "aelig", "ccedil", "egrave",
  "eacute", "ecirc", "euml", "igrave", "iacute", "icirc",
  "iuml", "eth", "ntilde", "ograve", "oacute", "ocirc", "otilde",
  "ouml", "divide", "oslash", "ugrave", "uacute", "ucirc",
  "uuml", "yacute", "thorn", "yuml"
};

static entity_table_t ent_uni_338_402[] = {
  /* 338 (0x0152) */
  "OElig", "oelig", NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 352 (0x0160) */
  "Scaron", "scaron", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 376 (0x0178) */
  "Yuml", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 400 (0x0190) */
  NULL, NULL, "fnof"
};

static entity_table_t ent_uni_spacing[] = {
  /* 710 */
  "circ",
  /* 711 - 730 */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 731 - 732 */
  NULL, "tilde"
};

static entity_table_t ent_uni_greek[] = {
  /* 913 */
  "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta",
  "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho",
  NULL, "Sigma", "Tau", "Upsilon", "Phi", "Chi", "Psi", "Omega",
  /* 938 - 944 are not mapped */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta",
  "iota", "kappa", "lambda", "mu", "nu", "xi", "omicron", "pi", "rho",
  "sigmaf", "sigma", "tau", "upsilon", "phi", "chi", "psi", "omega",
  /* 970 - 976 are not mapped */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "thetasym", "upsih",
  NULL, NULL, NULL,
  "piv"
};

static entity_table_t ent_uni_punct[] = {
  /* 8194 */
  "ensp", "emsp", NULL, NULL, NULL, NULL, NULL,
  "thinsp", NULL, NULL, "zwnj", "zwj", "lrm", "rlm",
  NULL, NULL, NULL, "ndash", "mdash", NULL, NULL, NULL,
  /* 8216 */
  "lsquo", "rsquo", "sbquo", NULL, "ldquo", "rdquo", "bdquo", NULL,
  "dagger", "Dagger", "bull", NULL, NULL, NULL, "hellip",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, "permil", NULL,
  /* 8242 */
  "prime", "Prime", NULL, NULL, NULL, NULL, NULL, "lsaquo", "rsaquo", NULL,
  NULL, NULL, "oline", NULL, NULL, NULL, NULL, NULL,
  "frasl"
};

static entity_table_t ent_uni_euro[] = {
  "euro"
};

static entity_table_t ent_uni_8465_8501[] = {
  /* 8465 */
  "image", NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8472 */
  "weierp", NULL, NULL, NULL,
  /* 8476 */
  "real", NULL, NULL, NULL, NULL, NULL,
  /* 8482 */
  "trade", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8501 */
  "alefsym",
};

static entity_table_t ent_uni_8592_9002[] = {
  /* 8592 (0x2190) */
  "larr", "uarr", "rarr", "darr", "harr", NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8608 (0x21a0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8624 (0x21b0) */
  NULL, NULL, NULL, NULL, NULL, "crarr", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8640 (0x21c0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8656 (0x21d0) */
  "lArr", "uArr", "rArr", "dArr", "hArr", "vArr", NULL, NULL,
  NULL, NULL, "lAarr", "rAarr", NULL, "rarrw", NULL, NULL,
  /* 8672 (0x21e0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8704 (0x2200) */
  "forall", "comp", "part", "exist", "nexist", "empty", NULL, "nabla",
  "isin", "notin", "epsis", "ni", "notni", "bepsi", NULL, "prod",
  /* 8720 (0x2210) */
  "coprod", "sum", "minus", "mnplus", "plusdo", NULL, "setmn", "lowast",
  "compfn", NULL, "radic", NULL, NULL, "prop", "infin", "ang90",
  /* 8736 (0x2220) */
  "ang", "angmsd", "angsph", "mid", "nmid", "par", "npar", "and",
  "or", "cap", "cup", "int", NULL, NULL, "conint", NULL,
  /* 8752 (0x2230) */
  NULL, NULL, NULL, NULL, "there4", "becaus", NULL, NULL,
  NULL, NULL, NULL, NULL, "sim", "bsim", NULL, NULL,
  /* 8768 (0x2240) */
  "wreath", "nsim", NULL, "sime", "nsime", "cong", NULL, "ncong",
  "asymp", "nap", "ape", NULL, "bcong", "asymp", "bump", "bumpe",
  /* 8784 (0x2250) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8800 (0x2260) */
  "ne", "equiv", NULL, NULL, "le", "ge", "lE", "gE",
  "lnE", "gnE", "Lt", "Gt", "twixt", NULL, "nlt", "ngt",
  /* 8816 (0x2270) */
  "nles", "nges", "lsim", "gsim", NULL, NULL, "lg", "gl",
  NULL, NULL, "pr", "sc", "cupre", "sscue", "prsim", "scsim",
  /* 8832 (0x2280) */
  "npr", "nsc", "sub", "sup", "nsub", "nsup", "sube", "supe",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8848 (0x2290) */
  NULL, NULL, NULL, NULL, NULL, "oplus", NULL, "otimes",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8864 (0x22a0) */
  NULL, NULL, NULL, NULL, NULL, "perp", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8880 (0x22b0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8896 (0x22c0) */
  NULL, NULL, NULL, NULL, NULL, "sdot", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8912 (0x22d0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8928 (0x22e0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8944 (0x22f0) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8960 (0x2300) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "lceil", "rceil", "lfloor", "rfloor", NULL, NULL, NULL, NULL,
  /* 8976 (0x2310) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  /* 8992 (0x2320) */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, "lang", "rang"
};

static entity_table_t ent_uni_9674[] = {
  /* 9674 */
  "loz"
};

static entity_table_t ent_uni_9824_9830[] = {
  /* 9824 */
  "spades", NULL, NULL, "clubs", NULL, "hearts", "diams"
};

struct html_entity_map {
  enum entity_charset charset; /* charset identifier */
  unsigned short basechar; /* char code at start of table */
  unsigned short endchar;  /* last char code in the table */
  entity_table_t *table;   /* the table of mappings */
};

static const struct html_entity_map entity_map[] = {
  { cs_cp1252,    0x80, 0x9f, ent_cp_1252 },
  { cs_cp1252,    0xa0, 0xff, ent_iso_8859_1 },
  { cs_8859_1,    0xa0, 0xff, ent_iso_8859_1 },
  { cs_8859_15,   0xa0, 0xff, ent_iso_8859_15 },
  { cs_utf_8,     0xa0, 0xff, ent_iso_8859_1 },
  { cs_utf_8,     338,  402,  ent_uni_338_402 },
  { cs_utf_8,     710,  732,  ent_uni_spacing },
  { cs_utf_8,     913,  982,  ent_uni_greek },
  { cs_utf_8,     8194, 8260, ent_uni_punct },
  { cs_utf_8,     8364, 8364, ent_uni_euro },
  { cs_utf_8,     8465, 8501, ent_uni_8465_8501 },
  { cs_utf_8,     8592, 9002, ent_uni_8592_9002 },
  { cs_utf_8,     9674, 9674, ent_uni_9674 },
  { cs_utf_8,     9824, 9830, ent_uni_9824_9830 },
  { cs_big5,      0xa0, 0xff, ent_iso_8859_1 },
  { cs_gb2312,    0xa0, 0xff, ent_iso_8859_1 },
  { cs_big5hkscs, 0xa0, 0xff, ent_iso_8859_1 },
  { cs_sjis,      0xa0, 0xff, ent_iso_8859_1 },
  { cs_eucjp,     0xa0, 0xff, ent_iso_8859_1 },
  /* Missing support for these at the moment
  { cs_koi8r,     0xa3, 0xff, ent_koi8r },
  { cs_cp1251,    0x80, 0xff, ent_cp_1251 },
  { cs_8859_5,    0xc0, 0xff, ent_iso_8859_5 },
  { cs_cp866,     0xc0, 0xff, ent_cp_866 },
  { cs_macroman,  0x0b, 0xff, ent_macroman },
  */
  { cs_terminator }
};

static const struct {
  const char *codeset;
  enum entity_charset charset;
} charset_map[] = {
  { "ISO-8859-1",     cs_8859_1 },
  { "ISO8859-1",      cs_8859_1 },
  { "ISO-8859-15",    cs_8859_15 },
  { "ISO8859-15",     cs_8859_15 },
  { "utf-8",          cs_utf_8 },
  { "cp1252",         cs_cp1252 },
  { "Windows-1252",   cs_cp1252 },
  { "1252",           cs_cp1252 },
  { "BIG5",           cs_big5 },
  { "950",            cs_big5 },
  { "GB2312",         cs_gb2312 },
  { "936",            cs_gb2312 },
  { "BIG5-HKSCS",     cs_big5hkscs },
  { "Shift_JIS",      cs_sjis },
  { "SJIS",           cs_sjis },
  { "932",            cs_sjis },
  { "EUCJP",          cs_eucjp },
  /* Missing support for these at the moment
  { "EUC-JP",         cs_eucjp },
  { "KOI8-R",         cs_koi8r },
  { "koi8-ru",        cs_koi8r },
  { "koi8r",          cs_koi8r },
  { "cp1251",         cs_cp1251 },
  { "Windows-1251",   cs_cp1251 },
  { "win-1251",       cs_cp1251 },
  { "iso8859-5",      cs_8859_5 },
  { "iso-8859-5",     cs_8859_5 },
  { "cp866",          cs_cp866 },
  { "866",            cs_cp866 },
  { "ibm866",         cs_cp866 },
  { "MacRoman",       cs_macroman },
  */
  { NULL }
};

static const struct {
  unsigned short charcode;
  const char *entity;
  int entitylen;
  int flags;
} basic_entities[] = {
  { '"',  "&quot;",   6,  ENT_HTML_QUOTE_DOUBLE },
  { '\'', "&#039;",   6,  ENT_HTML_QUOTE_SINGLE },
  { '\'', "&#39;",    5,  ENT_HTML_QUOTE_SINGLE },
  { '<',  "&lt;",     4,  0 },
  { '>',  "&gt;",     4,  0 },
  { 0, NULL, 0, 0 }
};

///////////////////////////////////////////////////////////////////////////////

static enum entity_charset determine_charset(const char *charset_hint) {
  enum entity_charset charset = cs_utf_8;

  if (charset_hint == NULL)
    return cs_utf_8;

  bool found = 0;
  size_t len = strlen(charset_hint);

  /* now walk the charset map and look for the codeset */
  for (int i = 0; charset_map[i].codeset; i++) {
    if (len == strlen(charset_map[i].codeset) &&
      strncasecmp(charset_hint, charset_map[i].codeset, len) == 0) {
      charset = charset_map[i].charset;
      found = 1;
      break;
    }
  }

  if (!found) {
    raise_warning("Charset `%' not supported, assuming utf-8", charset_hint);
  }

  return charset;
}

static int utf32_to_utf8(unsigned char *buf, int k) {
  int retval = 0;

  if (k < 0x80) {
    buf[0] = k;
    retval = 1;
  } else if (k < 0x800) {
    buf[0] = 0xc0 | (k >> 6);
    buf[1] = 0x80 | (k & 0x3f);
    retval = 2;
  } else if (k < 0x10000) {
    buf[0] = 0xe0 | (k >> 12);
    buf[1] = 0x80 | ((k >> 6) & 0x3f);
    buf[2] = 0x80 | (k & 0x3f);
    retval = 3;
  } else if (k < 0x200000) {
    buf[0] = 0xf0 | (k >> 18);
    buf[1] = 0x80 | ((k >> 12) & 0x3f);
    buf[2] = 0x80 | ((k >> 6) & 0x3f);
    buf[3] = 0x80 | (k & 0x3f);
    retval = 4;
  } else if (k < 0x4000000) {
    buf[0] = 0xf8 | (k >> 24);
    buf[1] = 0x80 | ((k >> 18) & 0x3f);
    buf[2] = 0x80 | ((k >> 12) & 0x3f);
    buf[3] = 0x80 | ((k >> 6) & 0x3f);
    buf[4] = 0x80 | (k & 0x3f);
    retval = 5;
  } else {
    buf[0] = 0xfc | (k >> 30);
    buf[1] = 0x80 | ((k >> 24) & 0x3f);
    buf[2] = 0x80 | ((k >> 18) & 0x3f);
    buf[3] = 0x80 | ((k >> 12) & 0x3f);
    buf[4] = 0x80 | ((k >> 6) & 0x3f);
    buf[5] = 0x80 | (k & 0x3f);
    retval = 6;
  }
  buf[retval] = '\0';

  return retval;
}

typedef __gnu_cxx::hash_map
<const char *, std::string, __gnu_cxx::hash<const char *>, eqstr>
HtmlEntityMap;

static volatile bool EntityMapInited = false;
static Mutex EntityMapMutex;
static HtmlEntityMap EntityMap[cs_end];
static HtmlEntityMap XHPEntityMap[cs_end];

static void init_entity_table() {
  for (unsigned int i = 0; entity_map[i].charset != cs_terminator; i++) {
    const html_entity_map &em = entity_map[i];
    const enum entity_charset charset = entity_map[i].charset;

    int index = 0;
    for (int ch = em.basechar; ch <= em.endchar; ch++, index++) {
      const char *entity = em.table[index];
      if (entity == NULL) {
        continue;
      }
      unsigned char buf[10];
      switch (charset) {
        case cs_8859_1:
        case cs_cp1252:
        case cs_8859_15:
        case cs_cp1251:
        case cs_8859_5:
        case cs_cp866:
        case cs_koi8r:
          buf[0] = ch;
          buf[1] = '\0';
          break;

        case cs_utf_8:
          utf32_to_utf8(buf, ch);
          break;

        default:
          continue;
      }
      EntityMap[charset][entity] = (const char *)buf;
      XHPEntityMap[charset][entity] = (const char *)buf;
    }

    EntityMap[charset]["quot"] = "\"";
    EntityMap[charset]["lt"] = "<";
    EntityMap[charset]["gt"] = ">";
    EntityMap[charset]["amp"] = "&";

    XHPEntityMap[charset]["quot"] = "\"";
    XHPEntityMap[charset]["lt"] = "<";
    XHPEntityMap[charset]["gt"] = ">";
    XHPEntityMap[charset]["amp"] = "&";
    // XHP-specific entities
    XHPEntityMap[charset]["apos"] = "\'";
    XHPEntityMap[charset]["cloud"] = "\u2601";
    XHPEntityMap[charset]["umbrella"] = "\u2602";
    XHPEntityMap[charset]["snowman"] = "\u2603";
    XHPEntityMap[charset]["snowflake"] = "\u2745";
    XHPEntityMap[charset]["comet"] = "\u2604";
    XHPEntityMap[charset]["thunderstorm"] = "\u2608";
  }

  // the first element is an empty table
  EntityMap[cs_terminator]["quot"] = "\"";
  EntityMap[cs_terminator]["lt"] = "<";
  EntityMap[cs_terminator]["gt"] = ">";
  EntityMap[cs_terminator]["amp"] = "&";
  // XHP-specific entities
  XHPEntityMap[cs_terminator]["apos"] = "\'";
  XHPEntityMap[cs_terminator]["cloud"] = "\u2601";
  XHPEntityMap[cs_terminator]["umbrella"] = "\u2602";
  XHPEntityMap[cs_terminator]["snowman"] = "\u2603";
  XHPEntityMap[cs_terminator]["snowflake"] = "\u2745";
  XHPEntityMap[cs_terminator]["comet"] = "\u2604";
  XHPEntityMap[cs_terminator]["thunderstorm"] = "\u2608";
}

///////////////////////////////////////////////////////////////////////////////

char *string_html_encode(const char *input, int &len, bool encode_double_quote,
                         bool encode_single_quote, bool utf8, bool nbsp) {
  ASSERT(input);
  if (!*input) {
    return NULL;
  }

  /**
   * Though seems to be wasting memory a lot, we have to realize most of the
   * time this function is called with small strings, or fragments of HTMLs.
   * Allocating/deallocating anything less than 1K is trivial these days, and
   * we want avoid string copying as much as possible. Of course, the return
   * char * is really sent back at large, occupying unnessary space for
   * potentially longer time than we need, we have to realize the two closest
   * solutions are not that much better, either:
   *
   * 1. pre-calculate size by iterating through the string once: too time
   *    consuming;
   * 2. take a guess and double buffer size when over: still wasting, and
   *    it may not save that much.
   */
  char *ret = (char *)malloc(len * 6 + 1);
  if (!ret) {
    return NULL;
  }
  char *q = ret;
  for (const char *p = input; *p; p++) {
    char c = *p;
    switch (c) {
    case '"':
      if (encode_double_quote) {
        *q++ = '&'; *q++ = 'q'; *q++ = 'u'; *q++ = 'o'; *q++ = 't'; *q++ = ';';
      } else {
        *q++ = c;
      }
      break;
    case '\'':
      if (encode_single_quote) {
        *q++ = '&'; *q++ = '#'; *q++ = '0'; *q++ = '3'; *q++ = '9'; *q++ = ';';
      } else {
        *q++ = c;
      }
      break;
    case '<':
      *q++ = '&'; *q++ = 'l'; *q++ = 't'; *q++ = ';';
      break;
    case '>':
      *q++ = '&'; *q++ = 'g'; *q++ = 't'; *q++ = ';';
      break;
    case '&':
      *q++ = '&'; *q++ = 'a'; *q++ = 'm'; *q++ = 'p'; *q++ = ';';
      break;
    case '\xc2':
      if (nbsp && utf8 && *(p+1) == '\xa0') {
        *q++ = '&'; *q++ = 'n'; *q++ = 'b'; *q++ = 's'; *q++ = 'p'; *q++ = ';';
        p++;
      } else {
        *q++ = c;
      }
      break;
    case '\xa0':
      if (nbsp && !utf8) {
        *q++ = '&'; *q++ = 'n'; *q++ = 'b'; *q++ = 's'; *q++ = 'p'; *q++ = ';';
      } else {
        *q++ = c;
      }
      break;
    default:
      *q++ = c;
      break;
    }
  }
  *q = 0;
  len = q - ret;
  return ret;
}

inline static bool decode_entity(char *entity, int *len,
                                 enum entity_charset charset, bool all,
                                 bool xhp = false) {
  // entity is 16 bytes, allocated statically below
  // default in PHP
  int quote_style = ENT_COMPAT;
  ASSERT(entity && *entity);
  if (entity[0] == '#') {
    int code;
    if (entity[1] == 'x' || entity[1] == 'X') {
      code = strtol(entity + 2, NULL, 16);
    } else {
      code = strtol(entity + 1, NULL, 10);
    }

    // since we don't support multibyte chars other than utf-8
    int l = 1;

    switch (charset) {
      case cs_utf_8:
      {
        unsigned char buf[10];
        int size = utf32_to_utf8(buf, code);
        memcpy(entity, buf, size + 1);
        l = size;
        break;
      }

      case cs_8859_1:
      case cs_8859_5:
      case cs_8859_15:
        if ((code >= 0x80 && code < 0xa0) || code > 0xff) {
          return false;
        } else {
          if (code == 39 || !quote_style) {
            return false;
          }
          entity[0] = code;
          entity[1] = '\0';
        }
        break;

      case cs_cp1252:
      case cs_cp1251:
      case cs_cp866:
        if (code > 0xff) {
          return false;
        }
        entity[0] = code;
        entity[1] = '\0';
        break;

      case cs_big5:
      case cs_big5hkscs:
      case cs_sjis:
      case cs_eucjp:
        if (code >= 0x80) {
          return false;
        }
        entity[0] = code;
        entity[1] = '\0';
        break;

      case cs_gb2312:
        if (code >= 0x81) {
          return false;
        }
        entity[0] = code;
        entity[1] = '\0';
        break;

      default:
        return false;
        break;
    }
    *len = l;
    return true;
  } else {
    HtmlEntityMap *entityMap;
    if (all) {
      entityMap = xhp ? &XHPEntityMap[charset] : &EntityMap[charset];
    } else {
      entityMap = xhp ? &XHPEntityMap[cs_terminator]
                      : &EntityMap[cs_terminator];
    }
    HtmlEntityMap::const_iterator iter = entityMap->find(entity);
    if (iter != entityMap->end()) {
      memcpy(entity, iter->second.c_str(), iter->second.length() + 1);
      *len = iter->second.length();
      return true;
    }
  }

  return false;
}

char *string_html_decode(const char *input, int &len,
                         const char *charset_hint, bool all,
                         bool xhp /* = false */) {
  ASSERT(input);
  if (!*input) {
    return NULL;
  }

  if (!EntityMapInited) {
    Lock lock(EntityMapMutex);
    if (!EntityMapInited) {
      init_entity_table();
      EntityMapInited = true;
    }
  }

  enum entity_charset charset = determine_charset(charset_hint);

  char *ret = (char *)malloc(len + 1);
  char *q = ret;
  for (const char *p = input; *p; p++) {
    char ch = *p;
    if (ch != '&') {
      *q++ = ch;
      continue;
    }
    p++;

    bool found = false;
    for (const char *t = p; *t; t++) {
      int l = t - p;
      if (l > 10) break;

      if (*t == ';') {
        if (l > 0) {
          char buf[16];
          memcpy(buf, p, l);
          buf[l] = '\0';
          if (decode_entity(buf, &l, charset, all, xhp)) {
            memcpy(q, buf, l);
            found = true;
            p = t;
            q += l;
          }
        }
        break;
      }
    }
    if (!found) {
      p--;
      *q++ = '&'; // not an entity
    }
  }
  *q = '\0';
  len = q - ret;
  return ret;
}

Array string_get_html_translation_table(int which, int quote_style) {
  char ind[2]; ind[1] = 0;
  enum entity_charset charset = determine_charset(NULL);

  Array ret;
  switch (which) {
  case HTML_ENTITIES:
    if (!EntityMapInited) {
      Lock lock(EntityMapMutex);
      if (!EntityMapInited) {
        init_entity_table();
        EntityMapInited = true;
      }
    }

    for (int j = 0; entity_map[j].charset != cs_terminator; j++) {
      const html_entity_map &em = entity_map[j];
      if (em.charset != charset)
        continue;

      for (int i = 0; i <= em.endchar - em.basechar; i++) {
        char buffer[16];

        if (em.table[i] == NULL)
          continue;
        /* what about wide chars here ?? */
        ind[0] = i + em.basechar;
        snprintf(buffer, sizeof(buffer), "&%s;", em.table[i]);
        ret.set(ind, String(buffer, CopyString));
      }
    }
    /* break thru */

  case HTML_SPECIALCHARS:
    for (int j = 0; basic_entities[j].charcode != 0; j++) {
      if (basic_entities[j].flags &&
          (quote_style & basic_entities[j].flags) == 0)
        continue;

      ind[0] = (unsigned char)basic_entities[j].charcode;
      ret.set(String(ind, 2, CopyString), basic_entities[j].entity);
    }
    ret.set("&", "&amp;");
    break;
  }

  return ret;
}

bool html_supported_charset(const char *charset) {
  size_t len = strlen(charset);
  for (int i = 0; charset_map[i].codeset; i++) {
     if (len == strlen(charset_map[i].codeset) &&
       strncasecmp(charset, charset_map[i].codeset, len) == 0) {
       return true;
     }
   }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
