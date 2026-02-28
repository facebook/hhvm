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
      | (_, pos, Aast.Cast (hint, (expr_ty, expr_pos, expr))) ->
        (* This check is implemented with pattern matching rather than
           subtyping because I don't want to accidentally error on unlawfully
           typed code (e.g., on TAnys). Luckily there are only four
           combinations to consider. *)
        let (env, expr_ty) = Tast_env.expand_type env expr_ty in
        begin
          let check () =
            let Equal = Tast_env.eq_typing_env in
            let cast =
              "(" ^ Typing_print.full ~hide_internals:true env expr_ty ^ ")"
            in
            let check_status = Tast_env.get_check_status env in
            let can_be_captured = Aast_utils.can_be_captured expr in
            Lints_diagnostics.redundant_cast
              ~can_be_captured
              ~check_status
              cast
              pos
              expr_pos
          in
          match (T.get_node expr_ty, snd hint) with
          | (T.Tprim expr_prim, A.Hprim hint_prim) -> begin
            match (expr_prim, hint_prim) with
            | (A.Tint, A.Tint)
            | (A.Tbool, A.Tbool)
            | (A.Tfloat, A.Tfloat) ->
              check ()
            | _ -> ()
          end
          | (T.Tclass ((_, id1), _, _), Happly ((_, id2), _))
            when String.equal Naming_special_names.Classes.cString id1
                 && String.equal Naming_special_names.Classes.cString id2 ->
            check ()
          | _ -> ()
        end
      | _ -> ()
  end
