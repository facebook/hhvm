%{
#include <compiler/parser/parser.h>

using namespace HPHP;

#define YYSTYPE Token
//#define YYSTYPE_IS_TRIVIAL 1
#define YLMM_PARSER_CLASS Parser
#define YLMM_LEX_STATIC
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#include <util/ylmm/yaccmm.hh>

#define _p _parser
#define BEXP(e...) _parser->onBinaryOpExp(e);
#define UEXP(e...) _parser->onUnaryOpExp(e);
%}

%expect 2

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
%right '~' T_INC T_DEC T_INT_CAST T_DOUBLE_CAST T_STRING_CAST T_ARRAY_CAST T_OBJECT_CAST T_BOOL_CAST T_UNSET_CAST '@'
%right '['
%left T_HPHP_NOTE

%nonassoc T_NEW T_CLONE
%token T_EXIT
%token T_IF
%left T_ELSEIF
%left T_ELSE
%left T_ENDIF
%token T_LNUMBER
%token T_DNUMBER
%token T_STRING
%token T_STRING_VARNAME
%token T_VARIABLE
%token T_NUM_STRING
%token T_INLINE_HTML
%token T_CHARACTER
%token T_BAD_CHARACTER
%token T_ENCAPSED_AND_WHITESPACE
%token T_CONSTANT_ENCAPSED_STRING
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
%token T_CLOSE_TAG
%token T_WHITESPACE
%token T_START_HEREDOC
%token T_END_HEREDOC
%token T_DOLLAR_OPEN_CURLY_BRACES
%token T_CURLY_OPEN
%token T_PAAMAYIM_NEKUDOTAYIM

%token T_HPHP_NOTE
%token T_HPHP_DECLARE

%%

start:
    top_statement_list                 { _p->saveParseTree(&$$);}
;

top_statement_list:
    top_statement_list
    top_statement                      { _p->addStatement(&$$,&$1,&$2);}
  |                                    { $$.reset();}
;
top_statement:
    statement                          { $$ = $1;}
  | function_declaration_statement     { $$ = $1;}
  | class_declaration_statement        { $$ = $1;}
  | T_HPHP_NOTE function_declaration_statement
                                       { _p->onHphpNoteStatement(&$$,&$1,&$2);}
  | T_HPHP_NOTE class_declaration_statement
                                       { _p->onHphpNoteStatement(&$$,&$1,&$2);}
  | T_HALT_COMPILER '(' ')' ';'        { $$.reset();}
  | T_HPHP_DECLARE hphp_declare_list ';'
                                       { $$.reset(); }
;
hphp_declare_list:
    hphp_declare_list ',' hphp_declare
                                       { $$.reset();}
  | hphp_declare                     { $$.reset();}
;
hphp_declare:
    '@' T_STRING                       { _p->addHphpSuppressError(&$2);}
  | T_STRING                           { _p->addHphpDeclare(&$1);}
;
inner_statement_list:
    inner_statement_list
    inner_statement                    { _p->addStatement(&$$,&$1,&$2);}
  |                                    { $$.reset();}
;
inner_statement:
    statement                          { $$ = $1;}
  | function_declaration_statement     { $$ = $1;}
  | class_declaration_statement        { $$ = $1;}
  | T_HPHP_NOTE function_declaration_statement
                                       { _p->onHphpNoteStatement(&$$,&$1,&$2);}
  | T_HPHP_NOTE class_declaration_statement
                                       { _p->onHphpNoteStatement(&$$,&$1,&$2);}
;
statement:
   expr ';'                            { _p->onExpStatement(&$$, &$1);}
 | statement_without_expr              { $$ = $1;}
 | T_HPHP_NOTE statement_without_expr  { _p->onHphpNoteStatement(&$$,&$1,&$2);}

