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
  hhas_fun     : Hhas_function.t list;
  hhas_classes : Hhas_class.t list;
  hhas_typedefs: Hhas_typedef.t list;
  hhas_main    : Hhas_main.t;
}

let make hhas_fun hhas_classes hhas_typedefs hhas_main =
  { hhas_fun; hhas_classes; hhas_typedefs; hhas_main }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let typedefs hhas_prog =
  hhas_prog.hhas_typedefs

let main hhas_prog =
  hhas_prog.hhas_main

open Instruction_sequence

let emit_main defs =
  let body_instrs, decl_vars, num_iters, num_cls_ref_slots, _, _, _, _ =
    Emit_body.from_ast
      ~namespace:Namespace_env.empty_with_default_popt
      ~skipawaitable:false
      ~scope:Ast_scope.Scope.toplevel
      ~return_value:(instr_int 1)
      ~default_dropthrough:None
      [] None defs in
  Hhas_main.make (Instruction_sequence.instr_seq_to_list body_instrs)
    decl_vars num_iters num_cls_ref_slots

open Closure_convert

let from_ast
  (parsed_functions,
  parsed_classes,
  parsed_typedefs,
  parsed_defs) =
  let st = initial_state (List.length parsed_classes) in
  let st, parsed_defs = convert_toplevel_prog st parsed_defs in
  let st, parsed_functions = List.map_env st parsed_functions convert_fun in
  let st, parsed_classes = List.map_env st parsed_classes convert_class in
  let closure_classes = Closure_convert.get_closure_classes st in
  let all_classes = parsed_classes @ closure_classes in
  let compiled_funs = Emit_function.from_asts parsed_functions in
  let compiled_funs = Generate_memoized.memoize_functions compiled_funs in
  let compiled_classes = Emit_class.from_asts all_classes in
  let compiled_classes = Generate_memoized.memoize_classes compiled_classes in
  let compiled_typedefs = Emit_typedef.from_asts parsed_typedefs in
  let compiled_defs = emit_main parsed_defs in
  make compiled_funs compiled_classes compiled_typedefs compiled_defs
