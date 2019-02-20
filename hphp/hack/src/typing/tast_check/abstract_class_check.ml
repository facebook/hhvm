(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Tast
open Core_kernel

module Env = Tast_env
module Cls = Typing_classes_heap
module SN = Naming_special_names

let check_expr env (pos, e) =
  match e with
  | Class_const ((_, CIparent), ((_, construct)))
    when construct = SN.Members.__construct ->
    let tenv = Env.tast_env_as_typing_env env in
    begin match Env.get_class env (Typing_env.get_parent_id tenv) with
    | Some parent_class when Cls.kind parent_class = Ast.Cabstract ->
      if fst (Cls.construct parent_class) = None then
        Errors.parent_abstract_call construct (fst pos) (Cls.pos parent_class);
    | _ -> ()
    end
  | _ -> ()

let check_method_body env m =
  let named_body = m.m_body in
  if m.m_abstract && named_body.fb_ast <> []
  then Errors.abstract_with_body m.m_name;
  let tenv = Env.tast_env_as_typing_env env in
  if not (Typing_env.is_decl tenv) && not m.m_abstract && named_body.fb_ast = []
  then Errors.not_abstract_without_body m.m_name

let check_class _ c =
  if c.c_kind = Ast.Cabstract && c.c_final then begin
    let err m =
      Errors.nonstatic_method_in_abstract_final_class (fst m.m_name) in
    List.iter c.c_methods err;
    Option.iter c.c_constructor err
  end

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env e = check_expr env e

  method! at_method_ env m = check_method_body env m

  method! at_class_ env c = check_class env c
end
