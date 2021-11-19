(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
%{
%}

(* Tokens *)

%token EOF                      (* end of file            *)
%token INCLUDE                  (* include                *)
%token INCLUDE_ONCE             (* include_once           *)
%token EVAL                     (* eval                   *)
%token REQUIRE                  (* require                *)
%token REQUIRE_ONCE             (* require_once           *)
%token LOGICAL_OR               (* or                     *)
%token LOGICAL_XOR              (* xor                    *)
%token LOGICAL_AND              (* and                    *)
%token PRINT                    (* print                  *)
%token YIELD                    (* yield                  *)
%token YIELD_FROM               (* yield from             *)
%token PLUS_EQUAL               (* +=                     *)
%token MINUS_EQUAL              (* -=                     *)
%token MUL_EQUAL                (* *=                     *)
%token DIV_EQUAL                (* /=                     *)
%token CONCAT_EQUAL             (* .=                     *)
%token MOD_EQUAL                (* %=                     *)
%token AND_EQUAL                (* &=                     *)
%token OR_EQUAL                 (* |=                     *)
%token XOR_EQUAL                (* ^=                     *)
%token SHL_EQUAL                (* <<=                    *)
%token SHR_EQUAL                (* >>=                    *)
%token BOOLEAN_OR               (* ||                     *)
%token BOOLEAN_AND              (* &&                     *)
%token IS_EQUAL                 (* ==                     *)
%token IS_NOT_EQUAL             (* !=                     *)
%token IS_IDENTICAL             (* ===                    *)
%token IS_NOT_IDENTICAL         (* !==                    *)
%token IS_SMALLER_OR_EQUAL      (* <=                     *)
%token IS_GREATER_OR_EQUAL      (* >=                     *)
%token SPACESHIP                (* <=>                    *)
%token SHL                      (* <<                     *)
%token SHR_PREFIX               (* >> (SHR_PREFIX RANGLE) *)
%token INSTANCEOF               (* instanceof             *)
%token INC                      (* ++                     *)
%token DEC                      (* --                     *)
%token INT_CAST                 (* (int)                  *)
%token DOUBLE_CAST              (* (double)               *)
%token STRING_CAST              (* (string)               *)
%token ARRAY_CAST               (* (array)                *)
%token OBJECT_CAST              (* (object)               *)
%token BOOL_CAST                (* (bool)                 *)
%token UNSET_CAST               (* (unset)                *)
%token NEW                      (* new                    *)
%token CLONE                    (* clone                  *)
%token EXIT                     (* exit                   *)
%token IF                       (* if                     *)
%token ELSEIF                   (* elseif                 *)
%token ELSE                     (* else                   *)
%token ENDIF                    (* endif                  *)
%token ECHO                     (* echo                   *)
%token DO                       (* do                     *)
%token WHILE                    (* while                  *)
%token FOR                      (* for                    *)
%token FOREACH                  (* foreach                *)
%token DECLARE                  (* declare                *)
%token AS                       (* as                     *)
%token SWITCH                   (* switch                 *)
%token CASE                     (* case                   *)
%token DEFAULT                  (* default                *)
%token BREAK                    (* break                  *)
%token CONTINUE                 (* continue               *)
%token GOTO                     (* goto                   *)
%token FUNCTION                 (* function               *)
%token CONST                    (* const                  *)
%token RETURN                   (* return                 *)
%token TRY                      (* try                    *)
%token CATCH                    (* catch                  *)
%token FINALLY                  (* finally                *)
%token THROW                    (* throw                  *)
%token USE                      (* use                    *)
%token INSTEADOF                (* insteadof              *)
%token GLOBAL                   (* global                 *)
%token STATIC                   (* static                 *)
%token ABSTRACT                 (* abstract               *)
%token FINAL                    (* final                  *)
%token PRIVATE                  (* private                *)
%token PROTECTED                (* protected              *)
%token PUBLIC                   (* public                 *)
%token VAR                      (* var                    *)
%token UNSET                    (* unset                  *)
%token ISSET                    (* isset                  *)
%token EMPTY                    (* empty                  *)
%token HALT_COMPILER            (* __halt_compiler        *)
%token CLASS                    (* class                  *)
%token TRAIT                    (* trait                  *)
%token ENUM                     (* enum                   *)
%token INTERFACE                (* interface              *)
%token EXTENDS                  (* extends                *)
%token IMPLEMENTS               (* implements             *)
%token ARROW                    (* ->                     *)
%token QUESTION_ARROW           (* ?->                    *)
%token DOUBLE_ARROW             (* =>                     *)
%token LONG_DOUBLE_ARROW        (* ==>                    *)
%token LIST                     (* list                   *)
%token ARRAY                    (* array                  *)
%token CALLABLE                 (* callable               *)
%token LINE                     (* __LINE__               *)
%token FILE                     (* __FILE__               *)
%token DIR                      (* __DIR__                *)
%token CLASS_C                  (* __CLASS__              *)
%token TRAIT_C                  (* __TRAIT__              *)
%token METHOD_C                 (* __METHOD__             *)
%token FUNC_C                   (* __FUNCTION__           *)
%token COMMENT                  (* comment                *)
%token DOC_COMMENT              (* doc comment            *)
%token OPEN_TAG                 (* open tag               *)
%token OPEN_TAG_WITH_ECHO       (* open tag with echo     *)
%token CLOSE_TAG                (* close tag              *)
%token CLOSE_TAG_OF_ECHO        (* close tag              *)
%token SPACES NEWLINE           (* whitespace             *)
%token START_HEREDOC            (* heredoc start          *)
%token END_HEREDOC              (* heredoc end            *)
%token DOLLAR_OPEN_CURLY_BRACES (* ${                     *)
%token CURLY_OPEN               (* {$                     *)
%token COLONCOLON               (* ::                     *)
%token NAMESPACE                (* namespace              *)
%token NAMESPACE_C              (* __NAMESPACE__          *)
%token ANTISLASH                (* \\                     *)
%token ELLIPSIS                 (* ...                    *)
%token COALESCE                 (* ??                     *)
%token POW                      (* **                     *)
%token POW_EQUAL                (* **=                    *)
%token EQUAL                    (* '='                    *)
%token CONCAT                   (* '.'                    *)
%token PLUS                     (* '+'                    *)
%token COMMA                    (* ','                    *)
%token QUESTION                 (* '?'                    *)
%token COLON                    (* ':'                    *)
%token OR                       (* '|'                    *)
%token MINUS                    (* '-'                    *)
%token MUL                      (* '*'                    *)
%token DIV                      (* '/'                    *)
%token MOD                      (* '%'                    *)
%token BANG                     (* '!'                    *)
%token LPAREN                   (* '('                    *)
%token RPAREN                   (* ')'                    *)
%token LBRACKET                 (* '['                    *)
%token RBRACKET                 (* ']'                    *)
%token LBRACE                   (* '{'                    *)
%token RBRACE                   (* '}'                    *)
%token LANGLE                   (* '<'                    *)
%token RANGLE                   (* '>'                    *)
%token XOR                      (* '^'                    *)
%token AND                      (* '&'                    *)
%token SEMICOLON                (* ';'                    *)
%token AT                       (* '@'                    *)
%token BACKQUOTE                (* '`'                    *)
%token DOUBLEQUOTE              (* '"'                    *)
%token DOLLAR                   (* '$'                    *)
%token DOLLARDOLLAR             (* '$$'                   *)
%token TILDE                    (* '~'                    *)

(* Extensions *)
%token
  SELF PARENT ASYNC AWAIT TYPE NEWTYPE SHAPE PIPE_ANGLE UNKNOWN
  XHP_ATTRIBUTE XHP_CHILDREN XHP_CATEGORY XHP_ANY XHP_PCDATA XHP_REQUIRED
  XHP_SLASH_GT XHP_GT FORK
  IS (* is *)
  INOUT (* inout *)
  QUESTION_QUESTION        (* '??'  *)
  QUESTION_QUESTION_EQUAL  (* '??=' *)
  USING
  CONCURRENT
  FOREACH_AS (*FIXME*)
  WHERE
  SUPER
  COROUTINE
  SUSPEND

%token<bool ref> FORK_LANGLE

%token<string> XHP_ATTR XHP_TEXT
%token<string list> XHP_COLONID_DEF XHP_PERCENTID_DEF XHP_OPEN_TAG
%token<string list option> XHP_CLOSE_TAG

%left INCLUDE INCLUDE_ONCE REQUIRE REQUIRE_ONCE
%left LOGICAL_OR
%left LOGICAL_XOR
%left LOGICAL_AND
%right PRINT SUSPEND
%right YIELD
%right DOUBLE_ARROW LONG_DOUBLE_ARROW PIPE_ANGLE
(*%right YIELD_FROM*)
%left EQUAL PLUS_EQUAL MINUS_EQUAL MUL_EQUAL DIV_EQUAL CONCAT_EQUAL
      MOD_EQUAL AND_EQUAL OR_EQUAL XOR_EQUAL SHL_EQUAL SHR_EQUAL
      POW_EQUAL QUESTION_QUESTION_EQUAL
%nonassoc pre_COLONCOLON
%left COLONCOLON
%nonassoc ARROW QUESTION_ARROW LBRACKET
%nonassoc LPAREN LBRACE
%left IS AS QUESTION (*SUPER*) QUESTION_QUESTION COLON
(*%right COALESCE*)
%left BOOLEAN_OR
%left BOOLEAN_AND
%left OR
%left XOR
%left AND
%nonassoc IS_EQUAL IS_NOT_EQUAL IS_IDENTICAL SPACESHIP
%right IS_NOT_IDENTICAL
%right LANGLE
%right RANGLE
%nonassoc IS_SMALLER_OR_EQUAL IS_GREATER_OR_EQUAL
%left SHL SHR_PREFIX
%left PLUS MINUS CONCAT
%left MUL DIV MOD
%right BANG
%nonassoc INSTANCEOF
%right OBJECT_CAST BOOL_CAST UNSET_CAST INT_CAST DOUBLE_CAST STRING_CAST ARRAY_CAST
       TILDE INC DEC AT AWAIT (*ASYNC*)
%right POW
%nonassoc CLONE (*CONCURRENT*)
%left pre_ELSE
%left ELSEIF
%left ELSE

%token <string> LNUMBER   (* integer number 42 (LNUMBER) *)
%token <string> DNUMBER   (* floating-point number 42.00 (DNUMBER) *)
%token <string> IDENT     (* identifier "foo" (IDENT) *)
%token <string> VARIABLE  (* variable "$foo" (VARIABLE) *)
%token <string> INLINE_HTML
%token <string> ENCAPSED_AND_WHITESPACE
    (* quoted-string and whitespace (ENCAPSED_AND_WHITESPACE) *)
%token <string> CONSTANT_ENCAPSED_STRING
    (* quoted-string (CONSTANT_ENCAPSED_STRING) *)
%token <string> VARNAME (* ${ varname } (VARNAME) *)
%token <string> NUM_STRING (* number (NUM_STRING) *)

%start<unit> start
%start<unit> dummy

%% (* Rules *)

start:
| top_statement* EOF { () }
;

top_statement:
| statement                                    { () }
| declaration                                  { () }
| HALT_COMPILER LPAREN RPAREN SEMICOLON        { () }
| NAMESPACE namespace_name SEMICOLON           { () }
| NAMESPACE namespace_name LBRACE top_statement* RBRACE { () }
| NAMESPACE LBRACE top_statement* RBRACE       { () }
| USE NAMESPACE? mixed_group_use_declaration SEMICOLON    { () }
| USE NAMESPACE? use_modifier group_use_declaration SEMICOLON { () }
| USE NAMESPACE? comma_list(use_declaration) SEMICOLON    { () }
| USE NAMESPACE? use_modifier comma_list(use_declaration) SEMICOLON { () }
| CONST comma_list(constant_declaration) SEMICOLON       { () }
;

(* Identifiers *)

identifier:
| IDENT           { () }
| XHP_COLONID_DEF { () }
| semi_reserved   { () }
;

name:
| namespace_name                     { () }
| NAMESPACE ANTISLASH namespace_name { () }
| ANTISLASH namespace_name           { () }
| xhp_reserved                       { () }
;

namespace_name:
| IDENT                           { () }
| XHP_COLONID_DEF                 { () }
| namespace_name ANTISLASH IDENT  { () }
;

class_special_name:
| STATIC              { () }
| SELF                { () }
| PARENT              { () }
| SHAPE               { () }
| ARRAY               { () }
;

unparametrized_class_name:
| STATIC              { () }
| SELF                { () }
| PARENT              { () }
| name                { () }
;

class_name:
| unparametrized_class_name { () }
| unparametrized_class_name type_arguments { () }
;

type_name:
| CALLABLE { () }
| SELF     { () }
| PARENT   { () }
| ARRAY    { () }
| name     { () }
;

(* Attributes *)

attribute:
| ANTISLASH attribute { () }
| IDENT { () }
| IDENT COLON attribute { () }
| IDENT LPAREN comma_list_trailing(expr) RPAREN { () }
;

attributes:
| SHL nonempty_comma_list_trailing(attribute) SHR_PREFIX RANGLE { () }
;

(* Declarations *)

declaration:
| attributes         { () }
| declare_function   { () }
| declare_class      { () }
| declare_trait      { () }
| declare_interface  { () }
| declare_enum       { () }
| declare_type       { () }
;

declare_function:
| ASYNC declare_function { () }
| COROUTINE declare_function { () }
| FUNCTION AND? identifier [@doc_comment]
    type_parameters?
    LPAREN comma_list_trailing(parameter) RPAREN return_type? [@fn_flags]
    where_constraints?
    LBRACE statement_inner* RBRACE [@fn_flags]
  { () }
;

where_constraints:
| WHERE comma_list_trailing(where_constraint) { () }
;

where_constraint:
| type_expr EQUAL type_expr { () }
| type_expr AS    type_expr { () }
| type_expr SUPER type_expr { () }
;

declare_class:
| class_modifier* CLASS identifier type_parameters?
    preceded(EXTENDS,class_name)?
    preceded(IMPLEMENTS, comma_list(class_name))? [@doc_comment]
    LBRACE statement_class* RBRACE
  { () }
;

class_modifier:
| ABSTRACT { () }
| FINAL    { () }
;

declare_trait:
| TRAIT identifier type_parameters?
    preceded(IMPLEMENTS, comma_list(class_name))? [@doc_comment]
    LBRACE statement_class* RBRACE
  { () }
;

declare_interface:
| INTERFACE identifier type_parameters?
    preceded(EXTENDS, comma_list(class_name))? [@doc_comment]
    LBRACE statement_class* RBRACE
  { () }
;

declare_enum:
| ENUM IDENT COLON type_expr preceded(AS, type_expr)?
    LBRACE lseparated_list_trailing(SEMICOLON, enum_member) RBRACE
  { () }
;

enum_member:
| identifier EQUAL expr { () }
;

declare_type:
| TYPE identifier type_parameters? preceded(AS, type_expr)? EQUAL
    type_expr SEMICOLON
  { () }
| NEWTYPE identifier type_parameters? preceded(AS, type_expr)?  EQUAL
    type_expr SEMICOLON
  { () }
;

(* Statements *)

statement:
| LBRACE statement_inner* RBRACE { () }
| statement_if { () }
| WHILE LPAREN expr RPAREN statement
  { () }
| DO statement
  WHILE LPAREN expr RPAREN SEMICOLON
  { () }
| FOR LPAREN comma_list(expr) SEMICOLON
             comma_list(expr) SEMICOLON
             comma_list(expr) RPAREN statement
  { () }
| SWITCH LPAREN expr RPAREN switch_body { () }
| BREAK expr? SEMICOLON    { () }
| CONTINUE expr? SEMICOLON { () }
| RETURN attributes? expr? SEMICOLON   { () }
| GLOBAL comma_list(expr_variable) SEMICOLON  { () }
| STATIC comma_list(initializable_variable) SEMICOLON  { () }
| ECHO comma_list(expr) SEMICOLON    { () }
| INLINE_HTML { () }
| expr SEMICOLON { () }
| UNSET LPAREN nonempty_comma_list_trailing(expr) RPAREN SEMICOLON { () }
| FOREACH LPAREN expr ioption(AWAIT) FOREACH_AS
                 foreach_variable RPAREN statement
    { () }
| FOREACH LPAREN expr ioption(AWAIT) FOREACH_AS
                 foreach_variable DOUBLE_ARROW foreach_variable RPAREN statement
    { () }
| DECLARE LPAREN comma_list(constant_declaration) RPAREN statement { () }
| SEMICOLON (* empty statement *) { () }
| TRY
    LBRACE statement_inner* RBRACE
    try_catch*
    try_finally?
  { () }
| THROW expr SEMICOLON { () }
| GOTO IDENT SEMICOLON { () }
| IDENT COLON { () }
| AWAIT? USING LPAREN expr_pair_list RPAREN SEMICOLON { () }
| AWAIT? USING LPAREN expr_pair_list RPAREN LBRACE statement_inner* RBRACE { () }
| AWAIT? USING expr_without_parenthesis SEMICOLON { () }
| CONCURRENT LBRACE statement_inner* RBRACE { () }
;

statement_inner:
| statement         { () }
| declare_function  { () }
| declare_class     { () }
| declare_trait     { () }
| declare_interface { () }
| HALT_COMPILER LPAREN RPAREN SEMICOLON { () }
;

initializable_variable:
| VARIABLE preceded(EQUAL,expr)? [@doc_comment] { () }
;

try_catch:
| CATCH
    LPAREN separated_list(OR, name) VARIABLE RPAREN
    LBRACE statement_inner* RBRACE
  { () }
;

try_finally:
| FINALLY LBRACE statement_inner* RBRACE { () }
;

foreach_variable:
| expr                               { () }
| AND expr_assignable                { () }
| LIST LPAREN expr_pair_list RPAREN  { () }
;

switch_body:
| LBRACE SEMICOLON? switch_case* RBRACE      { () }
;

switch_case:
| CASE expr switch_case_separator statement_inner* { () }
| DEFAULT switch_case_separator statement_inner* { () }
;

switch_case_separator:
| COLON     { () }
| SEMICOLON { () }
;

statement_if:
| if_without_else %prec pre_ELSE { () }
| if_without_else ELSE statement { () }
;

if_without_else:
| IF LPAREN expr RPAREN statement { () }
| if_without_else ELSEIF LPAREN expr RPAREN statement { () }
;

parameter:
| attributes? parameter_visibility INOUT? type_expr? AND? parameter_core { () }
;

parameter_visibility:
| (* empty *) { () }
| PUBLIC      { () }
| PRIVATE     { () }
| PROTECTED   { () }
;

parameter_core:
| VARIABLE preceded(EQUAL, expr)?  { () }
| ELLIPSIS VARIABLE { () }
| ELLIPSIS { () }
;

statement_class:
| USE comma_list(class_name) SEMICOLON { () }
| USE comma_list(class_name) LBRACE trait_adaptation* RBRACE { () }
| class_variable_modifiers type_expr?
    comma_list(initializable_variable) SEMICOLON { () }
| class_member_modifier* CONST comma_list(constant_declaration) SEMICOLON { () }
| class_member_modifier* COROUTINE? FUNCTION AND? identifier [@doc_comment]
    type_parameters?
    LPAREN comma_list_trailing(parameter) RPAREN
    return_type? where_constraints? [@fn_flags] class_method_body [@fn_flags]
  { () }
| REQUIRE EXTENDS class_name SEMICOLON { () }
| REQUIRE IMPLEMENTS class_name SEMICOLON { () }
(* const type extension *)
| class_member_modifier* CONST TYPE name
    preceded(AS, type_expr)?
    preceded(EQUAL, type_expr)? SEMICOLON
  { () }
(* xhp extension *)
| XHP_ATTRIBUTE comma_list_trailing(xhp_attribute_decl) SEMICOLON { () }
| XHP_CHILDREN xhp_children_decl SEMICOLON { () }
| XHP_CATEGORY comma_list_trailing(XHP_PERCENTID_DEF) SEMICOLON { () }
;

class_method_body:
| SEMICOLON (* abstract method *) { () }
| LBRACE statement_inner* RBRACE { () }
;

%inline class_variable_modifiers:
| class_member_modifier+ { () }
| VAR { () }
;

class_member_modifier:
| PUBLIC    { () }
| PROTECTED { () }
| PRIVATE   { () }
| STATIC    { () }
| ABSTRACT  { () }
| FINAL     { () }
| ASYNC     { () }
| attributes { () }
;

trait_adaptation:
| trait_precedence SEMICOLON { () }
| trait_method_reference AS trait_alias SEMICOLON { () }
;

trait_precedence:
| name COLONCOLON identifier INSTEADOF comma_list(name) { () }
;

trait_alias:
| IDENT { () }
| reserved_non_modifiers { () }
| class_member_modifier identifier? { () }
;

trait_method_reference:
| identifier { () }
| name COLONCOLON identifier { () }
;

constant_declaration:
| type_expr identifier preceded(EQUAL,expr)? [@doc_comment] { () }
| identifier preceded(EQUAL,expr)? [@doc_comment] { () }
;

(* USE statements *)

use_modifier:
| FUNCTION { () }
| CONST    { () }
| TYPE     { () }
;

group_use_declaration:
| namespace_name ANTISLASH
  LBRACE nonempty_comma_list_trailing(unprefixed_use_declaration) RBRACE
  { () }
| ANTISLASH namespace_name ANTISLASH
  LBRACE nonempty_comma_list_trailing(unprefixed_use_declaration) RBRACE
  { () }
;

mixed_group_use_declaration:
| namespace_name ANTISLASH
  LBRACE nonempty_comma_list_trailing(inline_use_declaration) RBRACE
  { () }
| ANTISLASH namespace_name ANTISLASH
  LBRACE nonempty_comma_list_trailing(inline_use_declaration) RBRACE
  { () }
;

inline_use_declaration:
| unprefixed_use_declaration          { () }
| use_modifier unprefixed_use_declaration { () }
;

unprefixed_use_declaration:
| namespace_name          { () }
| namespace_name AS IDENT { () }
;

use_declaration:
| ioption(ANTISLASH) unprefixed_use_declaration { () }
;

(* Type expressions *)

type_expr:
| QUESTION type_expr { () }
| AT type_expr { () }
| simple_type_expr %prec pre_COLONCOLON { () }
;

simple_type_expr:
| type_name { () }
| simple_type_expr type_arguments { () }
| simple_type_expr COLONCOLON type_name { () }
| SHAPE LPAREN comma_list_trailing(shape_field) RPAREN { () }
| LPAREN nonempty_comma_list_trailing(type_expr) RPAREN { () }
| LPAREN COROUTINE? FUNCTION
    LPAREN comma_list_trailing(type_expr_parameter) RPAREN return_type
  RPAREN
  { () }
(* ad-hoc tokens for (type) *)
| INT_CAST    { () } (* (int)    *)
| DOUBLE_CAST { () } (* (double) *)
| STRING_CAST { () } (* (string) *)
| OBJECT_CAST { () } (* (object) *)
| BOOL_CAST   { () } (* (bool)   *)
| ARRAY_CAST  { () } (* (array/dict/...) *)
;

%inline rangle:
| RANGLE { () }
| SHR_PREFIX { () }
;

type_arguments:
| FORK_LANGLE comma_list_trailing(type_expr) rangle { $1 := true }
;

type_parameters:
| FORK_LANGLE comma_list_trailing(type_parameter) rangle { $1 := true }
;

type_variance:
| (* empty *) { () }
| PLUS        { () }
| MINUS       { () }
;

type_parameter:
| type_variance type_name type_constraint { () }
;

type_constraint:
| (* empty *) { () }
| type_constraint AS type_expr { () }
| type_constraint SUPER type_expr { () }
;

type_expr_parameter:
| type_expr { () }
| INOUT type_expr { () }
| ELLIPSIS  { () }
| type_expr ELLIPSIS { () }
;

return_type:
| COLON type_expr { () }
;

(* Expressions *)

expr:
| open_expr(expr) { () }
| LPAREN expr RPAREN { () }
| LPAREN attributes expr RPAREN { () }
;

expr_without_parenthesis:
| open_expr(expr_without_parenthesis) { () }
;

open_expr(left):
| LNUMBER     { () }
| DNUMBER     { () }
| LINE        { () }
| FILE        { () }
| DIR         { () }
| TRAIT_C     { () }
| METHOD_C    { () }
| FUNC_C      { () }
| NAMESPACE_C { () }
| CLASS_C     { () }
| BANG  expr  { () }
| AWAIT expr  { () }
| TILDE expr  { () }
| CLONE expr  { () }
| AT    expr  { () }
| PRINT expr  { () }
| EXIT        { () }
| SUSPEND expr { () }
| INCLUDE expr { () }
| INCLUDE_ONCE expr { () }
| REQUIRE expr { () }
| REQUIRE_ONCE expr { () }
| EMPTY LPAREN expr RPAREN { () }
| EVAL LPAREN expr RPAREN { () }
| ISSET LPAREN nonempty_comma_list_trailing(expr) RPAREN { () }
| name type_arguments? { () }
| left COLONCOLON property_name type_arguments? { () }
| class_special_name COLONCOLON property_name type_arguments? { () }
| left arrow property_name type_arguments? { () }
| expr_assignable  { () }
| DOUBLEQUOTE encaps* DOUBLEQUOTE { () }
| START_HEREDOC encaps* END_HEREDOC { () }
| PLUS expr %prec INC { () }
| MINUS expr %prec INC { () }
| left expr_argument_list { () }
| left BOOLEAN_AND expr { () }
| left IS_IDENTICAL expr { () }
| left IS_NOT_IDENTICAL expr { () }
| left SPACESHIP expr { () }
| left EQUAL attributes? expr { () }
| left EQUAL AND expr { () }
| left IS_EQUAL expr { () }
| left IS_NOT_EQUAL expr { () }
| left LANGLE expr { () }
| left RANGLE expr { () }
| left IS_SMALLER_OR_EQUAL expr { () }
| left IS_GREATER_OR_EQUAL expr { () }
| NEW expr_class_reference { () }
| structured_literal { () }
| INT_CAST    expr  { () }
| DOUBLE_CAST expr  { () }
| STRING_CAST expr  { () }
| ARRAY_CAST  expr  { () }
| OBJECT_CAST expr  { () }
| BOOL_CAST   expr  { () }
| UNSET_CAST  expr  { () }
| left BOOLEAN_OR expr  { () }
| left LOGICAL_OR expr  { () }
| left LOGICAL_AND expr { () }
| left LOGICAL_XOR expr { () }
| left OR expr     { () }
| left AND expr    { () }
| left XOR expr    { () }
| left CONCAT expr { () }
| left PLUS expr   { () }
| left MINUS expr  { () }
| left MUL expr    { () }
| left POW expr    { () }
| left DIV expr    { () }
| left MOD expr    { () }
| left SHL expr    { () }
| left PIPE_ANGLE expr { () }
| VARIABLE LONG_DOUBLE_ARROW lambda_body { () }
| FORK LPAREN comma_list_trailing(parameter) RPAREN return_type?
    LONG_DOUBLE_ARROW lambda_body { () }
| ASYNC LPAREN comma_list_trailing(parameter) RPAREN return_type?
    LONG_DOUBLE_ARROW lambda_body { () }
| ASYNC VARIABLE LONG_DOUBLE_ARROW lambda_body { () }
| ASYNC LBRACE statement_inner* RBRACE { () }
| COROUTINE LPAREN comma_list_trailing(parameter) RPAREN return_type?
    LONG_DOUBLE_ARROW lambda_body { () }
| COROUTINE VARIABLE LONG_DOUBLE_ARROW lambda_body { () }
| COROUTINE LBRACE statement_inner* RBRACE { () }
| left LBRACKET expr_pair_list RBRACKET { () }
| left INC { () }
| INC expr { () }
| left DEC { () }
| DEC expr { () }
| left PLUS_EQUAL expr { () }
| left MINUS_EQUAL expr { () }
| left MUL_EQUAL expr { () }
| left POW_EQUAL expr { () }
| left DIV_EQUAL expr { () }
| left CONCAT_EQUAL expr { () }
| left MOD_EQUAL expr { () }
| left AND_EQUAL expr { () }
| left OR_EQUAL expr  { () }
| left XOR_EQUAL expr { () }
| left SHL_EQUAL expr { () }
| left SHR_EQUAL expr { () }
| left QUESTION_QUESTION_EQUAL expr { () }
| left SHR_PREFIX RANGLE expr  { () }
| left IS type_expr { () }
| left AS type_expr { () }
| left INSTANCEOF expr_class_reference { () }
| left QUESTION AS type_expr { () }
| left QUESTION COLON expr { () }
| left QUESTION expr COLON expr { () }
| left QUESTION_QUESTION expr { () }
| LIST LPAREN expr_pair_list RPAREN EQUAL expr { () }
| xhp_html { () }
| function_expr { () }
| YIELD { () }
| YIELD expr { () }
| YIELD expr DOUBLE_ARROW expr { () }
| YIELD BREAK { () }
(*| YIELD_FROM expr { () }*)
;

function_expr:
| STATIC function_expr
| ASYNC function_expr
| COROUTINE function_expr
| FUNCTION AND? [@doc_comment]
    LPAREN comma_list_trailing(parameter) RPAREN
     return_type? lexical_vars? [@fn_flags]
    LBRACE statement_inner* RBRACE [@fn_flags]
  { () }
;

lexical_vars:
| USE LPAREN comma_list_trailing(preceded(AND?,VARIABLE)) RPAREN { () }
;

%inline lambda_body:
| expr { () }
| LBRACE statement_inner* RBRACE { () }
;

expr_argument_list:
| LPAREN comma_list_trailing(expr_argument) RPAREN { () }
| LBRACE expr_pair_list RBRACE { () }
;

expr_argument:
| expr { () }
| AND expr { () }
| INOUT expr { () }
| ELLIPSIS expr { () }
| attributes expr_argument { () }
;

expr_pair_list:
| nonempty_comma_list(expr_array_pair?) { $1 }
;

expr_array_pair:
| expr DOUBLE_ARROW expr { () }
| expr { () }
| expr DOUBLE_ARROW AND expr_assignable { () }
| AND expr_assignable { () }
| expr DOUBLE_ARROW LIST LPAREN expr_pair_list RPAREN { () }
| LIST LPAREN expr_pair_list RPAREN { () }
| ELLIPSIS expr { () }
;

property_name:
| identifier { () }
| LBRACE expr RBRACE { () }
| expr_variable { () }
;

(*expr:
| name { () }
| class_name { () }
| LBRACKET expr_pair_list RBRACKET EQUAL expr { () }
| NEW expr_class_reference expr_argument_list? { () }
| NEW CLASS expr_argument_list? class_desc { () }
| expr COALESCE expr { () }
| BACKQUOTE encaps* BACKQUOTE { () }
| function_expr { () }
| expr arrow property_name type_arguments expr_argument_list { () }
| expr arrow property_name { () }
| expr COLONCOLON property_name { () }
| expr COLONCOLON property_name type_arguments expr_argument_list { () }
| expr expr_argument_list { () }
| expr LBRACKET expr_pair_list RBRACKET { () }
| LPAREN expr RPAREN { () }
;

simple_function_call:
| name ioption(type_arguments) expr_argument_list { () }
| class_name LBRACE expr_pair_list RBRACE { () }
| class_name COLONCOLON member_name expr_argument_list { () }
;

%inline lambda_body:
| expr { () }
| LBRACE statement_inner* RBRACE { () }
| LBRACE statement_inner* RBRACE expr_argument_list { () } (* FIXME *)
;
*)

expr_class_reference:
| class_name { () }
| expr_variable { () }
| LPAREN expr RPAREN { () }
;

structured_literal:
| ARRAY ioption(type_arguments) LPAREN expr_pair_list RPAREN { () }
| SHAPE ioption(type_arguments) LPAREN expr_pair_list RPAREN { () }
| LBRACKET expr_pair_list RBRACKET { () }
| ARRAY ioption(type_arguments) LBRACKET expr_pair_list RBRACKET { () }
| CONSTANT_ENCAPSED_STRING { () }
;

expr_assignable:
| expr_variable { () }
(*| name ioption(type_arguments) expr_argument_list { () }
| class_name LBRACE expr_pair_list RBRACE { () }
| expr_assignable expr_argument_list       { () }
| LPAREN expr RPAREN expr_argument_list    { () }
| structured_literal expr_argument_list    { () }
;*)

expr_variable:
| VARIABLE { () }
| DOLLAR LBRACE expr RBRACE { () }
| DOLLAR expr_variable { () }
| DOLLARDOLLAR { () }
;

(* member_name:
| identifier ioption(type_arguments) { () }
| LBRACE expr RBRACE  { () }
| expr_variable  { () }
;

property_name:
| identifier { () }
| LBRACE expr RBRACE { () }
| expr_variable { () }
| XHP_COLONID_DEF { () }
;
*)

encaps:
| ENCAPSED_AND_WHITESPACE { () }
| encaps_var { () }
;

encaps_var:
| VARIABLE { () }
| VARIABLE LBRACKET encaps_var_offset RBRACKET { () }
| VARIABLE arrow IDENT { () }
| DOLLAR_OPEN_CURLY_BRACES expr RBRACE { () }
| DOLLAR_OPEN_CURLY_BRACES VARNAME RBRACE { () }
| DOLLAR_OPEN_CURLY_BRACES VARNAME LBRACKET expr RBRACKET RBRACE
  { () }
| CURLY_OPEN expr RBRACE { () }
;

encaps_var_offset:
| IDENT            { () }
| NUM_STRING       { () }
| MINUS NUM_STRING { () }
| VARIABLE         { () }
;

(* XHP *)

xhp_html:
| XHP_OPEN_TAG xhp_attribute* XHP_GT xhp_child* XHP_CLOSE_TAG { () }
| XHP_OPEN_TAG xhp_attribute* XHP_SLASH_GT { () }
;

xhp_child:
| XHP_TEXT           { () }
| xhp_html           { () }
| LBRACE expr RBRACE { () }
;

xhp_attribute:
| XHP_ATTR EQUAL xhp_attribute_value { () }
| LBRACE ELLIPSIS? expr RBRACE       { () }
;

xhp_attribute_value:
| DOUBLEQUOTE encaps* DOUBLEQUOTE { () }
| LBRACE expr RBRACE { () }
| XHP_ATTR           { () }
;

xhp_attribute_decl:
| XHP_COLONID_DEF { () }
| xhp_attribute_decl_type xhp_attr_name preceded(EQUAL, expr)?  XHP_REQUIRED?
  { () }
;

xhp_attribute_decl_type:
| ENUM LBRACE comma_list_trailing(expr) RBRACE
| VAR
| type_expr
  { () }
;

xhp_attr_name:
| identifier
| xhp_attr_name MINUS identifier
| xhp_attr_name COLON identifier
  { () }
;

xhp_children_decl:
| XHP_ANY
| EMPTY
| xhp_children_paren_expr
  { () }
;

xhp_children_paren_expr:
| LPAREN xhp_children_decl_exprs RPAREN
| LPAREN xhp_children_decl_exprs RPAREN MUL
| LPAREN xhp_children_decl_exprs RPAREN QUESTION
| LPAREN xhp_children_decl_exprs RPAREN PLUS
  { () }
;

xhp_children_decl_expr:
| xhp_children_paren_expr        { () }
| xhp_children_decl_tag          { () }
| xhp_children_decl_tag MUL      { () }
| xhp_children_decl_tag QUESTION { () }
| xhp_children_decl_tag PLUS     { () }
;

xhp_children_decl_exprs:
| xhp_children_decl_expr { () }
| xhp_children_decl_exprs COMMA xhp_children_decl_expr { () }
| xhp_children_decl_exprs OR    xhp_children_decl_expr { () }
;

xhp_children_decl_tag:
| XHP_ANY           { () }
| XHP_PCDATA        { () }
| XHP_COLONID_DEF   { () }
| XHP_PERCENTID_DEF { () }
| IDENT             { () }
;

(* Shapes *)

shape_field:
| QUESTION? expr DOUBLE_ARROW type_expr { () }
| ELLIPSIS { () }
;

%inline arrow:
| ARROW          { () }
| QUESTION_ARROW { () }
;

(* Using keywords as identifiers *)

xhp_reserved:
| XHP_ATTRIBUTE { "attribute" }
| XHP_CATEGORY  { "category" }
| XHP_CHILDREN  { "children" }
| XHP_ANY       { "any" }
| XHP_PCDATA    { "pcdata" }
;

reserved_non_modifiers:
| xhp_reserved { $1 }
| INCLUDE      { "include" }
| INCLUDE_ONCE { "include_once" }
| EVAL         { "eval" }
| REQUIRE      { "require" }
| REQUIRE_ONCE { "require_once" }
| LOGICAL_OR   { "or" }
| LOGICAL_XOR  { "xor" }
| LOGICAL_AND  { "and" }
| INSTANCEOF   { "instanceof" }
| INOUT        { "inout" }
| IS           { "is" }
| NEW          { "new" }
| CLONE        { "clone" }
| EXIT         { "exit" }
| IF           { "if" }
| ELSEIF       { "elseif" }
| ELSE         { "else" }
| ENDIF        { "endif" }
| ECHO         { "echo" }
| DO           { "do" }
| WHILE        { "while" }
| FOR          { "for" }
| FOREACH      { "foreach" }
| DECLARE      { "declare" }
| AS           { "as" }
| TRY          { "try" }
| CATCH        { "catch" }
| FINALLY      { "finally" }
| THROW        { "throw" }
| USE          { "use" }
| INSTEADOF    { "insteadof" }
| GLOBAL       { "global" }
| VAR          { "var" }
| UNSET        { "unset" }
| ISSET        { "isset" }
| EMPTY        { "empty" }
| CONTINUE     { "continue" }
| GOTO         { "goto" }
| FUNCTION     { "function" }
| CONST        { "const" }
| RETURN       { "return" }
| PRINT        { "print" }
| YIELD        { "yield" }
| LIST         { "list" }
| SWITCH       { "switch" }
| CASE         { "case" }
| DEFAULT      { "default" }
| BREAK        { "break" }
| ARRAY        { "array" }
| CALLABLE     { "callable" }
| EXTENDS      { "extends" }
| IMPLEMENTS   { "implements" }
| NAMESPACE    { "namespace" }
| TRAIT        { "trait" }
| INTERFACE    { "interface" }
| CLASS        { "class" }
| TYPE         { "type" }
| ENUM         { "enum" }
| LINE         { "__LINE__" }
| FILE         { "__FILE__" }
| DIR          { "__DIR__" }
| CLASS_C      { "__CLASS__" }
| TRAIT_C      { "__TRAIT__" }
| METHOD_C     { "__METHOD__" }
| FUNC_C       { "__FUNCTION__" }
| NAMESPACE_C  { "__NAMESPACE__" }
| PARENT       { "parent" }
| SELF         { "self" }
| WHERE        { "where" }
| SHAPE        { "shape" }
| USING        { "using" }
| COROUTINE    { "coroutine" }
| SUSPEND      { "suspend" }
| NEWTYPE      { "newtype" }
;

semi_reserved:
| reserved_non_modifiers { $1 }
| STATIC    { "STATIC" }
| ABSTRACT  { "ABSTRACT" }
| FINAL     { "FINAL" }
| PRIVATE   { "PRIVATE" }
| PROTECTED { "PROTECTED" }
| PUBLIC    { "PUBLIC" }
| ASYNC     { "async" }
| SUPER     { "super" }
;

(* To silence some warnings *)
dummy:
| (* Tokens that are expected to be unused *)
  COMMENT DOC_COMMENT NEWLINE SPACES UNKNOWN
  OPEN_TAG OPEN_TAG_WITH_ECHO
  CLOSE_TAG CLOSE_TAG_OF_ECHO
  { () }
| (* Tokens that are expected to be used in final grammar *)
  AND_EQUAL BACKQUOTE BANG BOOLEAN_AND BOOLEAN_OR COALESCE CONCAT CONCAT_EQUAL
  CURLY_OPEN DEC DIV DIV_EQUAL DNUMBER DOLLAR_OPEN_CURLY_BRACES DOUBLEQUOTE
  ENCAPSED_AND_WHITESPACE END_HEREDOC FORK INC IS_EQUAL IS_GREATER_OR_EQUAL
  IS_IDENTICAL IS_NOT_EQUAL IS_SMALLER_OR_EQUAL LANGLE LONG_DOUBLE_ARROW
  MINUS_EQUAL MOD MOD_EQUAL MUL MUL_EQUAL NUM_STRING OR_EQUAL PIPE_ANGLE
  PLUS_EQUAL POW POW_EQUAL QUESTION_QUESTION QUESTION_QUESTION_EQUAL SHL_EQUAL
  SHR_EQUAL SPACESHIP START_HEREDOC TILDE UNSET_CAST VARNAME XHP_ATTR
  XHP_CLOSE_TAG XHP_GT XHP_OPEN_TAG XHP_PCDATA XHP_PERCENTID_DEF XHP_REQUIRED
  XHP_SLASH_GT XHP_TEXT XOR XOR_EQUAL YIELD_FROM
  { () }
;

(* Generic definitions *)

%inline lnonempty_list(X):
| X llist_aux(X) { $1 :: List.rev $2 }
;

%inline llist(X):
| llist_aux(X) { List.rev $1 }
;

llist_aux(X):
| (* empty *) { [] }
| llist_aux(X) X { $2 :: $1 }
;

%inline lseparated_list(sep, X):
| (* empty *) { [] }
| lseparated_nonempty_list(sep, X) { $1 }
;

%inline lseparated_list_trailing(sep, X):
| (* empty *) { [] }
| lseparated_nonempty_list(sep, X) sep? { $1 }
;

%inline lseparated_nonempty_list(sep, X):
| lseparated_nonempty_list_aux(sep, X) { List.rev $1 };
;

lseparated_nonempty_list_aux(sep, X):
| X { [$1] }
| lseparated_nonempty_list_aux(sep, X) sep X { $3 :: $1 }
;

%inline comma_list(X):
| lseparated_list(COMMA, X) { $1 }
;

%inline comma_list_trailing(X):
| lseparated_list_trailing(COMMA, X) { $1 }
;

%inline nonempty_comma_list(X):
| lseparated_nonempty_list(COMMA, X) { $1 }
;

%inline nonempty_comma_list_trailing(X):
| lseparated_nonempty_list(COMMA, X) COMMA? { $1 }
;


(* Some interesting conflicts:
 *
 * T::K < U > x
 * is ((T::K) < U) > x
 * or T::K<U> x;
 *
 * bla ? ($x) : foo
 * is a ternary
 * or the beginning of lambda binding $x and returning a value of type foo
 *
 * foreach (<expr> as $x
 * does as belong to foreach
 * or as is a sub-typing constraint on <expr> and ... ?
 *
 * <expr> as <type>->foo
 *   should be parsed as (<expr> as <type>)->foo
 *
 * what about:
 *   <expr> as <type>::bar ?
 * both:   <expr> as (<type>::bar)
 * and:    (<expr> as <type>)::bar
 * make sense. The former is more intuitive taken alone, but it is the opposite
 * interpretation of the first one.
 *)
