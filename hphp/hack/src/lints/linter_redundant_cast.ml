(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Typing_defs
module A = Aast_defs

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, pos, Aast.Cast (hint, (expr_ty, expr_pos, _))) ->
        (* This check is implemented with pattern matching rather than
           subtyping because I don't want to accidentally error on unlawfully
           typed code (e.g., on TAnys). Luckily there are only four
           combinations to consider. *)
        let (env, expr_ty) = Tast_env.expand_type env expr_ty in
        begin
          match (T.get_node expr_ty, snd hint) with
          | (T.Tprim expr_prim, A.Hprim hint_prim) -> begin
            match (expr_prim, hint_prim) with
            | (A.Tint, A.Tint)
            | (A.Tstring, A.Tstring)
            | (A.Tbool, A.Tbool)
            | (A.Tfloat, A.Tfloat) ->
              let typing_env = Tast_env.tast_env_as_typing_env env in
              let cast = "(" ^ Typing_print.full typing_env expr_ty ^ ")" in
              let check_status = Tast_env.get_check_status env in
              Lints_errors.redundant_cast ~check_status cast pos expr_pos
            | _ -> ()
          end
          | _ -> ()
        end
      | _ -> ()
  end
