(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let like_type_hints_enabled Naming_phase_env.{ like_type_hints_enabled; _ } =
    like_type_hints_enabled

  let everything_sdt Naming_phase_env.{ everything_sdt; _ } = everything_sdt

  let allow_like
      Naming_phase_env.
        { validate_like_hint = Validate_like_hint.{ allow_like }; _ } =
    allow_like

  let set_allow_like t ~allow_like =
    Naming_phase_env.
      { t with validate_like_hint = Validate_like_hint.{ allow_like } }
end

let on_expr_ (env, expr_, err) =
  let env =
    match expr_ with
    | Aast.(Is _ | As _ | Upcast _) -> Env.set_allow_like env ~allow_like:true
    | _ -> env
  in
  Ok (env, expr_, err)

let on_hint (env, hint, err_acc) =
  let (err, env) =
    match hint with
    | (pos, Aast.Hlike _)
      when not
             (Env.allow_like env
             || Env.like_type_hints_enabled env
             || Env.everything_sdt env) ->
      (Naming_phase_error.like_type pos :: err_acc, env)
    | (_, Aast.(Hfun _ | Happly _ | Haccess _ | Habstr _ | Hvec_or_dict _)) ->
      (err_acc, Env.set_allow_like env ~allow_like:false)
    | _ -> (err_acc, env)
  in
  Ok (env, hint, err)

let pass =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        { identity with on_expr_ = Some on_expr_; on_hint = Some on_hint })
