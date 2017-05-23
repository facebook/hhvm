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
  let (_,class_name) = ast_class.Ast.c_name in
  let (_,method_name) = ast_method.Ast.m_name in
  let ret =
    if method_name = Naming_special_names.Members.__construct
    || method_name = Naming_special_names.Members.__destruct
    then None
    else ast_method.Ast.m_ret in
  let method_id = Hhbc_id.Method.from_ast_name method_name in
  let method_is_async =
    ast_method.Ast.m_fun_kind = Ast_defs.FAsync
    || ast_method.Ast.m_fun_kind = Ast_defs.FAsyncGenerator in
  let default_dropthrough =
    if List.mem ast_method.Ast.m_kind Ast.Abstract
    then Some (gather [
      instr_string ("Cannot call abstract method " ^ Utils.strip_ns class_name
        ^ "::" ^ method_name ^ "()");
      instr (IOp (Fatal FatalOp.RuntimeOmitFrame))
    ])
    else
    if method_is_async
    then Some (gather [instr_null; instr_retc])
    else None in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  (* TODO: use something that can't be faked in user code *)
  let method_is_closure_body =
   snd ast_method.Ast.m_name = "__invoke"
   && String_utils.string_starts_with (snd ast_class.Ast.c_name) "Closure$" in
  let scope =
    if method_is_closure_body
    then Ast_scope.ScopeItem.Lambda :: scope else scope in
  let namespace = ast_class.Ast.c_namespace in
  let method_body, method_is_generator, method_is_pair_generator =
    Emit_body.emit_body
      ~scope:scope
      ~skipawaitable:(ast_method.Ast.m_fun_kind = Ast_defs.FAsync)
      ~is_closure_body:method_is_closure_body
      ~is_memoize_wrapper:false
      ~is_return_by_ref:ast_method.Ast.m_ret_by_ref
      ~default_dropthrough
      ~return_value:instr_null
      ~namespace
      ast_method.Ast.m_params
      ret
      [Ast.Stmt (Ast.Block ast_method.Ast.m_body)]
  in
  (* Horrible hack to get decl_vars in the same order as HHVM *)
  let captured_vars =
    if method_is_closure_body
    then List.concat_map ast_class.Ast.c_body (fun item ->
      match item with
      | Ast.ClassVars(_, _, cvl) ->
        List.map cvl (fun (_, (_,id), _) -> "$" ^ id)
      | _ -> []
      )
    else [] in
  let remove_this vars =
    List.filter vars (fun s -> s <> "$this") in
  let move_this vars =
    if List.mem vars "$this"
    then remove_this vars @ ["$this"]
    else vars in
  let method_decl_vars = Hhas_body.decl_vars method_body in
  let method_decl_vars =
    if method_is_closure_body
    then
      let method_decl_vars = move_this method_decl_vars in
      "$0Closure" :: captured_vars @
      List.filter method_decl_vars (fun v -> not (List.mem captured_vars v))
    else move_this method_decl_vars in
  let method_body = Hhas_body.with_decl_vars method_body method_decl_vars in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    method_id
    method_body
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_asts ast_class ast_methods =
  List.map ast_methods (from_ast ast_class)
