  (**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Typing_env
open Env
module TUtils = Typing_utils
module Type = Typing_ops
module Reason = Typing_reason
module C = Typing_continuations
module LEnvC = Typing_lenv_cont

(*****************************************************************************)
(* Module dealing with local environments. *)
(*****************************************************************************)

let get_all_locals env = env.lenv.local_types

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)
let union env local1 local2 =
  let (ty1, eid1), (ty2, eid2) = local1, local2 in
  let eid = if eid1 = eid2 then eid1 else Ident.tmp() in
  let env, ty =
    (* In principle this is covered by the is_sub_type_alt tests, but
     * that function isn't complete and sometimes returns None.
     *)
    if Typing_defs.ty_equal ty1 ty2
    then env, ty1
    else
    if Typing_subtype.is_sub_type_alt env ty1 ty2 = Some true
    then env, ty2
    else
    if Typing_subtype.is_sub_type_alt env ty2 ty1 = Some true
    then env, ty1
    else
      let env, ty1 = TUtils.unresolved env ty1 in
      let env, ty2 = TUtils.unresolved env ty2 in
      Type.union env.Env.pos Reason.URnone env ty1 ty2 in
  env, (ty, eid)

let get_cont env cont =
  let local_types = get_all_locals env in
  LEnvC.get_cont cont local_types

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
    LEnvC.restore_conts_from local_types ~from:fromlocals conts in
  Env.env_with_locals env local_types

let restore_and_merge_conts_from env fromlocals conts =
  let local_types = get_all_locals env in
  let env, local_types =
    LEnvC.restore_and_merge_conts_from env union local_types ~from:fromlocals
    conts in
  Env.env_with_locals env local_types

(* Merge all continuations in the provided list and update the 'next'
* continuation with the result. *)
let update_next_from_conts env cont_list =
  let local_types = get_all_locals env in
  let env, local_types = LEnvC.update_next_from_conts env union local_types
    cont_list in
  Env.env_with_locals env local_types

(* After this call, the provided continuation will be the union of itself and
* the next continuation *)
let save_and_merge_next_in_cont env cont =
  let local_types = get_all_locals env in
  let env, local_types = LEnvC.save_and_merge_next_in_cont env union
    local_types cont in
  Env.env_with_locals env local_types

let move_and_merge_next_in_cont env cont =
  let local_types = get_all_locals env in
  let env, local_types = LEnvC.move_and_merge_next_in_cont env union
    local_types cont in
  Env.env_with_locals env local_types

let union_contextopts = LEnvC.union_opts union

let union_by_cont env lenv1 lenv2 =
  let locals1 = lenv1.Env.local_types in
  let locals2 = lenv2.Env.local_types in
  let env, locals = LEnvC.union_by_cont env union locals1 locals2 in
  Env.env_with_locals env locals

(* Intersects the set of valid fake_members.
 * Fake members are introduced when we know that a member is not null.
 * Example: if($this->x) { ... $this->x is a fake member now ... }
 * What it means in practice is that the member behaves like a local, it can
 * change type.
 *)
let intersect_fake lenv1 lenv2 =
  let nextctxopt1 = LEnvC.get_cont_option C.Next lenv1.local_types in
  let nextctxopt2 = LEnvC.get_cont_option C.Next lenv2.local_types in
  let fake1 = lenv1.fake_members in
  let fake2 = lenv2.fake_members in
  let valid = match nextctxopt1, nextctxopt2 with
  | Some _, Some _
  | None, None -> SSet.inter fake1.valid fake2.valid
  | Some _, None -> fake1.valid
  | None, Some _ -> fake2.valid in
  { fake1 with Env.valid = valid }

let merge_reactivity parent_lenv lenv1 lenv2 =
  let nextctxopt1 = LEnvC.get_cont_option C.Next lenv1.local_types in
  let nextctxopt2 = LEnvC.get_cont_option C.Next lenv2.local_types in
  match nextctxopt1, nextctxopt2 with
  | Some _, Some _
  | None, None -> parent_lenv.local_reactive
  | Some _, None -> lenv1.local_reactive
  | None, Some _ -> lenv2.local_reactive

let union_lenvs_ env parent_lenv lenv1 lenv2 =
  let fake_members = intersect_fake lenv1 lenv2 in
  let local_using_vars = parent_lenv.local_using_vars in
  let tpenv = env.lenv.tpenv in
  let local_mutability = Typing_mutability_env.intersect_mutability
    parent_lenv.local_mutability lenv1.local_mutability lenv2.local_mutability in
  let local_reactive = merge_reactivity parent_lenv lenv1 lenv2 in
  let env = union_by_cont env lenv1 lenv2 in
  let lenv = { env.Env.lenv with
    fake_members;
    local_using_vars;
    tpenv;
    local_mutability;
    local_reactive;
  } in
  { env with lenv }, lenv

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
  | [_] -> env
  | lenv1 :: lenv2 :: lenvlist ->
    let env, lenv = union_lenvs_ env parent_lenv lenv1 lenv2 in
    union_lenv_list env parent_lenv (lenv :: lenvlist)

let stash_and_do env conts f =
  let parent_locals = get_all_locals env in
  let env = drop_conts env conts in
  let env, res = f env in
  let env = restore_conts_from env parent_locals conts in
  env, res

let env_with_empty_fakes env =
  { env with Env.lenv = {
      env.Env.lenv with Env.fake_members = Env.empty_fake_members;
    }
  }
