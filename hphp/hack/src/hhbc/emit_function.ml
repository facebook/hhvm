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
open Instruction_sequence

let from_ast : Ast.fun_ -> Hhas_function.t =
  fun ast_fun ->
  let function_name = Litstr.to_string @@ snd ast_fun.Ast.f_name in
  let body_instrs, function_params, function_return_type = Emit_body.from_ast
    ast_fun.Ast.f_tparams ast_fun.Ast.f_params ast_fun.Ast.f_ret
    ast_fun.Ast.f_body in
  let body_instrs = Label_rewriter.relabel_instrseq body_instrs in
  let function_decl_vars = extract_decl_vars body_instrs in
  let function_body = instr_seq_to_list body_instrs in
  let function_attributes =
    Emit_attribute.from_asts ast_fun.Ast.f_user_attributes in
  Hhas_function.make
    function_attributes
    function_name
    function_params
    function_return_type
    function_body
    function_decl_vars

let is_memoized attributes =
  let f attr = (Hhas_attribute.name attr) = "__Memoize" in
  List.exists attributes f

let from_asts ast_functions =
  let f ast_fun =
    let compiled = from_ast ast_fun in
    if is_memoized (Hhas_function.attributes compiled) then
      let (renamed, memoized) = Generate_memoized.memoize_function compiled in
      [ renamed; memoized ]
    else
      [ compiled ] in
  Core.List.bind ast_functions f
