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

let check_expr env (_, pos, e) =
  match e with
  | Class_const ((_, _, CIparent), (_, construct))
    when String.equal construct SN.Members.__construct ->
    let tenv = Env.tast_env_as_typing_env env in
    (match Typing_env.get_parent_class tenv with
    | Decl_entry.Found parent_class
      when Ast_defs.is_c_abstract (Cls.kind parent_class)
           && Option.is_none (fst (Cls.construct parent_class)) ->
      Typing_error_utils.add_typing_error
        ~env:(Env.tast_env_as_typing_env env)
        Typing_error.(
          primary
          @@ Primary.Parent_abstract_call
               { meth_name = construct; pos; decl_pos = Cls.pos parent_class })
    | _ -> ())
  | _ -> ()

let check_method_body m =
  let named_body = m.m_body in
  if m.m_abstract && not (List.is_empty named_body.fb_ast) then
    Errors.add_error
      Nast_check_error.(to_user_error @@ Abstract_with_body (fst m.m_name))

let check_class _ c =
  if Ast_defs.is_c_abstract c.c_kind && c.c_final then (
    let err m =
      Errors.add_error
        Nast_check_error.(
          to_user_error
          @@ Nonstatic_method_in_abstract_final_class (fst m.m_name))
    in
    let (c_constructor, _, c_methods) = split_methods c.c_methods in
    List.iter c_methods ~f:err;
    Option.iter c_constructor ~f:err;

    let (_, c_instance_vars) = split_vars c.c_vars in
    c_instance_vars
    |> List.filter ~f:(fun var -> Option.is_none var.cv_xhp_attr)
    |> List.iter ~f:(fun var ->
           Errors.add_error
             Nast_check_error.(
               to_user_error
               @@ Instance_property_in_abstract_final_class (fst var.cv_id)))
  )

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e = check_expr env e

    method! at_method_ _ m = check_method_body m

    method! at_class_ env c = check_class env c
  end
