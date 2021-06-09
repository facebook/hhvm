(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module C = Typing_continuations
module Cont = Typing_per_cont_env
module LEnvOps = Typing_per_cont_ops
module Env = Typing_env
module LEnv = Typing_lenv
module Reason = Typing_reason

(**
 * This type represents the structure refered to using the greek alphabet
 * letter 'gamma' in the type system specification.
 *
 * It is essentially a map from local ids to types.
 * For now, we reuse Typing_env.local_id_map but this may change.
 *)
type gamma = Typing_per_cont_env.per_cont_entry

(**
 * This type represents the structure refered to using the greek alphabet
 * letter 'delta' in the type system specification.
 *
 * It is a map from continuations to gammas
 *)
type delta = gamma Typing_continuations.Map.t

let empty_gamma : gamma = Typing_per_cont_env.empty_entry

let empty_delta : delta = Typing_continuations.Map.empty

(* For now we dummify the local id, i.e. we replace the unique integer part of
 * the local id with 0. Lookup of a variable in Gamma is therefore only based
 * on the variable name.
 * TODO When we typecheck lambda or lets, we'll have to do better *)
let dummify_local_id local_id =
  Local_id.make_unscoped (Local_id.get_name local_id)

let lookup local_id gamma =
  let local_id = dummify_local_id local_id in
  let tyopt = Local_id.Map.find_opt local_id gamma in
  (* Convert from type (local = ty * expression_id) to
   * (Tast.annotation = Pos.t * ty). Ignore the expression id.
   * TODO Avoid this conversion? Do we need the expression ids? *)
  let local_to_pos_ty (ty, _pos, _expr_id) = (Typing_defs.get_pos ty, ty) in
  Option.map tyopt ~f:local_to_pos_ty

let add_to_gamma local_id ty gamma =
  let local_id = dummify_local_id local_id in
  let pos_ty_to_local (p, ty) =
    Typing_defs.(mk (Reason.Rwitness p, get_node ty), Pos.none, 0)
  in
  let ty = pos_ty_to_local ty in
  {
    gamma with
    Typing_per_cont_env.local_types =
      Typing_local_types.add_to_local_types
        local_id
        ty
        gamma.Typing_per_cont_env.local_types;
  }

let get_cont = Cont.get_cont_option

let get_next_cont = get_cont C.Next

let set_cont_opt = Cont.replace_cont

let set_cont cont gamma = set_cont_opt cont (Some gamma)

let drop_cont = Cont.drop_cont

let empty_delta_with_cont cont gamma = set_cont cont gamma empty_delta

let empty_delta_with_next_cont = empty_delta_with_cont C.Next

(**
 * Union two deltas continuation by continuations. Similar to
 * Typing_lenv_cont.union_by_cont.
 *)
let union env delta1 delta2 =
  let union env local1 local2 =
    let (env, (ty, pos, eid)) = LEnv.union env local1 local2 in
    let (env, ty) = Env.expand_type env ty in
    (env, (ty, pos, eid))
  in
  let (_env, delta) = LEnvOps.union_by_cont env union delta1 delta2 in
  delta

(**
 * Apply a list of updates to a gamma.
 *)
let update_gamma gamma (updates : gamma) =
  let local_types = gamma.Typing_per_cont_env.local_types in
  let updates = updates.Typing_per_cont_env.local_types in
  {
    gamma with
    Typing_per_cont_env.local_types =
      Local_id.Map.fold
        Typing_local_types.add_to_local_types
        updates
        local_types;
  }

let make_local_id (name : string) : Local_id.t = Local_id.make_unscoped name
