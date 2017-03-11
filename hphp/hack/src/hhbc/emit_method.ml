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

let from_ast_no_memoization : Ast.class_ -> Ast.method_ -> Hhas_method.t  =
  fun ast_class ast_method ->
  let method_name = Litstr.to_string @@ snd ast_method.Ast.m_name in
  let method_is_abstract = List.mem ast_method.Ast.m_kind Ast.Abstract in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_private = List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected = List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public = List.mem ast_method.Ast.m_kind Ast.Public in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let method_attributes =
    Emit_attribute.from_asts ast_method.Ast.m_user_attributes in
  let tparams = ast_class.Ast.c_tparams @ ast_method.Ast.m_tparams in
  let body_instrs, method_params, method_return_type = Emit_body.from_ast
    tparams ast_method.Ast.m_params ast_method.Ast.m_ret
    ast_method.Ast.m_body in
  let method_body = instr_seq_to_list body_instrs in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    method_name
    method_params
    method_return_type
    method_body

let from_asts ast_class ast_methods =
  let is_memoized ast_method =
    let attributes = ast_method.Ast.m_user_attributes in
    Emit_attribute.ast_any_is_memoize attributes in
  let memoized_count = List.count ast_methods is_memoized in
  let folder (count, acc) ast_method =
    let compiled = from_ast_no_memoization ast_class ast_method in
    if Hhas_attribute.is_memoized (Hhas_method.attributes compiled) then
      let (renamed, memoized) =
        Generate_memoized.memoize_method compiled memoized_count count in
      (count + 1, memoized :: renamed :: acc)
    else
      (count, compiled :: acc) in
  let (_, methods) = Core.List.fold_left ast_methods ~init:(0, []) ~f:folder in
  List.rev methods
