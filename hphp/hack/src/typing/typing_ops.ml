(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Typing_defs
module Reason = Typing_reason

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

let log_sub_type env p ty_sub ty_super =
  let level =
    if
      not
        (Typing_utils.is_capability_i ty_sub
        || Typing_utils.is_capability_i ty_super)
    then
      1
    else
      3
  in
  Typing_log.(
    log_with_level env "sub" ~level (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos p)
          env
          [
            Log_head
              ( "Typing_ops.sub_type",
                [
                  Log_type_i ("ty_sub", ty_sub);
                  Log_type_i ("ty_super", ty_super);
                ] );
          ]))

let sub_type_i_apply_reason_callback
    ?(is_coeffect = false) p env ty_sub ty_super callback =
  log_sub_type env p ty_sub ty_super;
  Typing_utils.sub_type_i ~is_coeffect env ty_sub ty_super @@ Some callback

(** Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type_i
    ?(is_coeffect = false)
    p
    ur
    env
    ty_sub
    ty_super
    (on_error : Typing_error.Callback.t) =
  sub_type_i_apply_reason_callback
    ~is_coeffect
    p
    env
    ty_sub
    ty_super
    (Typing_error.Reasons_callback.with_claim
       on_error
       ~claim:(lazy (p, Reason.string_of_ureason ur)))

(** Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type_i_w_err_prefix
    ?(is_coeffect = false) p env ty_sub ty_super err_prefix =
  sub_type_i_apply_reason_callback
    ~is_coeffect
    p
    env
    ty_sub
    ty_super
    (Typing_error.Reasons_callback.Prefix err_prefix)

let sub_type p ur env ty_sub ty_super on_error =
  sub_type_i p ur env (LoclType ty_sub) (LoclType ty_super) on_error

let sub_type_w_err_prefix
    ?(is_coeffect = false)
    p
    env
    ty_sub
    ty_super
    (err_prefix : Typing_error.Primary.t) =
  sub_type_i_w_err_prefix
    ~is_coeffect
    p
    env
    (LoclType ty_sub)
    (LoclType ty_super)
    err_prefix

let sub_type_decl ?(is_coeffect = false) ~on_error p ur env ty_sub ty_super =
  let localize_no_subst = Typing_utils.localize_no_subst ~ignore_errors:true in
  let ((env, ty_err1), ty_super) = localize_no_subst env ty_super in
  let ((env, ty_err2), ty_sub) = localize_no_subst env ty_sub in
  let (env, ty_err3) =
    Typing_utils.sub_type env ~is_coeffect ty_sub ty_super
    @@ Some
         (Typing_error.Reasons_callback.prepend_reason
            on_error
            ~reason:(lazy (p, Reason.string_of_ureason ur)))
  in
  let ty_err_opt =
    Typing_error.multiple_opt
    @@ List.filter_map ~f:Fn.id [ty_err1; ty_err2; ty_err3]
  in
  (env, ty_err_opt)

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env on_error ty1 ty2 =
  let localize_no_subst = Typing_utils.localize_no_subst ~ignore_errors:true in
  let ((env, ty_err1), ty1) = localize_no_subst env ty1 in
  let ((env, ty_err2), ty2) = localize_no_subst env ty2 in
  let reason = lazy (p, Reason.string_of_ureason ur) in
  let (env, ty_err3) =
    Typing_utils.sub_type env ty2 ty1
    @@ Some (Typing_error.Reasons_callback.prepend_reason on_error ~reason)
  in
  let (env, ty_err4) =
    Typing_utils.sub_type env ty1 ty2
    @@ Some (Typing_error.Reasons_callback.prepend_reason on_error ~reason)
  in
  let ty_err_opt =
    Typing_error.multiple_opt
    @@ List.filter_map ~f:Fn.id [ty_err1; ty_err2; ty_err3; ty_err4]
  in
  (env, ty_err_opt)
