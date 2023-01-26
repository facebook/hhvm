(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

let on_expr_ on_error =
  let handler
        : 'a 'b.
          _ * ('a, 'b) Aast.expr_ ->
          (_ * ('a, 'b) Aast.expr_, _ * ('a, 'b) Aast.expr_) result =
   fun (env, expr_) ->
    let err_opt =
      match expr_ with
      | Aast.(Cast ((_, Hprim (Tint | Tbool | Tfloat | Tstring)), _)) -> None
      | Aast.(Cast ((_, Happly ((_, tycon_nm), _)), _))
        when String.(
               equal tycon_nm SN.Collections.cDict
               || equal tycon_nm SN.Collections.cVec) ->
        None
      | Aast.(Cast ((_, Aast.Hvec_or_dict (_, _)), _)) -> None
      | Aast.(Cast ((_, Aast.Hany), _)) ->
        (* We end up with a `Hany` when we have an arity error for dict/vec
           - we don't error on this case to preserve behaviour
        *)
        None
      | Aast.(Cast ((pos, _), _)) ->
        Some (Naming_phase_error.naming @@ Naming_error.Object_cast pos)
      | _ -> None
    in
    Option.iter ~f:on_error err_opt;
    Ok (env, expr_)
  in
  handler

let pass on_error =
  Naming_phase_pass.(
    bottom_up
      Ast_transform.{ identity with on_expr_ = Some (on_expr_ on_error) })
