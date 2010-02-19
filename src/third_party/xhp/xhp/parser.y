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
// PHP's if/else rules use right reduction rather than left reduction which
// means while parsing nested if/else's the stack grows until it the last
// statement is read. This is annoying, particularly because of a quirk in
// bison.
// http://www.gnu.org/software/bison/manual/html_node/Memory-Management.html
// Apparently if you compile a bison parser with g++ it can no longer grow
// the stack. The work around is to just make your initial stack ridiculously
// large. Unfortunately that increases memory usage while parsing which is
// dumb. Anyway, putting a TODO here to fix PHP's if/else grammar.
#define YYINITDEPTH 500
%}

%{
#undef yyextra
#define yyextra static_cast<yy_extra_type*>(xhpget_extra(yyscanner))
#undef yylineno
#define yylineno yyextra->first_lineno
#define cr(s) code_rope(s, yylineno)
#define push_state(s) xhp_new_push_state(s, (struct yyguts_t*) yyscanner)
#define pop_state() xhp_new_pop_state((struct yyguts_t*) yyscanner)
#define set_state(s) xhp_set_state(s, (struct yyguts_t*) yyscanner)
using namespace std;

static void yyerror(void* yyscanner, void* _, const char* error) {
  if (yyextra->terminated) {
    return;
  }
  yyextra->terminated = true;
  yyextra->error = error;
}

static void replacestr(string &source, const string &find, const string &rep) {
  size_t j;
  while ((j = source.find(find)) != std::string::npos) {
    source.replace(j, find.length(), rep);
  }
}

%}

%expect 9
// 2: PHP's if/else grammar
// 7: expr '[' dim_offset ']' -- shift will default to first grammar
%name-prefix = "xhp"
%pure-parser
%parse-param { void* yyscanner }
%parse-param { code_rope* root }
%lex-param { void* yyscanner }
%error-verbose

%left T_INCLUDE T_INCLUDE_ONCE T_EVAL T_REQUIRE T_REQUIRE_ONCE
%left ','
%left T_LOGICAL_OR
%left T_LOGICAL_XOR
%left T_LOGICAL_AND
%right T_PRINT
%left '=' T_PLUS_EQUAL T_MINUS_EQUAL T_MUL_EQUAL T_DIV_EQUAL T_CONCAT_EQUAL T_MOD_EQUAL T_AND_EQUAL T_OR_EQUAL T_XOR_EQUAL T_SL_EQUAL T_SR_EQUAL
%left '?' ':'
%left T_BOOLEAN_OR
%left T_BOOLEAN_AND
%left '|'
%left '^'
%left '&'
%nonassoc T_IS_EQUAL T_IS_NOT_EQUAL T_IS_IDENTICAL T_IS_NOT_IDENTICAL
%nonassoc '<' T_IS_SMALLER_OR_EQUAL '>' T_IS_GREATER_OR_EQUAL
%left T_SL T_SR
%left '+' '-' '.'
%left '*' '/' '%'
%right '!'
%nonassoc T_INSTANCEOF
%right '~' T_INC T_DEC T_INT_CAST T_DOUBLE_CAST T_STRING_CAST T_UNICODE_CAST T_BINARY_CAST T_ARRAY_CAST T_OBJECT_CAST T_BOOL_CAST T_UNSET_CAST '@'
%right '['
%nonassoc T_NEW T_CLONE
%token T_EXIT
%token T_IF
%left T_ELSEIF
%left T_ELSE
%left T_ENDIF

%token T_LNUMBER
%token T_DNUMBER
%token T_STRING
%token T_STRING_VARNAME /* unused in XHP: `foo` in `"$foo"` */
%token T_VARIABLE
%token T_NUM_STRING /* unused in XHP: `0` in `"$foo[0]"` */
%token T_INLINE_HTML
%token T_CHARACTER /* unused in vanilla PHP */
%token T_BAD_CHARACTER /* unused in vanilla PHP */
%token T_ENCAPSED_AND_WHITESPACE /* unused in XHP: ` ` in `" "` */
%token T_CONSTANT_ENCAPSED_STRING /* overloaded in XHP; replaces '"' encaps_list '"' */
%token T_BACKTICKS_EXPR /* new in XHP; replaces '`' backticks_expr '`' */
%token T_ECHO
%token T_DO
%token T_WHILE
%token T_ENDWHILE
%token T_FOR
%token T_ENDFOR
%token T_FOREACH
%token T_ENDFOREACH
%token T_DECLARE
%token T_ENDDECLARE
%token T_AS
%token T_SWITCH
%token T_ENDSWITCH
%token T_CASE
%token T_DEFAULT
%token T_BREAK
%token T_CONTINUE
%token T_GOTO
%token T_FUNCTION
%token T_CONST
%token T_RETURN
%token T_TRY
%token T_CATCH
%token T_THROW
%token T_USE
%token T_GLOBAL
%right T_STATIC T_ABSTRACT T_FINAL T_PRIVATE T_PROTECTED T_PUBLIC
%token T_VAR
%token T_UNSET
%token T_ISSET
%token T_EMPTY
%token T_HALT_COMPILER
%token T_CLASS
%token T_INTERFACE
%token T_EXTENDS
%token T_IMPLEMENTS
%token T_OBJECT_OPERATOR
%token T_DOUBLE_ARROW
%token T_LIST
%token T_ARRAY
%token T_CLASS_C
%token T_METHOD_C
%token T_FUNC_C
%token T_LINE
%token T_FILE
%token T_COMMENT
%token T_DOC_COMMENT
%token T_OPEN_TAG
%token T_OPEN_TAG_WITH_ECHO
%token T_OPEN_TAG_FAKE
%token T_CLOSE_TAG
%token T_WHITESPACE
%token T_START_HEREDOC /* unused in XHP; replaced with T_HEREDOC */
%token T_END_HEREDOC /* unused in XHP; replaced with T_HEREDOC */
%token T_HEREDOC /* new in XHP; replaces start_heredoc encaps_list T_END_HEREDOC */
%token T_DOLLAR_OPEN_CURLY_BRACES /* unused in XHP: `${` in `"${foo}"` */
%token T_CURLY_OPEN /* unused in XHP: `{$` in `"{$foo}"` */
%token T_PAAMAYIM_NEKUDOTAYIM
%token T_BINARY_DOUBLE /* unsused in XHP: `b"` in `b"foo"` */
%token T_BINARY_HEREDOC /* unsused in XHP: `b<<<` in `b<<<FOO` */
%token T_NAMESPACE
%token T_NS_C
%token T_DIR
%token T_NS_SEPARATOR

