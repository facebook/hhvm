%{
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

#include <errno.h>
#include "hphp/runtime/base/ini-parser/zend-ini.tab.hpp"
#include "hphp/runtime/base/ini-parser/zend-ini.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/util/logger.h"

using namespace HPHP;

/* Globals Macros */
struct ZendINIGlobals {
  int scanner_mode;
  std::string filename;
  int lineno;
  IniSetting::PFN_PARSER_CALLBACK callback;
  void *arg;
  YY_BUFFER_STATE state;
};
static ZendINIGlobals s_zend_ini;
#define SCNG(v) s_zend_ini.v

/* Eat leading whitespace */
#define EAT_LEADING_WHITESPACE() \
  while (yytext[0]) {                             \
    if (yytext[0] == ' ' || yytext[0] == '\t') {  \
      yytext++;                                   \
      yyleng--;                                   \
    } else {                                      \
      break;                                      \
    }                                             \
  }

/* Eat trailing whitespace + extra char */
#define EAT_TRAILING_WHITESPACE_EX(ch)            \
  while (yyleng > 0 && (                          \
    (ch != 'X' && yytext[yyleng - 1] ==  ch) ||   \
    yytext[yyleng - 1] == '\n' ||                 \
    yytext[yyleng - 1] == '\r' ||                 \
    yytext[yyleng - 1] == '\t' ||                 \
    yytext[yyleng - 1] == ' ')                    \
  ) {                                             \
    yyleng--;                                     \
  }

/* Eat trailing whitespace */
#define EAT_TRAILING_WHITESPACE()  EAT_TRAILING_WHITESPACE_EX('X')

#define RETURN_TOKEN(type, str, len) {            \
  *ini_lval = String(str, len, CopyString);       \
  return type;                                    \
}

static void zend_ini_escape_string(String &lval, char *str, int len,
                                   char quote_type) {
  register char *s, *t;
  char *end;

  String sval(str, len, CopyString);
  lval = sval;

  /* convert escape sequences */
  s = t = (char*)sval.data();
  end = s + sval.size();

  int length = sval.size();

  while (s < end) {
    if (*s == '\\') {
      s++;
      if (s >= end) {
        *t++ = '\\';
        continue;
      }
      switch (*s) {
        case '"':
          if (*s != quote_type) {
            *t++ = '\\';
            *t++ = *s;
            break;
          }
          /* fallthrough */
        case '\\':
        case '$':
          *t++ = *s;
          length--;
          break;
        default:
          *t++ = '\\';
          *t++ = *s;
          break;
      }
    } else {
      *t++ = *s;
    }
    if (*s == '\n' || (*s == '\r' && (*(s+1) != '\n'))) {
      SCNG(lineno)++;
    }
    s++;
  }
  *t = 0;

  if (length != lval.size()) {
    lval.setSize(length);
  }
}

#define YY_USE_PROTOS
#define YY_DECL int ini_lex_impl(String *ini_lval, void *loc)

#define GOTO_RESTART 9999

int ini_lex_impl(String *ini_lval, void *loc);
int ini_lex(String *ini_lval, void *loc) {
restart:
  int ret = ini_lex_impl(ini_lval, loc);
  if (ret == GOTO_RESTART) goto restart;
  return ret;
}

%}

%x ST_OFFSET
%x ST_SECTION_VALUE
%x ST_VALUE
%x ST_SECTION_RAW
%x ST_DOUBLE_QUOTES
%x ST_VARNAME
%x ST_RAW
%option noyywrap
%option stack

