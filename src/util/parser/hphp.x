%{
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

#define IS_LABEL_START(c) \
  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || \
   (c) == '_' || (c) >= 0x7F)
%}

%x ST_IN_HTML
%x ST_IN_SCRIPTING
%x ST_DOUBLE_QUOTES
%x ST_BACKQUOTE
%x ST_HEREDOC
%x ST_NOWDOC
%x ST_END_HEREDOC
%x ST_LOOKING_FOR_PROPERTY
%x ST_LOOKING_FOR_VARNAME
%x ST_VAR_OFFSET
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT

%x ST_XHP_CLOSE_TAG
%x ST_XHP_CHILD
%x ST_XHP_ATTRIBUTE
%x ST_XHP_STATEMENT
%x ST_XHP_ATTRIBUTE_DECL

%option stack

LNUM    [0-9]+
DNUM    ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
NOTLABEL   [^a-zA-Z_\x7f-\xff]
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")
XHPLABEL {LABEL}([:-]{LABEL})*

/*
 * LITERAL_DOLLAR matches unescaped $ that aren't followed by a label character
 * or a { and therefore will be taken literally. The case of literal $ before
 * a variable or "${" is handled in a rule for each string type
 */
DOUBLE_QUOTES_LITERAL_DOLLAR ("$"+([^a-zA-Z_\x7f-\xff$"\\{]|("\\"{ANY_CHAR})))
BACKQUOTE_LITERAL_DOLLAR     ("$"+([^a-zA-Z_\x7f-\xff$`\\{]|("\\"{ANY_CHAR})))

/*
 * CHARS matches everything up to a variable or "{$"
 * {'s are matched as long as they aren't followed by a $
 * The case of { before "{$" is handled in a rule for each string type
 *
 * For heredocs, matching continues across/after newlines if/when it's known
 * that the next line doesn't contain a possible ending label
 */
DOUBLE_QUOTES_CHARS ("{"*([^$"\\{]|("\\"{ANY_CHAR}))|{DOUBLE_QUOTES_LITERAL_DOLLAR})
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
<ST_IN_SCRIPTING>"extends"              { SETTOKEN; return T_EXTENDS;}
<ST_IN_SCRIPTING>"implements"           { SETTOKEN; return T_IMPLEMENTS;}

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
        STEPPOS;
        return T_INT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("real"|"double"|"float"){TABS_AND_SPACES}")" {
        STEPPOS;
        return T_DOUBLE_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"string"{TABS_AND_SPACES}")" {
        STEPPOS;
        return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"binary"{TABS_AND_SPACES}")" {
        STEPPOS;
        return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"array"{TABS_AND_SPACES}")" {
        STEPPOS;
        return T_ARRAY_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"object"{TABS_AND_SPACES}")" {
        STEPPOS;
        return T_OBJECT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("bool"|"boolean"){TABS_AND_SPACES}")" {
        STEPPOS;
        return T_BOOL_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("unset"){TABS_AND_SPACES}")" {
        STEPPOS;
        return T_UNSET_CAST;
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
<ST_IN_SCRIPTING>">>"                 { STEPPOS; return T_SR;}

<ST_IN_SCRIPTING>":"{XHPLABEL}  {
  switch (_scanner->lastToken()) {
    case ',': case '=': case '|': case '^': case '&': case '<': case '>':
    case '+': case '-': case '%': case '!': case '~': case '[': case '(':
    case '{': case '.': case ';':
    case T_LOGICAL_OR:   case T_LOGICAL_XOR:      case T_LOGICAL_AND:
    case T_PLUS_EQUAL:   case T_MINUS_EQUAL:      case T_MUL_EQUAL:
    case T_DIV_EQUAL:    case T_CONCAT_EQUAL:     case T_MOD_EQUAL:
    case T_AND_EQUAL:    case T_OR_EQUAL:         case T_XOR_EQUAL:
    case T_SL_EQUAL:     case T_SR_EQUAL:         case T_BOOLEAN_OR:
    case T_BOOLEAN_AND:  case T_IS_EQUAL:         case T_IS_NOT_EQUAL:
    case T_IS_IDENTICAL: case T_IS_NOT_IDENTICAL: case T_IS_SMALLER_OR_EQUAL:
    case T_ECHO:         case T_RETURN:           case T_IS_GREATER_OR_EQUAL:
    case T_EXTENDS:      case T_INSTANCEOF:       case T_DOUBLE_ARROW:
    case T_CLASS:
    case T_XHP_ATTRIBUTE:
      yytext++; yyleng--; // skipping the first colon
      SETTOKEN;
      return T_XHP_LABEL;
    case ')':
    case T_ELSE:
      // fall through, treating them as normal PHP syntax
    default:
      yyless(1);
      return ':';
  }
}

<ST_IN_SCRIPTING>"%"{XHPLABEL}  {
  switch (_scanner->lastToken()) {
    case ',': case '(': case '|':
    case T_XHP_CATEGORY:
      yytext++; yyleng--; // skipping "%"
      SETTOKEN;
      return T_XHP_CATEGORY_LABEL;
    default:
      yyless(1);
      return '%';
  }
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
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        if (yyg->yy_start_stack_ptr) yy_pop_state(yyscanner);
        return '}';
}

<ST_LOOKING_FOR_VARNAME>{LABEL} {
        SETTOKEN;
        yy_pop_state(yyscanner);
        yy_push_state(ST_IN_SCRIPTING, yyscanner);
        return T_STRING_VARNAME;
}

<ST_LOOKING_FOR_VARNAME>{ANY_CHAR} {
        yyless(0);
        yy_pop_state(yyscanner);
        yy_push_state(ST_IN_SCRIPTING, yyscanner);
}

<ST_IN_SCRIPTING>{LNUM} {
        SETTOKEN;
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Dec number is too big: %s", yytext);
        }
        return T_LNUMBER;
}

<ST_IN_SCRIPTING>{HNUM} {
        SETTOKEN;
        errno = 0;
        long ret = strtoull(yytext, NULL, 16);
        if (errno == ERANGE || ret < 0) {
                _scanner->error("Hex number is too big: %s", yytext);
        }
        return T_LNUMBER;
}

<ST_VAR_OFFSET>0|([1-9][0-9]*) { /* Offset could be treated as a long */
        SETTOKEN;
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
        if (ret == LLONG_MAX && errno == ERANGE) {
                _scanner->error("Offset number is too big: %s", yytext);
        }
        return T_NUM_STRING;
}

<ST_VAR_OFFSET>{LNUM}|{HNUM} { /* Offset must be treated as a string */
        SETTOKEN;
        return T_NUM_STRING;
}

<ST_IN_SCRIPTING>{DNUM}|{EXPONENT_DNUM} {
        SETTOKEN;
        return T_DNUMBER;
}

<ST_IN_SCRIPTING>"__CLASS__"            { SETTOKEN; return T_CLASS_C; }
<ST_IN_SCRIPTING>"__FUNCTION__"         { SETTOKEN; return T_FUNC_C;  }
<ST_IN_SCRIPTING>"__METHOD__"           { SETTOKEN; return T_METHOD_C;}
<ST_IN_SCRIPTING>"__LINE__"             { SETTOKEN; return T_LINE;    }
<ST_IN_SCRIPTING>"__FILE__"             { SETTOKEN; return T_FILE;    }
<ST_IN_SCRIPTING>"__DIR__"              { SETTOKEN; return T_DIR;     }
<ST_IN_SCRIPTING>"__NAMESPACE__"        { SETTOKEN; return T_NS_C;    }

<INITIAL>"#"[^\n]*"\n" {
        _scanner->setHashBang(yytext, yyleng);
        BEGIN(ST_IN_HTML);
        return T_INLINE_HTML;
}

<INITIAL>(([^<#]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN;
        BEGIN(ST_IN_HTML);
        return T_INLINE_HTML;
}

<ST_IN_HTML>(([^<]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN;
        return T_INLINE_HTML;
}

<INITIAL,ST_IN_HTML>"<?"|"<script"{WHITESPACE}+"language"{WHITESPACE}*"="{WHITESPACE}*("php"|"\"php\""|"\'php\'"){WHITESPACE}*">" {
        SETTOKEN;
        if (_scanner->shortTags() || yyleng > 2) {
                _scanner->setInScript(true);
                BEGIN(ST_IN_SCRIPTING);
                return T_OPEN_TAG;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML>"<%="|"<?=" {
        SETTOKEN;
        if ((yytext[1]=='%' && _scanner->aspTags()) ||
            (yytext[1]=='?' && _scanner->shortTags())) {
                BEGIN(ST_IN_SCRIPTING);
                return T_ECHO; //return T_OPEN_TAG_WITH_ECHO;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML>"<%" {
        SETTOKEN;
        if (_scanner->aspTags()) {
                BEGIN(ST_IN_SCRIPTING);
                return T_OPEN_TAG;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL,ST_IN_HTML>"<?php"([ \t]|{NEWLINE}) {
        STEPPOS;
        BEGIN(ST_IN_SCRIPTING);
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

<ST_VAR_OFFSET>{TOKENS}|[{}"`] {
        /* Only '[' can be valid, but returning other tokens will allow
           a more explicit parse error */
        return yytext[0];
}

<ST_VAR_OFFSET>[ \n\r\t\\'#] {
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

<ST_IN_SCRIPTING,ST_XHP_ATTRIBUTE,ST_XHP_STATEMENT,ST_XHP_ATTRIBUTE_DECL>{WHITESPACE} {
        STEPPOS;
        return T_WHITESPACE;
}

<ST_IN_SCRIPTING,ST_XHP_ATTRIBUTE,ST_XHP_STATEMENT,ST_XHP_ATTRIBUTE_DECL>"#"|"//" {
        BEGIN(ST_ONE_LINE_COMMENT);
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
                if (_scanner->isXhpState()) {
                  BEGIN(_scanner->getXhpState());
                } else {
                  BEGIN(ST_IN_SCRIPTING);
                }
                return T_COMMENT;
        }
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
        STEPPOS;
        if (_scanner->isXhpState()) {
          BEGIN(_scanner->getXhpState());
        } else {
          BEGIN(ST_IN_SCRIPTING);
        }
        return T_COMMENT;
}

<ST_ONE_LINE_COMMENT>"?>"|"%>" {
        if (_scanner->aspTags() || yytext[yyleng-2] != '%') {
                _scanner->setToken(yytext, yyleng-2, yytext, yyleng-2);
                yyless(yyleng-2);
                if (_scanner->isXhpState()) {
                  BEGIN(_scanner->getXhpState());
                } else {
                  BEGIN(ST_IN_SCRIPTING);
                }
                return T_COMMENT;
        } else {
                yymore();
        }
}

<ST_IN_SCRIPTING,ST_XHP_ATTRIBUTE,ST_XHP_STATEMENT,ST_XHP_ATTRIBUTE_DECL>"/**"{WHITESPACE} {
        BEGIN(ST_DOC_COMMENT);
        yymore();
}

<ST_IN_SCRIPTING,ST_XHP_ATTRIBUTE,ST_XHP_STATEMENT,ST_XHP_ATTRIBUTE_DECL>"/*" {
        BEGIN(ST_COMMENT);
        yymore();
}

<ST_COMMENT,ST_DOC_COMMENT>[^*]+ {
        yymore();
}

<ST_DOC_COMMENT>"*/" {
        STEPPOS;
        _scanner->setDocComment(yytext, yyleng);
        if (_scanner->isXhpState()) {
          BEGIN(_scanner->getXhpState());
        } else {
          BEGIN(ST_IN_SCRIPTING);
        }
        return T_DOC_COMMENT;
}

<ST_COMMENT>"*/" {
        STEPPOS;
        if (_scanner->isXhpState()) {
          BEGIN(_scanner->getXhpState());
        } else {
          BEGIN(ST_IN_SCRIPTING);
        }
        return T_COMMENT;
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
        yymore();
}

<ST_IN_SCRIPTING>"?>"{NEWLINE}? {
        STEPPOS;
        BEGIN(ST_IN_HTML);
        if (_scanner->full()) {
          return T_CLOSE_TAG;
        } else {
          return ';';
        }
}

<ST_IN_SCRIPTING>"</script"{WHITESPACE}*">"{NEWLINE}? {
    if (_scanner->inScript()) {
        _scanner->setInScript(false);
        STEPPOS;
        BEGIN(ST_IN_HTML);
        if (_scanner->full()) {
          return T_CLOSE_TAG;
        } else {
          return ';';
        }
    } else {
        yyless(1);
        return '<';
    }
}

<ST_IN_SCRIPTING>"%>"{NEWLINE}? {
        if (_scanner->aspTags()) {
                STEPPOS;
                BEGIN(ST_IN_HTML);
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

<ST_IN_SCRIPTING>(b?["]{DOUBLE_QUOTES_CHARS}*("{"*|"$"*)["]) {
        int bprefix = (yytext[0] != '"') ? 1 : 0;
        std::string strval =
          _scanner->escape(yytext + bprefix + 1,
                           yyleng - bprefix - 2, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>(b?[']([^'\\]|("\\"{ANY_CHAR}))*[']) {
        int bprefix = (yytext[0] != '\'') ? 1 : 0;
        std::string strval =
          _scanner->escape(yytext + bprefix + 1,
                           yyleng - bprefix - 2, '\'');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>b?["] {
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

        DECLARE_YYCURSOR;
        DECLARE_YYLIMIT;

        /* Check for ending label on the next line */
        if (label_len < YYLIMIT - YYCURSOR &&
            !memcmp(YYCURSOR, s, label_len)) {
                const char *end = YYCURSOR + label_len;
                if (*end == ';') {
                        end++;
                }
                if (*end == '\n' || *end == '\r') {
                        BEGIN(ST_END_HEREDOC);
                }
        }

        return T_START_HEREDOC;
}

<ST_IN_SCRIPTING>[`] {
        STEPPOS;
        BEGIN(ST_BACKQUOTE);
        return '`';
}

<ST_XHP_ATTRIBUTE>[{] {
  STEPPOS;
  _scanner->xhpReset();
  return '{';
}
<ST_XHP_ATTRIBUTE>["][^"]*["] {
  _scanner->setToken(yytext, yyleng, yytext+1, yyleng-2);
  _scanner->xhpReset();
  return T_XHP_TEXT;
}
<ST_XHP_CLOSE_TAG>[>] {
  STEPPOS;
  _scanner->xhpReset();
  return '>';
}
<ST_XHP_CLOSE_TAG>{XHPLABEL} {
  SETTOKEN;
  return T_XHP_LABEL;
}
<ST_XHP_CHILD>[{<] {
  STEPPOS;
  _scanner->xhpReset();
  return yytext[0];
}
<ST_XHP_CHILD>[^{<]+ {
  SETTOKEN;
  _scanner->xhpReset();
  return T_XHP_TEXT;
}

<ST_XHP_STATEMENT>"attribute" {
  STEPPOS;
  _scanner->xhpReset();
  return T_XHP_ATTRIBUTE;
}
<ST_XHP_STATEMENT>"category" {
  STEPPOS;
  _scanner->xhpReset();
  return T_XHP_CATEGORY;
}
<ST_XHP_STATEMENT>"children" {
  STEPPOS;
  _scanner->xhpReset();
  return T_XHP_CHILDREN;
}
<ST_XHP_STATEMENT>{ANY_CHAR} {
  _scanner->xhpReset();
  yyless(0);
}

<ST_XHP_ATTRIBUTE_DECL>("bool"|"boolean") {
  STEPPOS; _scanner->xhpReset(); return T_BOOL_CAST;
}
<ST_XHP_ATTRIBUTE_DECL>("int"|"integer") {
  STEPPOS; _scanner->xhpReset(); return T_INT_CAST;
}
<ST_XHP_ATTRIBUTE_DECL>("real"|"double"|"float") {
  STEPPOS; _scanner->xhpReset(); return T_DOUBLE_CAST;
}
<ST_XHP_ATTRIBUTE_DECL>"var" {
  STEPPOS; _scanner->xhpReset(); return T_VAR;
}
<ST_XHP_ATTRIBUTE_DECL>"array" {
  STEPPOS; _scanner->xhpReset(); return T_ARRAY_CAST;
}
<ST_XHP_ATTRIBUTE_DECL>"string" {
  STEPPOS; _scanner->xhpReset(); return T_STRING_CAST;
}
<ST_XHP_ATTRIBUTE_DECL>"enum" {
  STEPPOS; _scanner->xhpReset(); return T_XHP_ENUM;
}
<ST_XHP_ATTRIBUTE_DECL>"required" {
  STEPPOS; _scanner->xhpReset(); return T_XHP_REQUIRED;
}
<ST_XHP_ATTRIBUTE_DECL>{LABEL} {
  SETTOKEN; _scanner->xhpReset(); return T_STRING;
}
<ST_XHP_ATTRIBUTE_DECL>{ANY_CHAR} {
  _scanner->xhpReset();
  yyless(0);
}

<ST_HEREDOC>{ANY_CHAR} {
  int newline = 0;

  DECLARE_YYCURSOR;
  DECLARE_YYLIMIT;

  if (YYCURSOR > YYLIMIT) {
    return 0;
  }

  YYCURSOR--;

  int heredocLen = _scanner->getHeredocLabelLen();
  while (YYCURSOR < YYLIMIT) {
    switch (*YYCURSOR++) {
      case '\r':
        if (*YYCURSOR == '\n') {
          YYCURSOR++;
        }
        /* fall through */
      case '\n':
        /* Check for ending label on the next line */
        if (IS_LABEL_START(*YYCURSOR) &&
            heredocLen < YYLIMIT - YYCURSOR &&
            !memcmp(YYCURSOR, _scanner->getHeredocLabel(), heredocLen)) {
          const char *end = YYCURSOR + heredocLen;
          if (*end == ';') {
            end++;
          }
          if (*end == '\n' || *end == '\r') {
            /* newline before label will be subtracted from returned text, but
               yyleng/yytext will include it, for zend_highlight/strip,
               tokenizer, etc. */
            if (YYCURSOR[-2] == '\r' && YYCURSOR[-1] == '\n') {
              newline = 2; /* Windows newline */
            } else {
              newline = 1;
            }

            BEGIN(ST_END_HEREDOC);
            goto heredoc_scan_done;
          }
        }
        continue;
      case '$':
        if (IS_LABEL_START(*YYCURSOR) || *YYCURSOR == '{') {
          break;
        }
        continue;
      case '{':
        if (*YYCURSOR == '$') {
          break;
        }
        continue;
      case '\\':
        if (YYCURSOR < YYLIMIT && *YYCURSOR != '\n' && *YYCURSOR != '\r') {
          YYCURSOR++;
        }
        /* fall through */
      default:
        continue;
    }

    YYCURSOR--;
    break;
  }

heredoc_scan_done:
  yyleng = YYCURSOR - yytext;
  RESET_YYCURSOR;
  std::string strval = _scanner->escape(yytext, yyleng - newline, 0);
  _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
  return T_ENCAPSED_AND_WHITESPACE;
}

<ST_NOWDOC>{ANY_CHAR} {
  int newline = 0;

  DECLARE_YYCURSOR;
  DECLARE_YYLIMIT;

  if (YYCURSOR > YYLIMIT) {
    return 0;
  }

  YYCURSOR--;

  int heredocLen = _scanner->getHeredocLabelLen();
  while (YYCURSOR < YYLIMIT) {
    switch (*YYCURSOR++) {
      case '\r':
        if (*YYCURSOR == '\n') {
          YYCURSOR++;
        }
        /* fall through */
      case '\n':
        /* Check for ending label on the next line */
        if (IS_LABEL_START(*YYCURSOR) &&
            heredocLen < YYLIMIT - YYCURSOR &&
            !memcmp(YYCURSOR, _scanner->getHeredocLabel(), heredocLen)) {
          const char *end = YYCURSOR + heredocLen;
          if (*end == ';') {
            end++;
          }
          if (*end == '\n' || *end == '\r') {
            /* newline before label will be subtracted from returned text, but
               yyleng/yytext will include it, for zend_highlight/strip,
               tokenizer, etc. */
            if (YYCURSOR[-2] == '\r' && YYCURSOR[-1] == '\n') {
              newline = 2; /* Windows newline */
            } else {
              newline = 1;
            }

            BEGIN(ST_END_HEREDOC);
            goto nowdoc_scan_done;
          }
        }
        /* fall through */
      default:
        continue;
    }
  }

nowdoc_scan_done:
  yyleng = YYCURSOR - yytext;
  RESET_YYCURSOR;
  std::string strval(yytext, yyleng - newline);
  _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
  return T_ENCAPSED_AND_WHITESPACE;
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

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS}*("{"{2,}|"$"{2,}|(("{"+|"$"+)["])) {
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

<ST_DOUBLE_QUOTES>["] {
        BEGIN(ST_IN_SCRIPTING);
        return '"';
}

<ST_BACKQUOTE>[`] {
        BEGIN(ST_IN_SCRIPTING);
        return '`';
}

<ST_COMMENT,ST_DOC_COMMENT><<EOF>> {
        _scanner->error("Unterminated comment at end of file");
        return 0;
}

<ST_IN_SCRIPTING,ST_VAR_OFFSET>{ANY_CHAR} {
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

  void Scanner::xhpCloseTag() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    BEGIN(ST_XHP_CLOSE_TAG);
  }

  void Scanner::xhpChild() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    BEGIN(ST_XHP_CHILD);
  }

  void Scanner::xhpAttribute() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    setXhpState(ST_XHP_ATTRIBUTE);
    BEGIN(ST_XHP_ATTRIBUTE);
  }

  void Scanner::xhpStatement() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    setXhpState(ST_XHP_STATEMENT);
    BEGIN(ST_XHP_STATEMENT);
  }

  void Scanner::xhpAttributeDecl() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    setXhpState(ST_XHP_ATTRIBUTE_DECL);
    BEGIN(ST_XHP_ATTRIBUTE_DECL);
  }

  void Scanner::xhpReset() {
    struct yyguts_t *yyg = (struct yyguts_t *)m_yyscanner;
    setXhpState(0);
    BEGIN(ST_IN_SCRIPTING);
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
