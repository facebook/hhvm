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
  let forbid_this
      Naming_phase_env.{ elab_this_hint = Elab_this_hint.{ forbid_this; _ }; _ }
      =
    forbid_this

  let lsb Naming_phase_env.{ elab_this_hint = Elab_this_hint.{ lsb; _ }; _ } =
    lsb

  let set_forbid_this t ~forbid_this =
    let elab_this_hint = t.Naming_phase_env.elab_this_hint in
    let elab_this_hint =
      Naming_phase_env.Elab_this_hint.{ elab_this_hint with forbid_this }
    in
    Naming_phase_env.{ t with elab_this_hint }

  let set_lsb t ~lsb =
    let elab_this_hint = t.Naming_phase_env.elab_this_hint in
    let elab_this_hint =
      Naming_phase_env.Elab_this_hint.{ elab_this_hint with lsb }
    in
    Naming_phase_env.{ t with elab_this_hint }
end

let on_class_c_tparams (env, c_tparams) =
  let env = Env.set_forbid_this env ~forbid_this:true in
  Ok (env, c_tparams)

let on_class_c_extends (env, c_extends) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_extends)

let on_class_c_uses (env, c_uses) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_uses)

let on_class_c_xhp_attrs (env, c_xhp_attrs) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_xhp_attrs)

let on_class_c_xhp_attr_uses (env, c_xhp_attr_uses) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_xhp_attr_uses)

let on_class_c_reqs (env, c_reqs) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_reqs)

let on_class_c_implements (env, c_implements) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Ok (env, c_implements)

let on_class_var :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.class_var ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.class_var, _) result =
 fun (env, (Aast.{ cv_is_static; cv_user_attributes; _ } as cv)) ->
  let lsb =
    if cv_is_static then
      Some
        (not @@ Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes)
    else
      None
  in
  let env = Env.set_lsb env ~lsb in
  Ok (env, cv)

let on_class_var_cv_type (env, cv_type) =
  let forbid_this = Option.value ~default:false @@ Env.lsb env in
  let env = Env.set_lsb ~lsb:None @@ Env.set_forbid_this ~forbid_this env in
  Ok (env, cv_type)

let on_fun_f_ret (env, f_ret) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Ok (env, f_ret)

let on_expr_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.expr_ ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.expr_, _) result =
 fun (env, expr_) ->
  let env =
    match expr_ with
    | Aast.(Cast _ | Is _ | As _ | Upcast _) ->
      Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
    | _ -> env
  in
  Ok (env, expr_)

let on_hint on_error (env, hint) =
  match hint with
  | (pos, Aast.Hthis) when Env.forbid_this env ->
    on_error (Naming_phase_error.naming @@ Naming_error.This_type_forbidden pos);
    Ok (env, (pos, Aast.Herr))
  | _ -> Ok (env, hint)

let on_shape_field_info (env, sfi) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Ok (env, sfi)

let on_hint_fun_hf_return_ty (env, hf_return_ty) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Ok (env, hf_return_ty)

let on_targ (env, targ) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Ok (env, targ)

let pass on_error =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_class_c_tparams = Some on_class_c_tparams;
          on_class_c_extends = Some on_class_c_extends;
          on_class_c_uses = Some on_class_c_uses;
          on_class_c_xhp_attrs = Some on_class_c_xhp_attrs;
          on_class_c_xhp_attr_uses = Some on_class_c_xhp_attr_uses;
          on_class_c_reqs = Some on_class_c_reqs;
          on_class_c_implements = Some on_class_c_implements;
          on_class_var = Some on_class_var;
          on_class_var_cv_type = Some on_class_var_cv_type;
          on_fun_f_ret = Some on_fun_f_ret;
          on_expr_ = Some on_expr_;
          on_hint = Some (on_hint on_error);
          on_shape_field_info = Some on_shape_field_info;
          on_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
          on_targ = Some on_targ;
        })
