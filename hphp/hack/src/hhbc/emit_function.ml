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
  let namespace = ast_fun.Ast.f_namespace in
  let function_name, _ =
    Hhbc_id.Function.elaborate_id namespace ast_fun.Ast.f_name in
  let function_is_async =
    ast_fun.Ast.f_fun_kind = Ast_defs.FAsync
    || ast_fun.Ast.f_fun_kind = Ast_defs.FAsyncGenerator in
  let default_dropthrough =
    if function_is_async
    then Some (gather [instr_null; instr_retc])
    else None in
  let function_body, function_is_generator, function_is_pair_generator =
    Emit_body.emit_body
      ~scope:[Ast_scope.ScopeItem.Function ast_fun]
      ~is_closure_body:false
      ~is_memoize_wrapper:false
      ~skipawaitable:(ast_fun.Ast.f_fun_kind = Ast_defs.FAsync)
      ~default_dropthrough
      ~return_value:instr_null
      ~namespace
      ast_fun.Ast.f_params
      ast_fun.Ast.f_ret
      [Ast.Stmt (Ast.Block ast_fun.Ast.f_body)] in
  let function_attributes =
    Emit_attribute.from_asts ast_fun.Ast.f_user_attributes in
  Hhas_function.make
    function_attributes
    function_name
    function_body
    function_is_async
    function_is_generator
    function_is_pair_generator

let from_asts ast_functions =
  List.map from_ast ast_functions
