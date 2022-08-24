(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env

type type_member =
  | Error of Typing_error.t option
  | Exact of locl_ty
  | Abstract of {
      (* The bounds are optional: None for lower is equivalent
       * to nothing and None for upper is equivalent to mixed.
       * Having them optional ensures that [collect_bounds] below
       * does not pick up trivial bounds and clutter the tpenv. *)
      lower: locl_ty option;
      upper: locl_ty option;
    }

let make_missing_err ~on_error cls_id type_id =
  match on_error with
  | Some on_error ->
    Some
      Typing_error.(
        apply_reasons ~on_error
        @@ Secondary.Missing_type_constant
             { pos = fst cls_id; class_id = snd cls_id; type_id = snd type_id })
  | None -> None

(* Lookups a type member from the class decl of `cls_id`. *)
let lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id =
  let ety_env = { empty_expand_env with this_ty; on_error } in
  (* Things are not perfect here, localize may itself call into
   * Typing_taccess, leading to legacy behavior. *)
  let localize env decl_ty = Typing_phase.localize ~ety_env env decl_ty in
  match Env.get_class env (snd cls_id) with
  | None -> (env, Error (make_missing_err ~on_error cls_id type_id))
  | Some cls ->
    begin
      match Env.get_typeconst env cls (snd type_id) with
      | None -> (env, Error (make_missing_err ~on_error cls_id type_id))
      | Some { ttc_kind = TCConcrete tcc; _ } ->
        let ((env, err_opt), lty) = localize env tcc.tc_type in
        ( env,
          (match err_opt with
          | Some _ -> Error err_opt
          | None -> Exact lty) )
      | Some { ttc_kind = TCAbstract tca; _ } ->
        let ((env, err_opt_1), lty_as) =
          match tca.atc_as_constraint with
          | Some decl_ty ->
            let ((env, err_opt), lty) = localize env decl_ty in
            ((env, err_opt), Some lty)
          | None -> ((env, None), None)
        in
        let ((env, err_opt_2), lty_super) =
          match tca.atc_super_constraint with
          | Some decl_ty ->
            let ((env, err_opt), lty) = localize env decl_ty in
            ((env, err_opt), Some lty)
          | None -> ((env, None), None)
        in
        ( env,
          (match Option.first_some err_opt_1 err_opt_2 with
          | Some _ as err_opt -> Error err_opt
          | None -> Abstract { lower = lty_super; upper = lty_as }) )
    end

let lookup_class_type_member env ~on_error ~this_ty (cls_id, exact) type_id =
  let refined_type_member =
    match exact with
    | Nonexact cr ->
      begin
        match Class_refinement.get_type_ref type_id cr with
        | Some (TRexact ty) -> Exact ty
        (* TODO(refinements): For `TRloose _` we will return `Abstract _` *)
        | None -> Error None
      end
    | _ -> Error None
  in
  match refined_type_member with
  | Exact _ -> (env, refined_type_member)
  (* TODO(refinements): `Abstract _` will lookup the type member in the
   * class and combine the two results. *)
  | _ -> lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id

let make_dep_bound_type_member env ~on_error ~this_ty dep_kind bnd_ty type_id =
  let rec collect_bounds env bnd_ty =
    let (env, bnd_ty) = Env.expand_type env bnd_ty in
    match deref bnd_ty with
    | (_, Tclass (x_bnd, exact_bnd, _tyl_bnd)) ->
      let (env, type_member) =
        lookup_class_type_member
          env
          ~on_error
          ~this_ty
          (x_bnd, exact_bnd)
          type_id
      in
      (match type_member with
      | Error _ -> (env, [], [])
      | Exact ty -> (env, [ty], [ty])
      | Abstract { lower; upper } ->
        (env, Option.to_list lower, Option.to_list upper))
    | (_, Tintersection tyl) ->
      List.fold tyl ~init:(env, [], []) ~f:(fun (env, l, h) bnd_ty ->
          let (env, l', h') = collect_bounds env bnd_ty in
          (env, l @ l', h @ h'))
    | (_, _) -> (env, [], [])
  in
  let rigid_tvar_name = DependentKind.to_string dep_kind ^ "::" ^ snd type_id in
  let reason = Reason.Rnone in
  let ty = Typing_make_type.generic reason rigid_tvar_name in
  let (env, lower_bounds, upper_bounds) = collect_bounds env bnd_ty in
  let add_bounds bounds add env =
    List.fold bounds ~init:env ~f:(fun env bnd -> add env rigid_tvar_name bnd)
  in
  let env = add_bounds lower_bounds Env.add_lower_bound env in
  let env = add_bounds upper_bounds Env.add_upper_bound env in
  (env, ty)
