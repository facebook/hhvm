(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module TP = Type_parameter_env
module TySet = Typing_set
module Env = Typing_env
open Core_kernel
open Common
open Typing_defs

let single_upper_bound env r ul =
  Typing_intersection.intersect_list env r (TySet.elements ul)

let single_lower_bound env r ll =
  Typing_union.union_list env r (TySet.elements ll)

let remove_upper_bound tpenv tparam bound =
  match SMap.get tparam tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.TP.upper_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = TP.{ tparam_info with upper_bounds = bounds } in
    SMap.add tparam tparam_info tpenv

let remove_lower_bound tpenv tparam bound =
  match SMap.get tparam tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.TP.lower_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = TP.{ tparam_info with lower_bounds = bounds } in
    SMap.add tparam tparam_info tpenv

let remove_from_tpenv tpenv tparam =
  let tparam_ty = (Reason.Rnone, Tabstract (AKgeneric tparam, None)) in
  let lower_bounds = Env.get_tpenv_lower_bounds tpenv tparam in
  let remove_tparam_from_upper_bounds_of tparam tpenv =
    match tparam with
    | _, Tabstract (AKgeneric tparam, _) ->
      remove_upper_bound tpenv tparam tparam_ty
    | _ -> tpenv in
  let tpenv = TySet.fold remove_tparam_from_upper_bounds_of lower_bounds tpenv in
  let upper_bounds = Env.get_tpenv_upper_bounds tpenv tparam in
  let remove_tparam_from_lower_bounds_of tparam tpenv =
    match tparam with
    | _, Tabstract (AKgeneric tparam, _) ->
      remove_lower_bound tpenv tparam tparam_ty
    | _ -> tpenv in
  let tpenv = TySet.fold remove_tparam_from_lower_bounds_of upper_bounds tpenv in
  SMap.remove tparam tpenv

let get_tpenv_equal_bounds env name =
  let lower = Env.get_tpenv_lower_bounds env name in
  let upper = Env.get_tpenv_upper_bounds env name in
  TySet.inter lower upper


(** Given a list of type parameter names, attempt to simplify away those
type parameters by looking for a type to which they are equal in the tpenv.
If such a type exists, remove the type parameter from the tpenv.
Returns a set of substitutions mapping each type parameter name to the type
to which it is equal if found, otherwise to itself. *)
let simplify_tpenv env (tparams : ((_ * string) option * locl ty) list) r =
  let old_env = env in
  let tpenv = Env.get_tpenv env in
  (* For each tparam, "solve" it if it falls in any of those categories:
   *   - there exists a type ty to which it is equal
   *   - it is covariant and has upper bound ty (or mixed if absent)
   *   - it is contravariant and lower bound ty (or nothing if absent).
   * In which case remove tparam from tpenv and add substitution
   * (tparam -> ty) to substs. *)
  let (tpenv, substs) = List.fold tparams ~init:(tpenv, SMap.empty)
    ~f:(fun (tpenv, substs) (p_opt, (reason, _)) ->
      match p_opt with
      | None ->
        tpenv, substs
      | Some (tp, tparam_name) ->
      let equal_bounds = get_tpenv_equal_bounds tpenv tparam_name in
      let lower_bounds = Env.get_tpenv_lower_bounds tpenv tparam_name in
      let upper_bounds = Env.get_tpenv_upper_bounds tpenv tparam_name in
      let _env, lower_bound = single_lower_bound env reason lower_bounds in
      let _env, upper_bound = single_upper_bound env reason upper_bounds in
      (* remove tparam_name from tpenv, and in any lower/upper bound set
       * where it occurs.
       * We don't need to do any merging of lower/upper bounds,
       * because all its lower/upper bounds
       * are already lower/upper bounds of `bound` (and other bounds)
       * thanks to the transitive closure we've done in
       * Typing_subtype.add_constraint. *)
      match tp.tp_variance, TySet.choose_opt equal_bounds with
      | _, Some bound ->
        let tpenv = remove_from_tpenv tpenv tparam_name in
        let substs = SMap.add tparam_name bound substs in
        tpenv, substs
      | Ast_defs.Covariant, _ ->
        let tpenv = remove_from_tpenv tpenv tparam_name in
        let substs = SMap.add tparam_name upper_bound substs in
        tpenv, substs
      | Ast_defs.Contravariant, _->
        let tpenv = remove_from_tpenv tpenv tparam_name in
        let substs = SMap.add tparam_name lower_bound substs in
        tpenv, substs
      | _ ->
        let tparam_ty = (r, Tabstract (AKgeneric tparam_name, None)) in
        let substs = SMap.add tparam_name tparam_ty substs in
        tpenv, substs) in
    (* reduce the set of substitutions. For example, for a set of substitutions
     * like
     *   Ta -> Tb
     *   Tb -> int
     * simplify to:
     *   Ta -> int
     *   Tb -> int
     *)
    let rec reduce substs tparam =
      match SMap.find_opt tparam substs with
      | None -> substs, None
      | Some (_, Tabstract (AKgeneric tparam', _) as subst) when tparam' <> tparam ->
        let substs, new_subst_opt = reduce substs tparam' in
        begin match new_subst_opt with
        | None -> substs, Some subst
        | Some new_subst ->
          let substs = SMap.add tparam new_subst substs in
          substs, Some new_subst
        end
      | Some subst ->
        substs, Some subst in
    let reduce substs (p_opt, _) =
      match p_opt with None -> substs | Some (_, name) -> fst (reduce substs name) in
    let substs = List.fold tparams ~init:substs ~f:reduce in
    Env.log_env_change "simplify_tpenv" old_env @@ Env.env_with_tpenv env tpenv, substs
