(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Typing_env
open Typing_env_types
module C = Typing_continuations
module LEnvC = Typing_per_cont_env
module LEnvOps = Typing_per_cont_ops
module Union = Typing_union

(*****************************************************************************)
(* Module dealing with local environments. *)
(*****************************************************************************)

let get_all_locals env = env.lenv.per_cont_env

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)

let union env local1 local2 =
  let ((ty1, eid1), (ty2, eid2)) = (local1, local2) in
  let eid =
    if Ident.equal eid1 eid2 then
      eid1
    else
      Ident.tmp ()
  in
  let (env, ty) = Union.union env ty1 ty2 in
  (env, (ty, eid))

let get_cont_option env cont =
  let local_types = get_all_locals env in
  LEnvC.get_cont_option cont local_types

let drop_cont env cont =
  let local_types = get_all_locals env in
  let local_types = LEnvC.drop_cont cont local_types in
  Env.env_with_locals env local_types

let drop_conts env conts =
  let local_types = get_all_locals env in
  let local_types = LEnvC.drop_conts conts local_types in
  Env.env_with_locals env local_types

let replace_cont env cont ctxopt =
  let local_types = get_all_locals env in
  let local_types = LEnvC.replace_cont cont ctxopt local_types in
  Env.env_with_locals env local_types

let restore_conts_from env fromlocals conts =
  let local_types = get_all_locals env in
  let local_types =
    LEnvOps.restore_conts_from local_types ~from:fromlocals conts
  in
  Env.env_with_locals env local_types

let restore_and_merge_conts_from env fromlocals conts =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.restore_and_merge_conts_from
      env
      union
      local_types
      ~from:fromlocals
      conts
  in
  Env.env_with_locals env local_types

(* Merge all continuations in the provided list and update the 'next'
* continuation with the result. *)
let update_next_from_conts env cont_list =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.update_next_from_conts env union local_types cont_list
  in
  Env.env_with_locals env local_types

(* After this call, the provided continuation will be the union of itself and
* the next continuation *)
let save_and_merge_next_in_cont env cont =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.save_and_merge_next_in_cont env union local_types cont
  in
  Env.env_with_locals env local_types

let move_and_merge_next_in_cont env cont =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.move_and_merge_next_in_cont env union local_types cont
  in
  Env.env_with_locals env local_types

let union_contextopts = LEnvOps.union_opts union

let union_by_cont env lenv1 lenv2 =
  let locals1 = lenv1.per_cont_env in
  let locals2 = lenv2.per_cont_env in
  let (env, locals) = LEnvOps.union_by_cont env union locals1 locals2 in
  Env.env_with_locals env locals

let join_fake lenv1 lenv2 =
  let nextctxopt1 = LEnvC.get_cont_option C.Next lenv1.per_cont_env in
  let nextctxopt2 = LEnvC.get_cont_option C.Next lenv2.per_cont_env in
  match (nextctxopt1, nextctxopt2) with
  | (Some c1, Some c2) ->
    Typing_fake_members.join c1.LEnvC.fake_members c2.LEnvC.fake_members
  | (None, None) -> Typing_fake_members.empty
  | (Some c1, None) -> c1.LEnvC.fake_members
  | (None, Some c2) -> c2.LEnvC.fake_members

let merge_reactivity parent_lenv lenv1 lenv2 =
  let nextctxopt1 = LEnvC.get_cont_option C.Next lenv1.per_cont_env in
  let nextctxopt2 = LEnvC.get_cont_option C.Next lenv2.per_cont_env in
  match (nextctxopt1, nextctxopt2) with
  | (Some _, Some _)
  | (None, None) ->
    parent_lenv.local_reactive
  | (Some _, None) -> lenv1.local_reactive
  | (None, Some _) -> lenv2.local_reactive

let union_lenvs_ env parent_lenv lenv1 lenv2 =
  let fake_members = join_fake lenv1 lenv2 in
  let local_using_vars = parent_lenv.local_using_vars in
  let local_mutability =
    Typing_mutability_env.intersect_mutability
      parent_lenv.local_mutability
      lenv1.local_mutability
      lenv2.local_mutability
  in
  let local_reactive = merge_reactivity parent_lenv lenv1 lenv2 in
  let env = union_by_cont env lenv1 lenv2 in
  let lenv =
    { env.lenv with local_using_vars; local_mutability; local_reactive }
  in
  let per_cont_env =
    LEnvC.update_cont_entry C.Next lenv.per_cont_env (fun entry ->
        LEnvC.{ entry with fake_members })
  in
  let lenv = { lenv with per_cont_env } in
  ({ env with lenv }, lenv)

(* Used when we want the new local environment to be the union
 * of 2 local environments. Typical use case is an if statement.
 * $x = 0;
 * if(...) { $x = ''; } else { $x = 'foo'; }
 * We want $x to be a string past this point.
 * We check that the locals are defined in both branches
 * when that is the case, their type becomes the union (least upper bound)
 * of the types it had in each branch.
 *)
let union_lenvs env parent_lenv lenv1 lenv2 =
  fst @@ union_lenvs_ env parent_lenv lenv1 lenv2

let rec union_lenv_list env parent_lenv = function
  | []
  | [_] ->
    env
  | lenv1 :: lenv2 :: lenvlist ->
    let (env, lenv) = union_lenvs_ env parent_lenv lenv1 lenv2 in
    union_lenv_list env parent_lenv (lenv :: lenvlist)

let stash_and_do env conts f =
  let parent_locals = get_all_locals env in
  let env = drop_conts env conts in
  let (env, res) = f env in
  let env = restore_conts_from env parent_locals conts in
  (env, res)

let env_with_empty_fakes env =
  let per_cont_env =
    LEnvC.update_cont_entry C.Next env.lenv.per_cont_env (fun entry ->
        LEnvC.{ entry with fake_members = Typing_fake_members.empty })
  in
  { env with lenv = { env.lenv with per_cont_env } }

let has_next env =
  match get_cont_option env C.Next with
  | None -> false
  | Some _ -> true
