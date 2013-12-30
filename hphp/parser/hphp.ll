%{ /* -*- mode: c++ -*- */
#include "hphp/parser/scanner.h"

// macros for flex
#define YYSTYPE HPHP::ScannerToken
#define YYLTYPE HPHP::Location
#define YY_EXTRA_TYPE HPHP::Scanner*
#define _scanner yyextra
#define YY_INPUT(buf,result,max) _scanner->read(buf,result,max)
#define YY_FATAL_ERROR(msg) \
  do { \
    struct yyguts_t *yyg = (struct yyguts_t *)yyscanner; \
    _scanner->error(msg); \
  } while (0) \

#undef YY_READ_BUF_SIZE
#undef YY_BUF_SIZE
#define YY_READ_BUF_SIZE 1024*128 /* for reading from input */
#define YY_BUF_SIZE 1024*64 /* for pattern matching */

#define DECLARE_YYCURSOR \
  char *&cursor = yyg->yy_c_buf_p; *cursor = yyg->yy_hold_char;
#define DECLARE_YYLIMIT \
  char *limit = YY_CURRENT_BUFFER->yy_ch_buf + yyg->yy_n_chars;
#define YYCURSOR  cursor
#define YYLIMIT   limit
#define RESET_YYCURSOR yyg->yy_hold_char = *YYCURSOR; *YYCURSOR = '\0';

// macros for rules
#define RETTOKEN(t) do {_scanner->setToken(yytext, yyleng, t); return t;} \
  while (0)
#define RETSTEP(t)  do {_scanner->stepPos(yytext, yyleng, t); return t;} \
  while (0)
#define SETTOKEN(t) _scanner->setToken(yytext, yyleng, t)
#define STEPPOS(t)  _scanner->stepPos(yytext, yyleng, t)

#define XHP_ONLY_KEYWORD(tok) do {                           \
  RETTOKEN(_scanner->isXHPSyntaxEnabled() ? (tok) : T_STRING); \
} while (0)

#define HH_ONLY_KEYWORD(tok) do {                               \
  RETTOKEN(_scanner->isHHSyntaxEnabled() ? (tok) : T_STRING); \
} while (0)

#define IS_LABEL_START(c) \
  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || \
   (c) == '_' || (c) >= 0x7F)

/**
 * "Next token types" tell us how to interpret the next characters in the
 * input stream based on the previous token for the purpose of recognizing
 * XHP tags, XHP class names, XHP category names, type lists, and lambda
 * expressions.
 *   XhpTag:
 *     "<"[a-zA-Z_\x7f-\xff] will be treated as the start of an XHP tag
 *   XhpTagMaybe:
 *     "<"[a-zA-Z_\x7f-\xff] will be treated as possibly being the start of an
 *     XHP tag; we will scan ahead looking at subsequent characters to figure
 *     out if "<" is definitely the start of an XHP tag
 *   XhpClassName:
 *     ":"{XHPLABEL} will be treated as an XHP class name
 *   XhpCategoryName:
 *     "%"{XHPLABEL} will be treated as an XHP category name
 *   TypeListMaybe:
 *     "<" should be recognized as possibly being the start of a type list;
 *     this will be resolved by inspecting subsequent tokens
 *   LambdaMaybe:
 *     "(" should be recognized as possibly being the start of a lambda
 *     expression; this will be resolved by inspecting subsequent tokens
 */
namespace NextTokenType {
  static const int Normal = 0x1;
  static const int XhpTag = 0x2;
  static const int XhpTagMaybe = 0x4;
  static const int XhpClassName = 0x8;
  static const int XhpCategoryName = 0x10;
  static const int TypeListMaybe = 0x20;
  static const int LambdaMaybe = 0x40;
}

static int getNextTokenType(int t) {
  switch (t) {
    case '=': case '.': case '+': case '-': case '*': case '/': case '%':
    case '!': case '~': case '&': case '^': case '<': case '>': case '?':
    case ':': case '[': case '{': case ';': case '@': case -1:
    case T_LOGICAL_OR:
    case T_LOGICAL_XOR:
    case T_LOGICAL_AND:
    case T_SL:
    case T_SR:
    case T_BOOLEAN_OR:
    case T_BOOLEAN_AND:
    case T_IS_EQUAL:
    case T_IS_NOT_EQUAL:
    case T_IS_IDENTICAL:
    case T_IS_NOT_IDENTICAL:
    case T_IS_SMALLER_OR_EQUAL:
    case T_IS_GREATER_OR_EQUAL:
    case T_PLUS_EQUAL:
    case T_MINUS_EQUAL:
    case T_MUL_EQUAL:
    case T_DIV_EQUAL:
    case T_CONCAT_EQUAL:
    case T_MOD_EQUAL:
    case T_AND_EQUAL:
    case T_OR_EQUAL:
    case T_XOR_EQUAL:
    case T_SL_EQUAL:
    case T_SR_EQUAL:
    case T_ECHO:
    case T_PRINT:
    case T_CLONE:
    case T_EXIT:
    case T_RETURN:
    case T_YIELD:
    case T_AWAIT:
    case T_NEW:
    case T_INSTANCEOF:
    case T_DOUBLE_ARROW:
    case T_LAMBDA_ARROW:
    case T_NS_SEPARATOR:
    case T_INLINE_HTML:
    case T_INT_CAST:
    case T_DOUBLE_CAST:
    case T_STRING_CAST:
    case T_ARRAY_CAST:
    case T_OBJECT_CAST:
    case T_BOOL_CAST:
    case T_UNSET_CAST:
    case T_UNRESOLVED_LT:
    case T_AS:
      return NextTokenType::XhpTag |
             NextTokenType::XhpClassName |
             NextTokenType::LambdaMaybe;
    case ',': case '(': case '|': case T_UNRESOLVED_OP:
      return NextTokenType::XhpTag |
             NextTokenType::XhpClassName |
             NextTokenType::XhpCategoryName |
             NextTokenType::LambdaMaybe;
    case '}':
      return NextTokenType::XhpTagMaybe |
             NextTokenType::XhpClassName |
             NextTokenType::LambdaMaybe;
    case T_INC:
    case T_DEC:
      return NextTokenType::XhpTagMaybe;
    case T_EXTENDS:
    case T_CLASS:
    case T_PRIVATE:
    case T_PROTECTED:
    case T_PUBLIC:
    case T_STATIC:
      return NextTokenType::XhpClassName;
    case T_STRING:
    case T_XHP_CHILDREN:
    case T_XHP_REQUIRED:
    case T_XHP_ENUM:
    case T_ARRAY:
    case T_FROM:
    case T_IN:
    case T_WHERE:
    case T_JOIN:
    case T_ON:
    case T_EQUALS:
    case T_INTO:
    case T_LET:
    case T_ORDERBY:
    case T_ASCENDING:
    case T_DESCENDING:
    case T_SELECT:
    case T_GROUP:
    case T_BY:
      return NextTokenType::TypeListMaybe;
    case T_XHP_ATTRIBUTE:
      return NextTokenType::XhpClassName |
             NextTokenType::TypeListMaybe;
    case T_XHP_CATEGORY:
      return NextTokenType::XhpCategoryName |
             NextTokenType::TypeListMaybe;
    default:
      return NextTokenType::Normal;
  }
}

