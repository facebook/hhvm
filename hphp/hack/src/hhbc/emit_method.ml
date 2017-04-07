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
open Hhbc_ast

let from_ast : Ast.class_ -> Ast.method_ -> Hhas_method.t =
  fun ast_class ast_method ->
  let method_is_abstract =
    List.mem ast_method.Ast.m_kind Ast.Abstract ||
    ast_class.Ast.c_kind = Ast.Cinterface in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_private = List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected = List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public =
    List.mem ast_method.Ast.m_kind Ast.Public ||
    (not method_is_private && not method_is_protected) in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let method_attributes =
    Emit_attribute.from_asts ast_method.Ast.m_user_attributes in
  let tparams = ast_class.Ast.c_tparams @ ast_method.Ast.m_tparams in
  let (_,class_name) = ast_class.Ast.c_name in
  let (_,method_name) = ast_method.Ast.m_name in
  let ret =
    if method_name = Naming_special_names.Members.__construct
    then None else ast_method.Ast.m_ret in
  let default_instrs return_type =
      if List.mem ast_method.Ast.m_kind Ast.Abstract
      then gather [
        instr_string ("Cannot call abstract method " ^ Utils.strip_ns class_name
          ^ "::" ^ method_name ^ "()");
        instr (IOp (Fatal FatalOp.RuntimeOmitFrame))
      ]
      else let default_seq =
        gather [
          instr_null;
          instr_retc
        ] in
        (* TODO: the following cannot use Emit_body.has_type_constraint
         *       because it would not follow the current HHVM behaviour *)
        match return_type with
        | None -> default_seq
        | Some x when x. Hhas_type_info.type_info_user_type = Some "" ->
          default_seq
        | _ -> gather [
            instr_null;
            instr_verifyRetTypeC;
            instr_retc
          ] in
  let body_instrs,
      method_decl_vars,
      method_num_iters,
      method_num_cls_ref_slots,
      method_params,
      method_return_type,
      method_is_generator,
      method_is_pair_generator =
    Emit_body.from_ast
      ~class_name:(Some class_name)
      ~method_name:(Some method_name)
      ~has_this:(not method_is_static)
      tparams
      ast_method.Ast.m_params
      ret
      ast_method.Ast.m_body
      default_instrs
  in
  let method_is_async =
    ast_method.Ast.m_fun_kind = Ast_defs.FAsync
    || ast_method.Ast.m_fun_kind = Ast_defs.FAsyncGenerator in
  let method_is_closure_body = snd ast_method.Ast.m_name = "__invoke" in
  let method_decl_vars =
    if method_is_closure_body
    then
      let vars = "$0Closure" :: method_decl_vars in
      if method_is_static then vars else vars @ ["$this"]
    else method_decl_vars in
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
    method_num_iters
    method_num_cls_ref_slots
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_asts ast_class ast_methods =
  List.map ast_methods (from_ast ast_class)