statement_without_expr:
    '{' inner_statement_list '}'       { _p->onBlock(&$$, &$2);}

  | T_IF '(' expr ')'
    statement
    elseif_list
    else_single                        { _p->onIf(&$$,&$3,&$5,&$6,&$7);}

  | T_IF '(' expr ')' ':'
    inner_statement_list
    new_elseif_list
    new_else_single
    T_ENDIF ';'                        { _p->onIf(&$$,&$3,&$6,&$7,&$8);}

  | T_WHILE '(' expr ')'
    while_statement                    { _p->onWhile(&$$,&$3,&$5);}

  | T_DO statement
    T_WHILE '(' expr ')' ';'           { _p->onDo(&$$,&$2,&$5);}

  | T_FOR '(' for_expr ';'
    for_expr ';' for_expr ')'
    for_statement                      { _p->onFor(&$$,&$3,&$5,&$7,&$9);}

  | T_SWITCH '(' expr ')'
    switch_case_list                   { _p->onSwitch(&$$,&$3,&$5);}

  | T_BREAK ';'                        { _p->onBreak(&$$, NULL);}
  | T_BREAK expr ';'                   { _p->onBreak(&$$, &$2);}

  | T_CONTINUE ';'                     { _p->onContinue(&$$, NULL);}
  | T_CONTINUE expr ';'                { _p->onContinue(&$$, &$2);}

  | T_RETURN ';'                       { _p->onReturn(&$$, NULL);}
  | T_RETURN expr_without_variable ';' { _p->onReturn(&$$, &$2);}
  | T_RETURN variable ';'              { _p->onReturn(&$$, &$2);}

  | T_GLOBAL global_var_list ';'       { _p->onGlobal(&$$, &$2);}
  | T_STATIC static_var_list ';'       { _p->onStatic(&$$, &$2);}
  | T_ECHO echo_expr_list ';'          { _p->onEcho(&$$, &$2, 0);}
  | T_UNSET '(' unset_variables ')'
    ';'                                { _p->onUnset(&$$, &$3);}
  | ';'                                { $$.reset();}

  | T_INLINE_HTML                      { _p->onEcho(&$$, &$1, 1);}

  | T_FOREACH '(' variable
    T_AS foreach_variable
    foreach_optional_arg ')'
    foreach_statement                  { _p->onForEach(&$$,&$3,&$5,&$6,&$8);}

  | T_FOREACH '(' expr_without_variable
    T_AS variable
    foreach_optional_arg ')'
    foreach_statement                  { _p->onForEach(&$$,&$3,&$5,&$6,&$8);}

  | T_DECLARE '(' declare_list ')'
    declare_statement                  { _p->onBlock(&$$, &$5);}

  | T_TRY '{' inner_statement_list '}'
    T_CATCH '('
    fully_qualified_class_name
    T_VARIABLE ')'
    '{' inner_statement_list '}'
    additional_catches                 { _p->onTry(&$$,&$3,&$7,&$8,&$11,&$13);}

  | T_THROW expr ';'                   { _p->onThrow(&$$, &$2);}
;

