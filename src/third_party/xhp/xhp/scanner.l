/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
  | Copyright (c) 2009 - 2010 Facebook, Inc. (http://www.facebook.com)          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.00 of the Zend license,     |
  | that is bundled with this package in the file LICENSE.ZEND, and is   |
  | available through the world-wide-web at the following url:           |
  | http://www.zend.com/license/2_00.txt.                                |
  | If you did not receive a copy of the Zend license and are unable to  |
  | obtain it through the world-wide-web, please send a note to          |
  | license@zend.com so we can mail you a copy immediately.              |
  +----------------------------------------------------------------------+
*/

%{
#include "xhp.hpp"
#include <string.h>
#define push_state(s) xhp_new_push_state(s, yyg)
#define pop_state() xhp_new_pop_state(yyg)
#define set_state(s) xhp_set_state(s, yyg)
#define last_curly_token() (yyextra->curly_stack.empty() ? 0 : yyextra->curly_stack.top())
#define last_token() yyextra->last_token

#define YY_USER_ACTION \
  if (!yyg->yy_more_len) \
    yyextra->first_lineno = yyextra->lineno;
#define tok(t) \
  if (yyextra->has_doc_block) { \
    *yylval = yyextra->doc_block + code_rope(yytext, yyextra->first_lineno, yyextra->lineno - yyextra->first_lineno); \
    yyextra->has_doc_block = false; \
  } else { \
    *yylval = code_rope(yytext, yyextra->first_lineno, yyextra->lineno - yyextra->first_lineno); \
  } \
  return yy_token(t, yyg)
#ifdef DEBUG
  static void yy_log_token(int tok);
  #define tokt(t) *yylval = t; push_state(XHP_AFTER_ENT); yy_log_token(T_XHP_TEXT); return yyextra->last_token = T_XHP_TEXT;
#else
  #define tokt(t) *yylval = t; push_state(XHP_AFTER_ENT); return yyextra->last_token = T_XHP_TEXT;
#endif
#define YY_USER_INIT \
  if (yyextra->insert_token) { \
    yyg->yy_init = 0; \
    int ft = yyextra->insert_token; \
    yyextra->insert_token = 0; \
    return yy_token(ft, yyg); \
  }

using namespace std;

const char* yytokname(int tok);
static int yy_token(int tok, struct yyguts_t* yyg);
static void yy_scan_newlines(const char* text, struct yyguts_t* yyg);

static bool utf8ize(uint32_t v, char* buf /* [5] */) {
  if (v <= 0x7f) { // 0xxxxxxx
    buf[0] = v;
    buf[1] = 0;
  } else if (v <= 0x7ff) { // 110yyyxx 10xxxxxx
    buf[0] = 0xc0 | (v >> 6);
    buf[1] = 0x80 | (v & 0x3f);
    buf[2] = 0;
  } else if (v <= 0xffff) { // 1110yyyy 10yyyyxx 10xxxxxx
    buf[0] = 0xe0 | (v >> 12);
    buf[1] = 0x80 | ((v >> 6) & 0x3f);
    buf[2] = 0x80 | (v & 0x3f);
    buf[3] = 0;
  } else if (v <= 0x1fffff) { // 11110zzz 10zzyyyy 10yyyyxx 10xxxxxx
    buf[0] = 0xf0 | (v >> 18);
    buf[1] = 0x80 | ((v >> 12) & 0x3f);
    buf[2] = 0x80 | ((v >> 6) & 0x3f);
    buf[3] = 0x80 | (v & 0x3f);
    buf[4] = 0;
  } else {
    return false;
  }
  return true;
}

%}

%option prefix="xhp"
%option reentrant
%option case-insensitive
%option noyywrap nodefault
%option stack
%option bison-bridge
%option 8bit

 /* I think an interactive scanner is required because of the bison state 
  * pushing we do. I'm putting an explicit interactive declaration here in case
  * someone tries adding -CF or whatever to the make flags. */
%option interactive

%s PHP
%s PHP_COMMENT
%s PHP_EOL_COMMENT
%s PHP_DOC_COMMENT
%s PHP_HEREDOC_START
%s PHP_HEREDOC_NSTART
%s PHP_HEREDOC_NEWLINE
%s PHP_HEREDOC_DATA
%s PHP_NO_RESERVED_WORDS
%s PHP_NO_RESERVED_WORDS_PERSIST
%s XHP_LABEL
%s XHP_LABEL_WHITESPACE
%s XHP_ATTRS
%s XHP_ATTR_VAL
%s XHP_AFTER_ENT
%s XHP_CHILD
%s XHP_CHILD_START
%s XHP_INVALID_ENTITY
%s XHP_ATTR_TYPE_DECL
%s XHP_CHILDREN_DECL

LNUM [0-9]+
DNUM ([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)
EXPONENT_DNUM (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM "0x"[0-9a-fA-F]+

LABEL [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
BYTE (.|\n)

WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
NEWLINE ("\r\n"|"\n"|"\r")

%%

<XHP_ATTR_TYPE_DECL>{
  bool tok(T_XHP_BOOLEAN);
  int tok(T_XHP_NUMBER);
  var tok(T_VAR);
  array tok(T_XHP_ARRAY);
  string tok(T_XHP_STRING);
  enum tok(T_XHP_ENUM);
  @required tok(T_XHP_REQUIRED);
  "(" tok('(');
  ":" tok(T_XHP_COLON);
}

 /* Open / close PHP + inline HTML */
<INITIAL>{
  "<?php"([ \t]|{NEWLINE}) {
    yy_scan_newlines(yytext + 5, yyg);
    tok(T_OPEN_TAG);
  }
  "<?" {
    if (yyextra->short_tags) {
      tok(T_OPEN_TAG);
    } else {
      tok(T_INLINE_HTML);
    }
  }
  "<?=" {
    if (yyextra->short_tags) {
      tok(T_OPEN_TAG_WITH_ECHO);
    } else {
      tok(T_INLINE_HTML);
    }
  }
  "<%" {
    if (yyextra->asp_tags) {
      tok(T_OPEN_TAG);
    } else {
      tok(T_INLINE_HTML);
    }
  }
  "<%=" {
    if (yyextra->asp_tags) {
      tok(T_OPEN_TAG_WITH_ECHO);
    } else {
      tok(T_INLINE_HTML);
    }
  }
  "<"|[^<]* {
    yy_scan_newlines(yytext, yyg);
    tok(T_INLINE_HTML);
  }
}
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST>{
  ("?>"|"</script"{WHITESPACE}*">"){NEWLINE}? {
    yy_scan_newlines(yytext + 2, yyg);
    tok(T_CLOSE_TAG);
  }
  "%>" {
    if (yyextra->asp_tags) {
      tok(T_CLOSE_TAG);
    } else {
      yyless(1);
      tok(yytext[0]);
    }
  }
}

 /* Comments and whitespace */
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST,XHP_CHILDREN_DECL,XHP_ATTR_TYPE_DECL>{
  "#"|"//" {
    push_state(PHP_EOL_COMMENT);
    yymore();
  }
  "/**"{WHITESPACE} {
    yy_scan_newlines(yytext + 3, yyg);
    push_state(PHP_DOC_COMMENT);
    yymore();
  }
  "/*" {
    push_state(PHP_COMMENT);
    yymore();
  }
  {WHITESPACE}+ yy_scan_newlines(yytext, yyg);
}
<PHP_EOL_COMMENT>{
  {NEWLINE} {
    ++yyextra->lineno;
    pop_state();
  }
  [^\r\n?]+ yymore();
  "?>" {
    yyless(yyleng - 2);
    pop_state();
  }
  . yymore();
}
<PHP_DOC_COMMENT,PHP_COMMENT>{
  {NEWLINE} {
    ++yyextra->lineno;
    yymore();
  }
  [^*\r\n]+|"*" yymore();
}
<PHP_DOC_COMMENT>"*/" {
  yyextra->doc_block = code_rope(yytext, yyextra->first_lineno, yyextra->lineno - yyextra->first_lineno);
  yyextra->has_doc_block = true;
  pop_state();
}
<PHP_COMMENT>"*/" pop_state();

 /* Reserved words */
<PHP>{
  include tok(T_INCLUDE);
  include_once tok(T_INCLUDE_ONCE);
  eval tok(T_EVAL);
  require tok(T_REQUIRE);
  require_once tok(T_REQUIRE_ONCE);
  or tok(T_LOGICAL_OR);
  xor tok(T_LOGICAL_XOR);
  and tok(T_LOGICAL_AND);
  print tok(T_PRINT);
  instanceof tok(T_INSTANCEOF);
  new tok(T_NEW);
  clone tok(T_CLONE);
  exit tok(T_EXIT);
  if tok(T_IF);
  elseif tok(T_ELSEIF);
  else tok(T_ELSE);
  endif tok(T_ENDIF);
  echo tok(T_ECHO);
  do tok(T_DO);
  while tok(T_WHILE);
  endwhile tok(T_ENDWHILE);
  for tok(T_FOR);
  endfor tok(T_ENDFOR);
  foreach tok(T_FOREACH);
  endforeach tok(T_ENDFOREACH);
  declare tok(T_DECLARE);
  enddeclare tok(T_ENDDECLARE);
  as tok(T_AS);
  switch tok(T_SWITCH);
  endswitch tok(T_ENDSWITCH);
  case tok(T_CASE);
  default tok(T_DEFAULT);
  break tok(T_BREAK);
  continue tok(T_CONTINUE);
  goto tok(T_GOTO);
  function tok(T_FUNCTION);
  const tok(T_CONST);
  return tok(T_RETURN);
  try tok(T_TRY);
  catch tok(T_CATCH);
  throw tok(T_THROW);
  use tok(T_USE);
  global tok(T_GLOBAL);
  static tok(T_STATIC);
  abstract tok(T_ABSTRACT);
  final tok(T_FINAL);
  private tok(T_PRIVATE);
  protected tok(T_PROTECTED);
  public tok(T_PUBLIC);
  var tok(T_VAR);
  unset tok(T_UNSET);
  isset tok(T_ISSET);
  empty tok(T_EMPTY);
  __halt_compiler tok(T_HALT_COMPILER);
  class tok(T_CLASS);
  interface tok(T_INTERFACE);
  extends tok(T_EXTENDS);
  implements tok(T_IMPLEMENTS);
  list tok(T_LIST);
  array tok(T_ARRAY);
  __class__ tok(T_CLASS_C);
  __method__ tok(T_METHOD_C);
  __function__ tok(T_FUNC_C);
  __line__ tok(T_LINE);
  __file__ tok(T_FILE);
  namespace tok(T_NAMESPACE);
  __namespace__ tok(T_NS_C);
  __dir__ tok(T_DIR);
  attribute {
    if ((last_token() == '{' || last_token() == '}' || last_token() == ';') &&
        (yyextra->expecting_xhp_class_statements)) {
      tok(T_XHP_ATTRIBUTE);
    } else {
      tok(T_STRING);
    }
  }
  category {
    if ((last_token() == '{' || last_token() == '}' || last_token() == ';') &&
        (yyextra->expecting_xhp_class_statements)) {
      tok(T_XHP_CATEGORY);
    } else {
      tok(T_STRING);
    }
  }
  children {
    if ((last_token() == '{' || last_token() == '}' || last_token() == ';') &&
        (yyextra->expecting_xhp_class_statements)) {
      tok(T_XHP_CHILDREN);
    } else {
      tok(T_STRING);
    }
  }
}

 /* Operators */
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST,XHP_ATTR_TYPE_DECL>{
  "+=" tok(T_PLUS_EQUAL);
  "-=" tok(T_MINUS_EQUAL);
  "*=" tok(T_MUL_EQUAL);
  "/=" tok(T_DIV_EQUAL);
  ".=" tok(T_CONCAT_EQUAL);
  "%=" tok(T_MOD_EQUAL);
  "&=" tok(T_AND_EQUAL);
  "|=" tok(T_OR_EQUAL);
  "^=" tok(T_XOR_EQUAL);
  "<<=" tok(T_SL_EQUAL);
  ">>=" tok(T_SR_EQUAL);
  "||" tok(T_BOOLEAN_OR);
  "&&" tok(T_BOOLEAN_AND);
  "==" tok(T_IS_EQUAL);
  "!="|"<>" tok(T_IS_NOT_EQUAL);
  "===" tok(T_IS_IDENTICAL);
  "!==" tok(T_IS_NOT_IDENTICAL);
  "<=" tok(T_IS_SMALLER_OR_EQUAL);
  ">=" tok(T_IS_GREATER_OR_EQUAL);
  "<<" tok(T_SL);
  ">>" tok(T_SR);
  "++" tok(T_INC);
  "--" tok(T_DEC);
  "->" tok(T_OBJECT_OPERATOR);
  "=>" tok(T_DOUBLE_ARROW);
  "::" tok(T_PAAMAYIM_NEKUDOTAYIM);
  "\\" tok(T_NS_SEPARATOR);
  ":" {
    switch (yyextra->last_token) {
      case ',': case '=': case '|': case '^': case '&': case '<': case '>':
      case '+': case '-': case '%': case '!': case '~': case '[': case '(':
      case '{': case '.':
      case T_LOGICAL_OR: case T_LOGICAL_XOR: case T_LOGICAL_AND:
      case T_PLUS_EQUAL: case T_MINUS_EQUAL: case T_MUL_EQUAL:
      case T_DIV_EQUAL: case T_CONCAT_EQUAL: case T_MOD_EQUAL:
      case T_AND_EQUAL: case T_OR_EQUAL: case T_XOR_EQUAL:
      case T_SL_EQUAL: case T_SR_EQUAL: case T_BOOLEAN_OR:
      case T_BOOLEAN_AND: case T_IS_EQUAL: case T_IS_NOT_EQUAL:
      case T_IS_IDENTICAL: case T_IS_NOT_IDENTICAL: case T_IS_SMALLER_OR_EQUAL:
      case T_IS_GREATER_OR_EQUAL: case T_ECHO: case T_RETURN:
      case T_EXTENDS: case T_INSTANCEOF:
      case T_XHP_ATTRIBUTE:
        tok(T_XHP_COLON);
        break;
      default:
        tok(':');
        break;
    }
  }
}

 /* Casts */
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST>{
  "("{TABS_AND_SPACES}(int|integer){TABS_AND_SPACES}")" tok(T_INT_CAST);
  "("{TABS_AND_SPACES}(real|double|float){TABS_AND_SPACES}")" tok(T_DOUBLE_CAST);
  "("{TABS_AND_SPACES}string{TABS_AND_SPACES}")" tok(T_STRING_CAST);
  "("{TABS_AND_SPACES}unicode{TABS_AND_SPACES}")" tok(T_UNICODE_CAST);
  "("{TABS_AND_SPACES}binary{TABS_AND_SPACES}")" tok(T_BINARY_CAST);
  "("{TABS_AND_SPACES}array{TABS_AND_SPACES}")" tok(T_ARRAY_CAST);
  "("{TABS_AND_SPACES}object{TABS_AND_SPACES}")" tok(T_OBJECT_CAST);
  "("{TABS_AND_SPACES}(bool|boolean){TABS_AND_SPACES}")" tok(T_BOOL_CAST);
  "("{TABS_AND_SPACES}unset{TABS_AND_SPACES}")" tok(T_UNSET_CAST);
}

 /* Scalars (parsing these doesn't really matter since we just pass them through literally) */
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST,XHP_ATTR_TYPE_DECL>{
  {LNUM}|{HNUM} tok(T_LNUMBER);
  {DNUM}|{EXPONENT_DNUM} tok(T_DNUMBER);
  {LABEL} tok(T_STRING);
  "$"{LABEL} tok(T_VARIABLE);
  b?'(\\.|\\\n|[^\\']+)*'|b?\"(\\.|\\\n|[^\\\"]+)*\" {
    yy_scan_newlines(yytext, yyg);
    tok(T_CONSTANT_ENCAPSED_STRING);
  }
  `[^`]*` {
    yy_scan_newlines(yytext, yyg);
    tok(T_BACKTICKS_EXPR);
  }
}

 /* (HERE|NOW)DOC's */
<PHP,PHP_NO_RESERVED_WORDS,PHP_NO_RESERVED_WORDS_PERSIST>b?"<<<"{TABS_AND_SPACES} {
  push_state(PHP_HEREDOC_START);
  yyextra->heredoc_yyleng = yyleng;
  yymore();
}
<PHP_HEREDOC_START>{
  "'"{LABEL}"'"|\"{LABEL}\" {
    yyextra->heredoc_label = string(yytext + yyextra->heredoc_yyleng, yyleng - 1);
    set_state(PHP_HEREDOC_NSTART);
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
  {LABEL} {
    yyextra->heredoc_label = string(yytext + yyextra->heredoc_yyleng);
    set_state(PHP_HEREDOC_NSTART);
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
}
<PHP_HEREDOC_NSTART>{NEWLINE} {
  ++yyextra->lineno;
  yyextra->heredoc_data = yytext + yyleng;
  set_state(PHP_HEREDOC_DATA);
  yymore();
}
<PHP_HEREDOC_DATA>{
  [^\r\n]*{NEWLINE} {
    ++yyextra->lineno;
    set_state(PHP_HEREDOC_NEWLINE);
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
}
<PHP_HEREDOC_NEWLINE>{
  {LABEL};?{NEWLINE} {
    if (strncmp(yyextra->heredoc_label.c_str(), yytext + yyextra->heredoc_yyleng, yyextra->heredoc_label.size()) == 0) {
      switch (yytext[yyextra->heredoc_yyleng + yyextra->heredoc_label.size()]) {
        case ';': case '\n': case '\r':
          yyless(yyleng - (yyleng - yyextra->heredoc_yyleng - yyextra->heredoc_label.size()));
          pop_state();
          tok(T_HEREDOC);
      }
    }
    ++yyextra->lineno;
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
  [^\r\n]+ {
    set_state(PHP_HEREDOC_DATA);
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
  {NEWLINE} {
    ++yyextra->lineno;
    yyextra->heredoc_yyleng = yyleng;
    yymore();
  }
}

 /* XHP */
<XHP_LABEL_WHITESPACE>{
  {WHITESPACE}+ yy_scan_newlines(yytext, yyg);
}
<XHP_LABEL,XHP_LABEL_WHITESPACE>{
  ":" tok(T_XHP_COLON);
  "-" tok(T_XHP_HYPHEN);
  "::" {
    pop_state();
    tok(T_PAAMAYIM_NEKUDOTAYIM);
  }
  "--" {
    pop_state();
    tok(T_DEC);
  }
  {WHITESPACE} {
    yy_scan_newlines(yytext, yyg);
    pop_state();
    tok(T_XHP_WHITESPACE);
  }
  {LABEL} tok(T_STRING);
  . {
    pop_state();
    tok(yytext[0]);
  }
}

<XHP_ATTRS>{
  "="|"/"|">" tok(yytext[0]);
  {WHITESPACE}+ yy_scan_newlines(yytext, yyg);
  {LABEL} tok(T_STRING);
}

<XHP_ATTR_VAL>{
  [^&'\\"]+ tok(T_XHP_TEXT);
  \" {
    pop_state();
    tok('"');
  }
}

<XHP_CHILD_START>{
  {WHITESPACE}+ {
    /* ignore whitespace at the start */
    yy_scan_newlines(yytext, yyg);
    unput(' ');
    set_state(XHP_CHILD);
  }
  . {
    yyless(0);
    set_state(XHP_CHILD);
  }
}

<XHP_CHILD,XHP_AFTER_ENT,XHP_ATTR_VAL>{
  /* xml entities */
  (?-i:&quot;) tokt("\"");
  (?-i:&amp;) tokt("&");
  (?-i:&apos;) tokt("\\'");
  (?-i:&lt;) tokt("<")
  (?-i:&gt;) tokt(">");

  /* html entities */
  (?-i:&nbsp;) tokt("\u00A0");
  (?-i:&iexcl;) tokt("\u00A1");
  (?-i:&cent;) tokt("\u00A2");
  (?-i:&pound;) tokt("\u00A3");
  (?-i:&curren;) tokt("\u00A4");
  (?-i:&yen;) tokt("\u00A5");
  (?-i:&brvbar;) tokt("\u00A6");
  (?-i:&sect;) tokt("\u00A7");
  (?-i:&uml;) tokt("\u00A8");
  (?-i:&copy;) tokt("\u00A9");
  (?-i:&ordf;) tokt("\u00AA");
  (?-i:&laquo;) tokt("\u00AB");
  (?-i:&not;) tokt("\u00AC");
  (?-i:&shy;) tokt("\u00AD");
  (?-i:&reg;) tokt("\u00AE");
  (?-i:&macr;) tokt("\u00AF");
  (?-i:&deg;) tokt("\u00B0");
  (?-i:&plusmn;) tokt("\u00B1");
  (?-i:&sup2;) tokt("\u00B2");
  (?-i:&sup3;) tokt("\u00B3");
  (?-i:&acute;) tokt("\u00B4");
  (?-i:&micro;) tokt("\u00B5");
  (?-i:&para;) tokt("\u00B6");
  (?-i:&middot;) tokt("\u00B7");
  (?-i:&cedil;) tokt("\u00B8");
  (?-i:&sup1;) tokt("\u00B9");
  (?-i:&ordm;) tokt("\u00BA");
  (?-i:&raquo;) tokt("\u00BB");
  (?-i:&frac14;) tokt("\u00BC");
  (?-i:&frac12;) tokt("\u00BD");
  (?-i:&frac34;) tokt("\u00BE");
  (?-i:&iquest;) tokt("\u00BF");
  (?-i:&Agrave;) tokt("\u00C0");
  (?-i:&Aacute;) tokt("\u00C1");
  (?-i:&Acirc;) tokt("\u00C2");
  (?-i:&Atilde;) tokt("\u00C3");
  (?-i:&Auml;) tokt("\u00C4");
  (?-i:&Aring;) tokt("\u00C5");
  (?-i:&AElig;) tokt("\u00C6");
  (?-i:&Ccedil;) tokt("\u00C7");
  (?-i:&Egrave;) tokt("\u00C8");
  (?-i:&Eacute;) tokt("\u00C9");
  (?-i:&Ecirc;) tokt("\u00CA");
  (?-i:&Euml;) tokt("\u00CB");
  (?-i:&Igrave;) tokt("\u00CC");
  (?-i:&Iacute;) tokt("\u00CD");
  (?-i:&Icirc;) tokt("\u00CE");
  (?-i:&Iuml;) tokt("\u00CF");
  (?-i:&ETH;) tokt("\u00D0");
  (?-i:&Ntilde;) tokt("\u00D1");
  (?-i:&Ograve;) tokt("\u00D2");
  (?-i:&Oacute;) tokt("\u00D3");
  (?-i:&Ocirc;) tokt("\u00D4");
  (?-i:&Otilde;) tokt("\u00D5");
  (?-i:&Ouml;) tokt("\u00D6");
  (?-i:&times;) tokt("\u00D7");
  (?-i:&Oslash;) tokt("\u00D8");
  (?-i:&Ugrave;) tokt("\u00D9");
  (?-i:&Uacute;) tokt("\u00DA");
  (?-i:&Ucirc;) tokt("\u00DB");
  (?-i:&Uuml;) tokt("\u00DC");
  (?-i:&Yacute;) tokt("\u00DD");
  (?-i:&THORN;) tokt("\u00DE");
  (?-i:&szlig;) tokt("\u00DF");
  (?-i:&agrave;) tokt("\u00E0");
  (?-i:&aacute;) tokt("\u00E1");
  (?-i:&acirc;) tokt("\u00E2");
  (?-i:&atilde;) tokt("\u00E3");
  (?-i:&auml;) tokt("\u00E4");
  (?-i:&aring;) tokt("\u00E5");
  (?-i:&aelig;) tokt("\u00E6");
  (?-i:&ccedil;) tokt("\u00E7");
  (?-i:&egrave;) tokt("\u00E8");
  (?-i:&eacute;) tokt("\u00E9");
  (?-i:&ecirc;) tokt("\u00EA");
  (?-i:&euml;) tokt("\u00EB");
  (?-i:&igrave;) tokt("\u00EC");
  (?-i:&iacute;) tokt("\u00ED");
  (?-i:&icirc;) tokt("\u00EE");
  (?-i:&iuml;) tokt("\u00EF");
  (?-i:&eth;) tokt("\u00F0");
  (?-i:&ntilde;) tokt("\u00F1");
  (?-i:&ograve;) tokt("\u00F2");
  (?-i:&oacute;) tokt("\u00F3");
  (?-i:&ocirc;) tokt("\u00F4");
  (?-i:&otilde;) tokt("\u00F5");
  (?-i:&ouml;) tokt("\u00F6");
  (?-i:&divide;) tokt("\u00F7");
  (?-i:&oslash;) tokt("\u00F8");
  (?-i:&ugrave;) tokt("\u00F9");
  (?-i:&uacute;) tokt("\u00FA");
  (?-i:&ucirc;) tokt("\u00FB");
  (?-i:&uuml;) tokt("\u00FC");
  (?-i:&yacute;) tokt("\u00FD");
  (?-i:&thorn;) tokt("\u00FE");
  (?-i:&yuml;) tokt("\u00FF");
  (?-i:&OElig;) tokt("\u0152");
  (?-i:&oelig;) tokt("\u0153");
  (?-i:&Scaron;) tokt("\u0160");
  (?-i:&scaron;) tokt("\u0161");
  (?-i:&Yuml;) tokt("\u0178");
  (?-i:&fnof;) tokt("\u0192");
  (?-i:&circ;) tokt("\u02C6");
  (?-i:&tilde;) tokt("\u02DC");
  (?-i:&Alpha;) tokt("\u0391");
  (?-i:&Beta;) tokt("\u0392");
  (?-i:&Gamma;) tokt("\u0393");
  (?-i:&Delta;) tokt("\u0394");
  (?-i:&Epsilon;) tokt("\u0395");
  (?-i:&Zeta;) tokt("\u0396");
  (?-i:&Eta;) tokt("\u0397");
  (?-i:&Theta;) tokt("\u0398");
  (?-i:&Iota;) tokt("\u0399");
  (?-i:&Kappa;) tokt("\u039A");
  (?-i:&Lambda;) tokt("\u039B");
  (?-i:&Mu;) tokt("\u039C");
  (?-i:&Nu;) tokt("\u039D");
  (?-i:&Xi;) tokt("\u039E");
  (?-i:&Omicron;) tokt("\u039F");
  (?-i:&Pi;) tokt("\u03A0");
  (?-i:&Rho;) tokt("\u03A1");
  (?-i:&Sigma;) tokt("\u03A3");
  (?-i:&Tau;) tokt("\u03A4");
  (?-i:&Upsilon;) tokt("\u03A5");
  (?-i:&Phi;) tokt("\u03A6");
  (?-i:&Chi;) tokt("\u03A7");
  (?-i:&Psi;) tokt("\u03A8");
  (?-i:&Omega;) tokt("\u03A9");
  (?-i:&alpha;) tokt("\u03B1");
  (?-i:&beta;) tokt("\u03B2");
  (?-i:&gamma;) tokt("\u03B3");
  (?-i:&delta;) tokt("\u03B4");
  (?-i:&epsilon;) tokt("\u03B5");
  (?-i:&zeta;) tokt("\u03B6");
  (?-i:&eta;) tokt("\u03B7");
  (?-i:&theta;) tokt("\u03B8");
  (?-i:&iota;) tokt("\u03B9");
  (?-i:&kappa;) tokt("\u03BA");
  (?-i:&lambda;) tokt("\u03BB");
  (?-i:&mu;) tokt("\u03BC");
  (?-i:&nu;) tokt("\u03BD");
  (?-i:&xi;) tokt("\u03BE");
  (?-i:&omicron;) tokt("\u03BF");
  (?-i:&pi;) tokt("\u03C0");
  (?-i:&rho;) tokt("\u03C1");
  (?-i:&sigmaf;) tokt("\u03C2");
  (?-i:&sigma;) tokt("\u03C3");
  (?-i:&tau;) tokt("\u03C4");
  (?-i:&upsilon;) tokt("\u03C5");
  (?-i:&phi;) tokt("\u03C6");
  (?-i:&chi;) tokt("\u03C7");
  (?-i:&psi;) tokt("\u03C8");
  (?-i:&omega;) tokt("\u03C9");
  (?-i:&thetasym;) tokt("\u03D1");
  (?-i:&upsih;) tokt("\u03D2");
  (?-i:&piv;) tokt("\u03D6");
  (?-i:&ensp;) tokt("\u2002");
  (?-i:&emsp;) tokt("\u2003");
  (?-i:&thinsp;) tokt("\u2009");
  (?-i:&zwnj;) tokt("\u200C");
  (?-i:&zwj;) tokt("\u200D");
  (?-i:&lrm;) tokt("\u200E");
  (?-i:&rlm;) tokt("\u200F");
  (?-i:&ndash;) tokt("\u2013");
  (?-i:&mdash;) tokt("\u2014");
  (?-i:&lsquo;) tokt("\u2018");
  (?-i:&rsquo;) tokt("\u2019");
  (?-i:&sbquo;) tokt("\u201A");
  (?-i:&ldquo;) tokt("\u201C");
  (?-i:&rdquo;) tokt("\u201D");
  (?-i:&bdquo;) tokt("\u201E");
  (?-i:&dagger;) tokt("\u2020");
  (?-i:&Dagger;) tokt("\u2021");
  (?-i:&bull;) tokt("\u2022");
  (?-i:&hellip;) tokt("\u2026");
  (?-i:&permil;) tokt("\u2030");
  (?-i:&prime;) tokt("\u2032");
  (?-i:&Prime;) tokt("\u2033");
  (?-i:&lsaquo;) tokt("\u2039");
  (?-i:&rsaquo;) tokt("\u203A");
  (?-i:&oline;) tokt("\u203E");
  (?-i:&frasl;) tokt("\u2044");
  (?-i:&euro;) tokt("\u20AC");
  (?-i:&image;) tokt("\u2111");
  (?-i:&weierp;) tokt("\u2118");
  (?-i:&real;) tokt("\u211C");
  (?-i:&trade;) tokt("\u2122");
  (?-i:&alefsym;) tokt("\u2135");
  (?-i:&larr;) tokt("\u2190");
  (?-i:&uarr;) tokt("\u2191");
  (?-i:&rarr;) tokt("\u2192");
  (?-i:&darr;) tokt("\u2193");
  (?-i:&harr;) tokt("\u2194");
  (?-i:&crarr;) tokt("\u21B5");
  (?-i:&lArr;) tokt("\u21D0");
  (?-i:&uArr;) tokt("\u21D1");
  (?-i:&rArr;) tokt("\u21D2");
  (?-i:&dArr;) tokt("\u21D3");
  (?-i:&hArr;) tokt("\u21D4");
  (?-i:&forall;) tokt("\u2200");
  (?-i:&part;) tokt("\u2202");
  (?-i:&exist;) tokt("\u2203");
  (?-i:&empty;) tokt("\u2205");
  (?-i:&nabla;) tokt("\u2207");
  (?-i:&isin;) tokt("\u2208");
  (?-i:&notin;) tokt("\u2209");
  (?-i:&ni;) tokt("\u220B");
  (?-i:&prod;) tokt("\u220F");
  (?-i:&sum;) tokt("\u2211");
  (?-i:&minus;) tokt("\u2212");
  (?-i:&lowast;) tokt("\u2217");
  (?-i:&radic;) tokt("\u221A");
  (?-i:&prop;) tokt("\u221D");
  (?-i:&infin;) tokt("\u221E");
  (?-i:&ang;) tokt("\u2220");
  (?-i:&and;) tokt("\u2227");
  (?-i:&or;) tokt("\u2228");
  (?-i:&cap;) tokt("\u2229");
  (?-i:&cup;) tokt("\u222A");
  (?-i:&int;) tokt("\u222B");
  (?-i:&there4;) tokt("\u2234");
  (?-i:&sim;) tokt("\u223C");
  (?-i:&cong;) tokt("\u2245");
  (?-i:&asymp;) tokt("\u2248");
  (?-i:&ne;) tokt("\u2260");
  (?-i:&equiv;) tokt("\u2261");
  (?-i:&le;) tokt("\u2264");
  (?-i:&ge;) tokt("\u2265");
  (?-i:&sub;) tokt("\u2282");
  (?-i:&sup;) tokt("\u2283");
  (?-i:&nsub;) tokt("\u2284");
  (?-i:&sube;) tokt("\u2286");
  (?-i:&supe;) tokt("\u2287");
  (?-i:&oplus;) tokt("\u2295");
  (?-i:&otimes;) tokt("\u2297");
  (?-i:&perp;) tokt("\u22A5");
  (?-i:&sdot;) tokt("\u22C5");
  (?-i:&lceil;) tokt("\u2308");
  (?-i:&rceil;) tokt("\u2309");
  (?-i:&lfloor;) tokt("\u230A");
  (?-i:&rfloor;) tokt("\u230B");
  (?-i:&lang;) tokt("\u2329");
  (?-i:&rang;) tokt("\u232A");
  (?-i:&loz;) tokt("\u25CA");
  (?-i:&spades;) tokt("\u2660");
  (?-i:&clubs;) tokt("\u2663");
  (?-i:&hearts;) tokt("\u2665");
  (?-i:&diams;) tokt("\u2666");

  /* awesome entities */
  (?-i:&cloud;) tokt("\u2601");
  (?-i:&umbrella;) tokt("\u2602");
  (?-i:&snowman;) tokt("\u2603");
  (?-i:&snowflake;) tokt("\u2745");
  (?-i:&comet;) tokt("\u2604");
  (?-i:&thunderstorm;) tokt("\u2608");

  /* pseudo entities */
  ' tokt("\\'");
  "\\" tokt("\\\\");

  /* meta entities */
  (?-i:&#[0-9]+;) {
    char buf[5];
    utf8ize(atoi(yytext + 2), buf);
    tokt(buf);
  }
  (?-i:&#x)[A-F0-9]+; {
    char buf[5];
    char *_;
    utf8ize(strtol(yytext + 3, &_, 16), buf);
    tokt(buf);
  }

  /* not entities */
  & {
    yymore();
    BEGIN(XHP_INVALID_ENTITY);
  }
}

<XHP_INVALID_ENTITY>{
  {BYTE}{1,10} {
    for (char* ii = yytext; *ii; ++ii) {
      if (*ii == ';') {
        ii[1] = 0;
        break;
      }
    }
    if (!yyextra->terminated) {
      yyextra->error = string("Invalid entity: (") + yytext + ")";
      yyextra->terminated = true;
    }
  }
}

<XHP_AFTER_ENT>{
  [ \t\x0b\x0c\xa0\r\n]|\r\n {
    if (*yytext == '\r' || *yytext == '\n') {
      ++yyextra->lineno;
    }
    pop_state();
    yytext[0] = ' ';
    yytext[1] = 0;
    tok(T_XHP_TEXT);
  }
  . {
    pop_state();
    yyless(0);
  }
}

<XHP_CHILD>{
  [^&'<>\\{ \t\x0b\x0c\xa0\r\n]+{WHITESPACE}{0,1} {
    yy_scan_newlines(yytext, yyg);
    tok(T_XHP_TEXT);
  }
  {WHITESPACE}* {
    yy_scan_newlines(yytext, yyg);
    tok(T_XHP_TEXT);
  }
  {WHITESPACE}*"{" {
    yy_scan_newlines(yytext, yyg);
    tok('{');
  }
  {WHITESPACE}*"<" {
    yy_scan_newlines(yytext, yyg);
    tok('<');
  }
  {WHITESPACE}*"</" {
    yy_scan_newlines(yytext, yyg);
    tok(T_XHP_LT_DIV);
  }
  {WHITESPACE}*"</>" {
    yy_scan_newlines(yytext, yyg);
    tok(T_XHP_LT_DIV_GT);
  }
}

<XHP_CHILDREN_DECL>{
  any tok(T_XHP_ANY);
  pcdata tok(T_XHP_PCDATA);
  empty tok(T_XHP_EMPTY);
  {LABEL} tok(T_STRING);
  ";" {
    pop_state();
    tok(';');
  }
  ":" {
    tok(T_XHP_COLON);
  }
}

 /* Other */
<*>{BYTE} {
  tok(yytext[0]);
  // fix unused function warnings
  yy_top_state(NULL);
  yyunput(0, 0, NULL);
}

%%

#ifdef DEBUG
static const char* yy_state_name(int state) {
  switch (state) {
    case INITIAL:
      return "INITIAL";
    case PHP:
      return "PHP";
    case PHP_COMMENT:
      return "PHP_COMMENT";
    case PHP_EOL_COMMENT:
      return "PHP_EOL_COMMENT";
    case PHP_DOC_COMMENT:
      return "PHP_DOC_COMMENT";
    case PHP_HEREDOC_START:
      return "PHP_HEREDOC_START";
    case PHP_HEREDOC_NSTART:
      return "PHP_HEREDOC_NSTART";
    case PHP_HEREDOC_NEWLINE:
      return "PHP_HEREDOC_NEWLINE";
    case PHP_HEREDOC_DATA:
      return "PHP_HEREDOC_DATA";
    case PHP_NO_RESERVED_WORDS:
      return "PHP_NO_RESERVED_WORDS";
    case PHP_NO_RESERVED_WORDS_PERSIST:
      return "PHP_NO_RESERVED_WORDS_PERSIST";
    case XHP_LABEL:
      return "XHP_LABEL";
    case XHP_LABEL_WHITESPACE:
      return "XHP_LABEL_WHITESPACE";
    case XHP_ATTRS:
      return "XHP_ATTRS";
    case XHP_ATTR_VAL:
      return "XHP_ATTR_VAL";
    case XHP_AFTER_ENT:
      return "XHP_AFTER_ENT";
    case XHP_CHILD:
      return "XHP_CHILD";
    case XHP_CHILD_START:
      return "XHP_CHILD_START";
    case XHP_INVALID_ENTITY:
      return "XHP_INVALID_ENTITY";
    case XHP_ATTR_TYPE_DECL:
      return "XHP_ATTR_TYPE_DECL";
    case XHP_CHILDREN_DECL:
      return "XHP_CHILDREN_DECL";
    default:
      return "???";
  }
}

static void yy_log_token(int tok) {
  const char* tokname = yytokname(tok);
  if (tokname) {
    fprintf(stderr, "--> %s\n", tokname);
  } else {
    fprintf(stderr, "--> '%c'\n", tok);
  }
}
#endif

static int yy_token(int tok, yyguts_t* yyg) {
  if (YY_START == PHP_NO_RESERVED_WORDS) {
    pop_state();
  }

  switch (tok) {
    case T_OPEN_TAG:
    case T_OPEN_TAG_WITH_ECHO:
    case T_OPEN_TAG_FAKE:
      push_state(PHP);
      break;

    case T_CLOSE_TAG:
      pop_state();
      return ';';

    case T_OBJECT_OPERATOR:
    case T_PAAMAYIM_NEKUDOTAYIM:
    case T_FUNCTION:
      push_state(PHP_NO_RESERVED_WORDS);
      break;

    case '{':
      yyextra->curly_stack.push(tok);
      break;
  }
#ifdef DEBUG
  yy_log_token(tok);
#endif
  return yyextra->last_token = tok;
}

static inline void yy_scan_newlines(const char* text, struct yyguts_t* yyg) {
  for (; *text; ++text) {
    if (*text == '\r') {
      if (text[1] == '\n') {
        ++text;
      }
      ++yyextra->lineno;
    } else if (*text == '\n') {
      ++yyextra->lineno;
    }
  }
}

void xhp_new_push_state(int s, struct yyguts_t* yyg) {
#ifdef DEBUG
  fprintf(stderr, "--> PUSH(%s -> %s)\n", yy_state_name(YY_START), yy_state_name(s));
#endif
  yy_push_state(s, yyg);
}

void xhp_new_pop_state(struct yyguts_t* yyg) {
#ifdef DEBUG
  int s = YY_START;
#endif
  yy_pop_state(yyg);
#ifdef DEBUG
  fprintf(stderr, "--> POP(%s -> %s)\n", yy_state_name(s), yy_state_name(YY_START));
#endif
}

void xhp_set_state(int s, struct yyguts_t* yyg) {
#ifdef DEBUG
  fprintf(stderr, "--> SET(%s)\n", yy_state_name(s));
#endif
  BEGIN(s);
}
