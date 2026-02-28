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
  | Ast_defs.SFlit_str (pos, _)
  | Ast_defs.SFclassname (pos, _)
  | Ast_defs.SFclass_const (_, (pos, _)) ->
    pos

let error_if_duplicate_names fdl custom_err_config =
  let _ =
    List.fold_left fdl ~init:ShapeSet.empty ~f:(fun seen (name, _) ->
        if ShapeSet.mem name seen then
          Diagnostics.add_diagnostic
            (Naming_error_utils.to_user_diagnostic
               (Naming_error.Field_name_already_bound (get_pos name))
               custom_err_config);
        ShapeSet.add name seen)
  in
  ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env expr =
      match expr with
      | (_, _, Shape fdl) ->
        let custom_err_config = Nast_check_env.get_custom_error_config env in
        error_if_duplicate_names fdl custom_err_config
      | _ -> ()
  end
