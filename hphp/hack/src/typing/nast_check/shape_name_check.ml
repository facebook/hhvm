(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module ShapeSet = Ast_defs.ShapeSet

let get_pos name =
  match name with
  | Ast_defs.SFlit_int (pos, _)
  | Ast_defs.SFlit_str (pos, _)
  | Ast_defs.SFclass_const (_, (pos, _)) ->
    pos

let error_if_duplicate_names fdl =
  let _ =
    List.fold_left fdl ~init:ShapeSet.empty ~f:(fun seen (name, _) ->
        if ShapeSet.mem name seen then
          Errors.add_error
            Naming_error.(
              to_user_error @@ Field_name_already_bound (get_pos name));
        ShapeSet.add name seen)
  in
  ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr _ expr =
      match expr with
      | (_, _, Shape fdl) -> error_if_duplicate_names fdl
      | _ -> ()
  end
