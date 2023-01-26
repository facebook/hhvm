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

let on_expr_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.expr_ ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.expr_, _) result =
 fun (env, expr_) ->
  let env =
    match expr_ with
    | Aast.(Is _ | As _ | Upcast _) -> Env.set_allow_like env ~allow_like:true
    | _ -> env
  in
  Ok (env, expr_)

let on_hint on_error (env, hint) =
  let (err_opt, env) =
    match hint with
    | (pos, Aast.Hlike _)
      when not
             (Env.allow_like env
             || Env.like_type_hints_enabled env
             || Env.everything_sdt env) ->
      (Some (Naming_phase_error.like_type pos), env)
    | (_, Aast.(Hfun _ | Happly _ | Haccess _ | Habstr _ | Hvec_or_dict _)) ->
      (None, Env.set_allow_like env ~allow_like:false)
    | _ -> (None, env)
  in
  Option.iter ~f:on_error err_opt;
  Ok (env, hint)

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_expr_ = Some on_expr_;
          on_hint = Some (on_hint on_error);
        })
