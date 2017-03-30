(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence

let from_ast : Ast.fun_ -> Hhas_function.t =
  fun ast_fun ->
  let function_name = Litstr.to_string @@ snd ast_fun.Ast.f_name in
  let default_instrs _ = gather [instr_null; instr_retc] in
  let body_instrs,
      function_decl_vars,
      function_params,
      function_return_type,
      function_is_generator,
      function_is_pair_generator =
    Emit_body.from_ast
      ~self:None
      ast_fun.Ast.f_tparams
      ast_fun.Ast.f_params
      ast_fun.Ast.f_ret
      ast_fun.Ast.f_body
      default_instrs in
  let function_body = instr_seq_to_list body_instrs in
  let function_attributes =
    Emit_attribute.from_asts ast_fun.Ast.f_user_attributes in
  let function_is_async =
    ast_fun.Ast.f_fun_kind = Ast_defs.FAsync
    || ast_fun.Ast.f_fun_kind = Ast_defs.FAsyncGenerator
  in
  Hhas_function.make
    function_attributes
    function_name
    function_params
    function_return_type
    function_body
    function_decl_vars
    function_is_async
    function_is_generator
    function_is_pair_generator

let from_asts ast_functions =
  List.map from_ast ast_functions
