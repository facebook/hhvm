%{ /* -*- mode: c++ -*- */
#include <util/parser/scanner.h>

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
#define SETTOKEN _scanner->setToken(yytext, yyleng)
#define STEPPOS  _scanner->stepPos(yytext, yyleng)

#define HH_ONLY_KEYWORD(tok) do {                             \
  SETTOKEN;                                                   \
  return _scanner->hipHopSyntaxEnabled() ? tok : T_STRING;    \
} while (0)

#define IS_LABEL_START(c) \
  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || \
   (c) == '_' || (c) >= 0x7F)

/**
 * "Next token" types tell us how to treat a token based on the previous
 * token for the purpose of recognizing XHP tags, XHP class names, XHP
 * category names, and type lists.
 *   XhpTag:
 *     '<' should be recognized as the start of an XHP tag
 *   XhpTagMaybe:
 *     '<' should be recognized as possibly being the start of an XHP tag;
 *     this will be resolved by inspecting subsequent characters
 *   XhpClassName:
 *     ':' should be recognized as the start of an XHP class name
 *   XhpCategoryName:
 *     '%' should be recognized as the start of an XHP category name
 *   TypeListMaybe:
 *     '<' should be recognized as possibly being the start of a type list;
 *     this will be resolved by inspecting subsequent tokens
 */
namespace NextTokenType {
  static const int Normal = 0x1;
  static const int XhpTag = 0x2;
  static const int XhpTagMaybe = 0x4;
  static const int XhpClassName = 0x8;
  static const int XhpCategoryName = 0x10;
  static const int TypeListMaybe = 0x20;
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
    case T_NEW:
    case T_INSTANCEOF:
    case T_DOUBLE_ARROW:
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
      return NextTokenType::XhpTag |
             NextTokenType::XhpClassName;
    case ',': case '(': case '|':
      return NextTokenType::XhpTag |
             NextTokenType::XhpClassName |
             NextTokenType::XhpCategoryName;
    case '}':
      return NextTokenType::XhpTagMaybe |
             NextTokenType::XhpClassName;
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
%x ST_VAR_OFFSET
%x ST_LT_CHECK
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT

%x ST_XHP_IN_TAG
%x ST_XHP_END_SINGLETON_TAG
%x ST_XHP_END_CLOSE_TAG
%x ST_XHP_CHILD

%option stack

LNUM    [0-9]+
DNUM    ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+\-*/=%!~$<>?@]
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")
XHPLABEL {LABEL}([:-]{LABEL})*
COMMENT_REGEX ([\/][\*]([^\*]|(\*[^/]))*[\*][\/]|"//"[^\r\n]*{NEWLINE})
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

