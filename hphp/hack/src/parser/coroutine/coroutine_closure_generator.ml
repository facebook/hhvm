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
module EditableSyntax = Full_fidelity_editable_syntax

open EditableSyntax
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
  let cont_param =
    make_continuation_parameter_syntax
      ~visibility_syntax:private_syntax
      function_type in
  let sm_param = make_state_machine_parameter_syntax context function_type in
  let function_parameter_list =
    cont_param :: sm_param :: function_parameter_list in
  let ctor = make_constructor_decl_header_syntax
    constructor_member_name function_parameter_list in
  let call_parent_syntax =
    make_construct_parent_syntax [ continuation_variable_syntax ] in
  make_methodish_declaration_syntax ctor [ call_parent_syntax; ]

let do_resume_body =
  let select_state_machine_syntax =
    make_member_selection_expression_syntax
      this_syntax
      state_machine_member_name_syntax in
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

let generate_closure_body
    context
    function_type
    state_machine_data =
  generate_closure_properties state_machine_data
    @ [
      generate_constructor_method context function_type state_machine_data;
      generate_do_resume_method function_type;
    ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_closure
    context
    body
    function_type
    state_machine_data =
  if is_missing body then
    body
  else
    make_classish_declaration_syntax
      (make_closure_classname context)
      (make_closure_type_parameters context)
      [ make_closure_base_type_syntax function_type ]
      (generate_closure_body
        context
        function_type
        state_machine_data)
