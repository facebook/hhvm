(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      let env = Tast_env.tast_env_as_typing_env env in
      let is_mixed = Typing_utils.is_mixed env in
      let is_nothing = Typing_utils.is_nothing env in
      let pos_of_ty ~hole_pos ty =
        let current_decl_and_file = Typing_env.get_current_decl_and_file env in
        let ty_pos_opt =
          Pos_or_decl.fill_in_filename_if_in_current_decl
            ~current_decl_and_file
            (Typing_defs.get_pos ty)
        in
        match ty_pos_opt with
        | Some ty_pos -> ty_pos
        | None -> hole_pos
      in
      function
      | (_, _, Aast.Hole ((exp_ty, hole_pos, _), src_ty, _, Aast.UnsafeCast _))
        when is_mixed src_ty && not (is_mixed exp_ty) ->
        let ty_str = Typing_print.debug env exp_ty in
        let ty_str_opt =
          if String.length ty_str <= 20 then
            Some ty_str
          else
            None
        in
        Lints_errors.loose_unsafe_cast_lower_bound
          (pos_of_ty ~hole_pos src_ty)
          ty_str_opt
      | (_, _, Aast.Hole ((_, hole_pos, _), _, dst_ty, Aast.UnsafeCast _))
        when is_nothing dst_ty ->
        Lints_errors.loose_unsafe_cast_upper_bound (pos_of_ty ~hole_pos dst_ty)
      | _ -> ()
  end
