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

let from_ast : Ast.class_ -> Ast.method_ -> Hhas_method.t  =
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
  let (_,name) = ast_class.Ast.c_name in
  let ret =
    if method_name = Naming_special_names.Members.__construct
    then None else ast_method.Ast.m_ret in
  let body_instrs,
      method_decl_vars,
      method_params,
      method_return_type,
      method_is_generator,
      method_is_pair_generator =
    Emit_body.from_ast
      ~self:(Some name)
      tparams
      ast_method.Ast.m_params
      ret
      ast_method.Ast.m_body
  in
  let method_is_async =
    ast_method.Ast.m_fun_kind = Ast_defs.FAsync
    || ast_method.Ast.m_fun_kind = Ast_defs.FAsyncGenerator
  in
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
    method_decl_vars
    method_is_async
    method_is_generator
    method_is_pair_generator

let from_asts ast_class ast_methods =
  List.map ast_methods (from_ast ast_class)
