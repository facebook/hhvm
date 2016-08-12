(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

module Env = Typing_env
open Env
module TUtils = Typing_utils
module Type = Typing_ops
module Reason = Typing_reason

module LMap = Local_id.Map

(*****************************************************************************)
(* Module dealing with local environments. *)
(*****************************************************************************)

(* Intersects the set of valid fake_members.
 * Fake members are introduced when we know that a member is not null.
 * Example: if($this->x) { ... $this->x is a fake member now ... }
 * What it means in practice is that the member behaves like a local, it can
 * change type.
 *)
let intersect_fake fake1 fake2 =
  let valid = SSet.inter fake1.Env.valid fake2.Env.valid in
  let fake_members = { fake1 with Env.valid = valid } in
  fake_members

(* Used when we want the new local environment to be the intersection
 * of 2 local environments. Typical use case is an if statement.
 * $x = 0;
 * if(...) { $x = ''; } else { $x = 'foo'; }
 * We want $x to be a string past this point.
 * The intersection checks that the locals are defined in both branches
 * when that is the case, their type becomes:
 * Tunresolved [type_in_the_left_branch; type_in_the_right_branch].
 * If the type is missing in either of the branches, we fall back on
 * the type that was defined in the parent environment.
 *)
let intersect env parent_lenv lenv1 lenv2 =
  let fake_members = intersect_fake lenv1.fake_members lenv2.fake_members in
  let tpenv = env.lenv.tpenv in
  let parent_locals = parent_lenv.local_types in
  let env, new_locals =
    LMap.fold begin fun local_id (all_types1, ty1, eid1) (env, locals) ->
      match LMap.get local_id lenv2.local_types with
      | None -> env, locals
      | Some (all_types2, ty2, eid2) ->
          (* If the local has different expression ids then we generate a
           * new one when interecting
           *)
          let eid = if eid1 = eid2 then eid1 else Ident.tmp() in
          let env, ty1 = TUtils.unresolved env ty1 in
          let env, ty2 = TUtils.unresolved env ty2 in
          let (all_small, all_large) =
            if List.length all_types1 < List.length all_types2
            then (all_types1, all_types2)
            else (all_types2, all_types1) in
          let all_types =
            List.fold_left ~f:begin fun acc ty ->
              if List.mem acc ty then acc else ty::acc
            end ~init:all_large all_small in
          let env, ty = Type.unify env.Env.pos Reason.URnone env ty1 ty2 in
          env, LMap.add local_id (all_types, ty, eid) locals
    end lenv1.local_types (env, parent_locals)
  in
  { env with Env.lenv =
    { fake_members;
      local_types = new_locals;
      tpenv;
    }
  }

(* Integration is subtle. It consists in remembering all the types that
 * a local has had in a branch.
 * We need to keep this information because of constructions that disrupt
 * the control flow.
 * Example:
 * if(...) {
 *   $x = 0;
 * }
 * else {
 *   $x = '';
 *   throw new Exception('');
 * }
 * At this point, $x is an int, because the else branch is terminal.
 * But we still 'integrate the branch', that is, we remember that $x can have
 * type int OR string.
 * The integration will become useful on a try:
 * try {
 *   if(...) {
 *     $x = 0;
 *   }
 *   else {
 *     $x = '';
 *     throw new Exception('');
 *   }
 * }
 * catch(Exception $e) {
 *   What is the type of $x? <-----------------------------
 * }
 * You can see that we will need the types collected during the integration.
 * Because we collected all the possible types taken by $x, we can build the
 * local environment where $x is of type Tunresolved[int, string].
 * The conservative local environment is built with fully_integrate.
 *)
let integrate env parent_lenv child_lenv =
  let new_locals =
    LMap.fold begin fun local_id (child_all_types, child_ty, child_eid) locals ->
      match LMap.get local_id locals with
      | None ->
          LMap.add local_id (child_all_types, child_ty, child_eid) locals
      | Some (parent_all_types, _, parent_eid)
            when child_all_types == parent_all_types ->
          let eid = if child_eid = parent_eid then child_eid else Ident.tmp() in
          LMap.add local_id (child_all_types, child_ty, eid) locals
      | Some (parent_all_types, _, parent_eid) ->
          let eid = if child_eid = parent_eid then child_eid else Ident.tmp() in
          let all_types = List.fold_left ~f:begin fun all_types ty ->
            if List.exists all_types ((=) ty) then all_types else ty::all_types
          end ~init:child_all_types parent_all_types in
          LMap.add local_id (all_types, child_ty, eid) locals
    end child_lenv.local_types parent_lenv.local_types
  in
  { env with Env.lenv =
    { fake_members = child_lenv.fake_members;
      local_types = new_locals;
      tpenv = env.lenv.tpenv;
    }
  }

(* Same as intersect, but with a list of local environments *)
let intersect_list env parent_lenv term_lenv_l =
  let to_integrate, to_intersect =
    List.partition_map term_lenv_l begin fun (term, lenv) ->
      if term then `Fst lenv else `Snd lenv
    end in
  let env = List.fold_left to_integrate ~f:begin fun env lenv ->
    integrate env parent_lenv lenv
  end ~init:env in
  (match to_intersect with
  | [] -> env
  | [x] -> { env with Env.lenv = x }
  | lenv1 :: rl ->
      List.fold_left ~f:begin fun env lenv2 ->
        intersect env parent_lenv env.Env.lenv lenv2
      end ~init:{ env with Env.lenv = lenv1 } rl
  )

(* Function that changes the types of locals to a more conservative value.
 * When exiting from a construction that could have disrupted the
 * "natural" control-flow, we need to be more conservative with the
 * values of locals (cf: integrate).
 *)
let fully_integrate env parent_lenv =
  let child_lenv = env.Env.lenv in
  let fake_members =
    intersect_fake parent_lenv.fake_members child_lenv.fake_members in
  let env, locals =
    LMap.fold begin fun local_id (child_all_types,_, child_eid) (env, locals) ->
      let parent_all_types, parent_eid =
        match LMap.get local_id parent_lenv.local_types with
        | None -> [], -1
        | Some (parent_all_types, _, parent_eid) ->
            parent_all_types, parent_eid
      in
      if child_all_types == parent_all_types && parent_eid = child_eid
      then env, locals
      else if child_all_types == parent_all_types
      then
        match LMap.get local_id parent_lenv.local_types with
        | None -> env, locals
        | Some (_, parent_ty, _) ->
            let lcl = parent_all_types, parent_ty, Ident.tmp() in
            env, LMap.add local_id lcl locals
      else
        let eid = if child_eid = parent_eid then child_eid else Ident.tmp() in
        let env, child_all_types =
          List.map_env env child_all_types TUtils.unresolved in
        let env, ty =
          match child_all_types with
          | [] -> assert false
          | [first] -> env, first
          | first :: rest ->
              List.fold_left ~f:begin fun (env, ty_acc) ty ->
                Type.unify env.Env.pos Reason.URnone env ty_acc ty
              end ~init:(env, first) rest
        in
        env, LMap.add local_id (ty :: parent_all_types, ty, eid) locals
    end child_lenv.local_types (env, parent_lenv.local_types)
  in
  { env with Env.lenv =
    { fake_members; local_types=locals; tpenv=child_lenv.tpenv } }

let env_with_empty_fakes env =
  { env with Env.lenv = {
      env.Env.lenv with Env.fake_members = Env.empty_fake_members;
    }
  }
