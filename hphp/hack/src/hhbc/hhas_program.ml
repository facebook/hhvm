(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

type t = {
  hhas_adata   : Hhas_adata.t list;
  hhas_fun     : Hhas_function.t list;
  hhas_classes : Hhas_class.t list;
  hhas_typedefs: Hhas_typedef.t list;
  hhas_main    : Hhas_body.t;
}

let make hhas_adata hhas_fun hhas_classes hhas_typedefs hhas_main =
  { hhas_adata; hhas_fun; hhas_classes; hhas_typedefs; hhas_main }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let typedefs hhas_prog =
  hhas_prog.hhas_typedefs

let main hhas_prog =
  hhas_prog.hhas_main

let adata hhas_prog =
  hhas_prog.hhas_adata

open Instruction_sequence

let emit_main defs =
  let body, _is_generator, _is_pair_generator =
    Emit_body.emit_body
      ~namespace:Namespace_env.empty_with_default_popt
      ~is_closure_body:false
      ~is_memoize:false
      ~skipawaitable:false
      ~is_return_by_ref:false
      ~scope:Ast_scope.Scope.toplevel
      ~return_value:(instr_int 1)
      ~default_dropthrough:None
      [] None defs
  in
    body

open Closure_convert

let from_ast
  (parsed_functions,
  parsed_classes,
  parsed_typedefs,
  parsed_defs) =
  let inline_fn_defs, inline_class_defs = Inline_defs.from_ast parsed_defs in
  let parsed_functions = inline_fn_defs @ parsed_functions in
  let parsed_classes = inline_class_defs @ parsed_classes in
  let st = initial_state (List.length parsed_classes) in
  let st, parsed_defs = convert_toplevel_prog st parsed_defs in
  let st, parsed_functions = List.map_env st parsed_functions convert_fun in
  let st, parsed_classes = List.map_env st parsed_classes convert_class in
  let closure_classes = Closure_convert.get_closure_classes st in
  let all_classes = parsed_classes @ closure_classes in
  try
    let compiled_defs = emit_main parsed_defs in
    let compiled_funs = Emit_function.from_asts parsed_functions in
    let compiled_classes = Emit_class.from_asts all_classes in
    let compiled_typedefs = Emit_typedef.from_asts parsed_typedefs in
    let adata = Emit_adata.get_adata () in
    make adata compiled_funs compiled_classes compiled_typedefs compiled_defs
  with Emit_fatal.IncludeTimeFatalException (op, message) ->
    let body = Emit_body.make_body (Emit_fatal.emit_fatal op message)
      [] (* decl_vars *)
      false (*is_memoize_wrapper*)
      [] (* params *)
      None (* return_type_info *)
    in
      make [] [] [] [] body
