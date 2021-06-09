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
  Typing_log.(
    log_with_level env "sub" ~level:1 (fun () ->
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

(* Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type_i
    ?(is_coeffect = false)
    p
    ur
    env
    ty_sub
    ty_super
    (on_error : Errors.typing_error_callback) =
  log_sub_type env p ty_sub ty_super;
  Typing_utils.sub_type_i ~is_coeffect env ty_sub ty_super (fun ?code reasons ->
      on_error ?code (p, Reason.string_of_ureason ur) reasons)

let sub_type_i_res
    p ur env ty_sub ty_super (on_error : Errors.typing_error_callback) =
  log_sub_type env p ty_sub ty_super;
  Typing_utils.sub_type_i_res env ty_sub ty_super (fun ?code reasons ->
      on_error ?code (p, Reason.string_of_ureason ur) reasons)

let sub_type p ur env ty_sub ty_super on_error =
  sub_type_i p ur env (LoclType ty_sub) (LoclType ty_super) on_error

let sub_type_res p ur env ty_sub ty_super on_error =
  sub_type_i_res p ur env (LoclType ty_sub) (LoclType ty_super) on_error

let sub_type_decl ?(is_coeffect = false) ~on_error p ur env ty_sub ty_super =
  let localize_no_subst = Typing_utils.localize_no_subst ~ignore_errors:true in
  let (env, ty_super) = localize_no_subst env ty_super in
  let (env, ty_sub) = localize_no_subst env ty_sub in
  let env =
    Typing_utils.sub_type env ~is_coeffect ty_sub ty_super (fun ?code reasons ->
        on_error ?code ((p, Reason.string_of_ureason ur) :: reasons))
  in
  env

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env on_error ty1 ty2 =
  let localize_no_subst = Typing_utils.localize_no_subst ~ignore_errors:true in
  let (env, ty1) = localize_no_subst env ty1 in
  let (env, ty2) = localize_no_subst env ty2 in
  let env =
    Typing_utils.sub_type env ty2 ty1 (fun ?code reasons ->
        on_error ?code ((p, Reason.string_of_ureason ur) :: reasons))
  in
  let env =
    Typing_utils.sub_type env ty1 ty2 (fun ?code reasons ->
        on_error ?code ((p, Reason.string_of_ureason ur) :: reasons))
  in
  env
