(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names

let check_expr env (pos, e) =
  match e with
  | Class_const ((_, CIparent), (_, construct))
    when String.equal construct SN.Members.__construct ->
    let tenv = Env.tast_env_as_typing_env env in
    (match Typing_env.get_parent_class tenv with
    | Some parent_class
      when Ast_defs.(equal_class_kind (Cls.kind parent_class) Cabstract)
           && Option.is_none (fst (Cls.construct parent_class)) ->
      Errors.parent_abstract_call construct (fst pos) (Cls.pos parent_class)
    | _ -> ())
  | _ -> ()

let check_method_body env m =
  let named_body = m.m_body in
  if m.m_abstract && not (List.is_empty named_body.fb_ast) then
    Errors.abstract_with_body m.m_name;
  let tenv = Env.tast_env_as_typing_env env in
  if
    (not (Typing_env.is_decl tenv))
    && (not m.m_abstract)
    && List.is_empty named_body.fb_ast
  then
    Errors.not_abstract_without_body m.m_name

let check_class _ c =
  if Ast_defs.(equal_class_kind c.c_kind Cabstract) && c.c_final then (
    let err m =
      Errors.nonstatic_method_in_abstract_final_class (fst m.m_name)
    in
    let (c_constructor, _, c_methods) = split_methods c in
    List.iter c_methods err;
    Option.iter c_constructor err;

    let (_, c_instance_vars) = split_vars c in
    c_instance_vars
    |> List.filter ~f:(fun var -> Option.is_none var.cv_xhp_attr)
    |> List.iter ~f:(fun var ->
           Errors.instance_property_in_abstract_final_class (fst var.cv_id))
  )

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e = check_expr env e

    method! at_method_ env m = check_method_body env m

    method! at_class_ env c = check_class env c
  end