%}

%x ST_IN_HTML
%x ST_IN_SCRIPTING
%x ST_AFTER_HASHBANG
%x ST_DOUBLE_QUOTES
%x ST_BACKQUOTE
%x ST_HEREDOC
%x ST_NOWDOC
%x ST_END_HEREDOC
%x ST_LOOKING_FOR_PROPERTY
%x ST_LOOKING_FOR_VARNAME
%x ST_LOOKING_FOR_COLON
%x ST_VAR_OFFSET
%x ST_LT_CHECK
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT

%x ST_XHP_IN_TAG
%x ST_XHP_END_SINGLETON_TAG
%x ST_XHP_END_CLOSE_TAG
%x ST_XHP_CHILD
%x ST_XHP_COMMENT

%option stack

LNUM    [0-9]+
DNUM    ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\])|^&+\-*/=%!~$<>?@]
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")
XHPLABEL {LABEL}([:-]{LABEL})*
COMMENT_REGEX ("/*"([^\*]|("*"[^/]))*"*/"|("//"|"#")[^\r\n]*{NEWLINE})
WHITESPACE_AND_COMMENTS ([ \n\r\t]|({COMMENT_REGEX}))+

/*
 * LITERAL_DOLLAR matches unescaped $ that aren't followed by a label character
 * or a { and therefore will be taken literally. The case of literal $ before
 * a variable or "${" is handled in a rule for each string type
 */
