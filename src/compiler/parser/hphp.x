%{
#include <compiler/parser/hphp.tab.hpp>
#include <compiler/parser/scanner.h>
#define YLMM_SCANNER_CLASS HPHP::Scanner
#include <util/ylmm/lexmm.hh>
#include <errno.h>
#define SETTOKEN _scanner->setToken(yytext, yyleng, yytext, yyleng)
%}

%x ST_IN_SCRIPTING
%x ST_DOUBLE_QUOTES
%x ST_BACKQUOTE
%x ST_HEREDOC
%x ST_START_HEREDOC
%x ST_END_HEREDOC
%x ST_LOOKING_FOR_PROPERTY
%x ST_LOOKING_FOR_VARNAME
%x ST_VAR_OFFSET
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT
%option stack

LNUM    [0-9]+
DNUM    ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")

/*
 * LITERAL_DOLLAR matches unescaped $ that aren't followed by a label character
 * or a { and therefore will be taken literally. The case of literal $ before
 * a variable or "${" is handled in a rule for each string type
 */
DOUBLE_QUOTES_LITERAL_DOLLAR ("$"+([^a-zA-Z_\x7f-\xff$"\\{]|("\\"{ANY_CHAR})))
BACKQUOTE_LITERAL_DOLLAR     ("$"+([^a-zA-Z_\x7f-\xff$`\\{]|("\\"{ANY_CHAR})))
HEREDOC_LITERAL_DOLLAR       ("$"+([^a-zA-Z_\x7f-\xff$\n\r\\{]|("\\"[^\n\r])))
/*
 * Usually, HEREDOC_NEWLINE will just function like a simple NEWLINE, but some
 * special cases need to be handled. HEREDOC_CHARS doesn't allow a line to
 * match when { or $, and/or \ is at the end. (("{"*|"$"*)"\\"?) handles that,
 * along with cases where { or $, and/or \ is the ONLY thing on a line
 *
 * The other case is when a line contains a label, followed by ONLY
 * { or $, and/or \  Handled by ({LABEL}";"?((("{"+|"$"+)"\\"?)|"\\"))
 */
HEREDOC_NEWLINE ((({LABEL}";"?((("{"+|"$"+)"\\"?)|"\\"))|(("{"*|"$"*)"\\"?)){NEWLINE})

/*
 * This pattern is just used in the next 2 for matching { or literal $, and/or
 * \ escape sequence immediately at the beginning of a line or after a label
 */
HEREDOC_CURLY_OR_ESCAPE_OR_DOLLAR (("{"+[^$\n\r\\{])|("{"*"\\"[^\n\r])|{HEREDOC_LITERAL_DOLLAR})

/*
 * These 2 label-related patterns allow HEREDOC_CHARS to continue "regular"
 * matching after a newline that starts with either a non-label character or a
 * label that isn't followed by a newline. Like HEREDOC_CHARS, they won't match
 * a variable or "{$"  Matching a newline, and possibly label, up TO a variable
 * or "{$", is handled in the heredoc rules
 *
 * The HEREDOC_LABEL_NO_NEWLINE pattern (";"[^$\n\r\\{]) handles cases where ;
 * follows a label. [^a-zA-Z0-9_\x7f-\xff;$\n\r\\{] is needed to prevent a label
 * character or ; from matching on a possible (real) ending label
 */
HEREDOC_NON_LABEL ([^a-zA-Z_\x7f-\xff$\n\r\\{]|{HEREDOC_CURLY_OR_ESCAPE_OR_DOLLAR})
HEREDOC_LABEL_NO_NEWLINE ({LABEL}([^a-zA-Z0-9_\x7f-\xff;$\n\r\\{]|(";"[^$\n\r\\{])|(";"?{HEREDOC_CURLY_OR_ESCAPE_OR_DOLLAR})))
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
HEREDOC_CHARS       ("{"*([^$\n\r\\{]|("\\"[^\n\r]))|{HEREDOC_LITERAL_DOLLAR}|({HEREDOC_NEWLINE}+({HEREDOC_NON_LABEL}|{HEREDOC_LABEL_NO_NEWLINE})))

%%

<ST_IN_SCRIPTING>"exit"                 {SETTOKEN; return T_EXIT;}
<ST_IN_SCRIPTING>"die"                  {SETTOKEN; return T_EXIT;}
<ST_IN_SCRIPTING>"function"             {SETTOKEN; return T_FUNCTION;}
<ST_IN_SCRIPTING>"const"                {SETTOKEN; return T_CONST;}
<ST_IN_SCRIPTING>"return"               {SETTOKEN; return T_RETURN;}
<ST_IN_SCRIPTING>"try"                  {SETTOKEN; return T_TRY;}
<ST_IN_SCRIPTING>"catch"                {SETTOKEN; return T_CATCH;}
<ST_IN_SCRIPTING>"throw"                {SETTOKEN; return T_THROW;}
<ST_IN_SCRIPTING>"if"                   {SETTOKEN; return T_IF;}
<ST_IN_SCRIPTING>"elseif"               {SETTOKEN; return T_ELSEIF;}
<ST_IN_SCRIPTING>"endif"                {SETTOKEN; return T_ENDIF;}
<ST_IN_SCRIPTING>"else"                 {SETTOKEN; return T_ELSE;}
<ST_IN_SCRIPTING>"while"                {SETTOKEN; return T_WHILE;}
<ST_IN_SCRIPTING>"endwhile"             {SETTOKEN; return T_ENDWHILE;}
<ST_IN_SCRIPTING>"do"                   {SETTOKEN; return T_DO;}
<ST_IN_SCRIPTING>"for"                  {SETTOKEN; return T_FOR;}
<ST_IN_SCRIPTING>"endfor"               {SETTOKEN; return T_ENDFOR;}
<ST_IN_SCRIPTING>"foreach"              {SETTOKEN; return T_FOREACH;}
<ST_IN_SCRIPTING>"endforeach"           {SETTOKEN; return T_ENDFOREACH;}
<ST_IN_SCRIPTING>"declare"              {SETTOKEN; return T_DECLARE;}
<ST_IN_SCRIPTING>"enddeclare"           {SETTOKEN; return T_ENDDECLARE;}
<ST_IN_SCRIPTING>"instanceof"           {SETTOKEN; return T_INSTANCEOF;}
<ST_IN_SCRIPTING>"as"                   {SETTOKEN; return T_AS;}
<ST_IN_SCRIPTING>"switch"               {SETTOKEN; return T_SWITCH;}
<ST_IN_SCRIPTING>"endswitch"            {SETTOKEN; return T_ENDSWITCH;}
<ST_IN_SCRIPTING>"case"                 {SETTOKEN; return T_CASE;}
<ST_IN_SCRIPTING>"default"              {SETTOKEN; return T_DEFAULT;}
<ST_IN_SCRIPTING>"break"                {SETTOKEN; return T_BREAK;}
<ST_IN_SCRIPTING>"continue"             {SETTOKEN; return T_CONTINUE;}
<ST_IN_SCRIPTING>"echo"                 {SETTOKEN; return T_ECHO;}
<ST_IN_SCRIPTING>"print"                {SETTOKEN; return T_PRINT;}
<ST_IN_SCRIPTING>"class"                {SETTOKEN; return T_CLASS;}
<ST_IN_SCRIPTING>"interface"            {SETTOKEN; return T_INTERFACE;}
<ST_IN_SCRIPTING>"extends"              {SETTOKEN; return T_EXTENDS;}
<ST_IN_SCRIPTING>"implements"           {SETTOKEN; return T_IMPLEMENTS;}

<ST_IN_SCRIPTING>^"#HPHP_DECLARE"      {SETTOKEN; return T_HPHP_DECLARE;}

<ST_IN_SCRIPTING>"->" {
        SETTOKEN;
        yy_push_state(ST_LOOKING_FOR_PROPERTY);
        return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>"->" {
        SETTOKEN;
        return T_OBJECT_OPERATOR;
}

<ST_LOOKING_FOR_PROPERTY>{LABEL} {
        SETTOKEN;
        yy_pop_state();
        return T_STRING;
}

<ST_LOOKING_FOR_PROPERTY>{ANY_CHAR} {
        yyless(0);
        yy_pop_state();
}

<ST_IN_SCRIPTING>"::"                {SETTOKEN;return T_PAAMAYIM_NEKUDOTAYIM;}
<ST_IN_SCRIPTING>"new"               {SETTOKEN;return T_NEW;}
<ST_IN_SCRIPTING>"clone"             {SETTOKEN;return T_CLONE;}
<ST_IN_SCRIPTING>"var"               {SETTOKEN;return T_VAR;}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("int"|"integer"){TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_INT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("real"|"double"|"float"){TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_DOUBLE_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"string"{TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"binary"{TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_STRING_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"array"{TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_ARRAY_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}"object"{TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_OBJECT_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("bool"|"boolean"){TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_BOOL_CAST;
}

<ST_IN_SCRIPTING>"("{TABS_AND_SPACES}("unset"){TABS_AND_SPACES}")" {
        SETTOKEN;
        return T_UNSET_CAST;
}

<ST_IN_SCRIPTING>"eval"               {SETTOKEN; return T_EVAL;}
<ST_IN_SCRIPTING>"include"            {SETTOKEN; return T_INCLUDE;}
<ST_IN_SCRIPTING>"include_once"       {SETTOKEN; return T_INCLUDE_ONCE;}
<ST_IN_SCRIPTING>"require"            {SETTOKEN; return T_REQUIRE;}
<ST_IN_SCRIPTING>"require_once"       {SETTOKEN; return T_REQUIRE_ONCE;}
<ST_IN_SCRIPTING>"use"                {SETTOKEN; return T_USE;}
<ST_IN_SCRIPTING>"global"             {SETTOKEN; return T_GLOBAL;}
<ST_IN_SCRIPTING>"isset"              {SETTOKEN; return T_ISSET;}
<ST_IN_SCRIPTING>"empty"              {SETTOKEN; return T_EMPTY;}
<ST_IN_SCRIPTING>"__halt_compiler"    {SETTOKEN; return T_HALT_COMPILER;}
<ST_IN_SCRIPTING>"static"             {SETTOKEN; return T_STATIC;}
<ST_IN_SCRIPTING>"abstract"           {SETTOKEN; return T_ABSTRACT;}
<ST_IN_SCRIPTING>"final"              {SETTOKEN; return T_FINAL;}
<ST_IN_SCRIPTING>"private"            {SETTOKEN; return T_PRIVATE;}
<ST_IN_SCRIPTING>"protected"          {SETTOKEN; return T_PROTECTED;}
<ST_IN_SCRIPTING>"public"             {SETTOKEN; return T_PUBLIC;}
<ST_IN_SCRIPTING>"unset"              {SETTOKEN; return T_UNSET;}
<ST_IN_SCRIPTING>"=>"                 {SETTOKEN; return T_DOUBLE_ARROW;}
<ST_IN_SCRIPTING>"list"               {SETTOKEN; return T_LIST;}
<ST_IN_SCRIPTING>"array"              {SETTOKEN; return T_ARRAY;}
<ST_IN_SCRIPTING>"++"                 {SETTOKEN; return T_INC;}
<ST_IN_SCRIPTING>"--"                 {SETTOKEN; return T_DEC;}
<ST_IN_SCRIPTING>"==="                {SETTOKEN; return T_IS_IDENTICAL;}
<ST_IN_SCRIPTING>"!=="                {SETTOKEN; return T_IS_NOT_IDENTICAL;}
<ST_IN_SCRIPTING>"=="                 {SETTOKEN; return T_IS_EQUAL;}
<ST_IN_SCRIPTING>"!="|"<>"            {SETTOKEN; return T_IS_NOT_EQUAL;}
<ST_IN_SCRIPTING>"<="                 {SETTOKEN; return T_IS_SMALLER_OR_EQUAL;}
<ST_IN_SCRIPTING>">="                 {SETTOKEN; return T_IS_GREATER_OR_EQUAL;}
<ST_IN_SCRIPTING>"+="                 {SETTOKEN; return T_PLUS_EQUAL;}
<ST_IN_SCRIPTING>"-="                 {SETTOKEN; return T_MINUS_EQUAL;}
<ST_IN_SCRIPTING>"*="                 {SETTOKEN; return T_MUL_EQUAL;}
<ST_IN_SCRIPTING>"/="                 {SETTOKEN; return T_DIV_EQUAL;}
<ST_IN_SCRIPTING>".="                 {SETTOKEN; return T_CONCAT_EQUAL;}
<ST_IN_SCRIPTING>"%="                 {SETTOKEN; return T_MOD_EQUAL;}
<ST_IN_SCRIPTING>"<<="                {SETTOKEN; return T_SL_EQUAL;}
<ST_IN_SCRIPTING>">>="                {SETTOKEN; return T_SR_EQUAL;}
<ST_IN_SCRIPTING>"&="                 {SETTOKEN; return T_AND_EQUAL;}
<ST_IN_SCRIPTING>"|="                 {SETTOKEN; return T_OR_EQUAL;}
<ST_IN_SCRIPTING>"^="                 {SETTOKEN; return T_XOR_EQUAL;}
<ST_IN_SCRIPTING>"||"                 {SETTOKEN; return T_BOOLEAN_OR;}
<ST_IN_SCRIPTING>"&&"                 {SETTOKEN; return T_BOOLEAN_AND;}
<ST_IN_SCRIPTING>"OR"                 {SETTOKEN; return T_LOGICAL_OR;}
<ST_IN_SCRIPTING>"AND"                {SETTOKEN; return T_LOGICAL_AND;}
<ST_IN_SCRIPTING>"XOR"                {SETTOKEN; return T_LOGICAL_XOR;}
<ST_IN_SCRIPTING>"<<"                 {SETTOKEN; return T_SL;}
<ST_IN_SCRIPTING>">>"                 {SETTOKEN; return T_SR;}
<ST_IN_SCRIPTING>{TOKENS}             {SETTOKEN; return yytext[0];}

<ST_IN_SCRIPTING>"{" {
        SETTOKEN;
        yy_push_state(ST_IN_SCRIPTING);
        return '{';
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"${" {
        SETTOKEN;
        yy_push_state(ST_LOOKING_FOR_VARNAME);
        return T_DOLLAR_OPEN_CURLY_BRACES;
}

<ST_IN_SCRIPTING>"}" {
        SETTOKEN;
        if (yy_start_stack_ptr) yy_pop_state();
        return '}';
}

<ST_LOOKING_FOR_VARNAME>{LABEL} {
        SETTOKEN;
        yy_pop_state();
        yy_push_state(ST_IN_SCRIPTING);
        return T_STRING_VARNAME;
}

<ST_LOOKING_FOR_VARNAME>{ANY_CHAR} {
        yyless(0);
        yy_pop_state();
        yy_push_state(ST_IN_SCRIPTING);
}

<ST_IN_SCRIPTING>{LNUM} {
        SETTOKEN;
        errno = 0;
        long ret = strtoll(yytext, NULL, 0);
	if (errno == ERANGE) {
                _scanner->error("Dec number is too big: %s", yytext);
                return T_LNUMBER;
        }
        return ret < 0 ? T_DNUMBER : T_LNUMBER;
}

<ST_IN_SCRIPTING>{HNUM} {
        SETTOKEN;
	errno = 0;
        long ret = strtoull(yytext, NULL, 16);
	if (errno == ERANGE) {
                _scanner->error("Hex number is too big: %s", yytext);
                return T_LNUMBER;
        }
        return ret < 0 ? T_DNUMBER : T_LNUMBER;
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

<INITIAL>"#"[^\n]*"\n" {
        SETTOKEN;
        return T_INLINE_HTML;
}

<INITIAL>(([^<]|"<"[^?%s<]){1,400})|"<s"|"<" {
        SETTOKEN;
        return T_INLINE_HTML;
}

<INITIAL>"<?"|"<script"{WHITESPACE}+"language"{WHITESPACE}*"="{WHITESPACE}*("php"|"\"php\""|"\'php\'"){WHITESPACE}*">" {
        SETTOKEN;
        if (_scanner->shortTags() || yyleng > 2) {
                BEGIN(ST_IN_SCRIPTING);
                return T_OPEN_TAG;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL>"<%="|"<?=" {
        SETTOKEN;
        if ((yytext[1]=='%' && _scanner->aspTags()) ||
            (yytext[1]=='?' && _scanner->shortTags())) {
                BEGIN(ST_IN_SCRIPTING);
                return T_ECHO; //return T_OPEN_TAG_WITH_ECHO;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL>"<%" {
        SETTOKEN;
        if (_scanner->aspTags()) {
                BEGIN(ST_IN_SCRIPTING);
                return T_OPEN_TAG;
        } else {
                return T_INLINE_HTML;
        }
}

<INITIAL>"<?php"([ \t]|{NEWLINE}) {
        SETTOKEN;
        BEGIN(ST_IN_SCRIPTING);
        return T_OPEN_TAG;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE,ST_VAR_OFFSET>"$"{LABEL} {
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"->"[a-zA-Z_\x7f-\xff] {
        yyless(yyleng - 3);
        yy_push_state(ST_LOOKING_FOR_PROPERTY);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_DOUBLE_QUOTES,ST_HEREDOC,ST_BACKQUOTE>"$"{LABEL}"[" {
        yyless(yyleng - 1);
        yy_push_state(ST_VAR_OFFSET);
        _scanner->setToken(yytext, yyleng, yytext+1, yyleng-1);
        return T_VARIABLE;
}

<ST_VAR_OFFSET>"]" {
        yy_pop_state();
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
        yy_pop_state();
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_IN_SCRIPTING,ST_VAR_OFFSET>{LABEL} {
        SETTOKEN;
        return T_STRING;
}

<ST_IN_SCRIPTING>{WHITESPACE} {
        SETTOKEN;
        return T_WHITESPACE;
}

<ST_IN_SCRIPTING>"#"|"//" {
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
                SETTOKEN;
                BEGIN(ST_IN_SCRIPTING);
                return T_COMMENT;
        }
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
        SETTOKEN;
        BEGIN(ST_IN_SCRIPTING);
        return T_COMMENT;
}

<ST_ONE_LINE_COMMENT>"?>"|"%>" {
        if (_scanner->aspTags() || yytext[yyleng-2] != '%') {
                _scanner->setToken(yytext, yyleng-2, yytext, yyleng-2);
                yyless(yyleng-2);
                BEGIN(ST_IN_SCRIPTING);
                return T_COMMENT;
        } else {
                yymore();
        }
}

<ST_IN_SCRIPTING>"/**"{WHITESPACE} {
        BEGIN(ST_DOC_COMMENT);
        yymore();
}

<ST_IN_SCRIPTING>"/*" {
        BEGIN(ST_COMMENT);
        yymore();
}

<ST_COMMENT,ST_DOC_COMMENT>[^*]+ {
        yymore();
}

<ST_DOC_COMMENT>"*/" {
        SETTOKEN;
        _scanner->setDocComment(yytext, yyleng);
        BEGIN(ST_IN_SCRIPTING);
        return T_DOC_COMMENT;
}

<ST_COMMENT>"*/" {
        SETTOKEN;
        BEGIN(ST_IN_SCRIPTING);
#ifdef HPHP_NOTE
        if (yyleng > 6 && yytext[2] == '|' && yytext[yyleng-3] == '|') {
                _scanner->setToken(yytext, yyleng, yytext+3, yyleng-6);
                return T_HPHP_NOTE;
        }
#endif
        return T_COMMENT;
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
        yymore();
}

<ST_IN_SCRIPTING>("?>"|"</script"{WHITESPACE}*">"){NEWLINE}? {
        SETTOKEN;
        BEGIN(INITIAL);
        return ';'; //return T_CLOSE_TAG;
}

<ST_IN_SCRIPTING>"%>"{NEWLINE}? {
        if (_scanner->aspTags()) {
                SETTOKEN;
                BEGIN(INITIAL);
                return ';'; //return T_CLOSE_TAG;
        } else {
                yyless(1);
                _scanner->setToken(yytext, 1, yytext, 1);
                return yytext[0];
        }
}

<ST_IN_SCRIPTING>(b?["]{DOUBLE_QUOTES_CHARS}*("{"*|"$"*)["]) {
        int bprefix = (yytext[0] != '"') ? 1 : 0;
        std::string strval =
          _scanner->scanEscapeString(yytext + bprefix + 1,
                                     yyleng - bprefix - 2, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>(b?[']([^'\\]|("\\"{ANY_CHAR}))*[']) {
        int bprefix = (yytext[0] != '\'') ? 1 : 0;
        std::string strval =
          _scanner->scanEscapeString(yytext + bprefix + 1,
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

<ST_IN_SCRIPTING>b?"<<<"{TABS_AND_SPACES}{LABEL}{NEWLINE} {
        int bprefix = (yytext[0] != '<') ? 1 : 0;
        int label_len = yyleng-bprefix-3-1-(yytext[yyleng-2]=='\r'?1:0);
        char *s = yytext+bprefix+3;
        while ((*s == ' ') || (*s == '\t')) {
                s++;
                label_len--;
        }
        _scanner->setHeredocLabel(s, label_len);
        _scanner->setToken(yytext, yyleng, s, label_len);
        BEGIN(ST_HEREDOC);
        return T_START_HEREDOC;
}

<ST_IN_SCRIPTING>[`] {
        SETTOKEN;
        BEGIN(ST_BACKQUOTE);
        return '`';
}

<ST_START_HEREDOC>{ANY_CHAR} {
        yyless(0);
        BEGIN(ST_HEREDOC);
}

<ST_START_HEREDOC>{LABEL}";"?[\n\r] {
        int label_len = yyleng-1;
        if (yytext[label_len-1]==';') {
                label_len--;
        }
        if (label_len == _scanner->getHeredocLabelLen() &&
            !memcmp(yytext, _scanner->getHeredocLabel(), label_len)) {
                _scanner->setToken(yytext, label_len, yytext, label_len);
                yyless(label_len);
                _scanner->resetHeredoc();
                BEGIN(ST_IN_SCRIPTING);
                return T_END_HEREDOC;
        } else {
                yymore();
                BEGIN(ST_HEREDOC);
        }
}

<ST_HEREDOC>{HEREDOC_CHARS}*{HEREDOC_NEWLINE}+{LABEL}";"?[\n\r] {
        char *end = yytext + yyleng - 1;

        if (end[-1] == ';') {
                end--;
                yyleng--;
        }
        int heredocLen = _scanner->getHeredocLabelLen();
        if (yyleng > heredocLen &&
            !memcmp(end - heredocLen, _scanner->getHeredocLabel(),
                    heredocLen)) {
                int len = yyleng - heredocLen - 2;
                           /* 2 for newline before and after label */
                if (len > 0 &&
                    yytext[len - 1] == '\r' && yytext[len] == '\n') {
                        len--;
                }
                yyless(yyleng - 2);
                yyleng -= heredocLen - 1;
                std::string strval =
                  _scanner->scanEscapeString(yytext, len, 0);
                _scanner->setToken(yytext, yyleng,
                                   strval.c_str(), strval.length());
                BEGIN(ST_END_HEREDOC);
                return T_ENCAPSED_AND_WHITESPACE;
        } else {
          /* Go back to end of label, so the next match works correctly in
           * case of a variable or another label at the beginning of the
           * next line
           */
          yyless(yyleng - 1);
          yymore();
        }
}

<ST_END_HEREDOC>{ANY_CHAR} {
        BEGIN(ST_IN_SCRIPTING);
        SETTOKEN;
        return T_END_HEREDOC;
}

<ST_DOUBLE_QUOTES,ST_BACKQUOTE,ST_HEREDOC>"{$" {
        _scanner->setToken(yytext, 1, yytext, 1);
        yy_push_state(ST_IN_SCRIPTING);
        yyless(1);
        return T_CURLY_OPEN;
}

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS}+ {
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_DOUBLE_QUOTES>{DOUBLE_QUOTES_CHARS}*("{"{2,}|"$"{2,}|(("{"+|"$"+)["])) {
        yyless(yyleng - 1);
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, '"');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_BACKQUOTE>{BACKQUOTE_CHARS}+ {
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, '`');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_BACKQUOTE>{BACKQUOTE_CHARS}*("{"{2,}|"$"{2,}|(("{"+|"$"+)[`])) {
        yyless(yyleng - 1);
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, '`');
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_HEREDOC>{HEREDOC_CHARS}*({HEREDOC_NEWLINE}+({LABEL}";"?)?)? {
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, 0);
        _scanner->setToken(yytext, yyleng, strval.c_str(), strval.length());
        return T_ENCAPSED_AND_WHITESPACE;
}

<ST_HEREDOC>{HEREDOC_CHARS}*({HEREDOC_NEWLINE}+({LABEL}";"?)?)?("{"{2,}|"$"{2,}) {
        yyless(yyleng - 1);
        std::string strval = _scanner->scanEscapeString(yytext, yyleng, 0);
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
void _scanner_init() {
  BEGIN(INITIAL);
}
static void __attribute__((__unused__))
suppress_defined_but_not_used_warnings() {
  yy_fatal_error(0);
  yyunput(0, 0);
  yy_top_state();
}
