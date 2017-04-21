(**
 * Copyright (c) 2016, Facebook, Inc.
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

let null_syntax =
  make_token_syntax TokenKind.NullLiteral "null"

let return_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.Return "return"

let new_keyword_syntax =
  make_token_syntax ~space_after:true TokenKind.New "new"

let member_selection_syntax =
  make_token_syntax TokenKind.MinusGreaterThan "->"


(* Syntax creation functions *)

let rec delimit delimit_syntax = function
  | [] -> []
  | hd :: [] -> [make_list_item hd (make_missing ())]
  | hd :: tl -> (make_list_item hd delimit_syntax) :: delimit delimit_syntax tl

let make_delimited_list delimit_syntax items =
  make_list (delimit delimit_syntax items)

let make_expression_statement expression_syntax =
  make_expression_statement expression_syntax semicolon_syntax

let make_return_statement_syntax expression_syntax =
  make_return_statement return_keyword_syntax expression_syntax semicolon_syntax

let make_object_creation_expression_syntax classname arguments =
  let classname_syntax = make_token_syntax TokenKind.Classname classname in
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
  let receiver_name_syntax = make_token_syntax TokenKind.Name receiver_name in
  let qualified_receiver_syntax =
    make_qualified_name_expression receiver_name_syntax in
  let member_syntax = make_token_syntax TokenKind.Name member_name in
  let receiver_syntax =
    make_scope_resolution_expression
      qualified_receiver_syntax
      colon_colon_syntax
      member_syntax in
  make_function_call_expression_syntax receiver_syntax argument_list


let make_member_selection_expression_syntax receiver_syntax member_name =
  let member_name_syntax = make_token_syntax TokenKind.Name member_name in
  make_member_selection_expression
    receiver_syntax
    member_selection_syntax
    member_name_syntax

let make_parameter_declaration_syntax
    parameter_type_syntax
    parameter_variable =
  let parameter_variable_syntax =
    make_token_syntax TokenKind.Variable parameter_variable in
  make_parameter_declaration
    (* attribute *)  (make_missing ())
    (* visibility *)  (make_missing ())
    parameter_type_syntax
    parameter_variable_syntax
    (* default value *)  (make_missing ())

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


(* Coroutine-specific syntaxes *)

let continuation_variable =
  "$coroutineContinuation_generated"

let continuation_variable_syntax =
  make_token_syntax TokenKind.Variable continuation_variable

let make_continuation_parameter_syntax return_type_syntax =
  let continuation_type_syntax =
    make_generic_type_specifier_syntax
      "CoroutineContinuation"
      [return_type_syntax] in
  make_parameter_declaration_syntax
    continuation_type_syntax
    continuation_variable

let make_coroutine_result_type_syntax return_type_syntax =
  make_generic_type_specifier_syntax
    "CoroutineResult"
    [return_type_syntax]

let suspended_coroutine_result_classname =
  "SuspendedCoroutineResult"

let suspended_member_name =
  "suspended"

let make_state_machine_classname coroutine_function_name =
  coroutine_function_name ^ "_GeneratedStateMachine"

let resume_member_name =
  "resume"
