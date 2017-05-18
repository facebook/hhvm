(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

(**
 * Contains syntaxes used in coroutine code generation.
 *)

module EditableSyntax = Full_fidelity_editable_syntax
module EditableTrivia = Full_fidelity_editable_trivia
module TokenKind = Full_fidelity_token_kind

open EditableSyntax


(* Common factories *)

let make_syntax syntax =
  make syntax EditableSyntaxValue.NoValue

let single_space = [EditableTrivia.make_whitespace " "]

let make_token_syntax
    ?(space_before=false) ?(space_after=false) token_kind text =
  let before = if space_before then single_space else [] in
  let after = if space_after then single_space else [] in
  make_syntax @@ Token (EditableToken.make token_kind text before after)


(* General syntaxes *)

let left_brace_syntax =
  make_token_syntax ~space_after:true TokenKind.LeftBrace "{"

let right_brace_syntax =
  make_token_syntax
    ~space_before:true
    ~space_after:true
    TokenKind.RightBrace "}"

let left_paren_syntax =
  make_token_syntax TokenKind.LeftParen "("

let right_paren_syntax =
  make_token_syntax TokenKind.RightParen ")"

let left_angle_syntax =
  make_token_syntax TokenKind.LessThan "<"

let right_angle_syntax =
  make_token_syntax ~space_after:true TokenKind.GreaterThan ">"

let comma_syntax =
  make_token_syntax ~space_after:true TokenKind.Comma ","

let colon_colon_syntax =
  make_token_syntax TokenKind.ColonColon "::"

let semicolon_syntax =
  make_token_syntax TokenKind.Semicolon ";"

let colon_syntax =
  make_token_syntax ~space_after:true TokenKind.Colon ":"

let assignment_operator_syntax =
  make_token_syntax TokenKind.Equal "="

let not_identical_syntax =
  make_token_syntax TokenKind.ExclamationEqualEqual "!=="

let nullable_syntax =
  make_token_syntax TokenKind.Question "?"

let mixed_syntax =
  make_token_syntax ~space_after:true TokenKind.Mixed "mixed"

let mixed_type =
  make_simple_type_specifier mixed_syntax

(* TODO: Should be CoroutineUnit ? *)
let unit_syntax =
  make_token_syntax ~space_after:true TokenKind.Name "Unit"

let unit_type =
  make_simple_type_specifier unit_syntax

let int_syntax =
  make_token_syntax ~space_after:true TokenKind.Int "int"

let int_type =
  make_simple_type_specifier int_syntax

let void_syntax =
  make_token_syntax ~space_after:true TokenKind.Void "void"

let null_syntax =
  make_token_syntax TokenKind.NullLiteral "null"

let private_syntax =
  make_token_syntax ~space_after:true TokenKind.Private "private"

let public_syntax =
  make_token_syntax ~space_after:true TokenKind.Public "public"

let if_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.If "if"

let else_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Else "else"

let switch_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Switch "switch"

let case_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Case "case"

let class_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Class "class"

let default_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Default "default"

let function_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Function "function"

let goto_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Goto "goto"

let implements_syntax =
  make_token_syntax ~space_after:true TokenKind.Implements "implements"

let return_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Return "return"

let throw_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Throw "throw"

let new_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.New "new"

let member_selection_syntax =
  make_token_syntax TokenKind.MinusGreaterThan "->"

let this_variable =
  "$this"

let this_syntax =
  make_token_syntax TokenKind.Variable this_variable

let exception_type_syntax =
  make_token_syntax ~space_after:true TokenKind.Classname "Exception"


(* Syntax creation functions *)

let rec delimit delimit_syntax = function
  | [] -> []
  | hd :: [] -> [make_list_item hd (make_missing ())]
  | hd :: tl -> (make_list_item hd delimit_syntax) :: delimit delimit_syntax tl

let make_delimited_list delimit_syntax items =
  make_list (delimit delimit_syntax items)


let make_nullable_type_specifier_syntax type_syntax =
  make_nullable_type_specifier nullable_syntax type_syntax

let make_string_literal_syntax string_literal =
  make_literal_expression
    (make_token_syntax
      TokenKind.DoubleQuotedStringLiteral
      (Printf.sprintf "\"%s\"" string_literal))

let make_int_literal_syntax value =
  make_literal_expression
    (make_token_syntax TokenKind.DecimalLiteral (string_of_int value))

let make_qualified_name_syntax name =
  make_qualified_name_expression (make_token_syntax TokenKind.Name name)

let make_expression_statement expression_syntax =
  make_expression_statement expression_syntax semicolon_syntax

let make_compound_statement_syntax compound_statements =
  make_compound_statement
    left_brace_syntax
    (make_list compound_statements)
    right_brace_syntax

let make_return_statement_syntax expression_syntax =
  make_return_statement return_keyword_syntax expression_syntax semicolon_syntax

let make_return_missing_statement_syntax =
  make_return_statement_syntax (make_missing())

let make_throw_statement_syntax expression_syntax =
  make_throw_statement throw_keyword_syntax expression_syntax semicolon_syntax

let make_assignment_syntax_variable left assignment_expression_syntax =
  let assignment_binary_expression =
    make_binary_expression
      left
      assignment_operator_syntax
      assignment_expression_syntax in
  make_expression_statement assignment_binary_expression

let make_not_null_syntax operand =
  make_binary_expression operand not_identical_syntax null_syntax

let make_assignment_syntax receiver_variable assignment_expression_syntax =
  let receiver_variable_syntax =
    make_token_syntax TokenKind.Variable receiver_variable in
  make_assignment_syntax_variable
    receiver_variable_syntax assignment_expression_syntax

let make_object_creation_expression_syntax classname arguments =
  let classname_syntax = make_token_syntax TokenKind.Name classname in
  let arguments_syntax = make_delimited_list comma_syntax arguments in
  make_object_creation_expression
    new_keyword_syntax
    classname_syntax
    left_paren_syntax
    arguments_syntax
    right_paren_syntax

let make_function_call_expression_syntax receiver_syntax argument_list =
  let argument_list_syntax = make_delimited_list comma_syntax argument_list in
  make_function_call_expression
    receiver_syntax
    left_paren_syntax
    argument_list_syntax
    right_paren_syntax

let make_static_function_call_expression_syntax
    receiver_name
    member_name
    argument_list =
  let qualified_receiver_syntax = make_qualified_name_syntax receiver_name in
  let member_syntax = make_token_syntax TokenKind.Name member_name in
  let receiver_syntax =
    make_scope_resolution_expression
      qualified_receiver_syntax
      colon_colon_syntax
      member_syntax in
  make_function_call_expression_syntax receiver_syntax argument_list

let make_member_selection_expression_syntax receiver_syntax member_syntax =
  make_member_selection_expression
    receiver_syntax
    member_selection_syntax
    member_syntax

let make_parameter_declaration_syntax
    ?(visibility_syntax = make_missing ())
    parameter_type_syntax
    parameter_variable =
  let parameter_variable_syntax =
    make_token_syntax TokenKind.Variable parameter_variable in
  make_parameter_declaration
    (* attribute *)  (make_missing ())
    visibility_syntax
    parameter_type_syntax
    parameter_variable_syntax
    (* default value *)  (make_missing ())

let make_simple_type_specifier_syntax classname =
  let classname_syntax = make_token_syntax TokenKind.Classname classname in
  make_simple_type_specifier classname_syntax

let make_generic_type_specifier_syntax classname generic_argument_list =
  let classname_syntax = make_token_syntax TokenKind.Classname classname in
  let generic_argument_list_syntax =
    make_delimited_list comma_syntax generic_argument_list in
  let type_arguments_syntax =
    make_type_arguments
      left_angle_syntax
      generic_argument_list_syntax
      right_angle_syntax in
  make_generic_type_specifier classname_syntax type_arguments_syntax

let make_functional_type_syntax argument_types return_type_syntax =
  let argument_types_syntax = make_delimited_list comma_syntax argument_types in
  make_closure_type_specifier
    left_paren_syntax
    function_keyword_syntax
    left_paren_syntax
    argument_types_syntax
    right_paren_syntax
    colon_syntax
    return_type_syntax
    right_paren_syntax

(* TODO(tingley): Determine if it's worth tightening visibility here. *)
let make_classish_declaration_syntax classname implements_list classish_body =
  let classname_syntax =
    make_token_syntax ~space_after:true TokenKind.Classname classname in
  let implements_list_syntax =
    make_delimited_list comma_syntax implements_list in
  make_classish_declaration
    (* classish_attribute *) (make_missing ())
    (* classish_modifiers *) (make_missing ())
    class_keyword_syntax
    classname_syntax
    (* classish_type_parameters *) (make_missing ())
    (* classish_extends_keyword *) (make_missing ())
    (* classish_extends_list *) (make_missing ())
    (if is_missing implements_list_syntax then
      make_missing ()
    else
      implements_syntax)
    implements_list_syntax
    (make_compound_statement_syntax classish_body)

(* TODO(tingley): Determine if it's worth tightening visibility here. *)
let make_methodish_declaration_with_body_syntax
    function_decl_header_syntax
    function_body =
  make_methodish_declaration
    (* methodish_attribute *) (make_missing ())
    (* methodish_modifiers *) (make_missing ())
    function_decl_header_syntax
    function_body
    (* methodish_semicolon *) (make_missing ())

let make_methodish_declaration_syntax
    function_decl_header_syntax
    function_body =
  make_methodish_declaration_with_body_syntax
    function_decl_header_syntax
    (make_compound_statement_syntax function_body)

let make_function_decl_header_syntax
    name
    parameter_list
    return_type_syntax =
  let name_syntax = make_token_syntax TokenKind.Name name in
  let parameter_list_syntax = make_delimited_list comma_syntax parameter_list in
  make_function_declaration_header
    (* function_async *) (make_missing ())
    (* function_coroutine *) (make_missing ())
    function_keyword_syntax
    (* function_ampersand *) (make_missing ())
    name_syntax
    (* function_type_parameter_list *) (make_missing ())
    left_paren_syntax
    parameter_list_syntax
    right_paren_syntax
    (if is_missing return_type_syntax then make_missing () else colon_syntax)
    return_type_syntax
    (* function_where_clause *) (make_missing ())

let make_constructor_decl_header_syntax name parameter_list =
  make_function_decl_header_syntax name parameter_list (make_missing ())

let make_property_declaration_syntax type_syntax declaration_syntax =
  make_property_declaration
    (make_list [ public_syntax ])
    type_syntax
    declaration_syntax
    semicolon_syntax

let make_if_syntax condition_syntax true_statements =
  make_if_statement
    if_keyword_syntax
    left_paren_syntax
    condition_syntax
    right_paren_syntax
    (make_compound_statement_syntax true_statements)
    (make_missing ())
    (make_missing ())


(* Coroutine-specific syntaxes *)

(* TODO: Either (1) rename this to something less likely to conflict with
user variables or (2) put user variables in their own sub-closure.  *)

let next_label =
  "nextLabel"

let label_syntax =
  make_token_syntax ~space_after:true TokenKind.Variable ("$" ^ next_label)

let label_name_syntax =
  make_token_syntax ~space_after:true TokenKind.Name next_label

let continuation_variable =
  "$coroutineContinuation_generated"

let continuation_variable_syntax =
  make_token_syntax TokenKind.Variable continuation_variable

let make_continuation_type_syntax return_type_syntax =
  make_generic_type_specifier_syntax
    "CoroutineContinuation"
    [return_type_syntax]

let make_continuation_parameter_syntax
    ?visibility_syntax
    return_type_syntax =
  make_parameter_declaration_syntax
    ?visibility_syntax
    (make_continuation_type_syntax return_type_syntax)
    continuation_variable

let make_coroutine_result_type_syntax return_type_syntax =
  make_generic_type_specifier_syntax
    "CoroutineResult"
    [return_type_syntax]

let suspended_coroutine_result_classname =
  "SuspendedCoroutineResult"

let suspended_member_name =
  "create"

let is_suspended_member_name =
  "isSuspended"

let is_suspended_member_syntax =
  make_token_syntax TokenKind.Name is_suspended_member_name

let get_result_member_name =
  "getResult"

let get_result_member_syntax =
  make_token_syntax TokenKind.Name get_result_member_name

let make_closure_classname enclosing_classname function_name =
  Printf.sprintf "%s_%s_GeneratedClosure" enclosing_classname function_name

let make_closure_type_syntax enclosing_classname function_name =
  make_simple_type_specifier_syntax
    (make_closure_classname enclosing_classname function_name)

let closure_variable =
  "$closure"

(* $closure *)
let closure_variable_syntax =
  make_token_syntax TokenKind.Variable closure_variable

let make_closure_parameter_syntax enclosing_classname function_name =
  make_parameter_declaration_syntax
    (make_closure_type_syntax enclosing_classname function_name)
    closure_variable

let constructor_member_name =
  "__construct"

let resume_member_name =
  "resume"

let resume_member_name_syntax =
  make_token_syntax TokenKind.Name resume_member_name

let resume_with_exception_member_name =
  "resumeWithException"

let coroutine_data_variable =
  "$coroutineData"

let coroutine_data_variable_syntax =
  make_token_syntax TokenKind.Variable coroutine_data_variable

let coroutine_result_variable =
  "$coroutineResult"

let coroutine_result_variable_syntax =
  make_token_syntax TokenKind.Variable coroutine_result_variable

let make_coroutine_result_data_variable index =
  Printf.sprintf "$coroutineResultData%d" index

let coroutine_data_type_syntax =
  mixed_syntax

let coroutine_data_parameter_syntax =
  make_parameter_declaration_syntax
    coroutine_data_type_syntax
    coroutine_data_variable

let exception_variable =
  "$exception"

let exception_variable_syntax =
  make_token_syntax TokenKind.Variable exception_variable

let nullable_exception_type_syntax =
  make_nullable_type_specifier_syntax exception_type_syntax

let exception_parameter_syntax =
  make_parameter_declaration_syntax
    exception_type_syntax
    exception_variable

let nullable_exception_parameter_syntax =
  make_parameter_declaration_syntax
    nullable_exception_type_syntax
    exception_variable

let throw_unimplemented_syntax reason =
  let create_exception_syntax =
    make_object_creation_expression_syntax
      "Exception"
      [make_string_literal_syntax reason] in
  make_throw_statement_syntax create_exception_syntax

let make_state_machine_method_name function_name =
  Printf.sprintf "%s_GeneratedStateMachine" function_name

let state_machine_member_name =
  "stateMachineFunction"

let state_machine_member_name_syntax =
  make_token_syntax TokenKind.Name state_machine_member_name

let state_machine_variable_name =
  "$" ^ state_machine_member_name

let state_machine_variable_name_syntax =
  make_token_syntax TokenKind.Variable state_machine_variable_name

let make_state_machine_parameter_syntax enclosing_classname function_name =
  let state_machine_type_syntax =
    make_functional_type_syntax
      [
        make_closure_type_syntax enclosing_classname function_name;
        mixed_syntax;
        nullable_exception_type_syntax;
      ]
      (make_coroutine_result_type_syntax mixed_syntax) in
  make_parameter_declaration_syntax
    ~visibility_syntax:private_syntax
    state_machine_type_syntax
    state_machine_variable_name

let inst_meth_syntax =
  make_qualified_name_syntax "inst_meth"

let make_member_with_unknown_type_declaration_syntax variable_syntax =
  let declaration_syntax =
    make_property_declarator variable_syntax (make_missing ()) in
  make_property_declaration_syntax (make_missing ()) declaration_syntax

(* label0 *)
let goto_label_name number =
  let text = Printf.sprintf "label%d" number in
  make_token_syntax TokenKind.Name text

(* label0: *)
let goto_label_syntax number =
  make_goto_label (goto_label_name number) colon_syntax

(* labelerror *)
let error_label_name =
  make_token_syntax TokenKind.Name "labelerror"

(* labelerror: *)
let error_label_syntax =
  make_goto_label error_label_name colon_syntax

(* case 0: *)
let make_case_label_syntax number =
  let number = make_int_literal_syntax number in
  make_case_label case_keyword_syntax number colon_syntax

(* goto label0; *)
let make_goto_syntax number =
  let label = goto_label_name number in
  make_goto_statement goto_keyword_syntax label semicolon_syntax

(* goto labelerror; *)
let make_goto_error_syntax =
  make_goto_statement goto_keyword_syntax error_label_name semicolon_syntax

(* default: *)
let default_label_syntax =
  make_default_label default_keyword_syntax colon_syntax

(* case 0: goto label0; *)
let make_switch_section_syntax number =
  let label = make_case_label_syntax number in
  let statement = make_goto_syntax number in
  let fallthrough = make_missing() in
  make_switch_section label statement fallthrough

let default_section_syntax =
  make_switch_section
    default_label_syntax make_goto_error_syntax (make_missing())

let make_switch_sections number =
  let rec aux n acc =
    if n < 0 then acc
    else aux (n - 1) ((make_switch_section_syntax n) :: acc) in
  make_list (aux number [ default_section_syntax ])

(*
  switch($closure->nextLabel) {
    case 0: goto label0;
    ...
    default: goto labelerror;
  } *)
let make_coroutine_switch label_count =
  let sections = make_switch_sections label_count in
  make_switch_statement switch_keyword_syntax left_paren_syntax
    label_syntax right_paren_syntax left_brace_syntax sections
    right_brace_syntax

(* coroutine_unit() *)
let coroutine_unit_call_syntax =
  let name = make_token_syntax TokenKind.Name "coroutine_unit" in
  make_function_call_expression_syntax name []

(* $closure->nextLabel = x; *)
let set_next_label_syntax number =
  let number = make_int_literal_syntax number in
  make_assignment_syntax_variable label_syntax number
