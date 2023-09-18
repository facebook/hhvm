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

let union
    env
    ~join_pos
    Typing_local_types.
      {
        ty = ty1;
        defined = defined1;
        bound_ty = bound_ty1;
        pos = pos1;
        eid = eid1;
      }
    Typing_local_types.
      {
        ty = ty2;
        defined = defined2;
        bound_ty = bound_ty2;
        pos = pos2;
        eid = eid2;
      } =
  let (env, ty) = Union.union ~approx_cancel_neg:true env ty1 ty2 in
  let (env, bound_ty) =
    match (bound_ty1, bound_ty2) with
    | (None, None) -> (env, None)
    | (Some ty, None)
    | (None, Some ty) ->
      (env, Some ty)
    | (Some ty1, Some ty2) ->
      let (env, ty) =
        Typing_intersection.intersect
          ~r:(Typing_defs_core.get_reason ty1)
          env
          ty1
          ty2
      in
      (env, Some ty)
  in
  let pos =
    if phys_equal ty ty1 || Pos.equal Pos.none pos2 then
      pos1
    else if phys_equal ty ty2 || Pos.equal Pos.none pos1 then
      pos2
    else
      Pos.none
  in
  let eid =
    if Ident_provider.Ident.equal eid1 eid2 then
      eid1
    else
      Env.make_ident env
  in
  match bound_ty with
  | None ->
    Typing_local_types.
      (env, { ty; defined = defined1 && defined2; bound_ty; pos; eid })
  | Some bound_ty ->
    let (env, err_opt) =
      Typing_subtype.sub_type
        env
        ty
        bound_ty
        (Some (Typing_error.Reasons_callback.unify_error_at join_pos))
    in
    let ty =
      match err_opt with
      | None -> ty
      | Some err ->
        Typing_error_utils.add_typing_error ~env err;
        (* If the new type or bound violates the old one, then we want to
           check the remainder of the code with the type of the variable
           set to the bound *)
        bound_ty
    in
    Typing_local_types.
      ( env,
        {
          ty;
          defined = defined1 && defined2;
          bound_ty = Some bound_ty;
          pos;
          eid;
        } )

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

let restore_and_merge_conts_from env ~join_pos fromlocals conts =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.restore_and_merge_conts_from
      env
      (union ~join_pos)
      local_types
      ~from:fromlocals
      conts
  in
  Env.env_with_locals env local_types

(* Merge all continuations in the provided list and update the 'next'
   * continuation with the result. *)
let update_next_from_conts env ~join_pos cont_list =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.update_next_from_conts env (union ~join_pos) local_types cont_list
  in
  Env.env_with_locals env local_types

(* After this call, the provided continuation will be the union of itself and
   * the next continuation *)
let save_and_merge_next_in_cont env ~join_pos cont =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.save_and_merge_next_in_cont env (union ~join_pos) local_types cont
  in
  Env.env_with_locals env local_types

let move_and_merge_next_in_cont env ~join_pos cont =
  let local_types = get_all_locals env in
  let (env, local_types) =
    LEnvOps.move_and_merge_next_in_cont env (union ~join_pos) local_types cont
  in
  Env.env_with_locals env local_types

let union_contextopts ~join_pos = LEnvOps.union_opts (union ~join_pos)

let union_by_cont env ~join_pos lenv1 lenv2 =
  let locals1 = lenv1.per_cont_env in
  let locals2 = lenv2.per_cont_env in
  let (env, locals) =
    LEnvOps.union_by_cont env (union ~join_pos) locals1 locals2
  in
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

let union_lenvs_ env ~join_pos parent_lenv lenv1 lenv2 =
  let fake_members = join_fake lenv1 lenv2 in
  let local_using_vars = parent_lenv.local_using_vars in
  let env = union_by_cont env ~join_pos lenv1 lenv2 in
  let lenv = { env.lenv with local_using_vars } in
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
let union_lenvs env ~join_pos parent_lenv lenv1 lenv2 =
  fst @@ union_lenvs_ env ~join_pos parent_lenv lenv1 lenv2

let rec union_lenv_list env ~join_pos parent_lenv = function
  | []
  | [_] ->
    env
  | lenv1 :: lenv2 :: lenvlist ->
    let (env, lenv) = union_lenvs_ env ~join_pos parent_lenv lenv1 lenv2 in
    union_lenv_list env ~join_pos parent_lenv (lenv :: lenvlist)

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
