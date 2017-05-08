(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax

open EditableSyntax
open CoroutineSyntax

let extract_parameter_declarations function_parameter_list =
  function_parameter_list
    |> syntax_node_to_list
    |> Core_list.map ~f:syntax
    |> Core_list.map ~f:
      begin
      function
      | ListItem { list_item = { syntax = ParameterDeclaration p; _; }; _; } ->
          p
      | _ -> failwith "Parameter had unexpected type."
      end

let generate_hoisted_locals { function_parameter_list; _; } locals_and_params =
  let params =
    function_parameter_list
      |> extract_parameter_declarations
      |> Core_list.map ~f:(fun { parameter_name; _; } ->
        match syntax parameter_name with
        | Token { EditableToken.kind = TokenKind.Variable; text; _; } -> text
        | _ -> failwith "Parameter name was not a variable token.")
      |> SSet.of_list in
  SSet.diff locals_and_params params
    |> SSet.elements
    |> Core_list.map ~f:make_member_with_unknown_type_declaration_syntax

let make_parameters_public function_parameter_list =
  function_parameter_list
    |> extract_parameter_declarations
    |> Core_list.map ~f:
      begin
      fun p ->
        make_syntax
          (ParameterDeclaration { p with parameter_visibility = public_syntax })
      end

let generate_constructor_method
    classish_name
    function_name
    { function_parameter_list; function_type; _; } =
  let function_parameter_list =
    make_parameters_public function_parameter_list in
  let cont_param = make_continuation_parameter_syntax
    ~visibility_syntax:private_syntax function_type in
  let sm_param = make_state_machine_parameter_syntax
    classish_name function_name in
  let function_parameter_list =
    cont_param :: sm_param :: function_parameter_list in
  let ctor = make_constructor_decl_header_syntax
    constructor_member_name function_parameter_list in
  make_methodish_declaration_syntax ctor []

let generate_resume_body { methodish_function_body; _; } =
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
      [ this_syntax; coroutune_data_variable_syntax; null_syntax ] in
  [
    assign_state_machine_syntax;
    make_expression_statement call_state_machine_syntax;
  ]

let generate_resume_method method_node =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      resume_member_name
      [ coroutine_data_parameter_syntax ]
      void_syntax)
    (generate_resume_body method_node)

let generate_resume_with_exception_body _ =
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
      [ this_syntax; null_syntax; exception_variable_syntax ] in
  [
    assign_state_machine_syntax;
    make_expression_statement call_state_machine_syntax;
  ]

let generate_resume_with_exception_method method_node =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      resume_with_exception_member_name
      [ exception_parameter_syntax ]
      void_syntax)
    (generate_resume_with_exception_body method_node)

let generate_closure_body
    classish_name
    function_name
    method_node
    header_node
    locals_and_params =
  label_declaration_syntax
    :: generate_hoisted_locals header_node locals_and_params
    @ [
      generate_constructor_method classish_name function_name header_node;
      generate_resume_method method_node;
      generate_resume_with_exception_method method_node
    ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_closure
    classish_name
    function_name
    ({ methodish_function_decl_header; _; } as method_node)
    header_node
    locals_and_params =
  make_classish_declaration_syntax
    (make_closure_classname classish_name function_name)
    [ make_continuation_type_syntax mixed_syntax ]
    (generate_closure_body
      classish_name
      function_name
      method_node
      header_node
      locals_and_params)
