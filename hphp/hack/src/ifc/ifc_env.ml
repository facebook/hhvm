(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ifc_types
module Utils = Ifc_utils
module K = Typing_cont_key

(* See ifc_env.mli for the docmentation *)

(* Only elementary logic about the environments should
   be in this file to avoid circular dependencies;
   use higher-order functions to parameterize complex
   behavior! *)

(* - Read-only environments (renv) - *)

let new_policy_var { re_scope; re_pvar_counters; _ } prefix =
  let prefix = String.lstrip ~drop:(Char.equal '\\') prefix in
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

let new_renv scope decl_env saved_tenv =
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

let prep_renv renv this_ty_opt ret_ty exn_ty gpc_pol =
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

(* - Read/write environments (env) - *)

type yes = unit

type no = unit

type ('has_locals, 'can_throw) env = {
  e_nxt: cont;
  e_exn: cont option;
  e_acc: prop list;
  e_deps: SSet.t;
}

type blank_env = (no, no) env

type stmt_env = (yes, no) env

type expr_env = (yes, yes) env

let empty_cont = { k_pc = PSet.empty; k_vars = LMap.empty }

let empty_env =
  { e_nxt = empty_cont; e_exn = None; e_acc = []; e_deps = SSet.empty }

let prep_stmt env cont = { env with e_nxt = cont; e_exn = None }

let prep_expr env = { env with e_exn = None }

let get_lpc env = env.e_nxt.k_pc

let get_gpc renv env = PSet.add renv.re_gpc (get_lpc env)

let get_deps env = env.e_deps

let get_constraints env = env.e_acc

let get_locals env = env.e_nxt.k_vars

let get_next env = env.e_nxt

(* Not exported, see with_pc, with_pc_deps, and throw *)
let set_pc env pc = { env with e_nxt = { env.e_nxt with k_pc = pc } }

let with_pc env pc (fn : stmt_env -> blank_env * 'a) =
  let env = set_pc env pc in
  (* there is nothing to restore in the resulting env because
     fn returns a blank env (i.e., with e_nxt cleared) *)
  fn env

let with_pc_deps env deps = with_pc env (PSet.union (get_lpc env) deps)

let acc env update = { env with e_acc = update env.e_acc }

let add_dep env name = { env with e_deps = SSet.add name env.e_deps }

let close_expr env =
  let out_throw =
    match env.e_exn with
    | None -> KMap.empty
    | Some cont -> KMap.singleton K.Catch cont
  in
  let env = prep_stmt env env.e_nxt in
  (env, out_throw)

(* To merge contexts we need to compute union types for local variables;
   we don't want to be smart here, to avoid lagging behind the fancy
   heuristics in the Hack typechecker, so we are instead very dumb!
   This lack of sophistication is mitigated by the logic in the IFC
   analysis that keeps looking for better types in the annotated AST *)
let union_types t1 t2 =
  if phys_equal t1 t2 then
    t1
  else
    Tunion [t1; t2]

(* Merge two local envs, if a variable appears only in one local
   env, it will not appear in the result env *)
let merge_cont cont1 cont2 =
  let combine _ = Utils.combine_opts false union_types in
  let k_vars = LMap.merge combine cont1.k_vars cont2.k_vars in
  let k_pc = PSet.union cont1.k_pc cont2.k_pc in
  { k_vars; k_pc }

let merge_cont_opt = Utils.combine_opts true merge_cont

let throw env deps =
  let env = set_pc env (PSet.union (get_lpc env) deps) in
  { env with e_exn = merge_cont_opt env.e_exn (Some (get_next env)) }

let analyze_lambda_body env fn =
  let e_nxt = get_next env in
  let e_exn = env.e_exn in
  let env = fn env in
  { env with e_nxt; e_exn }

let get_local_type env lid = LMap.find_opt lid env.e_nxt.k_vars

let set_local_type env lid pty =
  let k_vars = LMap.add lid pty env.e_nxt.k_vars in
  { env with e_nxt = { env.e_nxt with k_vars } }

let set_local_type_opt env lid pty =
  let k_vars = LMap.update lid (fun _ -> pty) env.e_nxt.k_vars in
  { env with e_nxt = { env.e_nxt with k_vars } }

(* - Outcomes - *)

let merge_out out1 out2 = KMap.merge (fun _ -> merge_cont_opt) out1 out2

let close_stmt ?(merge = KMap.empty) env k =
  let out = KMap.singleton k (get_next env) in
  let out = merge_out out merge in
  let env = { env with e_nxt = empty_cont; e_exn = None } in
  (env, out)

let strip_cont out k = (KMap.remove k out, KMap.find_opt k out)

let merge_in_next out k =
  let (out, cont_k) = strip_cont out k in
  let (out, cont_next) = strip_cont out K.Next in
  match merge_cont_opt cont_k cont_next with
  | None -> out
  | Some cont -> KMap.add K.Next cont out

let merge_next_in out k =
  let (out, cont_k) = strip_cont out k in
  let (out, cont_next) = strip_cont out K.Next in
  match merge_cont_opt cont_k cont_next with
  | None -> out
  | Some cont -> KMap.add k cont out
