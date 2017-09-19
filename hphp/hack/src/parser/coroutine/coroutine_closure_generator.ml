(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineStateMachineData = Coroutine_state_machine_data
module CoroutineSyntax = Coroutine_syntax
module CoroutineTypeLowerer = Coroutine_type_lowerer
module Syntax = Full_fidelity_editable_positioned_syntax

open Syntax
open CoroutineSyntax

let generate_closure_properties { CoroutineStateMachineData.properties; _; } =
  Core_list.map ~f:make_member_with_unknown_type_declaration_syntax
    properties

let make_parameters_public_and_untyped
    { CoroutineStateMachineData.parameters; _; } =
  parameters
    |> Core_list.map ~f:
      begin
      fun p ->
        make_syntax
          (ParameterDeclaration {
            p with
              parameter_visibility = public_syntax;
              parameter_type = make_missing ();
          })
      end

let generate_constructor_method
    context
    function_type
    state_machine_data =
  let function_parameter_list =
    make_parameters_public_and_untyped state_machine_data in
  let cont_param = make_continuation_parameter_syntax function_type in
  let sm_param = make_state_machine_parameter_syntax context function_type in
  let function_parameter_list =
    cont_param :: sm_param :: function_parameter_list in
  let ctor = make_constructor_decl_header_syntax
    constructor_member_name function_parameter_list in
  let call_parent_syntax =
    make_construct_parent_syntax [ continuation_variable_syntax ] in
  make_methodish_declaration_syntax
    ~modifiers:[ public_syntax; final_syntax; ]
    ctor
    [ call_parent_syntax; ]

let select_state_machine_syntax =
  make_member_selection_expression_syntax
    this_syntax
    state_machine_member_name_syntax

let do_resume_body =
  let assign_state_machine_syntax =
    make_assignment_syntax
      state_machine_variable_name
      select_state_machine_syntax in
  let call_state_machine_syntax =
    make_function_call_expression_syntax
      state_machine_variable_name_syntax
      [
        this_syntax;
        coroutine_data_variable_syntax;
        exception_variable_syntax;
      ] in
  [
    assign_state_machine_syntax;
    make_return_statement_syntax call_state_machine_syntax;
  ]

let generate_do_resume_method function_type =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      do_resume_member_name
      [ coroutine_data_parameter_syntax; nullable_exception_parameter_syntax; ]
      (make_coroutine_result_type_syntax function_type))
    do_resume_body

let generate_clone_body { CoroutineStateMachineData.parameters; properties; } =
  (* $this->coroutineContinuation_generated *)
  let select_continuation_syntax =
    make_member_selection_expression_syntax
      this_syntax
      continuation_member_name_syntax in
  (* $this->coroutineContinuation_generated->clone *)
  let select_continuation_clone_syntax =
    make_member_selection_expression_syntax
      select_continuation_syntax
      clone_member_name_syntax in
  let clone_continuation_syntax =
    make_function_call_expression_syntax select_continuation_clone_syntax [] in
  (* [ $this->arg1; $this->arg2; ... ] *)
  let arg_list =
    let get_parameter_as_member_variable { parameter_name; _; } =
      parameter_name
        |> string_of_variable_token
        |> fun var -> String_utils.lstrip var "$"
        |> make_name_syntax
        |> make_member_selection_expression_syntax this_syntax in
    Core_list.map ~f:get_parameter_as_member_variable parameters in
  (* [
   *   $this->coroutineContinuation_generated->clone();
   *   $this->stateMachineFunction;
   *   $this->arg1;
   *   $this->arg2;
   *   ...
   * ]
   *)
  let parameters =
    clone_continuation_syntax ::
    select_state_machine_syntax ::
    arg_list in
  (* new static(args) *)
  let new_closure_syntax =
    make_typed_object_creation_expression_syntax static_syntax parameters in
  (* $closure = new static(args); *)
  let closure_assignment_syntax =
    make_assignment_syntax closure_variable new_closure_syntax in
  (* [ $closure->coroutineResultData1 = $this->coroutineResultData1; ... ] *)
  let copy_properties_syntaxes =
    let make_copy_property_syntax property_name_string =
      let property_member_name =
        property_name_string
          |> fun var -> String_utils.lstrip var "$"
          |> make_name_syntax in
      let this_member_syntax =
        make_member_selection_expression_syntax
          this_syntax
          property_member_name in
      let closure_member_syntax =
        make_member_selection_expression_syntax
          closure_variable_syntax
          property_member_name in
      make_assignment_syntax_variable
        closure_member_syntax
        this_member_syntax in
    Core_list.map ~f:make_copy_property_syntax (next_label :: properties) in
  (* return $closure; *)
  let return_statement_syntax =
    make_return_statement_syntax closure_variable_syntax in
  closure_assignment_syntax ::
    copy_properties_syntaxes @
    [ return_statement_syntax; ]

let generate_clone_method state_machine_data =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax clone_member_name [] this_type_syntax)
    (generate_clone_body state_machine_data)

let generate_closure_body
    context
    function_type
    state_machine_data =
  generate_closure_properties state_machine_data
    @ [
      generate_constructor_method context function_type state_machine_data;
      generate_do_resume_method function_type;
      generate_clone_method state_machine_data;
    ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *
 * Returns None if no closure needs to be generated for the function. For
 * example, if the function is abstract and does not have an implementation,
 * then it is not meaningful to generate a closure.
 *)
let generate_coroutine_closure
    context
    body
    function_type
    state_machine_data =
  Option.some_if
    (not @@ is_missing body)
    (
      make_classish_declaration_syntax
        (make_closure_classname context)
        (make_closure_type_parameters context)
        [ make_closure_base_type_syntax function_type ]
        (generate_closure_body
          context
          function_type
          state_machine_data)
    )