DOUBLE_QUOTES_LITERAL_DOLLAR ("$"+([^a-zA-Z_\x7f-\xff$\"\\{]|("\\"{ANY_CHAR})))
BACKQUOTE_LITERAL_DOLLAR     ("$"+([^a-zA-Z_\x7f-\xff$`\\{]|("\\"{ANY_CHAR})))

/*
 * CHARS matches everything up to a variable or "{$"
 * {'s are matched as long as they aren't followed by a $
 * The case of { before "{$" is handled in a rule for each string type
 *
 * For heredocs, matching continues across/after newlines if/when it's known
 * that the next line doesn't contain a possible ending label
 */
DOUBLE_QUOTES_CHARS ("{"*([^$\"\\{]|("\\"{ANY_CHAR}))|{DOUBLE_QUOTES_LITERAL_DOLLAR})
BACKQUOTE_CHARS     ("{"*([^$`\\{]|("\\"{ANY_CHAR}))|{BACKQUOTE_LITERAL_DOLLAR})

%%

<ST_IN_SCRIPTING>"exit"                 { RETTOKEN(T_EXIT);}
<ST_IN_SCRIPTING>"die"                  { RETTOKEN(T_EXIT);}
<ST_IN_SCRIPTING>"function"             { RETTOKEN(T_FUNCTION);}
<ST_IN_SCRIPTING>"const"                { RETTOKEN(T_CONST);}
<ST_IN_SCRIPTING>"return"               { RETTOKEN(T_RETURN); }
<ST_IN_SCRIPTING>"yield"                { RETTOKEN(T_YIELD);}
<ST_IN_SCRIPTING>"try"                  { RETTOKEN(T_TRY);}
<ST_IN_SCRIPTING>"catch"                { RETTOKEN(T_CATCH);}
<ST_IN_SCRIPTING>"finally"              { RETTOKEN(T_FINALLY);}
<ST_IN_SCRIPTING>"throw"                { RETTOKEN(T_THROW);}
<ST_IN_SCRIPTING>"if"                   { RETTOKEN(T_IF);}
<ST_IN_SCRIPTING>"elseif"               { RETTOKEN(T_ELSEIF);}
<ST_IN_SCRIPTING>"endif"                { RETTOKEN(T_ENDIF);}
<ST_IN_SCRIPTING>"else"                 { RETTOKEN(T_ELSE);}
<ST_IN_SCRIPTING>"while"                { RETTOKEN(T_WHILE);}
<ST_IN_SCRIPTING>"endwhile"             { RETTOKEN(T_ENDWHILE);}
<ST_IN_SCRIPTING>"do"                   { RETTOKEN(T_DO);}
<ST_IN_SCRIPTING>"for"                  { RETTOKEN(T_FOR);}
<ST_IN_SCRIPTING>"endfor"               { RETTOKEN(T_ENDFOR);}
<ST_IN_SCRIPTING>"foreach"              { RETTOKEN(T_FOREACH);}
<ST_IN_SCRIPTING>"endforeach"           { RETTOKEN(T_ENDFOREACH);}
<ST_IN_SCRIPTING>"declare"              { RETTOKEN(T_DECLARE);}
<ST_IN_SCRIPTING>"enddeclare"           { RETTOKEN(T_ENDDECLARE);}
<ST_IN_SCRIPTING>"instanceof"           { RETTOKEN(T_INSTANCEOF);}
<ST_IN_SCRIPTING>"as"                   { RETTOKEN(T_AS);}
<ST_IN_SCRIPTING>"switch"               { RETTOKEN(T_SWITCH);}
<ST_IN_SCRIPTING>"endswitch"            { RETTOKEN(T_ENDSWITCH);}
<ST_IN_SCRIPTING>"case"                 { RETTOKEN(T_CASE);}
<ST_IN_SCRIPTING>"default"              { RETTOKEN(T_DEFAULT);}
<ST_IN_SCRIPTING>"break"                { RETTOKEN(T_BREAK);}
<ST_IN_SCRIPTING>"continue"             { RETTOKEN(T_CONTINUE);}
<ST_IN_SCRIPTING>"goto"                 { RETTOKEN(T_GOTO);}
<ST_IN_SCRIPTING>"echo"                 { RETTOKEN(T_ECHO);}
<ST_IN_SCRIPTING>"print"                { RETTOKEN(T_PRINT);}
<ST_IN_SCRIPTING>"class"                { RETTOKEN(T_CLASS);}
<ST_IN_SCRIPTING>"interface"            { RETTOKEN(T_INTERFACE);}
<ST_IN_SCRIPTING>"trait"                { RETTOKEN(T_TRAIT);}
<ST_IN_SCRIPTING>"insteadof"            { RETTOKEN(T_INSTEADOF);}
<ST_IN_SCRIPTING>"extends"              { RETTOKEN(T_EXTENDS);}
<ST_IN_SCRIPTING>"implements"           { RETTOKEN(T_IMPLEMENTS);}
<ST_IN_SCRIPTING>"attribute"            { XHP_ONLY_KEYWORD(T_XHP_ATTRIBUTE); }
<ST_IN_SCRIPTING>"category"             { XHP_ONLY_KEYWORD(T_XHP_CATEGORY); }
<ST_IN_SCRIPTING>"children"             { XHP_ONLY_KEYWORD(T_XHP_CHILDREN); }
<ST_IN_SCRIPTING>"required"             { XHP_ONLY_KEYWORD(T_XHP_REQUIRED); }
<ST_IN_SCRIPTING>"enum"                 { XHP_ONLY_KEYWORD(T_XHP_ENUM); }

<ST_IN_SCRIPTING>"->" {
        STEPPOS(T_OBJECT_OPERATOR);
        yy_push_state(ST_LOOKING_FOR_PROPERTY, yyscanner);
        return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>"->" {
        RETSTEP(T_OBJECT_OPERATOR);
}

<ST_LOOKING_FOR_PROPERTY>{LABEL} {
        SETTOKEN(T_STRING);
        yy_pop_state(yyscanner);
        return T_STRING;
}

<ST_LOOKING_FOR_PROPERTY>{WHITESPACE} {
        RETSTEP(T_WHITESPACE);
}

<ST_LOOKING_FOR_PROPERTY>{ANY_CHAR} {
        yyless(0);
        yy_pop_state(yyscanner);
}

<ST_IN_SCRIPTING>"::"                { RETSTEP(T_DOUBLE_COLON);}
<ST_IN_SCRIPTING>"\\"                { RETTOKEN(T_NS_SEPARATOR);}
<ST_IN_SCRIPTING>"new"               { RETTOKEN(T_NEW);}
<ST_IN_SCRIPTING>"clone"             { RETTOKEN(T_CLONE);}
<ST_IN_SCRIPTING>"var"               { RETTOKEN(T_VAR);}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("int"|"integer"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_INT_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("real"|"double"|"float"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_DOUBLE_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("string"|"binary"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_STRING_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"array"{TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_ARRAY_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"object"{TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_OBJECT_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("bool"|"boolean"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_BOOL_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("unset"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION || !_scanner->isHHSyntaxEnabled()) {
    RETSTEP(T_UNSET_CAST);
  }
  yyless(1);
  RETSTEP('(');
}

<ST_IN_SCRIPTING>"callable"           { RETTOKEN(T_CALLABLE);}
<ST_IN_SCRIPTING>"eval"               { RETTOKEN(T_EVAL);}
<ST_IN_SCRIPTING>"include"            { RETTOKEN(T_INCLUDE);}
<ST_IN_SCRIPTING>"include_once"       { RETTOKEN(T_INCLUDE_ONCE);}
<ST_IN_SCRIPTING>"require"            { RETTOKEN(T_REQUIRE);}
<ST_IN_SCRIPTING>"require_once"       { RETTOKEN(T_REQUIRE_ONCE);}
<ST_IN_SCRIPTING>"namespace"          { RETTOKEN(T_NAMESPACE);}
<ST_IN_SCRIPTING>"use"                { RETTOKEN(T_USE);}
<ST_IN_SCRIPTING>"global"             { RETTOKEN(T_GLOBAL);}
<ST_IN_SCRIPTING>"isset"              { RETTOKEN(T_ISSET);}
<ST_IN_SCRIPTING>"empty"              { RETTOKEN(T_EMPTY);}
<ST_IN_SCRIPTING>"__halt_compiler"    { RETTOKEN(T_HALT_COMPILER);}
<ST_IN_SCRIPTING>"__compiler_halt_offset__" { RETTOKEN(T_COMPILER_HALT_OFFSET);}
<ST_IN_SCRIPTING>"static"             { RETTOKEN(T_STATIC);}
<ST_IN_SCRIPTING>"abstract"           { RETTOKEN(T_ABSTRACT);}
<ST_IN_SCRIPTING>"final"              { RETTOKEN(T_FINAL);}
<ST_IN_SCRIPTING>"private"            { RETTOKEN(T_PRIVATE);}
<ST_IN_SCRIPTING>"protected"          { RETTOKEN(T_PROTECTED);}
<ST_IN_SCRIPTING>"public"             { RETTOKEN(T_PUBLIC);}
<ST_IN_SCRIPTING>"unset"              { RETTOKEN(T_UNSET);}
<ST_IN_SCRIPTING>"==>"                { RETTOKEN(T_LAMBDA_ARROW);}
<ST_IN_SCRIPTING>"=>"                 { RETSTEP(T_DOUBLE_ARROW);}
<ST_IN_SCRIPTING>"list"               { RETTOKEN(T_LIST);}
<ST_IN_SCRIPTING>"array"              { RETTOKEN(T_ARRAY);}
<ST_IN_SCRIPTING>"++"                 { RETSTEP(T_INC);}
<ST_IN_SCRIPTING>"--"                 { RETSTEP(T_DEC);}
<ST_IN_SCRIPTING>"==="                { RETSTEP(T_IS_IDENTICAL);}
<ST_IN_SCRIPTING>"!=="                { RETSTEP(T_IS_NOT_IDENTICAL);}
<ST_IN_SCRIPTING>"=="                 { RETSTEP(T_IS_EQUAL);}
<ST_IN_SCRIPTING>"!="|"<>"            { RETSTEP(T_IS_NOT_EQUAL);}
<ST_IN_SCRIPTING>"<="                 { RETSTEP(T_IS_SMALLER_OR_EQUAL);}
<ST_IN_SCRIPTING>">="                 { RETSTEP(T_IS_GREATER_OR_EQUAL);}
<ST_IN_SCRIPTING>"+="                 { RETSTEP(T_PLUS_EQUAL);}
<ST_IN_SCRIPTING>"-="                 { RETSTEP(T_MINUS_EQUAL);}
<ST_IN_SCRIPTING>"*="                 { RETSTEP(T_MUL_EQUAL);}
<ST_IN_SCRIPTING>"/="                 { RETSTEP(T_DIV_EQUAL);}
<ST_IN_SCRIPTING>".="                 { RETSTEP(T_CONCAT_EQUAL);}
<ST_IN_SCRIPTING>"%="                 { RETSTEP(T_MOD_EQUAL);}
<ST_IN_SCRIPTING>"<<="                { RETSTEP(T_SL_EQUAL);}
<ST_IN_SCRIPTING>">>="                { RETSTEP(T_SR_EQUAL);}
<ST_IN_SCRIPTING>"&="                 { RETSTEP(T_AND_EQUAL);}
<ST_IN_SCRIPTING>"|="                 { RETSTEP(T_OR_EQUAL);}
<ST_IN_SCRIPTING>"^="                 { RETSTEP(T_XOR_EQUAL);}
<ST_IN_SCRIPTING>"||"                 { RETSTEP(T_BOOLEAN_OR);}
<ST_IN_SCRIPTING>"&&"                 { RETSTEP(T_BOOLEAN_AND);}
<ST_IN_SCRIPTING>"OR"                 { RETTOKEN(T_LOGICAL_OR);}
<ST_IN_SCRIPTING>"AND"                { RETTOKEN(T_LOGICAL_AND);}
<ST_IN_SCRIPTING>"XOR"                { RETTOKEN(T_LOGICAL_XOR);}
<ST_IN_SCRIPTING>"<<"                 { RETSTEP(T_SL);}

<ST_IN_SCRIPTING>"shape"              { HH_ONLY_KEYWORD(T_SHAPE); }
<ST_IN_SCRIPTING>"type"               { HH_ONLY_KEYWORD(T_UNRESOLVED_TYPE); }
<ST_IN_SCRIPTING>"newtype"            { HH_ONLY_KEYWORD(T_UNRESOLVED_NEWTYPE); }
<ST_IN_SCRIPTING>"await"              { HH_ONLY_KEYWORD(T_AWAIT);}
<ST_IN_SCRIPTING>"from"/{WHITESPACE_AND_COMMENTS}\$[a-zA-Z0-9_\x7f-\xff] {
  HH_ONLY_KEYWORD(T_FROM);
}
<ST_IN_SCRIPTING>"where"              { HH_ONLY_KEYWORD(T_WHERE); }
<ST_IN_SCRIPTING>"join"               { HH_ONLY_KEYWORD(T_JOIN); }
<ST_IN_SCRIPTING>"in"                 { HH_ONLY_KEYWORD(T_IN); }
<ST_IN_SCRIPTING>"on"                 { HH_ONLY_KEYWORD(T_ON); }
<ST_IN_SCRIPTING>"equals"             { HH_ONLY_KEYWORD(T_EQUALS); }
<ST_IN_SCRIPTING>"into"               { HH_ONLY_KEYWORD(T_INTO); }
<ST_IN_SCRIPTING>"let"                { HH_ONLY_KEYWORD(T_LET); }
<ST_IN_SCRIPTING>"orderby"            { HH_ONLY_KEYWORD(T_ORDERBY); }
<ST_IN_SCRIPTING>"ascending"          { HH_ONLY_KEYWORD(T_ASCENDING); }
<ST_IN_SCRIPTING>"descending"         { HH_ONLY_KEYWORD(T_DESCENDING); }
<ST_IN_SCRIPTING>"select"             { HH_ONLY_KEYWORD(T_SELECT); }
<ST_IN_SCRIPTING>"group"              { HH_ONLY_KEYWORD(T_GROUP); }
<ST_IN_SCRIPTING>"by"                 { HH_ONLY_KEYWORD(T_BY); }
<ST_IN_SCRIPTING>"async"/{WHITESPACE_AND_COMMENTS}[a-zA-Z0-9_\x7f-\xff] {
  HH_ONLY_KEYWORD(T_ASYNC);
}

<ST_IN_SCRIPTING>"tuple"/("("|{WHITESPACE_AND_COMMENTS}"(") {
  HH_ONLY_KEYWORD(T_TUPLE);
}

<ST_IN_SCRIPTING>"?"/":"[a-zA-Z_\x7f-\xff] {
  int ntt = getNextTokenType(_scanner->lastToken());
  if (!_scanner->isXHPSyntaxEnabled() ||
      ((ntt & NextTokenType::XhpClassName) && _scanner->lastToken() != '}')) {
    RETSTEP('?');
  }
  /* If XHP is enabled and "?:" occurs in a place where an XHP class name is
     not expected or it occurs after "}", drop into the ST_LOOKING_FOR_COLON
     state to avoid potentially treating ":" as the beginning of an XHP class
     name */
  BEGIN(ST_LOOKING_FOR_COLON);
  RETSTEP('?');
}

<ST_LOOKING_FOR_COLON>":" {
  BEGIN(ST_IN_SCRIPTING);
  RETSTEP(':');
}

<ST_IN_SCRIPTING>"..." {
  if (!_scanner->isHHSyntaxEnabled()) {
    yyless(1);
    RETSTEP('.');
  }
  RETTOKEN(T_VARARG);
}

<ST_IN_SCRIPTING>">>" {
  if (_scanner->getLookaheadLtDepth() < 2) {
    RETSTEP(T_SR);
  }
  yyless(1);
  RETSTEP('>');
}

<ST_IN_SCRIPTING>"<"[a-zA-Z_\x7f-\xff] {
  if (!_scanner->isXHPSyntaxEnabled()) {
    assert(!_scanner->isHHSyntaxEnabled());
    yyless(1);
    RETSTEP('<');
  }
  int ntt = getNextTokenType(_scanner->lastToken());
  if (ntt & NextTokenType::XhpTag) {
    yyless(1);
    STEPPOS(T_XHP_TAG_LT);
    yy_push_state(ST_XHP_IN_TAG, yyscanner);
    return T_XHP_TAG_LT;
  }
  if (ntt & NextTokenType::XhpTagMaybe) {
    // Shift to state state ST_LT_CHECK to do a more extensive check to
    // determine if this is the beginning of an XHP tag.
    yyless(0);
    BEGIN(ST_LT_CHECK);
    break;
  }
  yyless(1);
  if (_scanner->isHHSyntaxEnabled() && (ntt & NextTokenType::TypeListMaybe)) {
    // Return T_UNRESOLVED_LT; the scanner will inspect subseqent tokens
    // to resolve this.
    RETSTEP(T_UNRESOLVED_LT);
  }
  RETSTEP('<');
}

<ST_IN_SCRIPTING>"<" {
  if (_scanner->isHHSyntaxEnabled()) {
    int ntt = getNextTokenType(_scanner->lastToken());
    if (ntt & NextTokenType::TypeListMaybe) {
      // Return T_UNRESOLVED_LT; the scanner will inspect subseqent tokens
      // to resolve this.
      RETSTEP(T_UNRESOLVED_LT);
    }
  }
  RETSTEP('<');
}

<ST_LT_CHECK>"<"{XHPLABEL}(">"|"/>"|{WHITESPACE_AND_COMMENTS}(">"|"/>"|[a-zA-Z_\x7f-\xff])) {
  BEGIN(ST_IN_SCRIPTING);
  yyless(1);
  STEPPOS(T_XHP_TAG_LT);
  yy_push_state(ST_XHP_IN_TAG, yyscanner);
  return T_XHP_TAG_LT;
}

<ST_LT_CHECK>"<" {
  BEGIN(ST_IN_SCRIPTING);
  RETSTEP('<');
}

<ST_IN_SCRIPTING>":"{XHPLABEL}  {
  if (_scanner->isXHPSyntaxEnabled()) {
    int ntt = getNextTokenType(_scanner->lastToken());
    if (ntt & NextTokenType::XhpClassName) {
      yytext++; yyleng--; // skipping the first colon
      RETTOKEN(T_XHP_LABEL);
    }
  }
  yyless(1);
  RETSTEP(':');
}

<ST_IN_SCRIPTING>"%"{XHPLABEL}  {
  if (_scanner->isXHPSyntaxEnabled()) {
    int ntt = getNextTokenType(_scanner->lastToken());
    if (ntt & NextTokenType::XhpCategoryName) {
      yytext++; yyleng--; // skipping "%"
      RETTOKEN(T_XHP_CATEGORY_LABEL);
    }
  }
  yyless(1);
  RETSTEP('%');
}

<ST_IN_SCRIPTING>"("            {
  if (_scanner->isHHSyntaxEnabled()) {
    int ntt = getNextTokenType(_scanner->lastToken());
    if (ntt & NextTokenType::LambdaMaybe) {
      RETSTEP(T_UNRESOLVED_OP);
    }
  }
  RETSTEP(yytext[0]);
}

<ST_IN_SCRIPTING>{TOKENS}             {RETSTEP(yytext[0]);}

<ST_IN_SCRIPTING>"{" {
        STEPPOS('{');
        yy_push_state(ST_IN_SCRIPTING, yyscanner);
        return '{';
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"${" {
        STEPPOS(T_DOLLAR_OPEN_CURLY_BRACES);
        yy_push_state(ST_LOOKING_FOR_VARNAME, yyscanner);
        return T_DOLLAR_OPEN_CURLY_BRACES;
}

<ST_IN_SCRIPTING>"}"/":"[a-zA-Z_\x7f-\xff] {
        STEPPOS('}');
        // We need to be robust against a '}' in PHP code with
        // no corresponding '{'
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        if (yyg->yy_start_stack_ptr) {
          yy_pop_state(yyscanner);
          if (YY_START == ST_IN_SCRIPTING) {
            /* If XHP is enabled and "}:" occurs (and "}" does not cause us
               to transition to some state other than ST_IN_SCRIPTING), drop
               into the ST_LOOKING_FOR_COLON state to avoid potentially
               treating ":" as the beginning of an XHP class name */
            BEGIN(ST_LOOKING_FOR_COLON);
          }
        }
        return '}';
}

<ST_IN_SCRIPTING>"}" {
        STEPPOS('}');
        // We need to be robust against a '}' in PHP code with
        // no corresponding '{'
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        if (yyg->yy_start_stack_ptr) yy_pop_state(yyscanner);
        return '}';
}

<ST_LOOKING_FOR_VARNAME>{LABEL} {
        SETTOKEN(T_STRING_VARNAME);
        // Change state to IN_SCRIPTING; current state will be popped
        // when we encounter '}'
        BEGIN(ST_IN_SCRIPTING);
        return T_STRING_VARNAME;
}

<ST_LOOKING_FOR_VARNAME>{ANY_CHAR} {
        yyless(0);
        // Change state to IN_SCRIPTING; current state will be popped
        // when we encounter '}'
        BEGIN(ST_IN_SCRIPTING);
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{LNUM} {
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Dec number is too big: %s", yytext);
                if (_scanner->isHHFile()) {
                        RETTOKEN(T_HH_ERROR);
                }
        }
        RETTOKEN(T_LNUMBER);
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{HNUM} {
        errno = 0;
        long ret = strtoull(yytext, NULL, 16);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Hex number is too big: %s", yytext);
                if (_scanner->isHHFile()) {
                        RETTOKEN(T_HH_ERROR);
                }
        }
        RETTOKEN(T_LNUMBER);
}

<ST_VAR_OFFSET>0|([1-9][0-9]*) { /* Offset could be treated as a long */
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (ret == LLONG_MAX && errno == ERANGE) {
                _scanner->error("Offset number is too big: %s", yytext);
                if (_scanner->isHHFile()) {
                        RETTOKEN(T_HH_ERROR);
                }
        }
        RETTOKEN(T_NUM_STRING);
}

<ST_VAR_OFFSET>{LNUM}|{HNUM} { /* Offset must be treated as a string */
        RETTOKEN(T_NUM_STRING);
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{DNUM}|{EXPONENT_DNUM} {
        RETTOKEN(T_DNUMBER);
}

<ST_IN_SCRIPTING>"__CLASS__"            { RETTOKEN(T_CLASS_C); }
<ST_IN_SCRIPTING>"__TRAIT__"            { RETTOKEN(T_TRAIT_C); }
<ST_IN_SCRIPTING>"__FUNCTION__"         { RETTOKEN(T_FUNC_C); }
<ST_IN_SCRIPTING>"__METHOD__"           { RETTOKEN(T_METHOD_C);}
<ST_IN_SCRIPTING>"__LINE__"             { RETTOKEN(T_LINE); }
<ST_IN_SCRIPTING>"__FILE__"             { RETTOKEN(T_FILE); }
<ST_IN_SCRIPTING>"__DIR__"              { RETTOKEN(T_DIR); }
<ST_IN_SCRIPTING>"__NAMESPACE__"        { RETTOKEN(T_NS_C); }

<INITIAL>"#"[^\n]*"\n" {
        _scanner->setHashBang(yytext, yyleng, T_INLINE_HTML);
        BEGIN(ST_IN_SCRIPTING);
        yy_push_state(ST_AFTER_HASHBANG, yyscanner);
        return T_INLINE_HTML;
}

<INITIAL>(([^<#]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN(T_INLINE_HTML);
        BEGIN(ST_IN_SCRIPTING);
        yy_push_state(ST_IN_HTML, yyscanner);
        return T_INLINE_HTML;
}

<ST_IN_HTML,ST_AFTER_HASHBANG>(([^<]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN(T_INLINE_HTML);
        BEGIN(ST_IN_HTML);
        return T_INLINE_HTML;
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<?"|("<?php"([ \t]|{NEWLINE}))|"<script"{WHITESPACE}+"language"{WHITESPACE}*"="{WHITESPACE}*("php"|"\"php\""|"\'php\'"){WHITESPACE}*">" {
        if (_scanner->shortTags() || yyleng > 2) {
          SETTOKEN(T_OPEN_TAG);
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          return T_OPEN_TAG;
        } else {
          SETTOKEN(T_INLINE_HTML);
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
            yy_push_state(ST_IN_HTML, yyscanner);
          } else if (YY_START == ST_AFTER_HASHBANG) {
            BEGIN(ST_IN_HTML);
          }
          return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<%="|"<?=" {
        if ((yytext[1]=='%' && _scanner->aspTags()) ||
            (yytext[1]=='?' && _scanner->shortTags())) {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          RETTOKEN(T_ECHO); //return T_OPEN_TAG_WITH_ECHO;
        } else {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
            yy_push_state(ST_IN_HTML, yyscanner);
          } else if (YY_START == ST_AFTER_HASHBANG) {
            BEGIN(ST_IN_HTML);
          }
          RETTOKEN(T_INLINE_HTML);
        }
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<%" {
        if (_scanner->aspTags()) {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          RETTOKEN(T_OPEN_TAG);
        } else {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
            yy_push_state(ST_IN_HTML, yyscanner);
          } else if (YY_START == ST_AFTER_HASHBANG) {
            BEGIN(ST_IN_HTML);
          }
          RETTOKEN(T_INLINE_HTML);
        }
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<?hh"([ \t]|{NEWLINE}) {
        if (YY_START == INITIAL) {
          BEGIN(ST_IN_SCRIPTING);
        } else if (YY_START == ST_AFTER_HASHBANG) {
          yy_pop_state(yyscanner);
        } else {
          _scanner->error("HH mode: content before <?hh");
          return T_HH_ERROR;
        }
        STEPPOS(T_OPEN_TAG);
        _scanner->setHHFile();
        return T_OPEN_TAG;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE,ST_VAR_OFFSET>"$"{LABEL} {
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1, T_VARIABLE);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"->"[a-zA-Z_\x7f-\xff] {
        yyless(yyleng - 3);
        yy_push_state(ST_LOOKING_FOR_PROPERTY, yyscanner);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1, T_VARIABLE);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"[" {
        yyless(yyleng - 1);
        yy_push_state(ST_VAR_OFFSET, yyscanner);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1, T_VARIABLE);
        return T_VARIABLE;
}

<ST_VAR_OFFSET>"]" {
        yy_pop_state(yyscanner);
        return ']';
}

<ST_VAR_OFFSET>{TOKENS}|[({}\"`] {
        /* Only '[' can be valid, but returning other tokens will allow
           a more explicit parse error */
        return yytext[0];
}

<ST_VAR_OFFSET>[ \n\r\t\\\'#] {
        /* Invalid rule to return a more explicit parse error with proper
           line number */
        yyless(0);
        yy_pop_state(yyscanner);
        RETSTEP(T_ENCAPSED_AND_WHITESPACE);
}

<ST_IN_SCRIPTING,ST_VAR_OFFSET>{LABEL} {
        RETTOKEN(T_STRING);
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{WHITESPACE} {
        RETSTEP(T_WHITESPACE);
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>"#"|"//" {
        yy_push_state(ST_ONE_LINE_COMMENT, yyscanner);
        yymore();
}

<ST_ONE_LINE_COMMENT>"?"|"%"|">" {
        yymore();
}

<ST_ONE_LINE_COMMENT>[^\n\r?%>]*{ANY_CHAR} {
        switch (yytext[yyleng-1]) {
        case '?':
        case '%':
        case '>':
                yyless(yyleng-1);
                yymore();
                break;
        default:
                STEPPOS(T_COMMENT);
                yy_pop_state(yyscanner);
                return T_COMMENT;
        }
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
        STEPPOS(T_COMMENT);
        yy_pop_state(yyscanner);
        return T_COMMENT;
}

<ST_ONE_LINE_COMMENT>"?>"|"%>" {
        if (_scanner->isHHFile()) {
          _scanner->error("HH mode: ?> not allowed");
          return T_HH_ERROR;
        }
        if (_scanner->aspTags() || yytext[yyleng-2] != '%') {
          _scanner->setToken(yytext, yyleng-2, yytext, yyleng-2, T_COMMENT);
                yyless(yyleng-2);
                yy_pop_state(yyscanner);
                return T_COMMENT;
        } else {
                yymore();
        }
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>"/**"{WHITESPACE} {
        yy_push_state(ST_DOC_COMMENT, yyscanner);
        yymore();
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>"/*" {
        yy_push_state(ST_COMMENT, yyscanner);
        yymore();
}

<ST_COMMENT,ST_DOC_COMMENT>[^*]+ {
        yymore();
}

<ST_DOC_COMMENT>"*/" {
        SETTOKEN(T_DOC_COMMENT);
        yy_pop_state(yyscanner);
        return T_DOC_COMMENT;
}

<ST_COMMENT>"*/" {
        STEPPOS(T_COMMENT);
        yy_pop_state(yyscanner);
        return T_COMMENT;
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
        yymore();
}

<ST_XHP_COMMENT>[^-]+ {
        yymore();
}

<ST_XHP_COMMENT>"-->" {
        STEPPOS(T_COMMENT);
        yy_pop_state(yyscanner);
        return T_COMMENT;
}

<ST_XHP_COMMENT>"-" {
        yymore();
}

<ST_IN_SCRIPTING>"?>"{NEWLINE}? {
        if (_scanner->isHHFile()) {
          _scanner->error("HH mode: ?> not allowed");
          return T_HH_ERROR;
        }
        yy_push_state(ST_IN_HTML, yyscanner);
        if (_scanner->full()) {
          RETSTEP(T_CLOSE_TAG);
        } else {
          RETSTEP(';');
        }
}

<ST_IN_SCRIPTING>"</script"{WHITESPACE}*">"{NEWLINE}? {
        yy_push_state(ST_IN_HTML, yyscanner);
        if (_scanner->full()) {
          RETSTEP(T_CLOSE_TAG);
        } else {
          RETSTEP(';');
        }
}

<ST_IN_SCRIPTING>"%>"{NEWLINE}? {
        if (_scanner->aspTags()) {
                yy_push_state(ST_IN_HTML, yyscanner);
                if (_scanner->full()) {
                  RETSTEP(T_CLOSE_TAG);
                } else {
                  RETSTEP(';');
                }
        } else {
                yyless(1);
                _scanner->setToken(yytext, 1, yytext, 1);
                RETSTEP(yytext[0]);
        }
}

<ST_IN_SCRIPTING>(b?[\"]{DOUBLE_QUOTES_CHARS}*("{"*|"$"*)[\"]) {
        int bprefix = (yytext[0] != '"') ? 1 : 0;
        std::string strval =
          _scanner->escape(yytext + bprefix + 1,
                           yyleng - bprefix - 2, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>(b?[\']([^\'\\]|("\\"{ANY_CHAR}))*[\']?) {
        int bprefix = (yytext[0] != '\'') ? 1 : 0;
        int closed = (yytext[yyleng - 1] == '\'');
        std::string strval =
          _scanner->escape(yytext + bprefix + 1,
                           yyleng - bprefix - 2, '\'');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return closed ? T_CONSTANT_ENCAPSED_STRING : T_ENCAPSED_AND_WHITESPACE;
}

<ST_IN_SCRIPTING>b?[\"] {
        int bprefix = (yytext[0] != '"') ? 1 : 0;
        _scanner->setToken(yytext, yyleng, yytext + bprefix, yyleng - bprefix);
        BEGIN(ST_DOUBLE_QUOTES);
        return '\"';
}

<ST_IN_SCRIPTING>b?"<<<"{TABS_AND_SPACES}({LABEL}|[']{LABEL}[']|["]{LABEL}["]){NEWLINE} {
        int bprefix = (yytext[0] != '<') ? 1 : 0;
        int label_len = yyleng-bprefix-3-1-(yytext[yyleng-2]=='\r'?1:0);
        char *s = yytext+bprefix+3;
        while ((*s == ' ') || (*s == '\t')) {
                s++;
                label_len--;
        }
        if (*s == '\'') {
                s++;
                label_len -= 2;
                BEGIN(ST_NOWDOC);
        } else {
                if (*s == '"') {
                       s++;
                       label_len -= 2;
                }
                BEGIN(ST_HEREDOC);
        }
        _scanner->setHeredocLabel(s, label_len);
        _scanner->setToken(yytext, yyleng, s, label_len);
        return T_START_HEREDOC;
}

<ST_IN_SCRIPTING>[`] {
        STEPPOS('`');
        BEGIN(ST_BACKQUOTE);
        return '`';
}

<ST_XHP_IN_TAG>{XHPLABEL} {
        RETTOKEN(T_XHP_LABEL);
}

<ST_XHP_IN_TAG>"=" {
  RETSTEP(yytext[0]);
}

<ST_XHP_IN_TAG>["][^"]*["] {
  _scanner->setToken(yytext, yyleng, yytext+1, yyleng-2);
  return T_XHP_TEXT;
}

<ST_XHP_IN_TAG>[{] {
  STEPPOS('{');
  yy_push_state(ST_IN_SCRIPTING, yyscanner);
  return '{';
}

<ST_XHP_IN_TAG>">" {
  STEPPOS(T_XHP_TAG_GT);
  BEGIN(ST_XHP_CHILD);
  return T_XHP_TAG_GT;
}

<ST_XHP_IN_TAG>"/>" {
  BEGIN(ST_XHP_END_SINGLETON_TAG);
  yyless(1);
  return '/';
}

<ST_XHP_IN_TAG>{ANY_CHAR} {
  // This rule ensures we get a reasonable syntax error message
  // when unexpected characters occur inside XHP tags
  STEPPOS(yytext[0]);
  _scanner->error("Unexpected character in input: '%c' (ASCII=%d)",
                  yytext[0], yytext[0]);
  return yytext[0];
}

<ST_XHP_END_SINGLETON_TAG>">" {
  STEPPOS(T_XHP_TAG_GT);
  yy_pop_state(yyscanner);
  return T_XHP_TAG_GT;
}

<ST_XHP_CHILD>"<!--" {
  yy_push_state(ST_XHP_COMMENT, yyscanner);
  yymore();
}

<ST_XHP_CHILD>[^{<]+ {
  RETTOKEN(T_XHP_TEXT);
}

<ST_XHP_CHILD>"{" {
  STEPPOS('{');
  yy_push_state(ST_IN_SCRIPTING, yyscanner);
  return '{';
}

<ST_XHP_CHILD>"</" {
  BEGIN(ST_XHP_END_CLOSE_TAG);
  yyless(1);
  RETSTEP(T_XHP_TAG_LT);
}

<ST_XHP_END_CLOSE_TAG>"/" {
  RETSTEP('/');
}

<ST_XHP_END_CLOSE_TAG>{XHPLABEL} {
  RETTOKEN(T_XHP_LABEL);
}

<ST_XHP_END_CLOSE_TAG>">" {
  STEPPOS(T_XHP_TAG_GT);
  yy_pop_state(yyscanner);
  return T_XHP_TAG_GT;
}

<ST_XHP_CHILD>"<" {
  STEPPOS(T_XHP_TAG_LT);
  yy_push_state(ST_XHP_IN_TAG, yyscanner);
  return T_XHP_TAG_LT;
}

<ST_HEREDOC,ST_NOWDOC>{ANY_CHAR} {
  int refillResult = EOB_ACT_CONTINUE_SCAN;
  std::vector<std::string> docPieces;
  size_t totalDocSize = 0;
  std::string entireDoc;
  int docLabelLen = _scanner->getHeredocLabelLen();
  bool isHeredoc = (YYSTATE == ST_HEREDOC);
  DECLARE_YYCURSOR;
  DECLARE_YYLIMIT;

  YYCURSOR--;

  // The rules that lead to this state all consume an end-of-line.
  bool lookingForEndLabel = true;

  while (refillResult == EOB_ACT_CONTINUE_SCAN) {
    while (YYCURSOR < YYLIMIT) {
      switch (*YYCURSOR++) {
        case '\r':
          lookingForEndLabel = true;
          continue;
        case '\n':
          lookingForEndLabel = true;
          continue;
        case '$':
          lookingForEndLabel = false;
          if (isHeredoc) {
            if (YYCURSOR == YYLIMIT) {
              --YYCURSOR;
              goto doc_scan_get_more_buffer;
            }
            if (IS_LABEL_START(*YYCURSOR) || *YYCURSOR == '{') {
              --YYCURSOR;
              goto doc_scan_done;
            }
          }
          continue;
        case '{':
          lookingForEndLabel = false;
          if (isHeredoc) {
            if (YYCURSOR == YYLIMIT) {
              --YYCURSOR;
              goto doc_scan_get_more_buffer;
            }
            if (*YYCURSOR == '$') {
              --YYCURSOR;
              goto doc_scan_done;
            }
          }
          continue;
        case '\\':
          lookingForEndLabel = false;
          if (isHeredoc) {
            if (YYCURSOR == YYLIMIT) {
              --YYCURSOR;
              goto doc_scan_get_more_buffer;
            }
            if (*YYCURSOR != '\n' && *YYCURSOR != '\r') {
              YYCURSOR++;
            }
          }
          continue;
        default:
          if (lookingForEndLabel) {
            lookingForEndLabel = false;

            // Check for ending label on this line.
            if (!IS_LABEL_START(YYCURSOR[-1])) continue;

            // Adjust cursor to the start of the potential label.
            // If a label is recgonized, we want the cursor pointing at it.
            --YYCURSOR;

            if ((docLabelLen + 2) > (YYLIMIT - YYCURSOR)) {
              lookingForEndLabel = true;
              goto doc_scan_get_more_buffer;
            }

            if (!memcmp(YYCURSOR, _scanner->getHeredocLabel(), docLabelLen)) {
              const char *end = YYCURSOR + docLabelLen;
              if (*end == ';') {
                end++;
              }
              if (*end == '\n' || *end == '\r') {
                BEGIN(ST_END_HEREDOC);
                goto doc_scan_done;
              }
            }
            ++YYCURSOR; // No label found, consume this character.
          }
          continue;
      }
    }

doc_scan_get_more_buffer:
    // We ran off the end of the buffer, but no end label has been found.
    // Save off the string we have so far, re-fill the buffer, and repeat.
    yyleng = YYCURSOR - yytext;
    docPieces.emplace_back(yytext, yyleng);
    if (totalDocSize >= entireDoc.max_size() - yyleng) {
      _scanner->error("%sdoc too large", isHeredoc ? "Here" : "Now");
      return 0;
    }
    totalDocSize += yyleng;

    // yy_get_next_buffer() needs the text pointing at the data we want to keep
    // in the buffer, and the cursor pointing off the end. It will move what's
    // at yytext (if anything) to the beginning of the buffer and fill the rest
    // with new data.
    yytext = yytext + yyleng;
    yyleng = 0;
    YYCURSOR = YYLIMIT + 1;
    refillResult = yy_get_next_buffer(yyscanner);

    // Point to the beginning of the (possibly new) buffer.
    YYCURSOR = yyg->yy_c_buf_p = yytext;
    YYLIMIT = YY_CURRENT_BUFFER->yy_ch_buf + yyg->yy_n_chars;
  }

  _scanner->error("Unterminated %sdoc at end of file",
                  isHeredoc ? "here" : "now");
  return 0;

doc_scan_done:
  yyleng = YYCURSOR - yytext;
  totalDocSize += yyleng;
  RESET_YYCURSOR;

  if (totalDocSize > 0) {
    entireDoc.reserve(totalDocSize);

    for (const auto& piece: docPieces) {
      entireDoc.append(piece);
    }

    if (yyleng > 0) {
      entireDoc.append(yytext, yyleng);
    }

    // Newline before label will be subtracted from returned text, but
    // raw text will include it, for zend_highlight/strip, tokenizer, etc.
    int newline = 0;
    bool endLabelFound = (YYSTATE == ST_END_HEREDOC);
    if (endLabelFound && (entireDoc.length() > 0)) {
      auto it = entireDoc.end();
      if (*--it == '\n') {
        ++newline;
        if ((entireDoc.length() > 1) && (*--it == '\r')) {
          ++newline;
        }
      }
    }

    if (isHeredoc) {
      std::string escapedDoc = _scanner->escape(entireDoc.c_str(),
                                                entireDoc.length() - newline,
                                                0);
      _scanner->setToken(entireDoc.c_str(), entireDoc.length(),
                         escapedDoc.c_str(), escapedDoc.length());
    } else {
      _scanner->setToken(entireDoc.c_str(), entireDoc.length(),
                         entireDoc.c_str(), entireDoc.length() - newline);
    }
    return T_ENCAPSED_AND_WHITESPACE;
  } else {
    // No data before the label means we just go right to ST_END_HEREDOC
    // without forming a new token.
  }
}

<ST_END_HEREDOC>{LABEL} {
        BEGIN(ST_IN_SCRIPTING);
        RETSTEP(T_END_HEREDOC);
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"{$" {
        _scanner->setToken(yytext, 1, yytext, 1);
        yy_push_state(ST_IN_SCRIPTING, yyscanner);
        yyless(1);
        return T_CURLY_OPEN;
}

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS}+ {
        std::string strval = _scanner->escape(yytext, yyleng, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS}*("{"{2,}|"$"{2,}|(("{"+|"$"+)[\"])) {
        yyless(yyleng - 1);
        std::string strval = _scanner->escape(yytext, yyleng, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_BACKQUOTE>{BACKQUOTE_CHARS}+ {
        std::string strval = _scanner->escape(yytext, yyleng, '`');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_BACKQUOTE>{BACKQUOTE_CHARS}*("{"{2,}|"$"{2,}|(("{"+|"$"+)[`])) {
        yyless(yyleng - 1);
        std::string strval = _scanner->escape(yytext, yyleng, '`');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_DOUBLE_QUOTES>[\"] {
        BEGIN(ST_IN_SCRIPTING);
        return '"';
}

<ST_BACKQUOTE>[\`] {
        BEGIN(ST_IN_SCRIPTING);
        return '`';
}

<ST_COMMENT,ST_DOC_COMMENT><<EOF>> {
        _scanner->error("Unterminated comment at end of file");
        return 0;
}

<*>{ANY_CHAR} {
        _scanner->error("Unexpected character in input: '%c' (ASCII=%d)",
                        yytext[0], yytext[0]);
}

%%

namespace HPHP {
  void Scanner::init() {
    yylex_init_extra(this, &m_yyscanner);
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    BEGIN(INITIAL);
  }

  int Scanner::scan() {
    return yylex(m_token, m_loc, m_yyscanner);
  }

  void Scanner::reset() {
    void *yyscanner = (void *)m_yyscanner;
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    YY_FLUSH_BUFFER;
    yylex_destroy(m_yyscanner);
  }

  static void suppress_unused_errors() {
    yyunput(0,0,0);
    yy_top_state(0);
    suppress_unused_errors();
  }
}

extern "C" {
  int yywrap(yyscan_t yyscanner) {
    return 1;
  }
}