%token T_XHP_WHITESPACE
%token T_XHP_TEXT
%token T_XHP_LT_DIV
%token T_XHP_LT_DIV_GT
%token T_XHP_ATTRIBUTE
%token T_XHP_CATEGORY
%token T_XHP_CHILDREN
%token T_XHP_ANY
%token T_XHP_EMPTY
%token T_XHP_PCDATA
%token T_XHP_COLON
%token T_XHP_HYPHEN
%token T_XHP_BOOLEAN
%token T_XHP_NUMBER
%token T_XHP_ARRAY
%token T_XHP_STRING
%token T_XHP_ENUM
%token T_XHP_REQUIRED

%%

start:
  top_statement_list {
    *root = $1;
  }
;

top_statement_list:
  top_statement_list top_statement {
    $$ = $1 + $2;
  }
| /* empty */ {
    $$ = "";
  }
;

namespace_name:
  T_STRING
| namespace_name T_NS_SEPARATOR T_STRING {
    $$ = $1 + $2 + $3;
  }
;

top_statement:
  statement
| function_declaration_statement
| class_declaration_statement
| T_HALT_COMPILER '(' ')' ';' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_NAMESPACE namespace_name ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_NAMESPACE namespace_name '{' top_statement_list '}' {
    $$ = $1 + " " + $2 + $3 + $4 + $5;
  }
| T_NAMESPACE '{' top_statement_list '}' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_USE use_declarations ';' {
    $$ = $1 + " " + $2 + $3;
  }
| constant_declaration ';' {
    $$ = $1 + $2;
  }
;

use_declarations:
  use_declarations ',' use_declaration {
    $$ = $1 + $2 + $3;
  }
| use_declaration
;

use_declaration:
  namespace_name
| namespace_name T_AS T_STRING {
    $$ = $1 + $2 + $3;
  }
| T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2;
  }
| T_NS_SEPARATOR namespace_name T_AS T_STRING {
    $$ = $1 + $2 + " " + $3 + " " + $4;
  }
;

constant_declaration:
  constant_declaration ',' T_STRING '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_CONST T_STRING '=' static_scalar {
    $$ = $1 + " " + $2 + $3 + $4;
  }
;

inner_statement_list:
  inner_statement_list inner_statement {
    $$ = $1 + $2;
  }
| /* empty */ {
    $$ = "";
  }
;

inner_statement:
  statement
| function_declaration_statement
| class_declaration_statement
| T_HALT_COMPILER '(' ')' ';' {
    $$ = $1 + $2 + $3 + $4;
  }
;

statement:
  unticked_statement
| T_STRING ':' {
    $$ = $1 + $2;
  }
| T_OPEN_TAG
| T_OPEN_TAG_WITH_ECHO
| T_OPEN_TAG_FAKE {
    $$ = "";
  }
;

unticked_statement:
  '{' inner_statement_list '}' {
    $$ = $1 + $2 + $3;
  }
| T_IF '(' expr ')' statement elseif_list else_single {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7;
  }
| T_IF '(' expr ')' ':' inner_statement_list new_elseif_list new_else_single T_ENDIF ';' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9 + $10;
  }
