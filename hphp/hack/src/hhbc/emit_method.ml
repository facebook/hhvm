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

let from_ast_wrapper : bool -> _ ->
  Ast.class_ -> Ast.method_ -> Hhas_method.t =
  fun privatize make_name ast_class ast_method ->
  let method_is_abstract =
    List.mem ast_method.Ast.m_kind Ast.Abstract ||
    ast_class.Ast.c_kind = Ast.Cinterface in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let namespace = ast_class.Ast.c_namespace in
  let method_attributes =
    Emit_attribute.from_asts namespace ast_method.Ast.m_user_attributes in
  let method_is_private =
    privatize || List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected =
    not privatize && List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public =
    not privatize && (
    List.mem ast_method.Ast.m_kind Ast.Public ||
    (not method_is_private && not method_is_protected)) in
  let is_memoize =
    Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let (pos, original_name) = ast_method.Ast.m_name in
  let (_,class_name) = ast_class.Ast.c_name in
  let class_name = Utils.strip_ns class_name in
  let ret = ast_method.Ast.m_ret in
  let method_id = make_name ast_method.Ast.m_name in
  let method_is_async =
    ast_method.Ast.m_fun_kind = Ast_defs.FAsync
    || ast_method.Ast.m_fun_kind = Ast_defs.FAsyncGenerator in
  if ast_class.Ast.c_kind = Ast.Cinterface && not (List.is_empty ast_method.Ast.m_body)
  then Emit_fatal.raise_fatal_parse pos
    (Printf.sprintf "Interface method %s::%s cannot contain body" class_name original_name);
  if not method_is_static
    && ast_class.Ast.c_final
    && ast_class.Ast.c_kind = Ast.Cabstract
  then Emit_fatal.raise_fatal_parse pos
    ("Class " ^ class_name ^ " contains non-static method " ^ original_name
     ^ " and therefore cannot be declared 'abstract final'");
  let default_dropthrough =
    if List.mem ast_method.Ast.m_kind Ast.Abstract
    then Some (Emit_fatal.emit_fatal_runtimeomitframe pos
      ("Cannot call abstract method " ^ class_name
        ^ "::" ^ original_name ^ "()"))
    else None
  in
  let scope =
    [Ast_scope.ScopeItem.Method ast_method;
     Ast_scope.ScopeItem.Class ast_class] in
  (* TODO: use something that can't be faked in user code *)
  let method_is_closure_body =
   original_name = "__invoke"
   && String_utils.string_starts_with class_name "Closure$" in
  let scope =
    if method_is_closure_body
    then Ast_scope.ScopeItem.Lambda :: scope else scope in
  let method_body, method_is_generator, method_is_pair_generator =
    Emit_body.emit_body
      ~pos:ast_method.Ast.m_span
      ~scope:scope
      ~is_closure_body:method_is_closure_body
      ~is_memoize
      ~is_async:method_is_async
      ~skipawaitable:(ast_method.Ast.m_fun_kind = Ast_defs.FAsync)
      ~is_return_by_ref:ast_method.Ast.m_ret_by_ref
      ~default_dropthrough
      ~return_value:instr_null
      ~namespace
      ~doc_comment:ast_method.Ast.m_doc_comment
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
        let cvl = List.filter cvl ~f:(fun (_, (_, id), _) ->
          (String.length id) < 9 || (String.sub id 0 9) <> "86static_") in
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
    false (*method_no_injection*)
    method_id
    method_body
    (Hhas_pos.pos_to_span ast_method.Ast.m_span)
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body

let from_ast ast_class ast_method =
  let is_memoize = Emit_attribute.ast_any_is_memoize ast_method.Ast.m_user_attributes in
  let make_name (_, name) =
    if is_memoize
    then Hhbc_id.Method.from_ast_name (name ^ Emit_memoize_helpers.memoize_suffix)
    else Hhbc_id.Method.from_ast_name name
  in
  from_ast_wrapper is_memoize make_name ast_class ast_method


let from_asts ast_class ast_methods =
  List.map ast_methods (from_ast ast_class)