LNUM [0-9]+
DNUM ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
NUMBER [-]?{LNUM}|{DNUM}
ANY_CHAR (.|[\n\t])
NEWLINE  ("\r"|"\n"|"\r\n")
TABS_AND_SPACES [ \t]
WHITESPACE [ \t]+
CONSTANT [a-zA-Z][a-zA-Z0-9_]*
LABEL [^=\n\r\t;&|^$~(){}!"\[]+
TOKENS [:,.\[\]"'()|^&+-/*=%$!~<>?@{}]
OPERATORS [&|^~()!]
DOLLAR_CURLY "${"

SECTION_RAW_CHARS [^\]\n\r]
SINGLE_QUOTED_CHARS [^']
RAW_VALUE_CHARS [^\n\r;\000]

LITERAL_DOLLAR ("$"([^{\000]|("\\"{ANY_CHAR})))
VALUE_CHARS         ([^$= \t\n\r;&|~()!"'\000]|{LITERAL_DOLLAR})
SECTION_VALUE_CHARS ([^$\n\r;"'\]\\]|("\\"{ANY_CHAR})|{LITERAL_DOLLAR})
DOUBLE_QUOTES_CHARS ([^$\"]+|"\\"{ANY_CHAR}|"$"[^{])

%%

<INITIAL>"[" {
/* Section start */
  /* Enter section data lookup state */
  if (SCNG(scanner_mode) == IniSetting::RawScanner) {
    yy_push_state(ST_SECTION_RAW);
  } else {
    yy_push_state(ST_SECTION_VALUE);
  }
  return TC_SECTION;
}

<ST_VALUE,ST_SECTION_VALUE,ST_OFFSET>"'"{SINGLE_QUOTED_CHARS}+"'" {
/* Raw string */
  /* Eat leading and trailing single quotes */
  if (yytext[0] == '\'' && yytext[yyleng - 1] == '\'') {
    yytext++;
    yyleng = yyleng - 2;
  }
  RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_SECTION_RAW,ST_SECTION_VALUE>"]"{TABS_AND_SPACES}*{NEWLINE}? {
/* End of section */
  BEGIN(INITIAL);
  SCNG(lineno)++;
  return ']';
}

<INITIAL>{LABEL}"["{TABS_AND_SPACES}* {
/* Start of option with offset */
  /* Eat leading whitespace */
  EAT_LEADING_WHITESPACE();

  /* Eat trailing whitespace and [ */
  EAT_TRAILING_WHITESPACE_EX('[');

  /* Enter offset lookup state */
  yy_push_state(ST_OFFSET);

  RETURN_TOKEN(TC_OFFSET, yytext, yyleng);
}

<ST_OFFSET>{TABS_AND_SPACES}*"]" {
/* End of section or an option offset */
  BEGIN(INITIAL);
  return ']';
}

<ST_DOUBLE_QUOTES,ST_SECTION_VALUE,ST_VALUE,ST_OFFSET>{DOLLAR_CURLY} {
/* Variable start */
  yy_push_state(ST_VARNAME);
  return TC_DOLLAR_CURLY;
}

<ST_VARNAME>{LABEL} {
/* Variable name */
  /* Eat leading whitespace */
  EAT_LEADING_WHITESPACE();

  /* Eat trailing whitespace */
  EAT_TRAILING_WHITESPACE();

  RETURN_TOKEN(TC_VARNAME, yytext, yyleng);
}

<ST_VARNAME>"}" { /* Variable end */
  yy_pop_state();
  return '}';
}

<INITIAL,ST_VALUE>(?i:"true"|"on"|"yes"){TABS_AND_SPACES}* {
/* TRUE value (when used outside option value/offset this causes error!) */
  RETURN_TOKEN(BOOL_TRUE, "1", 1);
}

<INITIAL,ST_VALUE>(?i:"false"|"off"|"no"|"none"|"null"){TABS_AND_SPACES}* {
/* FALSE value (when used outside option value/offset this causes error!)*/
  RETURN_TOKEN(BOOL_FALSE, "", 0);
}

<INITIAL>{LABEL} {
/* Get option name */
  /* Eat leading whitespace */
  EAT_LEADING_WHITESPACE();

  /* Eat trailing whitespace */
  EAT_TRAILING_WHITESPACE();

  RETURN_TOKEN(TC_LABEL, yytext, yyleng);
}

<INITIAL>{TABS_AND_SPACES}*[=]{TABS_AND_SPACES}* {
/* Start option value */
  if (SCNG(scanner_mode) == IniSetting::RawScanner) {
    yy_push_state(ST_RAW);
  } else {
    yy_push_state(ST_VALUE);
  }
  return '=';
}

<ST_RAW>{RAW_VALUE_CHARS}+ {
/* Raw value, only used when SCNG(scanner_mode) == IniSetting::RawScanner. */
  /* Eat leading and trailing double quotes */
  if (yytext[0] == '"' && yytext[yyleng - 1] == '"') {
    yytext++;
    yyleng = yyleng - 2;
  }
  RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_SECTION_RAW>{SECTION_RAW_CHARS}+ {
/* Raw value, only used when SCNG(scanner_mode) == IniSetting::RawScanner. */
  RETURN_TOKEN(TC_RAW, yytext, yyleng);
}

<ST_VALUE,ST_RAW>{TABS_AND_SPACES}*{NEWLINE} {
/* End of option value */
  BEGIN(INITIAL);
  SCNG(lineno)++;
  return END_OF_LINE;
}

<ST_SECTION_VALUE,ST_VALUE,ST_OFFSET>{CONSTANT} {
/* Get constant option value */
  RETURN_TOKEN(TC_CONSTANT, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_VALUE,ST_OFFSET>{NUMBER} {
/* Get number option value as string */
  RETURN_TOKEN(TC_NUMBER, yytext, yyleng);
}

<INITIAL>{TOKENS} {
/* Disallow these chars outside option values */
  return yytext[0];
}

<ST_VALUE>{OPERATORS}{TABS_AND_SPACES}* {
/* Boolean operators */
  return yytext[0];
}

<ST_VALUE>[=] {
/* Make = used in option value to trigger error */
  yyless(0);
  BEGIN(INITIAL);
  return END_OF_LINE;
}

<ST_VALUE>{VALUE_CHARS}+ {
/* Get everything else as option/offset value */
  RETURN_TOKEN(TC_STRING, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_OFFSET>{SECTION_VALUE_CHARS}+ {
/* Get rest as section/offset value */
  RETURN_TOKEN(TC_STRING, yytext, yyleng);
}

<ST_SECTION_VALUE,ST_VALUE,ST_OFFSET>{TABS_AND_SPACES}*["] {
/* Double quoted '"' string start */
  yy_push_state(ST_DOUBLE_QUOTES);
  return '"';
}

<ST_DOUBLE_QUOTES>["]{TABS_AND_SPACES}* {
/* Double quoted '"' string ends */
  yy_pop_state();
  return '"';
}

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS} {
/* Escape double quoted string contents */
  if (yyleng > 1 && yytext[yyleng-2] == '$' && yytext[yyleng-1] == '"') {
    yyless(yyleng-1);
  }

  zend_ini_escape_string(*ini_lval, yytext, yyleng, '"');
  return TC_QUOTED_STRING;
}

<ST_SECTION_VALUE,ST_VALUE,ST_OFFSET>{WHITESPACE} {
  RETURN_TOKEN(TC_WHITESPACE, yytext, yyleng);
}

<INITIAL,ST_RAW>{TABS_AND_SPACES}+ {
  /* eat whitespace */
  return GOTO_RESTART;
}

<INITIAL>{TABS_AND_SPACES}*{NEWLINE} {
  SCNG(lineno)++;
  return END_OF_LINE;
}

<INITIAL,ST_VALUE,ST_RAW>{TABS_AND_SPACES}*[;][^\r\n]*{NEWLINE}? {
/* Comment */
  BEGIN(INITIAL);
  SCNG(lineno)++;
  return END_OF_LINE;
}

<INITIAL>{TABS_AND_SPACES}*[#][^\r\n]*{NEWLINE}? {
/* #Comment */
  Logger::Error("Comments starting with '#' are deprecated in %s on line %d",
                SCNG(filename).data(), SCNG(lineno));
  BEGIN(INITIAL);
  SCNG(lineno)++;
  return END_OF_LINE;
}

<ST_VALUE,ST_RAW><<EOF>> {
/* End of option value (if EOF is reached before EOL) */
  BEGIN(INITIAL);
  return END_OF_LINE;
}

%%

static void __attribute__((__unused__))
suppress_defined_but_not_used_warnings() {
  yyunput(0, 0);
  yy_top_state();
}

void zend_ini_scan(const String& str, int scanner_mode, const String& filename,
                   IniSetting::PFN_PARSER_CALLBACK callback, void *arg) {
  SCNG(scanner_mode) = scanner_mode;
  SCNG(filename) = filename.data();
  SCNG(lineno) = 1;
  SCNG(callback) = callback;
  SCNG(arg) = arg;

  BEGIN(INITIAL);

  /* Eat any UTF-8 BOM we find in the first 3 bytes */
  if (str.size() > 3 && memcmp(str.data(), "\xef\xbb\xbf", 3) == 0) {
    SCNG(state) = yy_scan_string(str.data() + 3);
  } else {
    SCNG(state) = yy_scan_string(str.data());
  }
}

void zend_ini_scan_cleanup() {
  yy_delete_buffer(SCNG(state));
  SCNG(state) = nullptr;
}

void zend_ini_callback(String *arg1, String *arg2, String *arg3,
                       int callback_type) {
  SCNG(callback)(arg1, arg2, arg3, callback_type, SCNG(arg));
}

void ini_error(const char *msg) {
  StringBuffer smsg;
  if (!SCNG(filename).empty()) {
    smsg.printf("%s in %s on line %d\n", msg, SCNG(filename).data(),
                SCNG(lineno));
  } else {
    smsg.append("Invalid configuration directive\n");
  }

  Logger::Warning("%s", smsg.data());
}
