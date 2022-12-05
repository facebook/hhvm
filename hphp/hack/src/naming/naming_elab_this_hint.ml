(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env : sig
  type t

  val empty : t

  val forbid_this : t -> bool

  val lsb : t -> bool option

  val set_forbid_this : t -> forbid_this:bool -> t

  val set_lsb : t -> lsb:bool option -> t
end = struct
  type t = {
    (* `this` is forbidden as a hint in this context *)
    forbid_this: bool;
    lsb: bool option;
  }

  let empty = { forbid_this = false; lsb = None }

  let forbid_this { forbid_this; _ } = forbid_this

  let lsb { lsb; _ } = lsb

  let set_forbid_this t ~forbid_this = { t with forbid_this }

  let set_lsb t ~lsb = { t with lsb }
end

let on_class_c_tparams (env, c_tparams, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:true in
  Naming_phase_pass.Cont.next (env, c_tparams, err_acc)

let on_class_c_extends (env, c_extends, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_extends, err_acc)

let on_class_c_uses (env, c_uses, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_uses, err_acc)

let on_class_c_xhp_attrs (env, c_xhp_attrs, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_xhp_attrs, err_acc)

let on_class_c_xhp_attr_uses (env, c_xhp_attr_uses, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_xhp_attr_uses, err_acc)

let on_class_c_reqs (env, c_reqs, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_reqs, err_acc)

let on_class_c_implements (env, c_implements, err_acc) =
  let env = Env.set_forbid_this env ~forbid_this:false in
  Naming_phase_pass.Cont.next (env, c_implements, err_acc)

let on_class_var
    (env, (Aast.{ cv_is_static; cv_user_attributes; _ } as cv), err_acc) =
  let lsb =
    if cv_is_static then
      Some
        (not @@ Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes)
    else
      None
  in
  let env = Env.set_lsb env ~lsb in
  Naming_phase_pass.Cont.next (env, cv, err_acc)

let on_class_var_cv_type (env, cv_type, err_acc) =
  let forbid_this = Option.value ~default:false @@ Env.lsb env in
  let env = Env.set_lsb ~lsb:None @@ Env.set_forbid_this ~forbid_this env in
  Naming_phase_pass.Cont.next (env, cv_type, err_acc)

let on_fun_f_ret (env, f_ret, err_acc) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Naming_phase_pass.Cont.next (env, f_ret, err_acc)

let on_expr_ (env, expr_, err_acc) =
  let env =
    match expr_ with
    | Aast.(Cast _ | Is _ | As _ | Upcast _) ->
      Env.(set_forbid_this empty ~forbid_this:false)
    | _ -> env
  in
  Naming_phase_pass.Cont.next (env, expr_, err_acc)

let on_hint (env, hint, err_acc) =
  match hint with
  | (pos, Aast.Hthis) when Env.forbid_this env ->
    let err = Err.naming @@ Naming_error.This_type_forbidden pos in
    Naming_phase_pass.Cont.finish (env, (pos, Aast.Herr), err)
  | _ -> Naming_phase_pass.Cont.next (env, hint, err_acc)

let on_shape_field_info (env, sfi, err_acc) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Naming_phase_pass.Cont.next (env, sfi, err_acc)

let on_hint_fun_hf_return_ty (env, hf_return_ty, err_acc) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Naming_phase_pass.Cont.next (env, hf_return_ty, err_acc)

let on_targ (env, targ, err_acc) =
  let env =
    Env.set_forbid_this ~forbid_this:false @@ Env.set_lsb ~lsb:None env
  in
  Naming_phase_pass.Cont.next (env, targ, err_acc)

let pass =
  Naming_phase_pass.(
    top_down
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
        on_hint = Some on_hint;
        on_shape_field_info = Some on_shape_field_info;
        on_hint_fun_hf_return_ty = Some on_hint_fun_hf_return_ty;
        on_targ = Some on_targ;
      })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