<ST_IN_SCRIPTING>"exit"                 { SETTOKEN; return T_EXIT;}
<ST_IN_SCRIPTING>"die"                  { SETTOKEN; return T_EXIT;}
<ST_IN_SCRIPTING>"function"             { SETTOKEN; return T_FUNCTION;}
<ST_IN_SCRIPTING>"const"                { SETTOKEN; return T_CONST;}
<ST_IN_SCRIPTING>"return"               { SETTOKEN; return T_RETURN;}
<ST_IN_SCRIPTING>"yield"                { SETTOKEN; return T_YIELD;}
<ST_IN_SCRIPTING>"try"                  { SETTOKEN; return T_TRY;}
<ST_IN_SCRIPTING>"catch"                { SETTOKEN; return T_CATCH;}
<ST_IN_SCRIPTING>"finally"              { SETTOKEN; return T_FINALLY;}
<ST_IN_SCRIPTING>"throw"                { SETTOKEN; return T_THROW;}
<ST_IN_SCRIPTING>"if"                   { SETTOKEN; return T_IF;}
<ST_IN_SCRIPTING>"elseif"               { SETTOKEN; return T_ELSEIF;}
<ST_IN_SCRIPTING>"endif"                { SETTOKEN; return T_ENDIF;}
<ST_IN_SCRIPTING>"else"                 { SETTOKEN; return T_ELSE;}
<ST_IN_SCRIPTING>"while"                { SETTOKEN; return T_WHILE;}
<ST_IN_SCRIPTING>"endwhile"             { SETTOKEN; return T_ENDWHILE;}
<ST_IN_SCRIPTING>"do"                   { SETTOKEN; return T_DO;}
<ST_IN_SCRIPTING>"for"                  { SETTOKEN; return T_FOR;}
<ST_IN_SCRIPTING>"endfor"               { SETTOKEN; return T_ENDFOR;}
<ST_IN_SCRIPTING>"foreach"              { SETTOKEN; return T_FOREACH;}
<ST_IN_SCRIPTING>"endforeach"           { SETTOKEN; return T_ENDFOREACH;}
<ST_IN_SCRIPTING>"declare"              { SETTOKEN; return T_DECLARE;}
<ST_IN_SCRIPTING>"enddeclare"           { SETTOKEN; return T_ENDDECLARE;}
<ST_IN_SCRIPTING>"instanceof"           { SETTOKEN; return T_INSTANCEOF;}
<ST_IN_SCRIPTING>"as"                   { SETTOKEN; return T_AS;}
<ST_IN_SCRIPTING>"switch"               { SETTOKEN; return T_SWITCH;}
<ST_IN_SCRIPTING>"endswitch"            { SETTOKEN; return T_ENDSWITCH;}
<ST_IN_SCRIPTING>"case"                 { SETTOKEN; return T_CASE;}
<ST_IN_SCRIPTING>"default"              { SETTOKEN; return T_DEFAULT;}
<ST_IN_SCRIPTING>"break"                { SETTOKEN; return T_BREAK;}
<ST_IN_SCRIPTING>"continue"             { SETTOKEN; return T_CONTINUE;}
<ST_IN_SCRIPTING>"goto"                 { SETTOKEN; return T_GOTO;}
<ST_IN_SCRIPTING>"echo"                 { SETTOKEN; return T_ECHO;}
<ST_IN_SCRIPTING>"print"                { SETTOKEN; return T_PRINT;}
<ST_IN_SCRIPTING>"class"                { SETTOKEN; return T_CLASS;}
<ST_IN_SCRIPTING>"interface"            { SETTOKEN; return T_INTERFACE;}
<ST_IN_SCRIPTING>"trait"                { SETTOKEN; return T_TRAIT;}
<ST_IN_SCRIPTING>"insteadof"            { SETTOKEN; return T_INSTEADOF;}
<ST_IN_SCRIPTING>"extends"              { SETTOKEN; return T_EXTENDS;}
<ST_IN_SCRIPTING>"implements"           { SETTOKEN; return T_IMPLEMENTS;}
<ST_IN_SCRIPTING>"attribute"            { SETTOKEN; return T_XHP_ATTRIBUTE;}
<ST_IN_SCRIPTING>"category"             { SETTOKEN; return T_XHP_CATEGORY;}
<ST_IN_SCRIPTING>"children"             { SETTOKEN; return T_XHP_CHILDREN;}
<ST_IN_SCRIPTING>"required"             { SETTOKEN; return T_XHP_REQUIRED;}
<ST_IN_SCRIPTING>"enum"                 { SETTOKEN; return T_XHP_ENUM;}

