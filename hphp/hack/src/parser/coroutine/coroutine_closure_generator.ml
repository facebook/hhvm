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

let generate_hoisted_locals { CoroutineStateMachineData.local_variables; _; } =
  local_variables
    |> SMap.values
    |> Core_list.map ~f:make_member_with_unknown_type_declaration_syntax

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
    classish_name
    header_node
    state_machine_data =
  let function_parameter_list =
    make_parameters_public_and_untyped state_machine_data in
  let cont_param =
    make_continuation_parameter_syntax
      ~visibility_syntax:private_syntax
      header_node in
  let sm_param =
    make_state_machine_parameter_syntax classish_name header_node in
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

let generate_do_resume_method { function_type; _; } =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      do_resume_member_name
      [ coroutine_data_parameter_syntax; nullable_exception_parameter_syntax; ]
      (make_coroutine_result_type_syntax function_type))
    do_resume_body

let generate_closure_body
    classish_name
    method_node
    header_node
    state_machine_data =
  generate_hoisted_locals state_machine_data
    @ [
      generate_constructor_method
        classish_name
        header_node
        state_machine_data;
      generate_do_resume_method header_node;
    ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_closure
    class_node
    ({ methodish_function_body; _; } as method_node)
    header_node
    state_machine_data =
  if is_missing methodish_function_body then
    methodish_function_body
  else
    make_classish_declaration_syntax
      (make_closure_classname class_node header_node)
      (make_closure_type_parameters class_node header_node)
      [ make_closure_base_type_syntax header_node ]
      (generate_closure_body
        class_node
        method_node
        header_node
        state_machine_data)
