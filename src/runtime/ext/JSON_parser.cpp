/* JSON_parser.c */

/* 2005-12-30 */

/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include <runtime/ext/JSON_parser.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <system/gen/php/classes/stdclass.h>
#include <runtime/base/zend/utf8_decode.h>

#define MAX_LENGTH_OF_LONG 20
static const char long_min_digits[] = "9223372036854775808";

using namespace HPHP;

#ifdef true
# undef true
#endif

#ifdef false
# undef false
#endif

#define true  1
#define false 0

/*
    Characters are mapped into these 32 symbol classes. This allows for
    significant reductions in the size of the state transition table.
*/

/* error */
#define S_ERR -1

/* space */
#define S_SPA 0

/* other whitespace */
#define S_WSP 1

/* {  */
#define S_LBE 2

/* } */
#define S_RBE 3

/* [ */
#define S_LBT 4

/* ] */
#define S_RBT 5

/* : */
#define S_COL 6

/* , */
#define S_COM 7

/* " */
#define S_QUO 8

/* \ */
#define S_BAC 9

/* / */
#define S_SLA 10

/* + */
#define S_PLU 11

/* - */
#define S_MIN 12

/* . */
#define S_DOT 13

/* 0 */
#define S_ZER 14

/* 123456789 */
#define S_DIG 15

/* a */
#define S__A_ 16

/* b */
#define S__B_ 17

/* c */
#define S__C_ 18

/* d */
#define S__D_ 19

/* e */
#define S__E_ 20

/* f */
#define S__F_ 21

/* l */
#define S__L_ 22

/* n */
#define S__N_ 23

/* r */
#define S__R_ 24

/* s */
#define S__S_ 25

/* t */
#define S__T_ 26

/* u */
#define S__U_ 27

/* ABCDF */
#define S_A_F 28

/* E */
#define S_E   29

/* everything else */
#define S_ETC 30


/*
    This table maps the 128 ASCII characters into the 32 character classes.
    The remaining Unicode characters should be mapped to S_ETC.
*/
static const int ascii_class[128] = {
    S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,
    S_ERR, S_WSP, S_WSP, S_ERR, S_ERR, S_WSP, S_ERR, S_ERR,
    S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,
    S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,

    S_SPA, S_ETC, S_QUO, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,
    S_ETC, S_ETC, S_ETC, S_PLU, S_COM, S_MIN, S_DOT, S_SLA,
    S_ZER, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG,
    S_DIG, S_DIG, S_COL, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,

    S_ETC, S_A_F, S_A_F, S_A_F, S_A_F, S_E  , S_A_F, S_ETC,
    S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,
    S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,
    S_ETC, S_ETC, S_ETC, S_LBT, S_BAC, S_RBT, S_ETC, S_ETC,

    S_ETC, S__A_, S__B_, S__C_, S__D_, S__E_, S__F_, S_ETC,
    S_ETC, S_ETC, S_ETC, S_ETC, S__L_, S_ETC, S__N_, S_ETC,
    S_ETC, S_ETC, S__R_, S__S_, S__T_, S__U_, S_ETC, S_ETC,
    S_ETC, S_ETC, S_ETC, S_LBE, S_ETC, S_RBE, S_ETC, S_ETC
};

/*<fb>*/
static const int loose_ascii_class[128] = {
  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,
  S_ERR, S_WSP, S_WSP, S_ERR, S_ERR, S_WSP, S_ERR, S_ERR,
  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,
  S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR, S_ERR,

  S_SPA, S_ETC, S_QUO, S_ETC, S_ETC, S_ETC, S_ETC, S_QUO,
  S_ETC, S_ETC, S_ETC, S_PLU, S_COM, S_MIN, S_DOT, S_SLA,
  S_ZER, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG, S_DIG,
  S_DIG, S_DIG, S_COL, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,

  S_ETC, S_A_F, S_A_F, S_A_F, S_A_F, S_E  , S_A_F, S_ETC,
  S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,
  S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC, S_ETC,
  S_ETC, S_ETC, S_ETC, S_LBT, S_BAC, S_RBT, S_ETC, S_ETC,

  S_ETC, S__A_, S__B_, S__C_, S__D_, S__E_, S__F_, S_ETC,
  S_ETC, S_ETC, S_ETC, S_ETC, S__L_, S_ETC, S__N_, S_ETC,
  S_ETC, S_ETC, S__R_, S__S_, S__T_, S__U_, S_ETC, S_ETC,
  S_ETC, S_ETC, S_ETC, S_LBE, S_ETC, S_RBE, S_ETC, S_ETC
};
/*</fb>*/



