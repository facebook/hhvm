(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Common

include Typing_env_types

module C = Typing_continuations
module CMap = Typing_continuations.Map
module LMap = Local_id.Map

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables *)
(*****************************************************************************)

let empty_locals = CMap.empty

let initial_locals = CMap.add C.Next LMap.empty empty_locals

let get_cont_option = CMap.get

exception Continuation_not_found of string

let get_cont cont m =
  try CMap.find cont m
  with Caml.Not_found ->
    (* Programming error. This is not supposed to happen. *)
    raise (Continuation_not_found ("There is no continuation " ^
      (C.to_string cont) ^ " in the locals."))

(* see typing_lenv_cont.mli for details *)
let try_get_conts conts m =
  let rec aux contl m =
    match contl with
    | [] -> raise (Continuation_not_found ("None of the continuations " ^
      (String.concat ~sep:", " (List.map ~f:C.to_string conts)) ^ " were found."))
    | cont :: contl ->
      begin match get_cont_option cont m with
      | None -> aux contl m
      | Some ctx -> ctx
      end in
  aux conts m

(* see typing_lenv_cont.mli for details *)
let add_to_cont name key value m =
  let cont = match CMap.get name m with
    | None -> LMap.empty
    | Some c -> c
  in
  let cont = LMap.add key value cont in
  CMap.add name cont m

(* see typing_lenv_cont.mli for details *)
let remove_from_cont name key m =
  match CMap.get name m with
  | None -> m
  | Some c -> CMap.add name (LMap.remove key c) m

let drop_cont = CMap.remove

let drop_conts conts map =
  List.fold ~f:(fun map cont -> drop_cont cont map) ~init:map conts

let replace_cont key valueopt map = match valueopt with
  | None -> drop_cont key map
  | Some value -> CMap.add key value map


(* see typing_lenv_cont.mli for details *)
let union union_types env context1 context2 =
  LMap.merge_env env
      ~combine:(fun env _ tyopt1 tyopt2 ->
    match tyopt1, tyopt2 with
    | Some ty1, Some ty2 ->
      let env, ty = union_types env ty1 ty2 in
      env, Some ty
    | _ -> env, None) (* TODO: we could do better here in case only in one side. *)
    context1 context2

let union_opts union_types env ctxopt1 ctxopt2 =
  match ctxopt1, ctxopt2 with
  | None, opt
  | opt, None -> env, opt
  | Some ctx1, Some ctx2 ->
    let env, ctx = union union_types env ctx1 ctx2 in
    env, Some ctx

(* Union a list of continuations *)
let union_conts env union_types local_types cont_list =
  let union_two env locals cont = union_opts union_types env locals
    (CMap.get cont local_types) in
  List.fold_left_env env cont_list ~f:union_two ~init:None

(* Union a list of source continuations and store the result in a
 * destination continuation. *)
let union_conts_and_update env union_types local_types ~from_conts ~to_cont =
  let env, unioned = union_conts env union_types local_types from_conts in
  env, replace_cont to_cont unioned local_types

(* see typing_lenv_cont.mli for details *)
let update_next_from_conts env union_types local_types cont_list =
  union_conts_and_update env union_types local_types
    ~from_conts:cont_list
    ~to_cont:C.Next

(* see typing_lenv_cont.mli for details *)
let save_and_merge_next_in_cont env union_types local_types cont =
  union_conts_and_update env union_types local_types
    ~from_conts:[C.Next; cont]
    ~to_cont:cont

(* see typing_lenv_cont.mli for details *)
let move_and_merge_next_in_cont env union_types local_types cont =
  let env, locals = save_and_merge_next_in_cont env union_types local_types cont in
  env, drop_cont C.Next locals

(* see typing_lenv_cont.mli for details *)
let union_by_cont env union_types locals1 locals2 =
  CMap.union_env env locals1 locals2
    ~combine:(fun env _ cont1 cont2 ->
      let env, ctx = union union_types env cont1 cont2 in
      env, Some ctx)

(* see typing_lenv_cont.mli for details *)
let restore_cont_from locals ~from:source_locals cont =
  let ctxopt = get_cont_option cont source_locals in
  replace_cont cont ctxopt locals

(* see typing_lenv_cont.mli for details *)
let restore_conts_from locals ~from conts =
  List.fold ~f:(restore_cont_from ~from) ~init:locals conts

let restore_and_merge_cont_from env union_types locals ~from cont =
  let ctxfromopt = get_cont_option cont from in
  let ctxtoopt = get_cont_option cont locals in
  let env, newctxopt = union_opts union_types env ctxfromopt ctxtoopt in
  env, replace_cont cont newctxopt locals

let restore_and_merge_conts_from env union_types locals ~from conts =
  List.fold_left_env env
    ~f:(fun env locals cont ->
      restore_and_merge_cont_from env union_types locals ~from cont)
    ~init:locals conts
