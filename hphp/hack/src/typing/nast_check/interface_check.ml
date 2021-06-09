(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let enforce_no_body m =
  match m.m_body.fb_ast with
  | [] -> ()
  | _ -> Errors.abstract_body (fst m.m_name)

let check_interface c =
  List.iter c.c_uses ~f:(fun (p, _) -> Errors.interface_use_trait p);

  let (statics, vars) = split_vars c in
  begin
    match vars with
    | hd :: _ ->
      let pos = fst hd.cv_id in
      Errors.interface_with_member_variable pos
    | _ -> ()
  end;

  begin
    match statics with
    | hd :: _ ->
      let pos = fst hd.cv_id in
      Errors.interface_with_static_member_variable pos
    | _ -> ()
  end;

  (* make sure interfaces do not contain partially abstract type constants *)
  List.iter c.c_typeconsts ~f:(fun tc ->
      match tc.c_tconst_kind with
      | TCPartiallyAbstract _ ->
        Errors.interface_with_partial_typeconst (fst tc.c_tconst_name)
      | _ -> ());

  (* make sure that interfaces only have empty public methods *)
  List.iter ~f:enforce_no_body c.c_methods

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ c =
      if Ast_defs.(equal_class_kind c.c_kind Cinterface) then check_interface c
  end
