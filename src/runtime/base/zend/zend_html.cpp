/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

typedef const char *const entity_table_t;

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
  unsigned short basechar; /* char code at start of table */
  unsigned short endchar;  /* last char code in the table */
  entity_table_t *table;   /* the table of mappings */
};

static const struct html_entity_map entity_map[] = {
  { 0xa0, 0xff, ent_iso_8859_1 },
  { 338,  402,  ent_uni_338_402 },
  { 710,  732,  ent_uni_spacing },
  { 913,  982,  ent_uni_greek },
  { 8194, 8260, ent_uni_punct },
  { 8364, 8364, ent_uni_euro },
  { 8465, 8501, ent_uni_8465_8501 },
  { 8592, 9002, ent_uni_8592_9002 },
  { 9674, 9674, ent_uni_9674 },
  { 9824, 9830, ent_uni_9824_9830 },
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

static bool EntityMapInited = false;
static Mutex EntityMapMutex;
static HtmlEntityMap EntityMap;

static void init_entity_table() {
  for (unsigned int i = 0; i < sizeof(entity_map)/sizeof(entity_map[0]); i++) {
    const html_entity_map &em = entity_map[i];

    int index = 0;
    for (int ch = em.basechar; ch <= em.endchar; ch++, index++) {
      const char *entity = em.table[index];
      if (entity) {
        unsigned char buf[10];
        utf32_to_utf8(buf, ch);
        EntityMap[entity] = (const char *)buf;
      }
    }
  }
  EntityMap["quot"] = "\"";
  EntityMap["lt"] = "<";
  EntityMap["gt"] = ">";
  EntityMap["amp"] = "&";
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

inline static void decode_entity(char *entity) {
  ASSERT(entity && *entity);
  if (entity[0] == '#') {
    int code;
    if (entity[1] == 'x' || entity[1] == 'X') {
      code = strtol(entity + 2, NULL, 16);
    } else {
      code = strtol(entity + 1, NULL, 10);
    }
    if (code) {
      unsigned char buf[10];
      int size = utf32_to_utf8(buf, code);
      memcpy(entity, buf, size + 1);
      return;
    }
  } else {
    HtmlEntityMap::const_iterator iter = EntityMap.find(entity);
    if (iter != EntityMap.end()) {
      memcpy(entity, iter->second.c_str(), iter->second.length() + 1);
      return;
    }
  }

  // bad entity
  std::string s = "&";
  s += entity;
  s += ";";
  memcpy(entity, s.c_str(), s.length() + 1);
}

char *string_html_decode(const char *input, int &len, bool utf8, bool nbsp) {
  ASSERT(input);
  if (!*input) {
    return NULL;
  }

  if (!EntityMapInited) {
    Lock lock(EntityMapMutex);
    if (!EntityMapInited) {
      EntityMapInited = true;
      init_entity_table();
    }
  }

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
          found = true;
          char buf[16];
          memcpy(buf, p, l);
          buf[l] = '\0';
          if (strcmp(buf, "nbsp") == 0) {
            if (nbsp) {
              if (utf8) {
                l = 2;
                *q = '\xc2'; *(q+1) = '\xa0';
              } else {
                l = 1;
                *q = '\xa0';
              }
            } else {
              l = 6;
              memcpy(q, "&nbsp;", l);
            }
          } else {
            decode_entity(buf);
            l = strlen(buf);
            memcpy(q, buf, l);
          }
          p = t;
          q += l;
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

  Array ret;
  switch (which) {
  case HTML_ENTITIES:
    if (!EntityMapInited) {
      Lock lock(EntityMapMutex);
      if (!EntityMapInited) {
        EntityMapInited = true;
        init_entity_table();
      }
    }

    for (int j = 0; j < (int)(sizeof(entity_map)/sizeof(entity_map[0])); j++) {
      const html_entity_map &em = entity_map[j];
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

///////////////////////////////////////////////////////////////////////////////
}
