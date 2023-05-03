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
  | _ ->
    Errors.add_error
      Nast_check_error.(to_user_error @@ Abstract_body (fst m.m_name))

let check_interface c =
  List.iter c.c_uses ~f:(fun (p, _) ->
      Errors.add_error
        Nast_check_error.(to_user_error @@ Interface_uses_trait p));

  let (statics, vars) = split_vars c.c_vars in
  begin
    match vars with
    | hd :: _ ->
      let pos = fst hd.cv_id in
      Errors.add_error
        Nast_check_error.(to_user_error @@ Interface_with_member_variable pos)
    | _ -> ()
  end;

  begin
    match statics with
    | hd :: _ ->
      let pos = fst hd.cv_id in
      Errors.add_error
        Nast_check_error.(
          to_user_error @@ Interface_with_static_member_variable pos)
    | _ -> ()
  end;

  (* make sure that interfaces only have empty public methods *)
  List.iter ~f:enforce_no_body c.c_methods

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ c =
      if Ast_defs.is_c_interface c.c_kind then check_interface c
  end
