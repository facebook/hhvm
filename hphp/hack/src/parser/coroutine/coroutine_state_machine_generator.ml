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

let generate_constructor_body =
  make_missing ()

let generate_constructor_method { function_type; _; } =
  make_methodish_declaration_syntax
    (make_constructor_decl_header_syntax
      constructor_member_name
      [ make_continuation_parameter_syntax function_type ])
    []

let generate_resume_body { methodish_function_body; _; } =
  let select_do_resume_syntax =
    make_member_selection_expression_syntax this_syntax do_resume_member_name in
  let call_do_resume_syntax =
    make_function_call_expression_syntax
      select_do_resume_syntax
      [ coroutune_data_variable_syntax; null_syntax ] in
  [ make_expression_statement call_do_resume_syntax ]

let generate_resume_method method_node =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      resume_member_name
      [ coroutine_data_parameter_syntax ]
      void_syntax)
    (generate_resume_body method_node)

let generate_resume_with_exception_body _ =
  let select_do_resume_syntax =
    make_member_selection_expression_syntax this_syntax do_resume_member_name in
  let call_do_resume_syntax =
    make_function_call_expression_syntax
      select_do_resume_syntax
      [ null_syntax; exception_variable_syntax ] in
  [ make_expression_statement call_do_resume_syntax ]

let generate_resume_with_exception_method method_node =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      resume_with_exception_member_name
      [ exception_parameter_syntax ]
      void_syntax)
    (generate_resume_with_exception_body method_node)

let generate_do_resume_body _ =
  [ throw_unimplemented_syntax "Coroutines are not yet implemented." ]

let generate_do_resume_method method_node =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      do_resume_member_name
      [ coroutine_data_parameter_syntax; nullable_exception_parameter_syntax ]
      void_syntax)
    (generate_do_resume_body method_node)

let generate_state_machine_body method_node header_node =
  [
    generate_constructor_method header_node;
    generate_resume_method method_node;
    generate_resume_with_exception_method method_node;
    generate_do_resume_method method_node;
  ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_state_machine
    classish_name
    function_name
    ({ methodish_function_decl_header; _; } as method_node)
    header_node =
  make_classish_declaration_syntax
    (make_state_machine_classname classish_name function_name)
    [ make_continuation_type_syntax mixed_syntax ]
    (generate_state_machine_body method_node header_node)
