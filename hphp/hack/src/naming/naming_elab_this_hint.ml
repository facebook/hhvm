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

module Env = struct
  type t = {
    (* `this` is forbidden as a hint in this context *)
    forbid_this: bool;
    lsb: bool option;
  }

  let empty = { forbid_this = false; lsb = None }

  let create ?(forbid_this = false) ?lsb () = { forbid_this; lsb }
end

let visitor =
  object (_self)
    inherit [_] Naming_visitors.mapreduce as super

    (* -- Classes ----------------------------------------------------------- *)
    method! on_class_c_tparams env c_tparams =
      let env = Env.{ env with forbid_this = true } in
      super#on_class_c_tparams env c_tparams

    method! on_class_c_extends env c_extends =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_extends env c_extends

    method! on_class_c_uses env c_uses =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_uses env c_uses

    method! on_class_c_xhp_attrs env c_xhp_attrs =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_xhp_attrs env c_xhp_attrs

    method! on_class_c_xhp_attr_uses env c_xhp_attr_uses =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_xhp_attr_uses env c_xhp_attr_uses

    method! on_class_c_reqs env c_reqs =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_reqs env c_reqs

    method! on_class_c_implements env c_implements =
      let env = Env.{ env with forbid_this = false } in
      super#on_class_c_implements env c_implements

    method! on_class_var
        env (Aast.{ cv_is_static; cv_user_attributes; _ } as cv) =
      let lsb =
        if cv_is_static then
          Some
            (not
            @@ Naming_attributes.mem SN.UserAttributes.uaLSB cv_user_attributes
            )
        else
          None
      in
      let env = Env.{ env with lsb } in
      super#on_class_var env cv

    method! on_class_var_cv_type Env.{ lsb; _ } cv_type =
      let forbid_this = Option.value ~default:false lsb in
      let env = Env.create ~forbid_this () in
      super#on_class_var_cv_type env cv_type

    (* -- Functions --------------------------------------------------------- *)

    method! on_fun_f_ret _env f_ret =
      let env = Env.create ~forbid_this:false () in
      super#on_fun_f_ret env f_ret

    (* -- Expressions ------------------------------------------------------- *)

    method! on_Cast _env hint expr =
      let env = Env.create ~forbid_this:false () in
      super#on_Cast env hint expr

    method! on_Is _env expr hint =
      let env = Env.create ~forbid_this:false () in
      super#on_Is env expr hint

    method! on_As _env expr hint is_final =
      let env = Env.create ~forbid_this:false () in
      super#on_As env expr hint is_final

    method! on_Upcast _env expr hint =
      let env = Env.create ~forbid_this:false () in
      super#on_Upcast env expr hint

    (* -- Hints   ----------------------------------------------------------- *)

    method! on_hint env hint =
      match hint with
      | (pos, Aast.Hthis) when env.Env.forbid_this ->
        let err = Err.naming @@ Naming_error.This_type_forbidden pos in
        ((pos, Aast.Herr), err)
      | _ -> super#on_hint env hint

    method! on_shape_field_info _env sfi =
      let env = Env.create ~forbid_this:false () in
      super#on_shape_field_info env sfi

    method! on_hint_fun_hf_return_ty _env hf_return_ty =
      let env = Env.create ~forbid_this:false () in
      super#on_hint_fun_hf_return_ty env hf_return_ty

    method! on_targ _env targ =
      let env = Env.create ~forbid_this:false () in
      super#on_targ env targ
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
