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

let on_class_c_tparams c_tparams ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:true in
  (ctx, Ok c_tparams)

let on_class_c_extends c_extends ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_extends)

let on_class_c_uses c_uses ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_uses)

let on_class_c_xhp_attrs c_xhp_attrs ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_xhp_attrs)

let on_class_c_xhp_attr_uses c_xhp_attr_uses ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_xhp_attr_uses)

let on_class_c_reqs c_reqs ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_reqs)

let on_class_c_implements c_implements ~ctx =
  let ctx = Env.set_forbid_this ctx ~forbid_this:false in
  (ctx, Ok c_implements)

let on_class_var (Aast.{ cv_is_static; cv_user_attributes; _ } as cv) ~ctx =
  let lsb =
    if cv_is_static then
      Some
        (not @@ Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes)
    else
      None
  in
  let ctx = Env.set_lsb ctx ~lsb in
  (ctx, Ok cv)

let on_class_var_cv_type cv_type ~ctx =
  let forbid_this = Option.value ~default:false @@ Env.lsb ctx in
  let ctx = Env.set_lsb ~lsb:None @@ Env.set_forbid_this ~forbid_this ctx in
  (ctx, Ok cv_type)

let on_fun_f_ret f_ret ~ctx =
  let ctx =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None ctx
  in
  (ctx, Ok f_ret)

let on_expr_ expr_ ~ctx =
  let ctx =
    match expr_ with
    | Aast.(Cast _ | Is _ | As _ | Upcast _) ->
      Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None ctx
    | _ -> ctx
  in
  (ctx, Ok expr_)

let on_hint on_error hint ~ctx =
  match hint with
  | (pos, Aast.Hthis) when Env.forbid_this ctx ->
    on_error (Naming_phase_error.naming @@ Naming_error.This_type_forbidden pos);
    (ctx, Ok (pos, Aast.Herr))
  | _ -> (ctx, Ok hint)

let on_shape_field_info sfi ~ctx =
  let ctx =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None ctx
  in
  (ctx, Ok sfi)

let on_hint_fun_hf_return_ty hf_return_ty ~ctx =
  let ctx =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None ctx
  in
  (ctx, Ok hf_return_ty)

let on_targ targ ~ctx =
  let ctx =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None ctx
  in
  (ctx, Ok targ)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_fld_class__c_tparams = Some on_class_c_tparams;
        on_fld_class__c_extends = Some on_class_c_extends;
        on_fld_class__c_uses = Some on_class_c_uses;
        on_fld_class__c_xhp_attrs = Some on_class_c_xhp_attrs;
        on_fld_class__c_xhp_attr_uses = Some on_class_c_xhp_attr_uses;
        on_fld_class__c_reqs = Some on_class_c_reqs;
        on_fld_class__c_implements = Some on_class_c_implements;
        on_ty_class_var = Some on_class_var;
        on_fld_class_var_cv_type = Some on_class_var_cv_type;
        on_fld_fun__f_ret = Some on_fun_f_ret;
        on_ty_expr_ = Some on_expr_;
        on_ty_hint = Some (on_hint on_error);
        on_ty_shape_field_info = Some on_shape_field_info;
        on_fld_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
        on_ty_targ = Some on_targ;
      }
