(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast

module SN = Naming_special_names

let enforce_no_body m =
  match m.m_body.fb_ast with
  | [] ->
    if m.m_visibility = Private
    then Errors.not_public_or_protected_interface (fst m.m_name)
  | _ -> Errors.abstract_body (fst m.m_name)

(* make sure that interface methods are not async, in line with HHVM *)
let enforce_not_async m =
  match m.m_fun_kind with
  | Ast.FAsync -> Errors.async_in_interface (fst m.m_name)
  | Ast.FAsyncGenerator -> Errors.async_in_interface (fst m.m_name)
  | _ -> ()

let check_interface c =
  List.iter c.c_uses (fun (p, _) -> Errors.interface_use_trait p);

  begin match c.c_vars with
  | hd::_ ->
    let pos = fst hd.cv_id in
    Errors.interface_with_member_variable pos
  | _ -> ()
  end;

  begin match c.c_static_vars with
  | hd::_ ->
    let pos = fst hd.cv_id in
    Errors.interface_with_static_member_variable pos
  | _ -> ()
  end;

  (* make sure interfaces do not contain partially abstract type constants *)
  List.iter c.c_typeconsts begin fun tc ->
    if tc.c_tconst_constraint <> None && tc.c_tconst_type <> None then
      Errors.interface_with_partial_typeconst (fst tc.c_tconst_name)
  end;

  (* make sure that interfaces only have empty public methods *)
  List.iter (c.c_static_methods @ c.c_methods) enforce_no_body;
  List.iter (c.c_static_methods @ c.c_methods) enforce_not_async;

  (* make sure constructor has no body *)
  Option.iter c.c_constructor enforce_no_body;
  Option.iter c.c_constructor enforce_not_async


let handler = object
  inherit Nast_visitor.handler_base

  method! at_class_ _ c =
    if c.c_kind = Ast.Cinterface then check_interface c

end
