(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Reason = Typing_reason
module Env = Typing_env
module MakeType = Typing_make_type

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

(* Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type p ur env ty_sub ty_super on_error =
  Typing_log.(
    log_with_level env "sub" 1 (fun () ->
        log_types
          p
          env
          [
            Log_head
              ( "Typing_ops.sub_type",
                [Log_type ("ty_sub", ty_sub); Log_type ("ty_super", ty_super)]
              );
          ]));
  Errors.try_add_err
    p
    (Reason.string_of_ureason ur)
    (fun () -> Typing_utils.sub_type env ty_sub ty_super on_error)
    (fun () -> env)

let sub_type_decl p ur env ty_sub ty_super =
  let (env, ty_super) = Typing_utils.localize_with_self env ty_super in
  let (env, ty_sub) = Typing_utils.localize_with_self env ty_sub in
  ignore (sub_type p ur env ty_sub ty_super Errors.unify_error)

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env ty1 ty2 =
  let (env, ty1) = Typing_utils.localize_with_self env ty1 in
  let (env, ty2) = Typing_utils.localize_with_self env ty2 in
  ignore (sub_type p ur env ty2 ty1 Errors.unify_error);
  ignore (sub_type p ur env ty1 ty2 Errors.unify_error)

module LeastUpperBound = struct
  open Typing_defs

  let exact_least_upper_bound e1 e2 =
    match (e1, e2) with
    | (Exact, Exact) -> Exact
    | (_, _) -> Nonexact
end
