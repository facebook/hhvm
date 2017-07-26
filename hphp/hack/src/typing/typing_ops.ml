(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

module Reason  = Typing_reason
module TUtils  = Typing_utils
module Env     = Typing_env
module Inst    = Decl_instantiate
module Unify   = Typing_unify
module TDef    = Typing_tdef
module SubType = Typing_subtype
module Phase   = Typing_phase

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

let sub_type p ur env ty_sub ty_super =
  let env = { env with Env.pos = p } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> SubType.sub_type env ty_sub ty_super)
    (fun () -> env)

let unify p ur env ty1 ty2 =
  let env = { env with Env.pos = p } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> Unify.unify env ty1 ty2)
    (fun () -> env, (Reason.Rwitness p, Tany))

let sub_type_decl p ur env ty_sub ty_super =
  let env, ty_super = Phase.localize_with_self env ty_super in
  let env, ty_sub = Phase.localize_with_self env ty_sub in
  ignore (sub_type p ur env ty_sub ty_super)

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env ty1 ty2 =
  let env, ty1 = Phase.localize_with_self env ty1 in
  let env, ty2 = Phase.localize_with_self env ty2 in
  ignore (sub_type p ur env ty2 ty1);
  ignore (sub_type p ur env ty1 ty2)

module LeastUpperBound = struct
  open Typing_defs

  (* @TODO expand this match to refine more types*)
  let pairwise_least_upper_bound env ty1 ty2 =
    if SubType.is_sub_type env ty1 ty2 then ty2
    else if SubType.is_sub_type env ty2 ty1 then ty1 else (fst ty1), Tmixed

  let rec full env types =
    match types with
    | [] -> None
    | [t] ->  Some t
    | ty1 :: ty2 :: ts ->
      full env ((pairwise_least_upper_bound env ty1 ty2) :: ts)

  let rec compute types =
    match types with
    | [] -> None
    | [t] ->  Some t
    | (tenv, p, k, ty1) :: (_, _, _, ty2) :: ts  ->
      let ty = pairwise_least_upper_bound tenv ty1 ty2 in
      compute ((tenv, p, k, ty) :: ts)

  end