| T_WHILE '(' expr ')' while_statement {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_DO statement T_WHILE '(' expr ')' ';' {
    $$ = $1 + " " + $2 + $3 + $4 + $5 + $6 + $7;
  }
| T_FOR '(' for_expr ';' for_expr ';' for_expr ')' for_statement {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9;
  }
| T_SWITCH '(' expr ')' switch_case_list {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_BREAK ';' {
    $$ = $1 + $2;
  }
| T_BREAK expr ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_CONTINUE ';' {
    $$ = $1 + $2;
  }
| T_CONTINUE expr ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_RETURN ';' {
    $$ = $1 + $2;
  }
| T_RETURN expr_without_variable ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_RETURN variable ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_GLOBAL global_var_list ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_STATIC static_var_list ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_ECHO echo_expr_list ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_INLINE_HTML
| expr ';' {
    $$ = $1 + $2;
  }
| T_UNSET '(' unset_variables ')' ';' {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_FOREACH '(' variable T_AS foreach_variable foreach_optional_arg ')' foreach_statement {
    $$ = $1 + $2 + $3 + " " + $4 + " " + $5 + $6 + $7 + $8;
  }
| T_FOREACH '(' expr_without_variable T_AS variable foreach_optional_arg ')' foreach_statement {
    $$ = $1 + $2 + $3 + " " + $4 + " " + $5 + $6 + $7 + $8;
  }
| T_DECLARE '(' declare_list ')' declare_statement {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| ';' /* empty statement */
| T_TRY '{' inner_statement_list '}' T_CATCH '(' fully_qualified_class_name T_VARIABLE ')' '{' inner_statement_list '}' additional_catches {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7 + " " + $8 + $9 + $10 + $11 + $12 + $13;
  }
| T_THROW expr ';' {
    $$ = $1 + " " + $2 + $3;
  }
| T_GOTO T_STRING ';' {
    $$ = $1 + " " + $2 + $3;
  }
;

additional_catches:
  non_empty_additional_catches
| /* empty */ {
    $$ = "";
  }
;

non_empty_additional_catches:
  additional_catch
| non_empty_additional_catches additional_catch {
    $$ = $1 + $2;
  }
;

additional_catch:
  T_CATCH '(' fully_qualified_class_name T_VARIABLE ')' '{' inner_statement_list '}' {
    $$ = $1 + $2 + $3 + " " + $4 + $5 + $6 + $7 + $8;
  }
;

unset_variables:
  unset_variable
| unset_variables ',' unset_variable {
    $$ = $1 + $2 + $3;
  }
;

unset_variable:
  variable
;

function_declaration_statement:
  unticked_function_declaration_statement
;

class_declaration_statement:
  unticked_class_declaration_statement
;

is_reference:
  /* empty */ {
    $$ = "";
  }
| '&'
;

unticked_function_declaration_statement:
  function is_reference T_STRING '(' parameter_list ')' '{' inner_statement_list '}' {
    $$ = $1 + " " + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9;
  }
;

unticked_class_declaration_statement:
  class_entry_type T_STRING extends_from implements_list '{' class_statement_list '}' {
    $$ = $1 + " " + $2 + $3 + $4 + $5 + $6 + $7;
  }
| interface_entry T_STRING interface_extends_list '{' class_statement_list '}' {
    $$ = $1 + " " + $2 + $3 + $4 + $5 + $6;
  }
;

class_entry_type:
  T_CLASS
| T_ABSTRACT T_CLASS {
    $$ = $1 + " " + $2;
  }
| T_FINAL T_CLASS {
    $$ = $1 + " " + $2;
  }
;

extends_from:
  /* empty */ {
    $$ = "";
  }
| T_EXTENDS fully_qualified_class_name {
    $$ = " " + $1 + " " + $2;
  }
;

interface_entry:
  T_INTERFACE
;

interface_extends_list:
  /* empty */ {
    $$ = "";
  }
| T_EXTENDS interface_list {
    $$ = $1 + " " + $2;
  }
;

implements_list:
  /* empty */ {
    $$ = "";
  }
| T_IMPLEMENTS interface_list {
    $$ = " " + $1 + " " + $2;
  }
;

interface_list:
  fully_qualified_class_name
| interface_list ',' fully_qualified_class_name {
    $$ = $1 + $2 + $3;
  }
;

foreach_optional_arg:
  /* empty */ {
    $$ = "";
  }
| T_DOUBLE_ARROW foreach_variable {
    $$ = $1 + $2;
  }
;

foreach_variable:
  variable
| '&' variable {
    $$ = $1 + $2;
  }
;

for_statement:
  statement
| ':' inner_statement_list T_ENDFOR ';' {
    $$ = $1 + $2 + $3 + $4;
  }
;

foreach_statement:
  statement
| ':' inner_statement_list T_ENDFOREACH ';' {
    $$ = $1 + $2 + $3 + $4;
  }
;

declare_statement:
  statement
| ':' inner_statement_list T_ENDDECLARE ';' {
    $$ = $1 + $2 + $3 + $4;
  }
;

declare_list:
  T_STRING '=' static_scalar {
    $$ = $1 + $2 + $3;
  }
| declare_list ',' T_STRING '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
;

switch_case_list:
  '{' case_list '}' {
    $$ = $1 + $2 + $3;
  }
| '{' ';' case_list '}' {
    $$ = $1 + $2 + $3 + $4;
  }
| ':' case_list T_ENDSWITCH ';' {
    $$ = $1 + $2 + $3 + $4;
  }
| ':' ';' case_list T_ENDSWITCH ';' {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
;

case_list:
  /* empty */ {
    $$ = "";
  }
| case_list T_CASE expr case_separator inner_statement_list {
    $$ = $1 + $2 + " " + $3 + $4 + $5;
  }
| case_list T_DEFAULT case_separator inner_statement_list {
    $$ = $1 + $2 + $3 + $4;
  }
;

case_separator:
  ':'
| ';'
;

while_statement:
  statement
| ':' inner_statement_list T_ENDWHILE ';' {
    $$ = $1 + $2 + $3 + $4;
  }
;

elseif_list:
  /* empty */ {
    $$ = "";
  }
| elseif_list T_ELSEIF '(' expr ')' statement {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
;

new_elseif_list:
  /* empty */ {
    $$ = "";
  }
| new_elseif_list T_ELSEIF '(' expr ')' ':' inner_statement_list {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7;
  }
;

else_single:
  /* empty */ {
    $$ = "";
  }
| T_ELSE statement {
    $$ = $1 + " " + $2;
  }
;

new_else_single:
  /* empty */ {
    $$ = "";
  }
| T_ELSE ':' inner_statement_list {
    $$ = $1 + $2 + $3;
  }
;

parameter_list:
  non_empty_parameter_list
| /* empty */ {
    $$ = "";
  }
;

non_empty_parameter_list:
  optional_class_type T_VARIABLE {
    $$ = $1 + $2;
  }
| optional_class_type '&' T_VARIABLE {
    $$ = $1 + $2 + $3;
  }
| optional_class_type '&' T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| optional_class_type T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4;
  }
| non_empty_parameter_list ',' optional_class_type T_VARIABLE {
    $$ = $1 + $2 + $3 + $4;
  }
| non_empty_parameter_list ',' optional_class_type '&' T_VARIABLE {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| non_empty_parameter_list ',' optional_class_type '&' T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7;
  }
| non_empty_parameter_list ',' optional_class_type T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
;

optional_class_type:
  /* empty */ {
    $$ = "";
  }
| fully_qualified_class_name {
    $$ = $1 + " ";
  }
| T_ARRAY {
    $$ = $1 + " ";
  }
;

function_call_parameter_list:
  non_empty_function_call_parameter_list
| /* empty */ {
    $$ = "";
  }
;

non_empty_function_call_parameter_list:
  expr_without_variable
| variable
| '&' w_variable {
    $$ = $1 + $2;
  }
| non_empty_function_call_parameter_list ',' expr_without_variable {
    $$ = $1 + $2 + $3;
  }
| non_empty_function_call_parameter_list ',' variable {
    $$ = $1 + $2 + $3;
  }
| non_empty_function_call_parameter_list ',' '&' w_variable {
    $$ = $1 + $2 + $3 + $4;
  }
;

global_var_list:
  global_var_list ',' global_var {
    $$ = $1 + $2 + $3;
  }
| global_var
;

global_var:
  T_VARIABLE
| '$' r_variable {
    $$ = $1 + $2;
  }
| '$' '{' expr '}' {
    $$ = $1 + $2 + $3 + $4;
  }
;

static_var_list:
  static_var_list ',' T_VARIABLE {
    $$ = $1 + $2 + $3;
  }
| static_var_list ',' T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_VARIABLE
| T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3;
  }
;

class_statement_list:
  class_statement_list class_statement {
    $$ = $1 + $2;
  }
| /* empty */ {
    $$ = "";
  }
;

class_statement:
  variable_modifiers class_variable_declaration ';' {
    $$ = $1 + " " + $2 + $3;
  }
| class_constant_declaration ';' {
    $$ = $1 + $2;
  }
| method_modifiers function {
    yyextra->old_expecting_xhp_class_statements = yyextra->expecting_xhp_class_statements;
    yyextra->expecting_xhp_class_statements = false;
  } is_reference T_STRING '(' parameter_list ')' method_body {
    yyextra->expecting_xhp_class_statements = yyextra->old_expecting_xhp_class_statements;
    $$ = $1 + $2 + " " + $4 + $5 + $6 + $7 + $8 + $9;
  }
;

method_body:
  ';' /* abstract method */
| '{' inner_statement_list '}' {
    $$ = $1 + $2 + $3;
  }
;

variable_modifiers:
  non_empty_member_modifiers
| T_VAR {
    $$ = $1 + " ";
  }
;

method_modifiers:
  /* empty */ {
    $$ = "";
  }
| non_empty_member_modifiers {
    $$ = $1 + " ";
  }
;

non_empty_member_modifiers:
  member_modifier
| non_empty_member_modifiers member_modifier {
    $$ = $1 + " " + $2;
  }
;

member_modifier:
  T_PUBLIC
| T_PROTECTED
| T_PRIVATE
| T_STATIC
| T_ABSTRACT
| T_FINAL
;

class_variable_declaration:
  class_variable_declaration ',' T_VARIABLE {
    $$ = $1 + $2 + $3;
  }
| class_variable_declaration ',' T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_VARIABLE
| T_VARIABLE '=' static_scalar {
    $$ = $1 + $2 + $3;
  }
;

class_constant_declaration:
  class_constant_declaration ',' T_STRING '=' static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| T_CONST T_STRING '=' static_scalar {
    $$ = $1 + " " + $2 + $3 + $4;
  }
;

echo_expr_list:
  echo_expr_list ',' expr {
    $$ = $1 + $2 + $3;
  }
| expr
;

for_expr:
  /* empty */ {
    $$ = "";
  }
| non_empty_for_expr
;


non_empty_for_expr:
  non_empty_for_expr ',' expr {
    $$ = $1 + $2 + $3;
  }
| expr
;

expr_without_variable:
  T_LIST '(' assignment_list ')' '=' expr {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| variable '=' expr {
    $$ = $1 + $2 + $3;
  }
| variable '=' '&' variable {
    $$ = $1 + $2 + $3 + $4;
  }
| variable '=' '&' T_NEW class_name_reference ctor_arguments {
    $$ = $1 + $2 + $3 + $4 + " " + $5 + $6;
  }
| T_NEW class_name_reference ctor_arguments {
    $$ = $1 + " " + $2 + $3;
  }
| T_CLONE expr {
    $$ = $1 + " " + $2;
  }
| variable T_PLUS_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_MINUS_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_MUL_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_DIV_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_CONCAT_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_MOD_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_AND_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_OR_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_XOR_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_SL_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| variable T_SR_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| rw_variable T_INC {
    $$ = $1 + $2;
  }
| T_INC rw_variable {
    $$ = $1 + $2;
  }
| rw_variable T_DEC {
    $$ = $1 + $2;
  }
| T_DEC rw_variable {
    $$ = $1 + $2;
  }
| expr T_BOOLEAN_OR expr {
    $$ = $1 + $2 + $3;
  }
| expr T_BOOLEAN_AND expr {
    $$ = $1 + $2 + $3;
  }
| expr T_LOGICAL_OR expr {
    $$ = $1 + $2 + $3;
  }
| expr T_LOGICAL_AND expr {
    $$ = $1 + $2 + $3;
  }
| expr T_LOGICAL_XOR expr {
    $$ = $1 + $2 + $3;
  }
| expr '|' expr {
    $$ = $1 + $2 + $3;
  }
| expr '&' expr {
    $$ = $1 + $2 + $3;
  }
| expr '^' expr {
    $$ = $1 + $2 + $3;
  }
| expr '.' expr {
    $$ = $1 + $2 + $3;
  }
| expr '+' expr {
    $$ = $1 + $2 + $3;
  }
| expr '-' expr {
    $$ = $1 + $2 + $3;
  }
| expr '*' expr {
    $$ = $1 + $2 + $3;
  }
| expr '/' expr {
    $$ = $1 + $2 + $3;
  }
| expr '%' expr {
    $$ = $1 + $2 + $3;
  }
| expr T_SL expr {
    $$ = $1 + $2 + $3;
  }
| expr T_SR expr {
    $$ = $1 + $2 + $3;
  }
| '+' expr %prec T_INC {
    $$ = $1 + $2;
  }
| '-' expr %prec T_INC {
    $$ = $1 + $2;
  }
| '!' expr {
    $$ = $1 + $2;
  }
| '~' expr {
    $$ = $1 + $2;
  }
| expr T_IS_IDENTICAL expr {
    $$ = $1 + $2 + $3;
  }
| expr T_IS_NOT_IDENTICAL expr {
    $$ = $1 + $2 + $3;
  }
| expr T_IS_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| expr T_IS_NOT_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| expr '<' expr {
    $$ = $1 + $2 + $3;
  }
| expr T_IS_SMALLER_OR_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| expr '>' expr {
    $$ = $1 + $2 + $3;
  }
| expr T_IS_GREATER_OR_EQUAL expr {
    $$ = $1 + $2 + $3;
  }
| expr T_INSTANCEOF class_name_reference {
    $$ = $1 + " " + $2 + " " + $3;
  }
| '(' expr ')' {
    $$ = $1 + $2 + $3;
  }
| expr '?' expr ':' expr {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| expr '?' ':' expr {
    $$ = $1 + $2 + $3 + $4;
  }
| internal_functions_in_yacc
| T_INT_CAST expr {
    $$ = $1 + $2;
  }
| T_DOUBLE_CAST expr {
    $$ = $1 + $2;
  }
| T_STRING_CAST expr {
    $$ = $1 + $2;
  }
| T_UNICODE_CAST expr {
    $$ = $1 + $2;
  }
| T_BINARY_CAST expr {
    $$ = $1 + $2;
  }
| T_ARRAY_CAST expr {
    $$ = $1 + $2;
  }
| T_OBJECT_CAST expr {
    $$ = $1 + $2;
  }
| T_BOOL_CAST expr {
    $$ = $1 + $2;
  }
| T_UNSET_CAST expr {
    $$ = $1 + $2;
  }
| T_EXIT exit_expr {
    $$ = $1 + $2;
  }
| '@' expr {
    $$ = $1 + $2;
  }
| scalar
| T_ARRAY '(' array_pair_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_BACKTICKS_EXPR
| T_PRINT expr {
    $$ = $1 + " " + $2;
  }
| function is_reference '(' parameter_list ')' lexical_vars '{' inner_statement_list '}' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9;
  }
| T_STATIC function is_reference '(' parameter_list ')' lexical_vars '{' inner_statement_list '}' {
    $$ = $1 + " " + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9 + $10;
  }
;

function:
  T_FUNCTION
;

lexical_vars:
  /* empty */
| T_USE '(' lexical_var_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
;

lexical_var_list:
  lexical_var_list ',' T_VARIABLE {
    $$ = $1 + $2 + $3;
  }
| lexical_var_list ',' '&' T_VARIABLE {
    $$ = $1 + $2 + $3 + $4;
  }
| T_VARIABLE
| '&' T_VARIABLE {
    $$ = $1 + $2;
  }
;

function_call:
  namespace_name '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_NAMESPACE T_NS_SEPARATOR namespace_name '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| T_NS_SEPARATOR namespace_name '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| class_name T_PAAMAYIM_NEKUDOTAYIM T_STRING '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| variable_class_name T_PAAMAYIM_NEKUDOTAYIM T_STRING '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| variable_class_name T_PAAMAYIM_NEKUDOTAYIM variable_without_objects '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| class_name T_PAAMAYIM_NEKUDOTAYIM variable_without_objects '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| variable_without_objects '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
;

class_name:
  T_STATIC
| namespace_name
| T_NAMESPACE T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2 + $3;
  }
| T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2;
  }
;

fully_qualified_class_name:
  namespace_name
| T_NAMESPACE T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2 + $3;
  }
| T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2;
  }
;

class_name_reference:
  class_name
| dynamic_class_name_reference
;

dynamic_class_name_reference:
  base_variable T_OBJECT_OPERATOR object_property dynamic_class_name_variable_properties {
    $$ = $1 + $2 + $3 + $4;
  }
| base_variable
;

dynamic_class_name_variable_properties:
  dynamic_class_name_variable_properties dynamic_class_name_variable_property {
    $$ = $1 + $2;
  }
| /* empty */ {
    $$ = "";
  }
;

dynamic_class_name_variable_property:
  T_OBJECT_OPERATOR object_property {
    $$ = $1 + $2;
  }
;

exit_expr:
  /* empty */ {
    $$ = "";
  }
| '(' ')' {
    $$ = $1 + $2;
  }
| '(' expr ')' {
    $$ = $1 + $2 + $3;
  }
;

ctor_arguments:
  /* empty */ {
    $$ = "";
  }
| '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3;
  }
