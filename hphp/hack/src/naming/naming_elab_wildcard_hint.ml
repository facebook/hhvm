(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let incr_tp_depth t =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with tp_depth = elab_wildcard_hint.tp_depth + 1 }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let reset_tp_depth t =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with tp_depth = 0 }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let allow_wildcard
      Naming_phase_env.
        { elab_wildcard_hint = Elab_wildcard_hint.{ allow_wildcard; _ }; _ } =
    allow_wildcard

  let set_allow_wildcard t ~allow_wildcard =
    let elab_wildcard_hint = t.Naming_phase_env.elab_wildcard_hint in
    let elab_wildcard_hint =
      Naming_phase_env.Elab_wildcard_hint.
        { elab_wildcard_hint with allow_wildcard }
    in
    Naming_phase_env.{ t with elab_wildcard_hint }

  let tp_depth
      Naming_phase_env.
        { elab_wildcard_hint = Elab_wildcard_hint.{ tp_depth; _ }; _ } =
    tp_depth
end

let on_expr_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.expr_ ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.expr_, _) result =
 fun (env, expr_) ->
  let env =
    match expr_ with
    | Aast.Cast _ -> Env.incr_tp_depth env
    | Aast.(Is _ | As _) -> Env.set_allow_wildcard env ~allow_wildcard:true
    | Aast.Upcast _ -> Env.set_allow_wildcard env ~allow_wildcard:false
    | _ -> env
  in
  Ok (env, expr_)

let on_targ (env, targ) =
  Ok (Env.set_allow_wildcard ~allow_wildcard:true @@ Env.incr_tp_depth env, targ)

let on_hint_ (env, hint_) =
  let env =
    match hint_ with
    | Aast.(
        ( Hunion _ | Hintersection _ | Hoption _ | Hlike _ | Hsoft _
        | Hrefinement _ )) ->
      Env.reset_tp_depth env
    | Aast.(Htuple _ | Happly _ | Habstr _ | Hvec_or_dict _) ->
      Env.incr_tp_depth env
    | _ -> env
  in
  Ok (env, hint_)

let on_shape_field_info (env, sfi) = Ok (Env.incr_tp_depth env, sfi)

let on_context on_error (env, hint) =
  match hint with
  | (pos, Aast.Happly ((_, tycon_name), _))
    when String.equal tycon_name SN.Typehints.wildcard ->
    on_error
      (Naming_phase_error.naming @@ Naming_error.Invalid_wildcard_context pos);
    Error (env, (pos, Aast.Herr))
  | _ -> Ok (env, hint)

let on_hint on_error (env, hint) =
  match hint with
  | (pos, Aast.Happly ((_, tycon_name), hints))
    when String.equal tycon_name SN.Typehints.wildcard ->
    if Env.(allow_wildcard env && tp_depth env >= 1) (* prevents 3 as _ *) then
      if not (List.is_empty hints) then (
        let err =
          Naming_phase_error.naming
          @@ Naming_error.Tparam_applied_to_type
               { pos; tparam_name = SN.Typehints.wildcard }
        in
        on_error err;
        Ok (env, (pos, Aast.Herr))
      ) else
        Ok (env, hint)
    else
      let err =
        Naming_phase_error.naming @@ Naming_error.Wildcard_hint_disallowed pos
      in
      on_error err;
      Ok (env, (pos, Aast.Herr))
  | _ -> Ok (env, hint)

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_expr_ = Some on_expr_;
          on_targ = Some on_targ;
          on_hint_ = Some on_hint_;
          on_shape_field_info = Some on_shape_field_info;
          on_context = Some (on_context on_error);
          on_hint = Some (on_hint on_error);
        })
