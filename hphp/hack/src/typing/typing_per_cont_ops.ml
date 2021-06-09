(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
module C = Typing_continuations
module LMap = Local_id.Map
module LEnvC = Typing_per_cont_env
open LEnvC

type 'a locals_merge_fn =
  'a ->
  Typing_local_types.local ->
  Typing_local_types.local ->
  'a * Typing_local_types.local

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)

let union union_types env context1 context2 =
  let (env, local_types) =
    LMap.merge_env
      env
      ~combine:(fun env _ tyopt1 tyopt2 ->
        match (tyopt1, tyopt2) with
        | (Some ty1, Some ty2) ->
          let (env, ty) = union_types env ty1 ty2 in
          (env, Some ty)
        | _ -> (env, None))
      (* TODO: we could do better here in case only in one side. *)
      context1.local_types
      context2.local_types
  in
  let (env, tpenv) =
    Type_parameter_env_ops.join env context1.tpenv context2.tpenv
  in
  let fake_members =
    Typing_fake_members.join context1.fake_members context2.fake_members
  in
  (env, { fake_members; local_types; tpenv })

let union_opts union_types env ctxopt1 ctxopt2 =
  match (ctxopt1, ctxopt2) with
  | (None, opt)
  | (opt, None) ->
    (env, opt)
  | (Some ctx1, Some ctx2) ->
    let (env, ctx) = union union_types env ctx1 ctx2 in
    (env, Some ctx)

(* Does per-cont entry ctx1 entail ctx2?
 *   For locals, for each x:t2 in ctx2 we require x:t1 in ctx1 with t1 <: t2
 *   For fake member info, we delegate to Typing_fake_members.sub
 *   TODO: tpenv. Not clear what to do here, as we generate fresh type parameter names
 *     and so use of is_sub_entry for loop iteration would return false even if safe
 *)
let is_sub_entry is_subtype env ctx1 ctx2 =
  LMap.for_all2
    ~f:(fun _k tyopt1 tyopt2 ->
      match (tyopt1, tyopt2) with
      | (_, None) -> true
      | (None, Some _) -> false
      | (Some (ty1, _, _id1), Some (ty2, _, _id2)) -> is_subtype env ty1 ty2)
    ctx1.local_types
    ctx2.local_types
  && Typing_fake_members.sub ctx1.fake_members ctx2.fake_members

let is_sub_opt_entry is_subtype env ctxopt1 ctxopt2 =
  match (ctxopt1, ctxopt2) with
  | (_, None) -> true
  | (None, _) -> false
  | (Some ctx1, Some ctx2) -> is_sub_entry is_subtype env ctx1 ctx2

(* Union a list of continuations *)
let union_conts env union_types local_types cont_list =
  let union_two env locals cont =
    union_opts union_types env locals (CMap.find_opt cont local_types)
  in
  List.fold_left_env env cont_list ~f:union_two ~init:None

(* Union a list of source continuations and store the result in a
 * destination continuation. *)
let union_conts_and_update env union_types local_types ~from_conts ~to_cont =
  let (env, unioned) = union_conts env union_types local_types from_conts in
  (env, replace_cont to_cont unioned local_types)

let update_next_from_conts env union_types local_types cont_list =
  union_conts_and_update
    env
    union_types
    local_types
    ~from_conts:cont_list
    ~to_cont:C.Next

let save_and_merge_next_in_cont env union_types local_types cont =
  union_conts_and_update
    env
    union_types
    local_types
    ~from_conts:[C.Next; cont]
    ~to_cont:cont

let move_and_merge_next_in_cont env union_types local_types cont =
  let (env, locals) =
    save_and_merge_next_in_cont env union_types local_types cont
  in
  (env, drop_cont C.Next locals)

let union_by_cont env union_types locals1 locals2 =
  CMap.union_env env locals1 locals2 ~combine:(fun env _ cont1 cont2 ->
      let (env, ctx) = union union_types env cont1 cont2 in
      (env, Some ctx))

let restore_cont_from locals ~from:source_locals cont =
  let ctxopt = get_cont_option cont source_locals in
  replace_cont cont ctxopt locals

let restore_conts_from locals ~from conts =
  List.fold ~f:(restore_cont_from ~from) ~init:locals conts

let restore_and_merge_cont_from env union_types locals ~from cont =
  let ctxfromopt = get_cont_option cont from in
  let ctxtoopt = get_cont_option cont locals in
  let (env, newctxopt) = union_opts union_types env ctxfromopt ctxtoopt in
  (env, replace_cont cont newctxopt locals)

let restore_and_merge_conts_from env union_types locals ~from conts =
  List.fold_left_env
    env
    ~f:(fun env locals cont ->
      restore_and_merge_cont_from env union_types locals ~from cont)
    ~init:locals
    conts