;

common_scalar:
  T_LNUMBER
| T_DNUMBER
| T_CONSTANT_ENCAPSED_STRING
| T_LINE
| T_FILE
| T_DIR
| T_CLASS_C
| T_METHOD_C
| T_FUNC_C
| T_NS_C
| T_HEREDOC
;

static_scalar: /* compile-time evaluated scalars */
  common_scalar
| namespace_name
| T_NAMESPACE T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2 + $3;
  }
| T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2;
  }
| '+' static_scalar {
    $$ = $1 + $2;
  }
| '-' static_scalar {
    $$ = $1 + $2;
  }
| T_ARRAY '(' static_array_pair_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| static_class_constant
;

static_class_constant:
  class_name T_PAAMAYIM_NEKUDOTAYIM T_STRING {
    $$ = $1 + $2 + $3;
  }
;

scalar:
  T_STRING_VARNAME
| class_constant
| namespace_name
| T_NAMESPACE T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2 + $3;
  }
| T_NS_SEPARATOR namespace_name {
    $$ = $1 + $2;
  }
| common_scalar
;

static_array_pair_list:
  /* empty */ {
    $$ = "";
  }
| non_empty_static_array_pair_list possible_comma
;

possible_comma:
  /* empty */ {
    $$ = "";
  }
| ','
;

