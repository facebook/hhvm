(**
 * Copyright (c) 2017, Facebook, Inc.
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

let generate_coroutine_state_machine_body =
  [
    make_coroutine_switch 0; (* TODO: Need label count. *)
    goto_label_syntax 0;
    throw_unimplemented_syntax "Coroutines are not yet implemented.";
    (* TODO: rewritten body here *)
    error_label_syntax;
    throw_unimplemented_syntax "A completed coroutine was resumed.";
  ]

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * declaration header and the function body into a desugared coroutine
 * implementation.
 *)
let generate_coroutine_state_machine
    classish_name
    function_name
    { methodish_function_decl_header; _; }
    { function_type; _; } =
  make_methodish_declaration_syntax
    (make_function_decl_header_syntax
      (make_state_machine_method_name function_name)
      [
        make_closure_parameter_syntax classish_name function_name;
        coroutine_data_parameter_syntax;
        nullable_exception_parameter_syntax;
      ]
      (make_coroutine_result_type_syntax function_type))
    (generate_coroutine_state_machine_body)
