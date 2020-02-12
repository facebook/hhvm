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
module Env = Typing_env
module MakeType = Typing_make_type

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

let log_sub_type env p ty_sub ty_super =
  Typing_log.(
    log_with_level env "sub" 1 (fun () ->
        log_types
          p
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
let sub_type_i p ur env ty_sub ty_super on_error =
  log_sub_type env p ty_sub ty_super;
  Typing_utils.sub_type_i env ty_sub ty_super (fun ?code errl ->
      let errl = (p, Reason.string_of_ureason ur) :: errl in
      on_error ?code errl)

let sub_type p ur env ty_sub ty_super on_error =
  sub_type_i p ur env (LoclType ty_sub) (LoclType ty_super) on_error

let sub_type_decl p ur env ty_sub ty_super =
  let localize_with_self = Typing_utils.localize_with_self ~pos:p ~quiet:true in
  let (env, ty_super) = localize_with_self env ty_super in
  let (env, ty_sub) = localize_with_self env ty_sub in
  let env = sub_type p ur env ty_sub ty_super Errors.unify_error in
  env

let sub_type_decl_on_error p ur env on_error ty_sub ty_super =
  let localize_with_self = Typing_utils.localize_with_self ~pos:p ~quiet:true in
  let (env, ty_super) = localize_with_self env ty_super in
  let (env, ty_sub) = localize_with_self env ty_sub in
  let env = sub_type p ur env ty_sub ty_super on_error in
  env

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env on_error ty1 ty2 =
  let localize_with_self = Typing_utils.localize_with_self ~pos:p ~quiet:true in
  let (env, ty1) = localize_with_self env ty1 in
  let (env, ty2) = localize_with_self env ty2 in
  let env = sub_type p ur env ty2 ty1 on_error in
  let env = sub_type p ur env ty1 ty2 on_error in
  env
