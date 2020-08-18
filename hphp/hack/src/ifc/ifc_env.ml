(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types
module Logic = Ifc_logic
module Utils = Ifc_utils
module K = Typing_cont_key
module LSet = Local_id.Set

(* Only elementary logic about the (read-only) environment should
   be in this file to avoid circular dependencies;
   use higher-order functions to parameterize
   complex behavior! *)

let new_policy_var { pre_scope; pre_pvar_counters; _ } prefix =
  let suffix =
    match Caml.Hashtbl.find_opt pre_pvar_counters prefix with
    | Some counter ->
      incr counter;
      "'" ^ string_of_int !counter
    | None ->
      Caml.Hashtbl.add pre_pvar_counters prefix (ref 0);
      ""
  in
  Pfree_var (prefix ^ suffix, pre_scope)

let new_proto_renv meta saved_tenv scope decl_env =
  {
    pre_scope = scope;
    pre_decl = decl_env;
    pre_pvar_counters = Caml.Hashtbl.create 10;
    pre_tenv = saved_tenv;
    pre_meta = meta;
  }

let new_renv proto_renv this_ty ret_ty global_pc exn =
  {
    re_proto = proto_renv;
    re_this = this_ty;
    re_ret = ret_ty;
    re_gpc = global_pc;
    re_exn = exn;
  }

let empty_lenv = { le_vars = LMap.empty; le_pc = PCSet.empty }

let new_env =
  { e_cont = KMap.singleton K.Next empty_lenv; e_acc = []; e_deps = SSet.empty }

let acc env update = { env with e_acc = update env.e_acc }

let add_dep env name = { env with e_deps = SSet.add name env.e_deps }

let get_cenv env = env.e_cont

let set_cenv env cenv = { env with e_cont = cenv }

(* Merge two local envs, if a variable appears only in one local
   env, it will not appear in the result env *)
let merge_lenv ~union env lenv1 lenv2 =
  let combine = Utils.mk_combine false union in
  let (env, le_vars) =
    LMap.merge_env env lenv1.le_vars lenv2.le_vars ~combine
  in
  let le_pc = PCSet.union lenv1.le_pc lenv2.le_pc in
  (env, { le_vars; le_pc })

(* Merge continuation envs, if a continuation is assigned a
   local env only in one of the arguments it will be kept as
   is in the result. This is because lack of a continuation
   means that some code was, up to now, dead code. *)
let merge_and_set_cenv ~union env cenv1 cenv2 =
  let combine = Utils.mk_combine true (merge_lenv ~union) in
  let (env, cenv) = KMap.merge_env env cenv1 cenv2 ~combine in
  set_cenv env cenv

let get_lenv_opt env k = KMap.find_opt k (get_cenv env)

let lenv_set_local_type lenv lid pty =
  { lenv with le_vars = LMap.add lid pty lenv.le_vars }

let set_cont env k lenv = set_cenv env (KMap.add k lenv (get_cenv env))

let set_local_type env lid pty =
  match get_lenv_opt env K.Next with
  | None -> env
  | Some lenv ->
    let lenv = lenv_set_local_type lenv lid pty in
    set_cont env K.Next lenv

let get_local_type env lid =
  match get_lenv_opt env K.Next with
  | None -> Tunion []
  | Some lenv -> LMap.find lid lenv.le_vars

let get_lpc_policy env k =
  match get_lenv_opt env k with
  | None -> PCSet.empty
  | Some lenv -> lenv.le_pc

let get_gpc_policy renv env k = PCSet.add renv.re_gpc (get_lpc_policy env k)

let push_pc env k pc =
  match get_lenv_opt env k with
  | None -> env
  | Some lenv ->
    let lenv = { lenv with le_pc = PCSet.add pc lenv.le_pc } in
    set_cont env k lenv

let set_pc env k pc =
  match get_lenv_opt env k with
  | None -> env
  | Some lenv -> set_cont env k { lenv with le_pc = pc }

let merge_lenv_into ~union env lenv k =
  match get_lenv_opt env k with
  | None -> set_cont env k lenv
  | Some lenv' ->
    let (env, merged_lenv) = merge_lenv ~union env lenv lenv' in
    set_cont env k merged_lenv

let merge_conts_into ~union env ks k_to =
  let f (env, lenv) k =
    match (lenv, get_lenv_opt env k) with
    | (None, l)
    | (l, None) ->
      (env, l)
    | (Some l1, Some l2) ->
      let (env, merged_lenv) = merge_lenv ~union env l1 l2 in
      (env, Some merged_lenv)
  in
  let (env, lenv) = List.fold ~f ~init:(env, None) ks in
  match lenv with
  | None -> env
  | Some lenv -> merge_lenv_into ~union env lenv k_to

let merge_conts_from ~union env from_cenv =
  let f env k =
    match KMap.find_opt k from_cenv with
    | None -> env
    | Some lenv -> merge_lenv_into ~union env lenv k
  in
  List.fold ~f ~init:env

let filter_conts env pred =
  let pred k _ = pred k in
  set_cenv env @@ KMap.filter pred env.e_cont

let drop_conts env ks =
  filter_conts env @@ fun k -> not @@ List.mem ~equal:K.equal ks k

(* Merge conts and then clear them *)
let move_conts_into ~union env ks k_to =
  let env = merge_conts_into ~union env ks k_to in
  drop_conts env ks

let merge_pcs_into env ks k_to =
  let f pc k = PCSet.union (get_lpc_policy env k) pc in
  let pc = List.fold ~f ~init:PCSet.empty ks in
  match get_lenv_opt env k_to with
  | None -> env
  | Some lenv ->
    set_cont env k_to { lenv with le_pc = PCSet.union pc lenv.le_pc }

(* Freshen lids and drop all other locals *)
let freshen_cenv ~freshen renv env lids =
  let lids = List.map ~f:snd lids |> LSet.of_list in
  let freshen_lenv env _ lenv =
    let (env, vars) =
      LMap.filter (fun lid _ -> LSet.mem lid lids) lenv.le_vars
      |> LMap.map_env (fun e _ -> freshen renv e) env
    in
    (env, { lenv with le_vars = vars })
  in
  let (env, cenv) = KMap.map_env freshen_lenv env env.e_cont in
  { env with e_cont = cenv }
