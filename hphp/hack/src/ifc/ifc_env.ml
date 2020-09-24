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

(* Only elementary logic about the environments should
   be in this file to avoid circular dependencies;
   use higher-order functions to parameterize complex
   behavior! *)

let new_policy_var { re_scope; re_pvar_counters; _ } prefix =
  let suffix =
    match Caml.Hashtbl.find_opt re_pvar_counters prefix with
    | Some counter ->
      incr counter;
      "'" ^ string_of_int !counter
    | None ->
      Caml.Hashtbl.add re_pvar_counters prefix (ref 0);
      ""
  in
  Pfree_var (prefix ^ suffix, re_scope)

(* Creates a read-only environment sufficient to call functions
   from ifc_lift.ml *)
let new_renv scope decl_env saved_tenv : proto_renv =
  {
    re_scope = scope;
    re_decl = decl_env;
    re_pvar_counters = Caml.Hashtbl.create 10;
    re_tenv = saved_tenv;
    (* the fields below are initialized with dummy values *)
    re_this = None;
    re_ret = ();
    re_gpc = pbot;
    re_exn = ();
  }

(* Prepares a read-only environment to type-check a function *)
let prep_renv renv this_ty_opt ret_ty gpc_pol exn_ty : renv =
  {
    re_scope = renv.re_scope;
    re_decl = renv.re_decl;
    re_pvar_counters = renv.re_pvar_counters;
    re_tenv = renv.re_tenv;
    re_this = this_ty_opt;
    re_ret = ret_ty;
    re_gpc = gpc_pol;
    re_exn = exn_ty;
  }

let empty_lenv = { le_vars = LMap.empty; le_pc = PSet.empty }

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
  let le_pc = PSet.union lenv1.le_pc lenv2.le_pc in
  (env, { le_vars; le_pc })

(* Merge continuation envs, if a continuation is assigned a
   local env only in one of the arguments it will be kept as
   is in the result. This is because lack of a continuation
   means that some code was, up to now, dead code. *)
let merge_cenvs ~union env cenv1 cenv2 =
  let combine = Utils.mk_combine true (merge_lenv ~union) in
  KMap.merge_env env cenv1 cenv2 ~combine

let merge_and_set_cenv ~union env cenv1 cenv2 =
  let (env, cenv) = merge_cenvs ~union env cenv1 cenv2 in
  set_cenv env cenv

let get_lenv_opt env k = KMap.find_opt k (get_cenv env)

let set_lenv env k lenv = set_cenv env (KMap.add k lenv (get_cenv env))

let set_local_type env lid pty =
  match get_lenv_opt env K.Next with
  | None -> env
  | Some lenv ->
    let lenv = { lenv with le_vars = LMap.add lid pty lenv.le_vars } in
    set_lenv env K.Next lenv

let get_local_type env lid =
  Option.(
    get_lenv_opt env K.Next >>= fun lenv -> LMap.find_opt lid lenv.le_vars)

let fold_locals env fn =
  match get_lenv_opt env K.Next with
  | None -> (fun acc -> acc)
  | Some lenv -> LMap.fold fn lenv.le_vars

let get_lpc env k =
  match get_lenv_opt env k with
  | None -> PSet.empty
  | Some lenv -> lenv.le_pc

let set_lpc env k pc =
  match get_lenv_opt env k with
  | None -> env
  | Some lenv -> set_lenv env k { lenv with le_pc = pc }

let get_pc renv env k = PSet.add renv.re_gpc (get_lpc env k)

let push_pcs env k pcset =
  match get_lenv_opt env k with
  | None -> env
  | Some lenv ->
    let lenv = { lenv with le_pc = PSet.union pcset lenv.le_pc } in
    set_lenv env k lenv

let merge_lenv_into ~union env lenv k =
  match get_lenv_opt env k with
  | None -> set_lenv env k lenv
  | Some lenv' ->
    let (env, merged_lenv) = merge_lenv ~union env lenv lenv' in
    set_lenv env k merged_lenv

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
  filter_conts env (fun k -> not @@ List.mem ~equal:K.equal ks k)

(* Saves the continuations in ks and runs fn with these slots
   cleared, restores the saved continuations once fn is done;
   handy to analyze loops, for example *)
let with_stashed_conts env ks fn =
  let stashed = List.map ~f:(fun k -> (k, KMap.find_opt k env.e_cont)) ks in
  let env = drop_conts env ks in
  let env = fn env in
  let env =
    List.fold stashed ~init:env ~f:(fun env (k, ke) ->
        match ke with
        | None -> set_cenv env @@ KMap.remove k env.e_cont
        | Some ke -> set_cenv env @@ KMap.add k ke env.e_cont)
  in
  env

(* Merge conts and then clear them *)
let move_conts_into ~union env ks k_to =
  let env = merge_conts_into ~union env ks k_to in
  drop_conts env ks

let merge_pcs_into env ks k_to =
  let f pc k = PSet.union (get_lpc env k) pc in
  let pc = List.fold ~f ~init:PSet.empty ks in
  match get_lenv_opt env k_to with
  | None -> env
  | Some lenv ->
    set_lenv env k_to { lenv with le_pc = PSet.union pc lenv.le_pc }

(* Freshen the types of locals in 'lids' and drop all other locals *)
let freshen_cenv ~freshen renv env lids =
  let lids = List.map ~f:snd lids |> LSet.of_list in
  let freshen_lenv env _ lenv =
    let (env, vars) =
      LMap.filter (fun lid _ -> LSet.mem lid lids) lenv.le_vars
      |> LMap.map_env (fun env _ l -> freshen renv env l) env
    in
    (env, { lenv with le_vars = vars })
  in
  let (env, cenv) = KMap.map_env freshen_lenv env env.e_cont in
  { env with e_cont = cenv }

let with_fresh_conts ~union env ks f =
  let base_cenv = get_cenv env in
  let env = drop_conts env ks in
  let env = f env in
  merge_conts_from ~union env base_cenv ks
