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
  hhas_main    : Hhas_main.t;
}

let make hhas_fun hhas_classes hhas_main =
  { hhas_fun; hhas_classes; hhas_main }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let main hhas_prog =
  hhas_prog.hhas_main

let emit_main block =
  let return_seq _ = Instruction_sequence.empty in
  let body_instrs, decl_vars, num_iters, _, _, _, _ =
    Emit_body.from_ast ~self:None [] [] None block return_seq in
  Hhas_main.make (Instruction_sequence.instr_seq_to_list body_instrs)
    decl_vars num_iters

let from_ast
  (parsed_functions,
  parsed_classes,
  _parsed_typedefs,
  _parsed_consts,
  parsed_statements) =
  let env = Closure_convert.initial_env (List.length parsed_classes) in
  let env, parsed_functions =
    List.map_env env parsed_functions Closure_convert.convert_fun in
  let env, parsed_classes =
    List.map_env env parsed_classes Closure_convert.convert_class in
  let env, parsed_statements =
    Closure_convert.convert_block env parsed_statements in
  let closure_classes = Closure_convert.get_closure_classes env in
  let all_classes = parsed_classes @ closure_classes in
  let compiled_funs = Emit_function.from_asts parsed_functions in
  let compiled_funs = Generate_memoized.memoize_functions compiled_funs in
  let compiled_classes = Emit_class.from_asts all_classes in
  let compiled_classes = Generate_memoized.memoize_classes compiled_classes in
  let _compiled_typedefs = [] in (* TODO *)
  let _compiled_consts = [] in (* TODO *)
  let pos = Pos.none in
  (* Main method returns 1 by default? *)
  let parsed_statements =
    parsed_statements @
    [Ast.Return (pos, Some (pos, Ast.Int(pos, "1"))) ] in
  let compiled_statements = emit_main parsed_statements in
  make compiled_funs compiled_classes compiled_statements