<ST_IN_SCRIPTING>"->" {
        STEPPOS;
        yy_push_state(ST_LOOKING_FOR_PROPERTY, yyscanner);
        return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>"->" {
        STEPPOS;
        return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>{LABEL} {
        SETTOKEN;
        yy_pop_state(yyscanner);
        return T_STRING;
}

<ST_LOOKING_FOR_PROPERTY>{ANY_CHAR} {
        yyless(0);
        yy_pop_state(yyscanner);
}

<ST_IN_SCRIPTING>"::"                { STEPPOS;return T_PAAMAYIM_NEKUDOTAYIM;}
<ST_IN_SCRIPTING>"\\"                { SETTOKEN;return T_NS_SEPARATOR;}
<ST_IN_SCRIPTING>"new"               { SETTOKEN;return T_NEW;}
<ST_IN_SCRIPTING>"clone"             { SETTOKEN;return T_CLONE;}
<ST_IN_SCRIPTING>"var"               { SETTOKEN;return T_VAR;}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("int"|"integer"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_INT_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("real"|"double"|"float"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_DOUBLE_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("string"|"binary"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_STRING_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"array"{TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_ARRAY_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"object"{TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_OBJECT_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("bool"|"boolean"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_BOOL_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("unset"){TABS_AND_SPACES}")" {
  if (_scanner->lastToken() != T_FUNCTION) {
    STEPPOS;
    return T_UNSET_CAST;
  }
  yyless(1);
  STEPPOS;
  return '(';
}

<ST_IN_SCRIPTING>"eval"               { SETTOKEN; return T_EVAL;}
<ST_IN_SCRIPTING>"include"            { SETTOKEN; return T_INCLUDE;}
<ST_IN_SCRIPTING>"include_once"       { SETTOKEN; return T_INCLUDE_ONCE;}
<ST_IN_SCRIPTING>"require"            { SETTOKEN; return T_REQUIRE;}
<ST_IN_SCRIPTING>"require_once"       { SETTOKEN; return T_REQUIRE_ONCE;}
<ST_IN_SCRIPTING>"namespace"          { SETTOKEN; return T_NAMESPACE;}
<ST_IN_SCRIPTING>"use"                { SETTOKEN; return T_USE;}
<ST_IN_SCRIPTING>"global"             { SETTOKEN; return T_GLOBAL;}
<ST_IN_SCRIPTING>"isset"              { SETTOKEN; return T_ISSET;}
<ST_IN_SCRIPTING>"empty"              { SETTOKEN; return T_EMPTY;}
<ST_IN_SCRIPTING>"__halt_compiler"    { SETTOKEN; return T_HALT_COMPILER;}
<ST_IN_SCRIPTING>"__compiler_halt_offset__" {
  SETTOKEN;
  return T_COMPILER_HALT_OFFSET;
}
<ST_IN_SCRIPTING>"static"             { SETTOKEN; return T_STATIC;}
<ST_IN_SCRIPTING>"abstract"           { SETTOKEN; return T_ABSTRACT;}
<ST_IN_SCRIPTING>"final"              { SETTOKEN; return T_FINAL;}
<ST_IN_SCRIPTING>"private"            { SETTOKEN; return T_PRIVATE;}
<ST_IN_SCRIPTING>"protected"          { SETTOKEN; return T_PROTECTED;}
<ST_IN_SCRIPTING>"public"             { SETTOKEN; return T_PUBLIC;}
<ST_IN_SCRIPTING>"unset"              { SETTOKEN; return T_UNSET;}
<ST_IN_SCRIPTING>"=>"                 { STEPPOS; return T_DOUBLE_ARROW;}
<ST_IN_SCRIPTING>"list"               { SETTOKEN; return T_LIST;}
<ST_IN_SCRIPTING>"array"              { SETTOKEN; return T_ARRAY;}
<ST_IN_SCRIPTING>"++"                 { STEPPOS; return T_INC;}
<ST_IN_SCRIPTING>"--"                 { STEPPOS; return T_DEC;}
<ST_IN_SCRIPTING>"==="                { STEPPOS; return T_IS_IDENTICAL;}
<ST_IN_SCRIPTING>"!=="                { STEPPOS; return T_IS_NOT_IDENTICAL;}
<ST_IN_SCRIPTING>"=="                 { STEPPOS; return T_IS_EQUAL;}
<ST_IN_SCRIPTING>"!="|"<>"            { STEPPOS; return T_IS_NOT_EQUAL;}
<ST_IN_SCRIPTING>"<="                 { STEPPOS; return T_IS_SMALLER_OR_EQUAL;}
<ST_IN_SCRIPTING>">="                 { STEPPOS; return T_IS_GREATER_OR_EQUAL;}
<ST_IN_SCRIPTING>"+="                 { STEPPOS; return T_PLUS_EQUAL;}
<ST_IN_SCRIPTING>"-="                 { STEPPOS; return T_MINUS_EQUAL;}
<ST_IN_SCRIPTING>"*="                 { STEPPOS; return T_MUL_EQUAL;}
<ST_IN_SCRIPTING>"/="                 { STEPPOS; return T_DIV_EQUAL;}
<ST_IN_SCRIPTING>".="                 { STEPPOS; return T_CONCAT_EQUAL;}
<ST_IN_SCRIPTING>"%="                 { STEPPOS; return T_MOD_EQUAL;}
<ST_IN_SCRIPTING>"<<="                { STEPPOS; return T_SL_EQUAL;}
<ST_IN_SCRIPTING>">>="                { STEPPOS; return T_SR_EQUAL;}
<ST_IN_SCRIPTING>"&="                 { STEPPOS; return T_AND_EQUAL;}
<ST_IN_SCRIPTING>"|="                 { STEPPOS; return T_OR_EQUAL;}
<ST_IN_SCRIPTING>"^="                 { STEPPOS; return T_XOR_EQUAL;}
<ST_IN_SCRIPTING>"||"                 { STEPPOS; return T_BOOLEAN_OR;}
<ST_IN_SCRIPTING>"&&"                 { STEPPOS; return T_BOOLEAN_AND;}
<ST_IN_SCRIPTING>"OR"                 { SETTOKEN; return T_LOGICAL_OR;}
<ST_IN_SCRIPTING>"AND"                { SETTOKEN; return T_LOGICAL_AND;}
<ST_IN_SCRIPTING>"XOR"                { SETTOKEN; return T_LOGICAL_XOR;}
<ST_IN_SCRIPTING>"<<"                 { STEPPOS; return T_SL;}
<ST_IN_SCRIPTING>"..."                { SETTOKEN; return T_VARARG; }

<ST_IN_SCRIPTING>"shape"              { HH_ONLY_KEYWORD(T_SHAPE); }
<ST_IN_SCRIPTING>"type"               { HH_ONLY_KEYWORD(T_UNRESOLVED_TYPE); }
<ST_IN_SCRIPTING>"newtype"            { HH_ONLY_KEYWORD(T_UNRESOLVED_TYPE); }

<ST_IN_SCRIPTING>">>" {
  if (_scanner->getLookaheadLtDepth() < 2) {
    STEPPOS;
    return T_SR;
  }
  yyless(1);
  STEPPOS;
  return '>';
}

<ST_IN_SCRIPTING>"<"[a-zA-Z_\x7f-\xff] {
  int ntt = getNextTokenType(_scanner->lastToken());
  if (ntt & NextTokenType::XhpTag) {
    yyless(1);
    STEPPOS;
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
  STEPPOS;
  if (_scanner->hipHopSyntaxEnabled() && (ntt & NextTokenType::TypeListMaybe)) {
    // Return T_UNRESOLVED_LT; the scanner will inspect subseqent tokens
    // to resolve this.
    return T_UNRESOLVED_LT;
  }
  return '<';
}

<ST_IN_SCRIPTING>"<" {
  STEPPOS;
  if (_scanner->hipHopSyntaxEnabled()) {
    int ntt = getNextTokenType(_scanner->lastToken());
    if (ntt & NextTokenType::TypeListMaybe) {
      // Return T_UNRESOLVED_LT; the scanner will inspect subseqent tokens
      // to resolve this.
      return T_UNRESOLVED_LT;
    }
  }
  return '<';
}

<ST_LT_CHECK>"<"{XHPLABEL}(">"|"/>"|{WHITESPACE_AND_COMMENTS}(">"|"/>"|[a-zA-Z_\x7f-\xff])) {
  BEGIN(ST_IN_SCRIPTING);
  yyless(1);
  STEPPOS;
  yy_push_state(ST_XHP_IN_TAG, yyscanner);
  return T_XHP_TAG_LT;
}

<ST_LT_CHECK>"<" {
  BEGIN(ST_IN_SCRIPTING);
  STEPPOS;
  return '<';
}

<ST_IN_SCRIPTING>":"{XHPLABEL}  {
  int ntt = getNextTokenType(_scanner->lastToken());
  if (ntt & NextTokenType::XhpClassName) {
    yytext++; yyleng--; // skipping the first colon
    SETTOKEN;
    return T_XHP_LABEL;
  }
  yyless(1);
  STEPPOS;
  return ':';
}

<ST_IN_SCRIPTING>"%"{XHPLABEL}  {
  int ntt = getNextTokenType(_scanner->lastToken());
  if (ntt & NextTokenType::XhpCategoryName) {
    yytext++; yyleng--; // skipping "%"
    SETTOKEN;
    return T_XHP_CATEGORY_LABEL;
  }
  yyless(1);
  STEPPOS;
  return '%';
}

<ST_IN_SCRIPTING>{TOKENS}             {STEPPOS; return yytext[0];}

<ST_IN_SCRIPTING>"{" {
        STEPPOS;
        yy_push_state(ST_IN_SCRIPTING, yyscanner);
        return '{';
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"${" {
        STEPPOS;
        yy_push_state(ST_LOOKING_FOR_VARNAME, yyscanner);
        return T_DOLLAR_OPEN_CURLY_BRACES;
}

<ST_IN_SCRIPTING>"}" {
        STEPPOS;
        // We need to be robust against a '}' in PHP code with
        // no corresponding '{'
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        if (yyg->yy_start_stack_ptr) yy_pop_state(yyscanner);
        return '}';
}

<ST_LOOKING_FOR_VARNAME>{LABEL} {
        SETTOKEN;
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
        SETTOKEN;
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Dec number is too big: %s", yytext);
                if (_scanner->isHackMode()) {
                        return T_HACK_ERROR;
                }
        }
        return T_LNUMBER;
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{HNUM} {
        SETTOKEN;
        errno = 0;
        long ret = strtoull(yytext, NULL, 16);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Hex number is too big: %s", yytext);
                if (_scanner->isHackMode()) {
                        return T_HACK_ERROR;
                }
        }
        return T_LNUMBER;
}

<ST_VAR_OFFSET>0|([1-9][0-9]*) { /* Offset could be treated as a long */
        SETTOKEN;
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (ret == LLONG_MAX && errno == ERANGE) {
                _scanner->error("Offset number is too big: %s", yytext);
                if (_scanner->isHackMode()) {
                        return T_HACK_ERROR;
                }
        }
        return T_NUM_STRING;
}

<ST_VAR_OFFSET>{LNUM}|{HNUM} { /* Offset must be treated as a string */
        SETTOKEN;
        return T_NUM_STRING;
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{DNUM}|{EXPONENT_DNUM} {
        SETTOKEN;
        return T_DNUMBER;
}

<ST_IN_SCRIPTING>"__CLASS__"            { SETTOKEN; return T_CLASS_C; }
<ST_IN_SCRIPTING>"__TRAIT__"            { SETTOKEN; return T_TRAIT_C; }
<ST_IN_SCRIPTING>"__FUNCTION__"         { SETTOKEN; return T_FUNC_C;  }
<ST_IN_SCRIPTING>"__METHOD__"           { SETTOKEN; return T_METHOD_C;}
<ST_IN_SCRIPTING>"__LINE__"             { SETTOKEN; return T_LINE;    }
<ST_IN_SCRIPTING>"__FILE__"             { SETTOKEN; return T_FILE;    }
<ST_IN_SCRIPTING>"__DIR__"              { SETTOKEN; return T_DIR;     }
<ST_IN_SCRIPTING>"__NAMESPACE__"        { SETTOKEN; return T_NS_C;    }

<INITIAL>"#"[^\n]*"\n" {
        _scanner->setHashBang(yytext, yyleng);
        BEGIN(ST_IN_SCRIPTING);
        yy_push_state(ST_AFTER_HASHBANG, yyscanner);
        return T_INLINE_HTML;
}

<INITIAL>(([^<#]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN;
        BEGIN(ST_IN_SCRIPTING);
        yy_push_state(ST_IN_HTML, yyscanner);
        return T_INLINE_HTML;
}

<ST_IN_HTML,ST_AFTER_HASHBANG>(([^<]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN;
        BEGIN(ST_IN_HTML);
        return T_INLINE_HTML;
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<?"|("<?php"([ \t]|{NEWLINE}))|"<script"{WHITESPACE}+"language"{WHITESPACE}*"="{WHITESPACE}*("php"|"\"php\""|"\'php\'"){WHITESPACE}*">" {
        SETTOKEN;
        if (_scanner->shortTags() || yyleng > 2) {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          return T_OPEN_TAG;
        } else {
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
        SETTOKEN;
        if ((yytext[1]=='%' && _scanner->aspTags()) ||
            (yytext[1]=='?' && _scanner->shortTags())) {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          return T_ECHO; //return T_OPEN_TAG_WITH_ECHO;
        } else {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
            yy_push_state(ST_IN_HTML, yyscanner);
          } else if (YY_START == ST_AFTER_HASHBANG) {
            BEGIN(ST_IN_HTML);
          }
          return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<%" {
        SETTOKEN;
        if (_scanner->aspTags()) {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
          } else {
            yy_pop_state(yyscanner);
          }
          return T_OPEN_TAG;
        } else {
          if (YY_START == INITIAL) {
            BEGIN(ST_IN_SCRIPTING);
            yy_push_state(ST_IN_HTML, yyscanner);
          } else if (YY_START == ST_AFTER_HASHBANG) {
            BEGIN(ST_IN_HTML);
          }
          return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML,ST_AFTER_HASHBANG>"<?hh"([ \t]|{NEWLINE}) {
        if (YY_START == INITIAL) {
          BEGIN(ST_IN_SCRIPTING);
        } else if (YY_START == ST_AFTER_HASHBANG) {
          yy_pop_state(yyscanner);
        } else {
          _scanner->error("Hack mode: content before <?hh");
          return T_HACK_ERROR;
        }
        STEPPOS;
        _scanner->setHackMode();
        return T_OPEN_TAG;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE,ST_VAR_OFFSET>"$"{LABEL} {
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"->"[a-zA-Z_\x7f-\xff] {
        yyless(yyleng - 3);
        yy_push_state(ST_LOOKING_FOR_PROPERTY, yyscanner);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"[" {
        yyless(yyleng - 1);
        yy_push_state(ST_VAR_OFFSET, yyscanner);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_VAR_OFFSET>"]" {
        yy_pop_state(yyscanner);
        return ']';
}

<ST_VAR_OFFSET>{TOKENS}|[{}\"`] {
        /* Only '[' can be valid, but returning other tokens will allow
           a more explicit parse error */
        return yytext[0];
}

<ST_VAR_OFFSET>[ \n\r\t\\\'#] {
        /* Invalid rule to return a more explicit parse error with proper
           line number */
        yyless(0);
        yy_pop_state(yyscanner);
        STEPPOS;
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_IN_SCRIPTING,ST_VAR_OFFSET>{LABEL} {
        SETTOKEN;
        return T_STRING;
}

<ST_IN_SCRIPTING,ST_XHP_IN_TAG>{WHITESPACE} {
        STEPPOS;
        return T_WHITESPACE;
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
                STEPPOS;
                yy_pop_state(yyscanner);
                return T_COMMENT;
        }
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
        STEPPOS;
        yy_pop_state(yyscanner);
        return T_COMMENT;
}

<ST_ONE_LINE_COMMENT>"?>"|"%>" {
        if (_scanner->isHackMode()) {
          _scanner->error("Hack mode: ?> not allowed");
          return T_HACK_ERROR;
        }
        if (_scanner->aspTags() || yytext[yyleng-2] != '%') {
                _scanner->setToken(yytext, yyleng-2, yytext, yyleng-2);
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
        SETTOKEN;
        yy_pop_state(yyscanner);
        return T_DOC_COMMENT;
}

<ST_COMMENT>"*/" {
        STEPPOS;
        yy_pop_state(yyscanner);
        return T_COMMENT;
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
        yymore();
}

<ST_IN_SCRIPTING>"?>"{NEWLINE}? {
        if (_scanner->isHackMode()) {
          _scanner->error("Hack mode: ?> not allowed");
          return T_HACK_ERROR;
        }
        STEPPOS;
        yy_push_state(ST_IN_HTML, yyscanner);
        if (_scanner->full()) {
          return T_CLOSE_TAG;
        } else {
          return ';';
        }
}

<ST_IN_SCRIPTING>"</script"{WHITESPACE}*">"{NEWLINE}? {
        STEPPOS;
        yy_push_state(ST_IN_HTML, yyscanner);
        if (_scanner->full()) {
          return T_CLOSE_TAG;
        } else {
          return ';';
        }
}

<ST_IN_SCRIPTING>"%>"{NEWLINE}? {
        if (_scanner->aspTags()) {
                STEPPOS;
                yy_push_state(ST_IN_HTML, yyscanner);
                if (_scanner->full()) {
                  return T_CLOSE_TAG;
                } else {
                  return ';';
                }
        } else {
                yyless(1);
                _scanner->setToken(yytext, 1, yytext, 1);
                return yytext[0];
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
        STEPPOS;
        BEGIN(ST_BACKQUOTE);
        return '`';
}

<ST_XHP_IN_TAG>{XHPLABEL} {
  SETTOKEN;
  return T_XHP_LABEL;
}

<ST_XHP_IN_TAG>"=" {
  STEPPOS;
  return yytext[0];
}

<ST_XHP_IN_TAG>["][^"]*["] {
  _scanner->setToken(yytext, yyleng, yytext+1, yyleng-2);
  return T_XHP_TEXT;
}

<ST_XHP_IN_TAG>[{] {
  STEPPOS;
  yy_push_state(ST_IN_SCRIPTING, yyscanner);
  return '{';
}

<ST_XHP_IN_TAG>">" {
  STEPPOS;
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
  STEPPOS;
  _scanner->error("Unexpected character in input: '%c' (ASCII=%d)",
                  yytext[0], yytext[0]);
  return yytext[0];
}

<ST_XHP_END_SINGLETON_TAG>">" {
  STEPPOS;
  yy_pop_state(yyscanner);
  return T_XHP_TAG_GT;
}

<ST_XHP_CHILD>[^{<]+ {
  SETTOKEN;
  return T_XHP_TEXT;
}

<ST_XHP_CHILD>"{" {
  STEPPOS;
  yy_push_state(ST_IN_SCRIPTING, yyscanner);
  return '{';
}

<ST_XHP_CHILD>"</" {
  BEGIN(ST_XHP_END_CLOSE_TAG);
  yyless(1);
  STEPPOS;
  return T_XHP_TAG_LT;
}

<ST_XHP_END_CLOSE_TAG>"/" {
  STEPPOS;
  return '/';
}

<ST_XHP_END_CLOSE_TAG>{XHPLABEL} {
  SETTOKEN;
  return T_XHP_LABEL;
}

<ST_XHP_END_CLOSE_TAG>">" {
  STEPPOS;
  yy_pop_state(yyscanner);
  return T_XHP_TAG_GT;
}

<ST_XHP_CHILD>"<" {
  STEPPOS;
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
        STEPPOS;
        return T_END_HEREDOC;
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