/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. A new state is a number between
    0 and 29. An action is a negative number between -1 and -9. A JSON text is
    accepted if the end of the text is in state 9 and mode is MODE_DONE.
*/
static const int state_transition_table[30][31] = {
/* 0*/ { 0, 0,-8,-1,-6,-1,-1,-1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/* 1*/ { 1, 1,-1,-9,-1,-1,-1,-1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/* 2*/ { 2, 2,-8,-1,-6,-5,-1,-1, 3,-1,-1,-1,20,-1,21,22,-1,-1,-1,-1,-1,13,-1,17,-1,-1,10,-1,-1,-1,-1},
/* 3*/ { 3,-1, 3, 3, 3, 3, 3, 3,-4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 4*/ {-1,-1,-1,-1,-1,-1,-1,-1, 3, 3, 3,-1,-1,-1,-1,-1,-1, 3,-1,-1,-1, 3,-1, 3, 3,-1, 3, 5,-1,-1,-1},
/* 5*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 6, 6, 6, 6, 6, 6, 6, 6,-1,-1,-1,-1,-1,-1, 6, 6,-1},
/* 6*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 7, 7, 7,-1,-1,-1,-1,-1,-1, 7, 7,-1},
/* 7*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8, 8, 8, 8, 8,-1,-1,-1,-1,-1,-1, 8, 8,-1},
/* 8*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 3, 3, 3, 3, 3, 3, 3, 3,-1,-1,-1,-1,-1,-1, 3, 3,-1},
/* 9*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*10*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,11,-1,-1,-1,-1,-1,-1},
/*11*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,-1,-1,-1},
/*12*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*13*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*14*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,15,-1,-1,-1,-1,-1,-1,-1,-1},
/*15*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,16,-1,-1,-1,-1,-1},
/*16*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*17*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,18,-1,-1,-1},
/*18*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,-1},
/*19*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1},
/*20*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*21*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,23,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*22*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,23,22,22,-1,-1,-1,-1,24,-1,-1,-1,-1,-1,-1,-1,-1,24,-1},
/*23*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,23,23,-1,-1,-1,-1,24,-1,-1,-1,-1,-1,-1,-1,-1,24,-1},
/*24*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,25,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*25*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*26*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*27*/ {27,27,-1,-1,-1,-1,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*28*/ {28,28,-8,-1,-6,-1,-1,-1, 3,-1,-1,-1,20,-1,21,22,-1,-1,-1,-1,-1,13,-1,17,-1,-1,10,-1,-1,-1,-1},
/*29*/ {29,29,-1,-1,-1,-1,-1,-1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

/*<fb>*/
/*
  Alternate "loose" transition table to support unquoted keys.
*/
static const int loose_state_transition_table[31][31] = {
/* 0*/ { 0, 0,-8,-1,-6,-1,-1,-1, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/* 1*/ { 1, 1,-1,-9,-1,-1,-1,-1, 3,-1,-1,-1,-1,-1,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
/* 2*/ { 2, 2,-8,-1,-6,-5,-1,-1, 3,-1,-1,-1,20,-1,21,22,-1,-1,-1,-1,-1,13,-1,17,-1,-1,10,-1,-1,-1,-1},
/* 3*/ { 3,-1, 3, 3, 3, 3, 3, 3,-4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 4*/ {-1,-1,-1,-1,-1,-1,-1,-1, 3, 3, 3,-1,-1,-1,-1,-1,-1, 3,-1,-1,-1, 3,-1, 3, 3,-1, 3, 5,-1,-1,-1},
/* 5*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 6, 6, 6, 6, 6, 6, 6, 6,-1,-1,-1,-1,-1,-1, 6, 6,-1},
/* 6*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 7, 7, 7, 7, 7, 7, 7, 7,-1,-1,-1,-1,-1,-1, 7, 7,-1},
/* 7*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 8, 8, 8, 8, 8, 8, 8, 8,-1,-1,-1,-1,-1,-1, 8, 8,-1},
/* 8*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 3, 3, 3, 3, 3, 3, 3, 3,-1,-1,-1,-1,-1,-1, 3, 3,-1},
/* 9*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*10*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,11,-1,-1,-1,-1,-1,-1},
/*11*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,12,-1,-1,-1},
/*12*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*13*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*14*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,15,-1,-1,-1,-1,-1,-1,-1,-1},
/*15*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,16,-1,-1,-1,-1,-1},
/*16*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*17*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,18,-1,-1,-1},
/*18*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,19,-1,-1,-1,-1,-1,-1,-1,-1},
/*19*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 9,-1,-1,-1,-1,-1,-1,-1,-1},
/*20*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*21*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,23,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*22*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,23,22,22,-1,-1,-1,-1,24,-1,-1,-1,-1,-1,-1,-1,-1,24,-1},
/*23*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,23,23,-1,-1,-1,-1,24,-1,-1,-1,-1,-1,-1,-1,-1,24,-1},
/*24*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,25,25,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*25*/ {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*26*/ { 9, 9,-1,-7,-1,-5,-1,-3,-1,-1,-1,-1,-1,-1,26,26,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*27*/ {27,27,-1,-1,-1,-1,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
/*28*/ {28,28,-8,-1,-6,-5,-1,-1, 3,-1,-1,-1,20,-1,21,22,-1,-1,-1,-1,-1,13,-1,17,-1,-1,10,-1,-1,-1,-1},
/*29*/ {29,29,-1,-7,-1,-1,-1,-7, 3,-1,-1,-1,-1,-1,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30},
/*30*/ {30,-1,30,30,30,30,-10,30,-4,4,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30}
};
/*</fb>*/


#define JSON_PARSER_MAX_DEPTH 512

/**
 * A stack maintains the states of nested structures.
 */
struct json_parser {
  int the_stack[JSON_PARSER_MAX_DEPTH];
  Variant the_zstack[JSON_PARSER_MAX_DEPTH];
  int the_top;
  int the_mark; // the watermark
};

IMPLEMENT_THREAD_LOCAL(json_parser, s_json_parser);

class JsonParserCleaner {
public:
  JsonParserCleaner(json_parser *json) : m_json(json) {}
  ~JsonParserCleaner() {
    for (int i = 0; i <= m_json->the_mark; i++) {
      m_json->the_zstack[i].unset();
    }
  }
private:
  json_parser *m_json;
};

/**
 * These modes can be pushed on the PDA stack.
 */
#define MODE_DONE   1
#define MODE_KEY    2
#define MODE_OBJECT 3
#define MODE_ARRAY  4

/**
 * Push a mode onto the stack. Return false if there is overflow.
 */
static int push(json_parser *json, int mode) {
  json->the_top += 1;
  if (json->the_top >= JSON_PARSER_MAX_DEPTH) {
    return false;
  }
  json->the_stack[json->the_top] = mode;
  if (json->the_top > json->the_mark) {
    json->the_mark = json->the_top;
  }
  return true;
}


/**
 * Pop the stack, assuring that the current mode matches the expectation.
 * Return false if there is underflow or if the modes mismatch.
 */
static int pop(json_parser *json, int mode) {
  if (json->the_top < 0 || json->the_stack[json->the_top] != mode) {
    return false;
  }
  json->the_stack[json->the_top] = 0;
  json->the_top -= 1;
  return true;
}

static int dehexchar(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - ('A' - 10);
  if (c >= 'a' && c <= 'f') return c - ('a' - 10);
  return -1;
}

static void json_create_zval(Variant &z, StringBuffer &buf, int type) {
  switch (type) {
  case KindOfInt64:
    {
      const char *p = buf.data();
      ASSERT(p);
      if (p == NULL) {
        z = 0LL;
        return;
      }

      bool neg = (buf.charAt(0) == '-');

      int len = buf.size();
      if (neg) len--;
      if (len >= MAX_LENGTH_OF_LONG - 1) {
        if (len == MAX_LENGTH_OF_LONG - 1) {
          int cmp = strcmp(p + (neg ? 1 : 0), long_min_digits);
          if (!(cmp < 0 || (cmp == 0 && neg))) {
            z = strtod(p, NULL);
            return;
          }
        } else {
          z = strtod(p, NULL);
          return;
        }
      }
      z = strtoll(buf.data(), NULL, 10);
    }
    break;
  case KindOfDouble:
    z = buf.data() ? strtod(buf.data(), NULL) : 0.0;
    break;
  case KindOfString:
    z = buf.detach();
    break;
  case KindOfBoolean:
    z = (buf.data() && (*buf.data() == 't'));
    break;
  default:
    z = null;
    break;
  }
}

static void utf16_to_utf8(StringBuffer &buf, unsigned short utf16) {
  if (utf16 < 0x80) {
    buf += (char)utf16;
  } else if (utf16 < 0x800) {
    buf += (char)(0xc0 | (utf16 >> 6));
    buf += (char)(0x80 | (utf16 & 0x3f));
  } else if ((utf16 & 0xfc00) == 0xdc00
             && buf.size() >= 3
             && ((unsigned char)buf.charAt(buf.size() - 3)) == 0xed
             && ((unsigned char)buf.charAt(buf.size() - 2) & 0xf0) == 0xa0
             && ((unsigned char)buf.charAt(buf.size() - 1) & 0xc0) == 0x80) {
    /* found surrogate pair */
    unsigned long utf32;

    utf32 = (((buf.charAt(buf.size() - 2) & 0xf) << 16)
             | ((buf.charAt(buf.size() - 1) & 0x3f) << 10)
             | (utf16 & 0x3ff)) + 0x10000;
    buf.resize(buf.size() - 3);

    buf += (char)(0xf0 | (utf32 >> 18));
    buf += (char)(0x80 | ((utf32 >> 12) & 0x3f));
    buf += (char)(0x80 | ((utf32 >> 6) & 0x3f));
    buf += (char)(0x80 | (utf32 & 0x3f));
  } else {
    buf += (char)(0xe0 | (utf16 >> 12));
    buf += (char)(0x80 | ((utf16 >> 6) & 0x3f));
    buf += (char)(0x80 | (utf16 & 0x3f));
  }
}

static void object_set(Variant &var, StringBuffer &key, CVarRef value,
                       int assoc) {
  String data = key.detach();
  if (!assoc) {
    // We know it is stdClass, and everything is public (and dynamic).
    if (data.empty()) {
      var.getObjectData()->o_setPublic("_empty_", value);
    } else {
      var.getObjectData()->o_setPublic(data, value);
    }
  } else {
    var.set(data, value);
  }
}

static void attach_zval(json_parser *json, int up, int cur, StringBuffer &key,
                        int assoc) {
  Variant &root = json->the_zstack[up];
  Variant &child =  json->the_zstack[cur];
  int up_mode = json->the_stack[up];

  if (up_mode == MODE_ARRAY) {
    root.append(ref(child));
  } else if (up_mode == MODE_OBJECT) {
    object_set(root, key, ref(child), assoc);
  }
}

#define SWAP_BUFFERS(from, to) do { \
    StringBuffer *tmp = from;       \
    from = to;                      \
    to = tmp;                       \
  } while(0);
#define JSON_RESET_TYPE() do { type = -1; } while(0);
#define JSON(x) the_json->x

/**
 * The JSON_parser takes a UTF-8 encoded string and determines if it is a
 * syntactically correct JSON text. Along the way, it creates a PHP variable.
 *
 * It is implemented as a Pushdown Automaton; that means it is a finite state
 * machine with a stack.
 */
bool JSON_parser(Variant &z, const char *p, int length, bool assoc/*<fb>*/,
                 bool loose/*</fb>*/) {
  int b;  /* the next character */
  int c;  /* the next character class */
  int s;  /* the next state */
  json_parser *the_json = s_json_parser.get(); /* the parser state */
  JsonParserCleaner cleaner(the_json);
  int the_state = 0;

  /*<fb>*/
  int qchr = 0;
  int const *byte_class;
  if (loose) {
    byte_class = loose_ascii_class;
  } else {
    byte_class = ascii_class;
  }
  /*</fb>*/

  StringBuffer sb_buf(127), sb_key(127);
  StringBuffer *buf = &sb_buf;
  StringBuffer *key = &sb_key;

  int type = -1;
  unsigned short utf16 = 0;

  JSON(the_mark) = JSON(the_top) = -1;
  push(the_json, MODE_DONE);

  UTF8To16Decoder decoder(p, length, loose);
  for (;;) {
    b = decoder.decode();
    if (b == UTF8_END) break; // UTF-8 decoding finishes successfully.
    if (b == UTF8_ERROR) {
      return false;
    }
    ASSERT(b >= 0);

    if ((b & 127) == b) {
      /*<fb>*/
      c = byte_class[b];
      /*</fb>*/
      if (c <= S_ERR) {
        return false;
      }
    } else {
      c = S_ETC;
    }
    /*
      Get the next state from the transition table.
    */

    /*<fb>*/
    if (loose) {
      s = loose_state_transition_table[the_state][c];
    } else {
      s = state_transition_table[the_state][c];
    }

    if (s == -4) {
      if (b != qchr) {
        s = 3;
      } else {
        qchr = 0;
      }
    }
    /*</fb>*/

    if (s < 0) {
      /*
        Perform one of the predefined actions.
      */
      switch (s) {
        /*
          empty }
        */
      case -9:
        if (!pop(the_json, MODE_KEY)) {
          return false;
        }
        the_state = 9;
        break;
        /*
          {
        */
      case -8:
        if (!push(the_json, MODE_KEY)) {
          return false;
        }

        the_state = 1;
        if (JSON(the_top) > 0) {
          Variant &top = JSON(the_zstack)[JSON(the_top)];
          if (JSON(the_top) == 1) {
            top.assignRef(z);
          } else {
            top.unset();
          }
          if (!assoc) {
            top = NEWOBJ(c_stdClass)();
          } else {
            top = Array::Create();
          }
          if (JSON(the_top) > 1) {
            attach_zval(the_json, JSON(the_top-1), JSON(the_top), *key, assoc);
          }
          JSON_RESET_TYPE();
        }
        break;
        /*
          }
        */
      case -7:
        /*** BEGIN Facebook: json_utf8_loose ***/
        /*
          If this is a trailing comma in an object definition,
          we're in MODE_KEY. In that case, throw that off the
          stack and restore MODE_OBJECT so that we pretend the
          trailing comma just didn't happen.
        */
        if (loose) {
          if (pop(the_json, MODE_KEY)) {
            push(the_json, MODE_OBJECT);
          }
        }
        /*** END Facebook: json_utf8_loose ***/

        if (type != -1 &&
            JSON(the_stack)[JSON(the_top)] == MODE_OBJECT) {
          Variant mval;
          json_create_zval(mval, *buf, type);
          Variant &top = JSON(the_zstack)[JSON(the_top)];
          object_set(top, *key, mval, assoc);
          buf->reset();
          JSON_RESET_TYPE();
        }


        if (!pop(the_json, MODE_OBJECT)) {
          return false;
        }
        the_state = 9;
        break;
        /*
          [
        */
      case -6:
        if (!push(the_json, MODE_ARRAY)) {
          return false;
        }
        the_state = 2;

        if (JSON(the_top) > 0) {
          if (JSON(the_top) == 1) {
            JSON(the_zstack)[JSON(the_top)].assignRef(z);
          } else {
            JSON(the_zstack)[JSON(the_top)].unset();
          }
          JSON(the_zstack)[JSON(the_top)] = Array::Create();
          if (JSON(the_top) > 1) {
            attach_zval(the_json, JSON(the_top-1), JSON(the_top), *key, assoc);
          }
          JSON_RESET_TYPE();
        }
        break;
        /*
          ]
        */
      case -5:
        {
          if (type != -1 &&
               JSON(the_stack)[JSON(the_top)] == MODE_ARRAY) {
            Variant mval;
            json_create_zval(mval, *buf, type);
            JSON(the_zstack)[JSON(the_top)].append(mval);
            buf->reset();
            JSON_RESET_TYPE();
          }

          if (!pop(the_json, MODE_ARRAY)) {
            return false;
          }
          the_state = 9;
        }
        break;
        /*
          "
        */
      case -4:
        switch (JSON(the_stack)[JSON(the_top)]) {
        case MODE_KEY:
          the_state = 27;
          SWAP_BUFFERS(buf, key);
          JSON_RESET_TYPE();
          break;
        case MODE_ARRAY:
        case MODE_OBJECT:
          the_state = 9;
          break;
        case MODE_DONE:
          if (type == KindOfString) {
            z = buf->detach();
            the_state = 9;
            break;
          }
          /* fall through if not KindOfString */
        default:
          return false;
        }
        break;
        /*
          ,
        */
      case -3:
        {
          Variant mval;
          if (type != -1 &&
              (JSON(the_stack)[JSON(the_top)] == MODE_OBJECT ||
               JSON(the_stack)[JSON(the_top)] == MODE_ARRAY)) {
            json_create_zval(mval, *buf, type);
          }

          switch (JSON(the_stack)[JSON(the_top)]) {
          case MODE_OBJECT:
            if (pop(the_json, MODE_OBJECT) &&
                push(the_json, MODE_KEY)) {
              if (type != -1) {
                Variant &top = JSON(the_zstack)[JSON(the_top)];
                object_set(top, *key, mval, assoc);
              }
              the_state = 29;
            }
            break;
          case MODE_ARRAY:
            if (type != -1) {
              JSON(the_zstack)[JSON(the_top)].append(mval);
            }
            the_state = 28;
            break;
          default:
            return false;
          }
          buf->reset();
          JSON_RESET_TYPE();
        }
        break;

        /*<fb>*/
        /*
          : (after unquoted string)
        */
      case -10:
        if (JSON(the_stack)[JSON(the_top)] == MODE_KEY) {
          the_state = 27;
          SWAP_BUFFERS(buf, key);
          JSON_RESET_TYPE();
          s = -2;
        } else {
          s = 3;
          break;
        }
        /*</fb>*/

        /*
          :
        */
      case -2:
        if (pop(the_json, MODE_KEY) && push(the_json, MODE_OBJECT)) {
          the_state = 28;
          break;
        }
        /*
          syntax error
        */
      case -1:
        return false;
      }
    } else {
      /*
        Change the state and iterate.
      */
      if (type == KindOfString) {
        if (/*<fb>*/(/*</fb>*/s == 3/*<fb>*/ || s == 30)/*</fb>*/ &&
            the_state != 8) {
          if (the_state != 4) {
            utf16_to_utf8(*buf, b);
          } else {
            switch (b) {
            case 'b': buf->append('\b'); break;
            case 't': buf->append('\t'); break;
            case 'n': buf->append('\n'); break;
            case 'f': buf->append('\f'); break;
            case 'r': buf->append('\r'); break;
            default:
              utf16_to_utf8(*buf, b);
              break;
            }
          }
        } else if (s == 6) {
          utf16 = dehexchar(b) << 12;
        } else if (s == 7) {
          utf16 += dehexchar(b) << 8;
        } else if (s == 8) {
          utf16 += dehexchar(b) << 4;
        } else if (s == 3 && the_state == 8) {
          utf16 += dehexchar(b);
          utf16_to_utf8(*buf, utf16);
        }
      } else if ((type < 0 || type == KindOfNull) &&
                 (c == S_DIG || c == S_ZER)) {
        type = KindOfInt64;
        buf->append((char)b);
      } else if (type == KindOfInt64 && s == 24) {
        type = KindOfDouble;
        buf->append((char)b);
      } else if ((type < 0 || type == KindOfNull || type == KindOfInt64) &&
                 c == S_DOT) {
        type = KindOfDouble;
        buf->append((char)b);
      } else if (type != KindOfString && c == S_QUO) {
        type = KindOfString;
        /*<fb>*/qchr = b;/*</fb>*/
      } else if ((type < 0 || type == KindOfNull || type == KindOfInt64 ||
                  type == KindOfDouble) &&
                 ((the_state == 12 && s == 9) ||
                  (the_state == 16 && s == 9))) {
        type = KindOfBoolean;
      } else if (type < 0 && the_state == 19 && s == 9) {
        type = KindOfNull;
      } else if (type != KindOfString && c > S_WSP) {
        utf16_to_utf8(*buf, b);
      }

      the_state = s;
    }
  }

  return the_state == 9 && pop(the_json, MODE_DONE);
}