additional_catches:
    non_empty_additional_catches       { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_additional_catches:
    additional_catch                   { _p->addStatement(&$$, NULL, &$1);}
  | non_empty_additional_catches
    additional_catch                   { _p->addStatement(&$$, &$1, &$2);}
;
additional_catch:
    T_CATCH '('
    fully_qualified_class_name
    T_VARIABLE ')'
    '{' inner_statement_list '}'       { _p->onCatch(&$$, &$3, &$4, &$7);}
;

unset_variables:
    unset_variable                     { _p->onExprListElem(&$$, NULL, &$1);}
  | unset_variables ',' unset_variable { _p->onExprListElem(&$$, &$1, &$3);}
;
unset_variable:
    variable                           { $$ = $1;}
;

is_reference:
    '&'                                { $$ = 1;}
  |                                    { $$.reset();}
;

function_declaration_statement:
    T_FUNCTION is_reference T_STRING   { _p->onFunctionStart();}
    '(' parameter_list ')'
    '{' inner_statement_list '}'       { _p->onFunction(&$$,&$2,&$3,&$6,&$9);}
;

class_declaration_statement:
    class_entry_type T_STRING          { _p->onClassStart();}
    extends_from
    implements_list '{'
    class_statement_list '}'           { _p->onClass(&$$,&$1,&$2,&$4,&$5,&$7);}

  | T_INTERFACE T_STRING               { _p->onClassStart();}
    interface_extends_list '{'
    class_statement_list '}'           { _p->onInterface(&$$,&$2,&$4,&$6);}
;
class_entry_type:
    T_CLASS                            { $$ = T_CLASS;}
  | T_ABSTRACT T_CLASS                 { $$ = T_ABSTRACT;}
  | T_FINAL T_CLASS                    { $$ = T_FINAL;}
;
extends_from:
    T_EXTENDS
    fully_qualified_class_name         { $$ = $2;}
  |                                    { $$.reset();}
;
implements_list:
    T_IMPLEMENTS interface_list        { $$ = $2;}
  |                                    { $$.reset();}
;
interface_extends_list:
    T_EXTENDS interface_list           { $$ = $2;}
  |                                    { $$.reset();}
;
interface_list:
    fully_qualified_class_name         { _p->onInterfaceName(&$$, NULL, &$1);}
  | interface_list ','
    fully_qualified_class_name         { _p->onInterfaceName(&$$, &$1, &$3);}
;

foreach_optional_arg:
    T_DOUBLE_ARROW foreach_variable    { $$ = $2;}
  |                                    { $$.reset();}
;
foreach_variable:
    variable                           { $$ = $1;}
  | '&' variable                       { $$ = $2; $$ = 1;}
;

for_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDFOR ';'                       { $$ = $2;}
;
foreach_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDFOREACH ';'                   { $$ = $2;}
;
while_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDWHILE ';'                     { $$ = $2;}
;
declare_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDDECLARE ';'                   { $$ = $2;}
;

declare_list:
    T_STRING '=' static_scalar
  | declare_list ','
    T_STRING '=' static_scalar
;

switch_case_list:
    '{' case_list '}'                  { $$ = $2;}
  | '{' ';' case_list '}'              { $$ = $3;}
  | ':' case_list T_ENDSWITCH ';'      { $$ = $2;}
  | ':' ';' case_list T_ENDSWITCH ';'  { $$ = $3;}
;
case_list:
    case_list T_CASE expr
    case_separator
    inner_statement_list               { _p->onCase(&$$,&$1,&$3,&$5);}
  | case_list T_DEFAULT case_separator
    inner_statement_list               { _p->onCase(&$$,&$1,NULL,&$4);}
  |                                    { $$.reset();}
;
case_separator:
    ':'                                { $$.reset();}
  | ';'                                { $$.reset();}
;

elseif_list:
    elseif_list T_ELSEIF '(' expr ')'
    statement                          { _p->onElseIf(&$$,&$1,&$4,&$6);}
  |                                    { $$.reset();}
;
new_elseif_list:
    new_elseif_list T_ELSEIF
    '(' expr ')' ':'
    inner_statement_list               { _p->onElseIf(&$$,&$1,&$4,&$7);}
  |                                    { $$.reset();}
;
else_single:
    T_ELSE statement                   { $$ = $2;}
  |                                    { $$.reset();}
;
new_else_single:
    T_ELSE ':' inner_statement_list    { $$ = $3;}
  |                                    { $$.reset();}
;

parameter_list:
    non_empty_parameter_list           { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_parameter_list:
    optional_class_type T_VARIABLE     { _p->onParam(&$$,NULL,&$1,&$2,0,NULL);}
  | optional_class_type '&' T_VARIABLE { _p->onParam(&$$,NULL,&$1,&$3,1,NULL);}
  | optional_class_type '&' T_VARIABLE
    '=' static_scalar                  { _p->onParam(&$$,NULL,&$1,&$3,1,&$5);}
  | optional_class_type T_VARIABLE
    '=' static_scalar                  { _p->onParam(&$$,NULL,&$1,&$2,0,&$4);}
  | non_empty_parameter_list ','
    optional_class_type T_VARIABLE     { _p->onParam(&$$,&$1,&$3,&$4,0,NULL);}
  | non_empty_parameter_list ','
    optional_class_type '&' T_VARIABLE { _p->onParam(&$$,&$1,&$3,&$5,1,NULL);}
  | non_empty_parameter_list ','
    optional_class_type '&' T_VARIABLE
    '=' static_scalar                  { _p->onParam(&$$,&$1,&$3,&$5,1,&$7);}
  | non_empty_parameter_list ','
    optional_class_type T_VARIABLE
    '=' static_scalar                  { _p->onParam(&$$,&$1,&$3,&$4,0,&$6);}
;
optional_class_type:
    T_STRING                           { $$ = $1;}
  | T_ARRAY                            { $$.setText("array");}
  |                                    { $$.reset();}
;

function_call_parameter_list:
    non_empty_fcall_parameter_list     { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_fcall_parameter_list:
    expr_without_variable              { _p->onCallParam(&$$,NULL,&$1,0);}
  | variable                           { _p->onCallParam(&$$,NULL,&$1,0);}
  | '&' w_variable                     { _p->onCallParam(&$$,NULL,&$2,1);}
  | non_empty_fcall_parameter_list ','
    expr_without_variable              { _p->onCallParam(&$$,&$1,&$3,0);}
  | non_empty_fcall_parameter_list ','
    variable                           { _p->onCallParam(&$$,&$1,&$3,0);}
  | non_empty_fcall_parameter_list ','
    '&' w_variable                     { _p->onCallParam(&$$,&$1,&$4,1);}
;

global_var_list:
    global_var_list ',' global_var     { _p->onGlobalVar(&$$, &$1, &$3);}
  | global_var                         { _p->onGlobalVar(&$$, NULL, &$1);}
;
global_var:
    T_VARIABLE                         { $$ = $1;}
  | '$' r_variable                     { $$ = $2; $$ = 1;}
  | '$' '{' expr '}'                   { $$ = $3; $$ = 1;}
;

static_var_list:
    static_var_list ',' T_VARIABLE     { _p->onVariable(&$$,&$1,&$3,0);}
  | static_var_list ',' T_VARIABLE
    '=' static_scalar                  { _p->onVariable(&$$,&$1,&$3,&$5);}
  | T_VARIABLE                         { _p->onVariable(&$$,0,&$1,0);}
  | T_VARIABLE '=' static_scalar       { _p->onVariable(&$$,0,&$1,&$3);}
;

class_statement_list:
    class_statement_list
    class_statement                    { _p->addStatement(&$$, &$1, &$2);}
  |                                    { $$.reset();}
;
class_statement:
    variable_modifiers
    class_variable_declaration ';'     { _p->onClassVariable(&$$,&$1,&$2);}
  | class_constant_declaration ';'     { _p->onClassVariable(&$$,NULL,&$1);}
  | method_modifiers T_FUNCTION
    is_reference T_STRING '('          { _p->onFunctionStart();}
    parameter_list ')' method_body     { _p->onMethod(&$$,&$1,&$3,&$4,&$7,
                                                      &$9);}
  | T_HPHP_NOTE
    method_modifiers T_FUNCTION
    is_reference T_STRING '('          { _p->onFunctionStart();}
    parameter_list ')' method_body     { _p->onMethod(&$$,&$2,&$4,&$5,&$8,
                                                      &$10);
                                         _p->onHphpNoteStatement(&$$,&$1,&$$);}
;
method_body:
    ';'                                { $$.reset();}
  | '{' inner_statement_list '}'       { _p->finishStatement(&$$, &$2);
                                         $$ = 1;}
;
variable_modifiers:
    non_empty_member_modifiers         { $$ = $1;}
  | T_VAR                              { $$.reset();}
;
method_modifiers:
    non_empty_member_modifiers         { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_member_modifiers:
    member_modifier                    { _p->onMemberModifier(&$$,NULL,&$1);}
  | non_empty_member_modifiers
    member_modifier                    { _p->onMemberModifier(&$$,&$1,&$2);}
;
member_modifier:
    T_PUBLIC                           { $$ = T_PUBLIC;}
  | T_PROTECTED                        { $$ = T_PROTECTED;}
  | T_PRIVATE                          { $$ = T_PRIVATE;}
  | T_STATIC                           { $$ = T_STATIC;}
  | T_ABSTRACT                         { $$ = T_ABSTRACT;}
  | T_FINAL                            { $$ = T_FINAL;}
;
class_variable_declaration:
    class_variable_declaration ','
    T_VARIABLE                         { _p->onVariable(&$$,&$1,&$3,0);}
  | class_variable_declaration ','
    T_VARIABLE '=' static_scalar       { _p->onVariable(&$$,&$1,&$3,&$5);}
  | T_VARIABLE                         { _p->onVariable(&$$,0,&$1,0);}
  | T_VARIABLE '=' static_scalar       { _p->onVariable(&$$,0,&$1,&$3);}
;
class_constant_declaration:
    class_constant_declaration ','
    T_STRING '=' static_scalar         { _p->onVariable(&$$,&$1,&$3,&$5,1);}
  | T_CONST T_STRING '=' static_scalar { _p->onVariable(&$$,0,&$2,&$4,1);}
;

echo_expr_list:
    echo_expr_list ',' expr            { _p->onExprListElem(&$$, &$1, &$3);}
  | expr                               { _p->onExprListElem(&$$, NULL, &$1);}
;

for_expr:
    non_empty_for_expr                 { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_for_expr:
    non_empty_for_expr ',' expr        { _p->onExprListElem(&$$, &$1, &$3);}
  | expr                               { _p->onExprListElem(&$$, NULL, &$1);}
;

expr_without_variable:
    T_LIST '(' assignment_list ')'
    '=' expr                           { _p->onListAssignment(&$$, &$3, &$6);}
  | variable '=' expr                  { _p->onAssign(&$$, &$1, &$3, 0);}
  | variable '=' '&' variable          { _p->onAssign(&$$, &$1, &$4, 1);}
  | variable '=' '&' T_NEW
    class_name_reference
    ctor_arguments                     { _p->onAssignNew(&$$,&$1,&$5,&$6);}
  | T_NEW class_name_reference
    ctor_arguments                     { _p->onNewObject(&$$, &$2, &$3);}
  | T_CLONE expr                       { UEXP(&$$,&$2,T_CLONE,1);}
  | variable T_PLUS_EQUAL expr         { BEXP(&$$,&$1,&$3,T_PLUS_EQUAL);}
  | variable T_MINUS_EQUAL expr        { BEXP(&$$,&$1,&$3,T_MINUS_EQUAL);}
  | variable T_MUL_EQUAL expr          { BEXP(&$$,&$1,&$3,T_MUL_EQUAL);}
  | variable T_DIV_EQUAL expr          { BEXP(&$$,&$1,&$3,T_DIV_EQUAL);}
  | variable T_CONCAT_EQUAL expr       { BEXP(&$$,&$1,&$3,T_CONCAT_EQUAL);}
  | variable T_MOD_EQUAL expr          { BEXP(&$$,&$1,&$3,T_MOD_EQUAL);}
  | variable T_AND_EQUAL expr          { BEXP(&$$,&$1,&$3,T_AND_EQUAL);}
  | variable T_OR_EQUAL expr           { BEXP(&$$,&$1,&$3,T_OR_EQUAL);}
  | variable T_XOR_EQUAL expr          { BEXP(&$$,&$1,&$3,T_XOR_EQUAL);}
  | variable T_SL_EQUAL expr           { BEXP(&$$,&$1,&$3,T_SL_EQUAL);}
  | variable T_SR_EQUAL expr           { BEXP(&$$,&$1,&$3,T_SR_EQUAL);}
  | rw_variable T_INC                  { UEXP(&$$,&$1,T_INC,0);}
  | T_INC rw_variable                  { UEXP(&$$,&$2,T_INC,1);}
  | rw_variable T_DEC                  { UEXP(&$$,&$1,T_DEC,0);}
  | T_DEC rw_variable                  { UEXP(&$$,&$2,T_DEC,1);}
  | expr T_BOOLEAN_OR expr             { BEXP(&$$,&$1,&$3,T_BOOLEAN_OR);}
  | expr T_BOOLEAN_AND expr            { BEXP(&$$,&$1,&$3,T_BOOLEAN_AND);}
  | expr T_LOGICAL_OR expr             { BEXP(&$$,&$1,&$3,T_LOGICAL_OR);}
  | expr T_LOGICAL_AND expr            { BEXP(&$$,&$1,&$3,T_LOGICAL_AND);}
  | expr T_LOGICAL_XOR expr            { BEXP(&$$,&$1,&$3,T_LOGICAL_XOR);}
  | expr '|' expr                      { BEXP(&$$,&$1,&$3,'|');}
  | expr '&' expr                      { BEXP(&$$,&$1,&$3,'&');}
  | expr '^' expr                      { BEXP(&$$,&$1,&$3,'^');}
  | expr '.' expr                      { BEXP(&$$,&$1,&$3,'.');}
  | expr '+' expr                      { BEXP(&$$,&$1,&$3,'+');}
  | expr '-' expr                      { BEXP(&$$,&$1,&$3,'-');}
  | expr '*' expr                      { BEXP(&$$,&$1,&$3,'*');}
  | expr '/' expr                      { BEXP(&$$,&$1,&$3,'/');}
  | expr '%' expr                      { BEXP(&$$,&$1,&$3,'%');}
  | expr T_SL expr                     { BEXP(&$$,&$1,&$3,T_SL);}
  | expr T_SR expr                     { BEXP(&$$,&$1,&$3,T_SR);}
  | '+' expr %prec T_INC               { UEXP(&$$,&$2,'+',1);}
  | '-' expr %prec T_INC               { UEXP(&$$,&$2,'-',1);}
  | '!' expr                           { UEXP(&$$,&$2,'!',1);}
  | '~' expr                           { UEXP(&$$,&$2,'~',1);}
  | expr T_IS_IDENTICAL expr           { BEXP(&$$,&$1,&$3,T_IS_IDENTICAL);}
  | expr T_IS_NOT_IDENTICAL expr       { BEXP(&$$,&$1,&$3,T_IS_NOT_IDENTICAL);}
  | expr T_IS_EQUAL expr               { BEXP(&$$,&$1,&$3,T_IS_EQUAL);}
  | expr T_IS_NOT_EQUAL expr           { BEXP(&$$,&$1,&$3,T_IS_NOT_EQUAL);}
  | expr '<' expr                      { BEXP(&$$,&$1,&$3,'<');}
  | expr T_IS_SMALLER_OR_EQUAL expr    { BEXP(&$$,&$1,&$3,
                                              T_IS_SMALLER_OR_EQUAL);}
  | expr '>' expr                      { BEXP(&$$,&$1,&$3,'>');}
  | expr T_IS_GREATER_OR_EQUAL expr    { BEXP(&$$,&$1,&$3,
                                              T_IS_GREATER_OR_EQUAL);}
  | expr T_INSTANCEOF
    class_name_reference               { BEXP(&$$,&$1,&$3,T_INSTANCEOF);}
  | '(' expr ')'                       { UEXP(&$$,&$2,'(',1);}
  | expr '?' expr ':' expr             { _p->onQOp(&$$, &$1, &$3, &$5);}
  | internal_functions                 { $$ = $1;}
  | T_INT_CAST expr                    { UEXP(&$$,&$2,T_INT_CAST,1);}
  | T_DOUBLE_CAST expr                 { UEXP(&$$,&$2,T_DOUBLE_CAST,1);}
  | T_STRING_CAST expr                 { UEXP(&$$,&$2,T_STRING_CAST,1);}
  | T_ARRAY_CAST expr                  { UEXP(&$$,&$2,T_ARRAY_CAST,1);}
  | T_OBJECT_CAST expr                 { UEXP(&$$,&$2,T_OBJECT_CAST,1);}
  | T_BOOL_CAST expr                   { UEXP(&$$,&$2,T_BOOL_CAST,1);}
  | T_UNSET_CAST expr                  { UEXP(&$$,&$2,T_UNSET_CAST,1);}
  | T_EXIT exit_expr                   { UEXP(&$$,&$2,T_EXIT,1);}
  | '@' expr                           { UEXP(&$$,&$2,'@',1);}
  | scalar                             { $$ = $1;}
  | T_ARRAY '(' array_pair_list ')'    { UEXP(&$$,&$3,T_ARRAY,1);}
  | '`' encaps_list '`'                { _p->onEncapsList(&$$,'`',&$2);}
  | T_PRINT expr                       { UEXP(&$$,&$2,T_PRINT,1);}
  | T_HPHP_NOTE expr                   { _p->onHphpNoteExpr(&$$,&$1,&$2);}
;

function_call:
    T_STRING '('
    function_call_parameter_list ')'   { _p->onCall(&$$,0,&$1,&$3,NULL);}
  | variable_without_objects '('
    function_call_parameter_list ')'   { _p->onCall(&$$,1,&$1,&$3,NULL);}
  | static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    T_STRING '('
    function_call_parameter_list ')'   { _p->onCall(&$$,0,&$3,&$5,&$1);}
  | static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    variable_without_objects '('
    function_call_parameter_list ')'   { _p->onCall(&$$,1,&$3,&$5,&$1);}
;
static_class_name:
    T_STRING                           { _p->onScalar(&$$, T_STRING, &$1);}
  | T_STATIC                           { _p->onScalar(&$$, T_STRING, &$1);}
  | reference_variable                 { $$ = $1;}
;
fully_qualified_class_name:
    T_STRING                           { $$ = $1;}
;
class_name_reference:
    T_STRING                           { _p->onScalar(&$$, T_STRING, &$1);}
  | dynamic_class_name_reference       { $$ = $1;}
;
dynamic_class_name_reference:
    base_variable                      { _p->pushObject(&$1);}
    T_OBJECT_OPERATOR object_property
    object_properties                  { _p->popObject(&$$);}
  | base_variable                      { _p->pushObject(&$1);
                                         _p->popObject(&$$);}
;
object_properties:
    object_properties
    dynamic_class_name_variable_prop   { }
  |                                    { }
;
dynamic_class_name_variable_prop:
    T_OBJECT_OPERATOR object_property  { }
;

exit_expr:
    '(' ')'                            { $$.reset();}
  | '(' expr ')'                       { $$ = $2;}
  |                                    { $$.reset();}
;

ctor_arguments:
    '('
    function_call_parameter_list ')'   { $$ = $2;}
  |                                    { $$.reset();}
;

common_scalar:
    T_LNUMBER                          { _p->onScalar(&$$, T_LNUMBER, &$1);}
  | T_DNUMBER                          { _p->onScalar(&$$, T_DNUMBER, &$1);}
  | T_CONSTANT_ENCAPSED_STRING         { _p->onScalar(&$$,
                                         T_CONSTANT_ENCAPSED_STRING, &$1);}
  | T_LINE                             { _p->onScalar(&$$, T_LINE, &$1);}
  | T_FILE                             { UEXP(&$$,&$1,T_FILE,1);}
  | T_CLASS_C                          { _p->onScalar(&$$, T_CLASS_C, &$1);}
  | T_METHOD_C                         { _p->onScalar(&$$, T_METHOD_C, &$1);}
  | T_FUNC_C                           { _p->onScalar(&$$, T_FUNC_C, &$1);}
;
static_scalar:
    common_scalar                      { $$ = $1;}
  | T_STRING                           { _p->onConstant(&$$, &$1);}
  | '+' static_scalar                  { UEXP(&$$,&$2,'+',1);}
  | '-' static_scalar                  { UEXP(&$$,&$2,'-',1);}
  | T_ARRAY '('
    static_array_pair_list ')'         { UEXP(&$$,&$3,T_ARRAY,1);}
  | static_class_constant              { $$ = $1;}
;
static_class_constant:
    T_STRING T_PAAMAYIM_NEKUDOTAYIM
    T_STRING                           { _p->onClassConst(&$$, &$1, &$3);}
;
scalar:
    T_STRING                           { _p->onConstant(&$$, &$1);}
  | T_STRING_VARNAME                   { _p->onConstant(&$$, &$1);}
  | class_constant                     { $$ = $1;}
  | common_scalar                      { $$ = $1;}
  | '"' encaps_list '"'                { _p->onEncapsList(&$$,'"',&$2);}
  | '\'' encaps_list '\''              { _p->onEncapsList(&$$,'\'',&$2);}
  | T_START_HEREDOC encaps_list
    T_END_HEREDOC                      { _p->onEncapsList(&$$,T_START_HEREDOC,
                                                          &$2);}
;
static_array_pair_list:
    non_empty_static_array_pair_list
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
possible_comma:
    ','                                { $$.reset();}
  |                                    { $$.reset();}
;
non_empty_static_array_pair_list:
    non_empty_static_array_pair_list
    ',' static_scalar T_DOUBLE_ARROW
    static_scalar                      { _p->onArrayPair(&$$,&$1,&$3,&$5,0);}
  | non_empty_static_array_pair_list
    ',' static_scalar                  { _p->onArrayPair(&$$,&$1,  0,&$3,0);}
  | static_scalar T_DOUBLE_ARROW
    static_scalar                      { _p->onArrayPair(&$$,  0,&$1,&$3,0);}
  | static_scalar                      { _p->onArrayPair(&$$,  0,  0,&$1,0);}
;

expr:
    r_variable                         { $$ = $1;}
  | expr_without_variable              { $$ = $1;}
;
r_variable:
    variable                           { $$ = $1;}
;
w_variable:
    variable                           { $$ = $1;}
;
rw_variable:
    variable                           { $$ = $1;}
;
variable:
    base_variable_with_function_calls  { _p->pushObject(&$1);}
    T_OBJECT_OPERATOR object_property
    method_or_not                      { _p->appendMethodParams(&$5);}
    variable_properties                { _p->popObject(&$$);}
  | base_variable_with_function_calls  { _p->pushObject(&$1);
                                         _p->popObject(&$$);}
;
variable_properties:
    variable_properties
    variable_property                  { }
  |                                    { }
;
variable_property:
    T_OBJECT_OPERATOR object_property
    method_or_not                      { _p->appendMethodParams(&$3);}
;
method_or_not:
    '('
    function_call_parameter_list ')'   { $$ = $2; $$.num = 1;}
  |                                    { $$.reset();}
;

variable_without_objects:
    reference_variable                 { $$ = $1;}
  | simple_indirect_reference
    reference_variable                 { _p->onIndirectRef(&$$,&$1,&$2);}
;
static_member:
    static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    variable_without_objects           { _p->onStaticMember(&$$,&$1,&$3);}
;

base_variable_with_function_calls:
    base_variable                      { $$ = $1;}
  | function_call                      { $$ = $1;}
;
base_variable:
    reference_variable                 { $$ = $1;}
  | simple_indirect_reference
    reference_variable                 { _p->onIndirectRef(&$$,&$1,&$2);}
  | static_member                      { $$ = $1; $$ = 2;}
;
reference_variable:
    reference_variable
    '[' dim_offset ']'                 { _p->onRefDim(&$$, &$1, &$3);}
  | reference_variable '{' expr '}'    { _p->onRefDim(&$$, &$1, &$3);}
  | compound_variable                  { $$ = $1;}
;
compound_variable:
    T_VARIABLE                         { _p->onSimpleVariable(&$$, &$1);}
  | '$' '{' expr '}'                   { _p->onDynamicVariable(&$$, &$3, 0);}
;
dim_offset:
    expr                               { $$ = $1;}
  |                                    { $$.reset();}
;

object_property:
    object_dim_list                    { }
  | variable_without_objects           { _p->appendProperty(&$1);}
;
object_dim_list:
    object_dim_list '[' dim_offset ']' { _p->appendRefDim(&$3);}
  | object_dim_list '{' expr '}'       { _p->appendRefDim(&$3);}
  | variable_name                      { _p->appendProperty(&$1);}
;
variable_name:
    T_STRING                           { $$ = $1;}
  | '{' expr '}'                       { $$ = $2;}
;

simple_indirect_reference:
    '$'                                { $$ = 1;}
  | simple_indirect_reference '$'      { $$++;}
;

assignment_list:
    assignment_list ',' alist_element  { _p->onExprListElem(&$$,&$1,&$3);}
  | alist_element                      { _p->onExprListElem(&$$,NULL,&$1);}
;
alist_element:
    variable                           { $$ = $1;}
  | T_LIST '(' assignment_list ')'     { _p->onListAssignment(&$$, &$3, NULL);}
  |                                    { $$.reset();}
;

array_pair_list:
    non_empty_array_pair_list
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_array_pair_list:
    non_empty_array_pair_list
    ',' expr T_DOUBLE_ARROW expr       { _p->onArrayPair(&$$,&$1,&$3,&$5,0);}
  | non_empty_array_pair_list ',' expr { _p->onArrayPair(&$$,&$1,  0,&$3,0);}
  | expr T_DOUBLE_ARROW expr           { _p->onArrayPair(&$$,  0,&$1,&$3,0);}
  | expr                               { _p->onArrayPair(&$$,  0,  0,&$1,0);}
  | non_empty_array_pair_list
    ',' expr T_DOUBLE_ARROW
    '&' w_variable                     { _p->onArrayPair(&$$,&$1,&$3,&$6,1);}
  | non_empty_array_pair_list ','
    '&' w_variable                     { _p->onArrayPair(&$$,&$1,  0,&$4,1);}
  | expr T_DOUBLE_ARROW '&' w_variable { _p->onArrayPair(&$$,  0,&$1,&$4,1);}
  | '&' w_variable                     { _p->onArrayPair(&$$,  0,  0,&$2,1);}
;

encaps_list:
    encaps_list encaps_var             { _p->addEncap(&$$, &$1, &$2, -1);}
  | encaps_list
    T_ENCAPSED_AND_WHITESPACE          { _p->addEncap(&$$, &$1, &$2, 0);}
  |                                    { $$.reset();}
;
encaps_var:
    T_VARIABLE                         { _p->onSimpleVariable(&$$, &$1);}
  | T_VARIABLE '['
    encaps_var_offset ']'              { _p->encapRefDim(&$$, &$1, &$3);}
  | T_VARIABLE T_OBJECT_OPERATOR
    T_STRING                           { _p->encapObjProp(&$$, &$1, &$3);}
  | T_DOLLAR_OPEN_CURLY_BRACES
    expr '}'                           { _p->onDynamicVariable(&$$, &$2, 1);}
  | T_DOLLAR_OPEN_CURLY_BRACES
    T_STRING_VARNAME '[' expr ']' '}'  { _p->encapArray(&$$, &$2, &$4);}
  | T_CURLY_OPEN variable '}'          { $$ = $2;}
;
encaps_var_offset:
    T_STRING                           { $$ = $1; $$ = T_STRING;}
  | T_NUM_STRING                       { $$ = $1; $$ = T_NUM_STRING;}
  | T_VARIABLE                         { $$ = $1; $$ = T_VARIABLE;}
;

internal_functions:
    T_ISSET '(' isset_variables ')'    { UEXP(&$$,&$3,T_ISSET,1);}
  | T_EMPTY '(' variable ')'           { UEXP(&$$,&$3,T_EMPTY,1);}
  | T_INCLUDE expr                     { UEXP(&$$,&$2,T_INCLUDE,1);}
  | T_INCLUDE_ONCE expr                { UEXP(&$$,&$2,T_INCLUDE_ONCE,1);}
  | T_EVAL '(' expr ')'                { UEXP(&$$,&$3,T_EVAL,1);}
  | T_REQUIRE expr                     { UEXP(&$$,&$2,T_REQUIRE,1);}
  | T_REQUIRE_ONCE expr                { UEXP(&$$,&$2,T_REQUIRE_ONCE,1);}
;

isset_variables:
    variable                           { _p->onExprListElem(&$$, NULL, &$1);}
  | isset_variables ',' variable       { _p->onExprListElem(&$$, &$1, &$3);}
;

class_constant:
  static_class_name
  T_PAAMAYIM_NEKUDOTAYIM T_STRING      { _p->onClassConst(&$$, &$1, &$3);}
;
%%

static int __attribute__((unused)) suppress_warning = yydebug;
