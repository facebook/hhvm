(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(* We use the same namespace as constants within the class so we cannot have
 * a const and type const with the same name
 *)
let error_if_repeated_name member_or_const =
  let seen = Stdlib.Hashtbl.create 0 in
  List.iter member_or_const ~f:(fun (pos, name) ->
      match Stdlib.Hashtbl.find_opt seen name with
      | Some p' ->
        Errors.add_error
          Naming_error.(
            to_user_error
            @@ Error_name_already_bound { pos; name; prev_pos = p' })
      | None -> Stdlib.Hashtbl.replace seen name pos)

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ _ c =
      let typeconst_names =
        List.map c.c_typeconsts ~f:(fun tc -> tc.c_tconst_name)
      in
      let const_names = List.map c.c_consts ~f:(fun cc -> cc.cc_id) in
      let combined_names = List.append typeconst_names const_names in
      error_if_repeated_name combined_names;
      ()
  end
