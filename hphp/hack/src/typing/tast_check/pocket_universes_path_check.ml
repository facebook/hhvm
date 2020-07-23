(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* open Core_kernel *)
open Aast
open Hh_prelude
module Env = Tast_env
module Cls = Decl_provider.Class

(** Pocket Universes are an ongoing research project.  To enable
  * experimentation, the pocket universe syntax can be enabled
  * via the .hhconfig option:
  *  pocket_universe_enabled_paths = nowhere|everywhere|dir1,..,dirn
  * option in the .hhconfig file.  If omitted, it defaults to
  * nowhere.  The directories dir1,..,dirn are relative to the
  * path of the .hhconfig file.
  *)

let is_in_path path file =
  if
    not
      (Relative_path.equal_prefix
         (Relative_path.prefix path)
         (Relative_path.prefix file))
  then
    false
  else
    String.is_prefix
      ~prefix:(Relative_path.suffix path)
      (Relative_path.suffix file)

let inside_allowed_directory env file =
  let (everywhere, paths) =
    GlobalOptions.tco_pu_enabled_paths (Tast_env.get_tcopt env)
  in
  if everywhere then
    true
  else
    List.exists ~f:(fun path -> is_in_path path file) paths

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_user_attribute env { ua_name = (pos, name); _ } =
      if String.equal name Naming_special_names.UserAttributes.uaPu then
        if not (inside_allowed_directory env (Pos.filename pos)) then
          Errors.pu_reserved_syntax pos

    method! at_hint env (pos, hint_) =
      match hint_ with
      | Hpu_access _ (* (_,_,_) *) ->
        if not (inside_allowed_directory env (Pos.filename pos)) then
          Errors.pu_reserved_syntax pos
      | _ -> ()

    method! at_expr env (pos, expr) =
      match expr with
      | PU_atom _
      | PU_identifier _ ->
        if not (inside_allowed_directory env (Pos.filename (fst pos))) then
          Errors.pu_reserved_syntax (fst pos)
      | _ -> ()

    method! at_class_ env c =
      match c.c_pu_enums with
      | [] -> ()
      | _ ->
        if not (inside_allowed_directory env (Pos.filename (fst c.c_name))) then
          List.iter
            ~f:(fun e -> Errors.pu_reserved_syntax (fst e.pu_name))
            c.c_pu_enums
  end