non_empty_static_array_pair_list:
  non_empty_static_array_pair_list ',' static_scalar T_DOUBLE_ARROW static_scalar {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| non_empty_static_array_pair_list ',' static_scalar {
    $$ = $1 + $2 + $3;
  }
| static_scalar T_DOUBLE_ARROW static_scalar {
    $$ = $1 + $2 + $3;
  }
| static_scalar
;

expr:
  r_variable
| expr_without_variable
;

r_variable:
  variable
;

w_variable:
  variable
;

rw_variable:
  variable
;

variable:
  base_variable_with_function_calls T_OBJECT_OPERATOR object_property method_or_not variable_properties {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| base_variable_with_function_calls
;

variable_properties:
  variable_properties variable_property {
    $$ = $1 + $2;
  }
| /* empty */ {
    $$ = "";
  }
;

variable_property:
  T_OBJECT_OPERATOR object_property method_or_not {
    $$ = $1 + $2 + $3;
  }
;

method_or_not:
  '(' function_call_parameter_list ')' {
    $$ = $1 + $2 + $3;
  }
| /* empty */ {
    $$ = "";
  }
;

variable_without_objects:
  reference_variable
| simple_indirect_reference reference_variable {
    $$ = $1 + $2;
  }
;

static_member:
  class_name T_PAAMAYIM_NEKUDOTAYIM variable_without_objects {
    $$ = $1 + $2 + $3;
  }
| variable_class_name T_PAAMAYIM_NEKUDOTAYIM variable_without_objects {
    $$ = $1 + $2 + $3;
  }
;

variable_class_name:
  reference_variable
;

base_variable_with_function_calls:
  base_variable
| function_call
;

base_variable:
  reference_variable
| simple_indirect_reference reference_variable {
    $$ = $1 + $2;
  }
| static_member
;

reference_variable:
  reference_variable '[' dim_offset ']' {
    $$ = $1 + $2 + $3 + $4;
  }
| reference_variable '{' expr '}' {
    $$ = $1 + $2 + $3;
  }
| compound_variable
;

compound_variable:
  T_VARIABLE
| '$' '{' expr '}' {
    $$ = $1 + $2 + $3 + $4;
  }
;

dim_offset:
  /* empty */ {
    $$ = "";
  }
| expr
;

object_property:
  object_dim_list
| variable_without_objects
;

object_dim_list:
  object_dim_list '[' dim_offset ']' {
    $$ = $1 + $2 + $3 + $4;
  }
| object_dim_list '{' expr '}' {
    $$ = $1 + $2 + $3 + $4;
  }
| variable_name
;

variable_name:
  T_STRING
| '{' expr '}' {
    $$ = $1 + $2 + $3;
  }
;

simple_indirect_reference:
  '$'
| simple_indirect_reference '$' {
    $$ = $1 + $2;
  }
;

assignment_list:
  assignment_list ',' assignment_list_element {
    $$ = $1 + $2 + $3;
  }
| assignment_list_element
;

assignment_list_element:
  variable
| T_LIST '(' assignment_list ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| /* empty */ {
    $$ = "";
  }
;

array_pair_list:
  /* empty */ {
    $$ = "";
  }
| non_empty_array_pair_list possible_comma
;

non_empty_array_pair_list:
  non_empty_array_pair_list ',' expr T_DOUBLE_ARROW expr {
    $$ = $1 + $2 + $3 + $4 + $5;
  }
| non_empty_array_pair_list ',' expr {
    $$ = $1 + $2 + $3;
  }
| expr T_DOUBLE_ARROW expr {
    $$ = $1 + $2 + $3;
  }
| expr
| non_empty_array_pair_list ',' expr T_DOUBLE_ARROW '&' w_variable {
    $$ = $1 + $2 + $3 + $4 + $5 + $6;
  }
| non_empty_array_pair_list ',' '&' w_variable {
    $$ = $1 + $2 + $3 + $4;
  }
| expr T_DOUBLE_ARROW '&' w_variable {
    $$ = $1 + $2 + $3 + $4;
  }
| '&' w_variable {
    $$ = $1 + $2;
  }
;

internal_functions_in_yacc:
  T_ISSET '(' isset_variables ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_EMPTY '(' variable ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_INCLUDE expr {
    $$ = $1 + " " + $2;
  }
| T_INCLUDE_ONCE expr {
    $$ = $1 + " " + $2;
  }
| T_EVAL '(' expr ')' {
    $$ = $1 + $2 + $3 + $4;
  }
| T_REQUIRE expr {
    $$ = $1 + " " + $2;
  }
| T_REQUIRE_ONCE expr {
    $$ = $1 + " " + $2;
  }
;

isset_variables:
  variable
| isset_variables ',' variable {
    $$ = $1 + $2 + $3;
  }
;

class_constant:
  class_name T_PAAMAYIM_NEKUDOTAYIM T_STRING {
    $$ = $1 + $2 + $3;
  }
| variable_class_name T_PAAMAYIM_NEKUDOTAYIM T_STRING {
    $$ = $1 + $2 + $3;
  }
;

//
// XHP Extensions

// Tags
expr_without_variable:
  xhp_tag_expression {
    $$ = $1;
    yyextra->used = true;
  }
;

xhp_tag_expression:
  xhp_singleton
| xhp_tag_open xhp_children xhp_tag_close {
    if (yyextra->include_debug) {
      char line[16];
      sprintf(line, "%lu", (unsigned long)$1.lineno());
      $$ = $1 + $2 + "), __FILE__, " + line +")";
    } else {
      $$ = $1 + $2 + "))";
    }
  }
;

xhp_singleton:
  xhp_tag_start xhp_attributes '/' '>' {
    pop_state(); // XHP_ATTRS
    if (yyextra->include_debug) {
      char line[16];
      sprintf(line, "%lu", (unsigned long)$1.lineno());
      $$ = "new xhp_" + $1 + "(array(" + $2 + "), array(), __FILE__, " + line + ")";
    } else {
      $$ = "new xhp_" + $1 + "(array(" + $2 + "), array())";
    }
  }
;

xhp_tag_open:
  xhp_tag_start xhp_attributes '>' {
    pop_state(); // XHP_ATTRS
    push_state(XHP_CHILD_START);
    yyextra->pushTag($1.c_str());
    $$ = "new xhp_" + $1 + "(array(" + $2 + "), array(";
  }
;

xhp_tag_close:
  T_XHP_LT_DIV xhp_label_no_space '>' {
    pop_state(); // XHP_CHILD_START
    if (yyextra->peekTag() != $2.c_str()) {
      string e1 = $2.c_str();
      string e2 = yyextra->peekTag();
      replacestr(e1, "__", ":");
      replacestr(e1, "_", "-");
      replacestr(e2, "__", ":");
      replacestr(e2, "_", "-");
      string e = "syntax error, mismatched tag </" + e1 + ">, expecting </" + e2 +">";
      yyerror(yyscanner, NULL, e.c_str());
      yyextra->terminated = true;
    }
    yyextra->popTag();
    if (yyextra->haveTag()) {
      set_state(XHP_CHILD_START);
    }
  }
| T_XHP_LT_DIV_GT {
    // empty end tag -- SGML SHORTTAG
    pop_state(); // XHP_CHILD_START
    yyextra->popTag();
    if (yyextra->haveTag()) {
      set_state(XHP_CHILD_START);
    }
    $$ = "))";
  }
;

xhp_tag_start:
  '<' xhp_label_immediate {
    $$ = $2;
  }
;

// Children
xhp_literal_text:
  T_XHP_TEXT
| xhp_literal_text T_XHP_TEXT {
    $$ = $1 + $2;
  }
;

xhp_children:
  /* empty */ {
    $$ = "";
  }
| xhp_literal_text {
    set_state(XHP_CHILD_START);
    $$ = "'" + $1 + "',";
  }
| xhp_children xhp_child {
    set_state(XHP_CHILD_START);
    $$ = $1 + $2 + ",";
  }
| xhp_children xhp_child xhp_literal_text {
    set_state(XHP_CHILD_START);
    $$ = $1 + $2 + ",'" + $3 + "',";
  }
;

xhp_child:
  xhp_tag_expression
| '{' {
    push_state(PHP);
    yyextra->pushStack();
  } expr '}' {
    pop_state();
    yyextra->popStack();
  } {
    set_state(XHP_CHILD_START);
    $$ = $3;
  }
;

// Attributes
xhp_attributes:
  /* empty */ {
    $$ = "";
    push_state(XHP_ATTRS);
  }
| xhp_attributes xhp_attribute {
    $$ = $1 + $2 + ",";
  }
;

xhp_attribute:
  xhp_label_pass '=' xhp_attribute_value {
    $$ = "'" + $1 + "' => " + $3;
  }
;

xhp_attribute_value:
  '"' { push_state(XHP_ATTR_VAL); } xhp_attribute_quoted_value '"' {
    $$ = $3;
  }
| '{' { push_state(PHP); } expr { pop_state(); } '}' {
    $$ = $3;
  }
;

xhp_attribute_quoted_value:
  /* empty */ {
    $$ = "''";
  }
| xhp_literal_text {
    // XHP_ATTR_VAL is popped by the time this code runs
    $$ = "'" + $1 + "'";
  }
;

// Misc
xhp_label_immediate:
  { push_state(XHP_LABEL); } xhp_label_ xhp_whitespace_hack {
    pop_state();
    $$ = $2;
  }
;

xhp_label_no_space:
  { push_state(XHP_LABEL); } xhp_label_ {
    pop_state();
    $$ = $2;
  }
;

xhp_label_pass:
  { push_state(XHP_LABEL_WHITESPACE); } xhp_label_pass_ xhp_whitespace_hack {
    pop_state();
    $$ = $2;
  }
;

xhp_label:
  { push_state(XHP_LABEL_WHITESPACE); } xhp_label_ xhp_whitespace_hack {
    pop_state();
    $$ = $2;
  }
;

xhp_label_:
  T_STRING {
    // XHP_LABEL is popped in the scanner on " ", ">", "/", or "="
    push_state(XHP_LABEL);
    $$ = $1;
  }
| xhp_label_ T_XHP_COLON T_STRING {
    $$ = $1 + "__" + $3;
  }
| xhp_label_ T_XHP_HYPHEN T_STRING {
    $$ = $1 + "_" + $3;
  }
;

xhp_label_pass_:
  T_STRING {
    // XHP_LABEL is popped in the scanner on " ", ">", "/", or "="
    push_state(XHP_LABEL);
    $$ = $1;
  }
| xhp_label_pass_ T_XHP_COLON T_STRING {
    $$ = $1 + ":" + $3;
  }
| xhp_label_pass_ T_XHP_HYPHEN T_STRING {
    $$ = $1 + "-" + $3;
  }
;

xhp_whitespace_hack:
  T_XHP_WHITESPACE
| /* empty */
;

// Elements
class_declaration_statement:
  class_entry_type ':' xhp_label_immediate extends_from implements_list '{' {
    yyextra->expecting_xhp_class_statements = true;
    yyextra->attribute_decls = "";
    yyextra->attribute_inherit = "";
    yyextra->used_attributes = false;
  } class_statement_list {
    yyextra->expecting_xhp_class_statements = false;
  } '}' {
    $$ = $1 + " xhp_" + $3 + $4 + $5 + $6 + $8;
    if (yyextra->used_attributes) {
      $$ = $$ +
        "protected static function &__xhpAttributeDeclaration() {" +
          "static $_ = -1;" +
          "if ($_ === -1) {" +
            "$_ = array_merge(parent::__xhpAttributeDeclaration(), " +
              yyextra->attribute_inherit +
              "array(" + yyextra->attribute_decls + "));" +
          "}" +
          "return $_;"
        "}";
    }
    $$ = $$ + $10;
    yyextra->used = true;
  }
;

// Element attribute declaration
class_statement:
  T_XHP_ATTRIBUTE { push_state(XHP_ATTR_TYPE_DECL); } xhp_attribute_decls ';' {
    pop_state();
    yyextra->used = true;
    yyextra->used_attributes = true;
    $$ = ""; // this will be injected when the class closes
  }
;

xhp_attribute_decls:
  xhp_attribute_decl {}
| xhp_attribute_decls ',' xhp_attribute_decl {}
;

xhp_attribute_decl:
  xhp_attribute_decl_type xhp_label_pass xhp_attribute_default xhp_attribute_is_required {
    $2.strip_lines();
    yyextra->attribute_decls = yyextra->attribute_decls +
      "'" + $2 + "'=>array(" + $1 + "," + $3 + ", " + $4 + "),"
  }
| T_XHP_COLON xhp_label_immediate {
    $2.strip_lines();
    yyextra->attribute_inherit = yyextra->attribute_inherit +
      "xhp_" + $2 + "::__xhpAttributeDeclaration(),";
  }
;

xhp_attribute_decl_type:
  T_XHP_STRING {
    $$ = "1, null";
  }
| T_XHP_BOOLEAN {
    $$ = "2, null";
  }
| T_XHP_NUMBER {
    $$ = "3, null";
  }
| T_XHP_ARRAY {
    $$ = "4, null";
  }
| class_name {
    $$ = "5, '" + $1 + "'";
  }
| T_VAR {
    $$ = "6, null";
  }
| T_XHP_ENUM '{' { push_state(PHP); } xhp_attribute_enum { pop_state(); } '}' {
    $$ = "7, array(" + $4 + ")";
  }
;

xhp_attribute_enum:
  common_scalar {
    $1.strip_lines();
    $$ = $1;
  }
| xhp_attribute_enum ',' common_scalar {
    $3.strip_lines();
    $$ = $1 + ", " + $3;
  }
;

xhp_attribute_default:
  '=' common_scalar {
    $2.strip_lines();
    $$ = $2;
  }
| '=' T_STRING {
    $2.strip_lines();
    $$ = $2;
  }
| /* empty */ {
    $$ = "null";
  }
;

xhp_attribute_is_required:
  T_XHP_REQUIRED {
    $$ = "1";
  }
| /* empty */ {
    $$ = "0";
  }
;

// Element category declaration
class_statement:
  T_XHP_CATEGORY { push_state(PHP_NO_RESERVED_WORDS_PERSIST); } xhp_category_list ';' {
    pop_state();
    yyextra->used = true;
    $$ =
      "protected function &__xhpCategoryDeclaration() {\
         static $_ = array(" + $3 + ");" +
        "return $_;" +
      "}";
  }
;

xhp_category_list:
  '%' xhp_label_immediate {
    $$ = "'" + $2 + "' => 1";
  }
| xhp_category_list ',' '%' xhp_label_immediate {
    $$ = $1 + ",'" + $4 + "' => 1";
  }
;

// Element child list
class_statement:
  T_XHP_CHILDREN { push_state(XHP_CHILDREN_DECL); } xhp_children_decl ';' {
    // XHP_CHILDREN_DECL is popped in the scanner on ';'
    yyextra->used = true;
    $$ = "protected function &__xhpChildrenDeclaration() {" + $3 + "}";
  }
;

xhp_children_decl:
  xhp_children_paren_expr {
    $$ = "static $_ = " + $1 + "; return $_;";
  }
| T_XHP_ANY {
    $$ = "static $_ = 1; return $_;";
  }
| T_XHP_EMPTY {
    $$ = "static $_ = 0; return $_;";
  }
;

xhp_children_paren_expr:
  '(' xhp_children_decl_expr ')' {
    $$ = "array(0, 5, " + $2 + ")";
  }
| '(' xhp_children_decl_expr ')' '*' {
    $$ = "array(1, 5, " + $2 + ")";
  }
| '(' xhp_children_decl_expr ')' '?' {
    $$ = "array(2, 5, " + $2 + ")";
  }
| '(' xhp_children_decl_expr ')' '+' {
    $$ = "array(3, 5, " + $2 + ")";
  }
;

xhp_children_decl_expr:
  xhp_children_paren_expr
| xhp_children_decl_tag {
    $$ = "array(0, " + $1 + ")";
  }
| xhp_children_decl_tag '*' {
    $$ = "array(1, " + $1 + ")";
  }
| xhp_children_decl_tag '?' {
    $$ = "array(2, " + $1 + ")";
  }
| xhp_children_decl_tag '+' {
    $$ = "array(3, " + $1 + ")";
  }
| xhp_children_decl_expr ',' xhp_children_decl_expr {
    $$ = "array(4, " + $1 + "," + $3 + ")"
  }
| xhp_children_decl_expr '|' xhp_children_decl_expr {
    $$ = "array(5, " + $1 + "," + $3 + ")"
  }
;

xhp_children_decl_tag:
  T_XHP_ANY {
    $$ = "1, null";
  }
| T_XHP_PCDATA {
    $$ = "2, null";
  }
| T_XHP_COLON xhp_label {
    $$ = "3, \'xhp_" + $2 + "\'";
  }
| '%' xhp_label {
    $$ = "4, \'" + $2 + "\'";
  }
;

// Make XHP classes usable anywhere you see a real class
class_name:
  T_XHP_COLON xhp_label_immediate {
    pop_state();
    push_state(PHP);
    yyextra->used = true;
    $$ = "xhp_" + $2;
  }
;

fully_qualified_class_name:
  T_XHP_COLON xhp_label_immediate {
    pop_state();
    push_state(PHP);
    yyextra->used = true;
    $$ = "xhp_" + $2;
  }
;

// Fix the "bug" in PHP's grammar where you can't chain the [] operator on a
// function call.
// This introduces some shift/reduce conflicts. We want the shift here to fall
// back to regular PHP grammar. In the case where it's an extension of the PHP
// grammar our code gets picked up.
expr_without_variable:
  expr '[' dim_offset ']' {
    if (yyextra->idx_expr) {
      yyextra->used = true;
      $$ = "__xhp_idx(" + $1 + ", " + $3 + ")";
    } else {
      $$ = $1 + $2 + $3 + $4;
    }
  }
;


%%

const char* yytokname(int tok) {
  if (tok < 255) {
    return NULL;
  }
  return yytname[YYTRANSLATE(tok)];
}
