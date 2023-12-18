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
module TySet = Typing_set

type type_member =
  | NotYetAvailable
  | Error of Typing_error.t option
  | Exact of locl_ty
  | Abstract of {
      name: pos_id;
      (* The bounds are optional: None for lower is equivalent
       * to nothing and None for upper is equivalent to mixed.
       * Having them optional ensures that [collect_bounds] below
       * does not pick up trivial bounds and clutter the tpenv. *)
      lower: locl_ty option;
      upper: locl_ty option;
    }

type unknown_concrete_class_kind =
  | EDT of Expression_id.t
  | This

let make_missing_err ~on_error cls_id const_name =
  match on_error with
  | Some on_error ->
    Some
      Typing_error.(
        apply_reasons ~on_error
        @@ Secondary.Missing_class_constant
             { pos = fst cls_id; class_name = snd cls_id; const_name })
  | None -> None

(* Lookups a type member from the class decl of `cls_id`. *)
let lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id =
  let ety_env = { empty_expand_env with this_ty; on_error } in
  (* Things are not perfect here, localize may itself call into
   * Typing_taccess, leading to legacy behavior. *)
  let localize env decl_ty = Typing_utils.localize ~ety_env env decl_ty in
  match Env.get_class env (snd cls_id) with
  | Decl_entry.NotYetAvailable -> (env, NotYetAvailable)
  | Decl_entry.DoesNotExist ->
    (env, Error (make_missing_err ~on_error cls_id (snd type_id)))
  | Decl_entry.Found cls -> begin
    match Env.get_typeconst env cls (snd type_id) with
    | None -> (env, Error (make_missing_err ~on_error cls_id (snd type_id)))
    | Some { ttc_kind = TCConcrete tcc; _ } ->
      let ((env, err_opt), lty) = localize env tcc.tc_type in
      ( env,
        (match err_opt with
        | Some _ -> Error err_opt
        | None -> Exact lty) )
    | Some { ttc_kind = TCAbstract tca; ttc_name = name; _ } ->
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
        | None -> Abstract { name; lower = lty_super; upper = lty_as }) )
  end

let lookup_class_type_member env ~on_error ~this_ty (cls_id, exact) type_id =
  let (combine_lower, combine_upper) =
    let dedup_then ~f tys =
      let ts = TySet.of_list tys in
      if TySet.is_empty ts then
        None
      else
        Some (f (TySet.elements ts))
    in
    ( dedup_then ~f:(Typing_make_type.union Reason.Rnone),
      dedup_then ~f:(Typing_make_type.intersection Reason.Rnone) )
  in
  let refined_type_member =
    match exact with
    | Nonexact cr -> begin
      match Class_refinement.get_refined_const type_id cr with
      | Some { rc_bound = TRexact ty; _ } -> Exact ty
      | Some { rc_bound = TRloose { tr_lower; tr_upper }; _ } ->
        let lower = combine_lower tr_lower in
        let upper = combine_upper tr_upper in
        (* FIXME(refinements): The position is pointing at
         * the class when we would like to point in the
         * refinement. *)
        let name = (fst cls_id, snd type_id) in
        Abstract { name; lower; upper }
      | None -> Error None
    end
    | _ -> Error None
  in
  match refined_type_member with
  | Exact _ -> (env, refined_type_member)
  | Abstract { name = _; lower = mem_lower; upper = mem_upper } ->
    (* In this case, we still lookup the class decl to potentially
     * obtain more information on the type member. *)
    (match
       lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id
     with
    | (env, NotYetAvailable) -> (env, NotYetAvailable)
    | (env, Error _) -> (env, refined_type_member)
    | (_env, Exact _) as result -> result
    | (env, Abstract { name = cls_name; lower = cls_lower; upper = cls_upper })
      ->
      let to_list b1 b2 = List.filter_map ~f:Fn.id [b1; b2] in
      let lower = combine_lower (to_list mem_lower cls_lower) in
      let upper = combine_upper (to_list mem_upper cls_upper) in
      (env, Abstract { name = cls_name; lower; upper }))
  | _ -> lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id

let make_type_member env ~on_error ~this_ty ucc_kind bnd_tys type_id =
  let rec collect_bounds env lo_bnds up_bnds = function
    | bnd_ty :: bnd_tys ->
      let (env, bnd_ty) = Env.expand_type env bnd_ty in
      (match deref bnd_ty with
      | (_, Tclass (x_bnd, exact_bnd, _tyl_bnd)) ->
        let (env, type_member) =
          lookup_class_type_member
            env
            ~on_error
            ~this_ty
            (x_bnd, exact_bnd)
            type_id
        in
        let (lo_bnds, up_bnds) =
          match type_member with
          | NotYetAvailable
          | Error _ ->
            (lo_bnds, up_bnds)
          | Exact ty -> (ty :: lo_bnds, ty :: up_bnds)
          | Abstract { name = _; lower; upper } ->
            let maybe_add bnds =
              Option.fold ~init:bnds ~f:(Fun.flip List.cons)
            in
            (maybe_add lo_bnds lower, maybe_add up_bnds upper)
        in
        collect_bounds env lo_bnds up_bnds bnd_tys
      | (_, Tintersection tyl) ->
        collect_bounds env lo_bnds up_bnds (tyl @ bnd_tys)
      | (_, _) -> collect_bounds env lo_bnds up_bnds bnd_tys)
    | [] -> (env, lo_bnds, up_bnds)
  in
  let rigid_tvar_name =
    let ucc_string_id =
      match ucc_kind with
      | This -> "this"
      | EDT eid -> DependentKind.to_string (DTexpr eid)
    in
    ucc_string_id ^ "::" ^ snd type_id
  in
  let reason = Reason.Rnone in
  let ty = Typing_make_type.generic reason rigid_tvar_name in
  let (env, lower_bounds, upper_bounds) = collect_bounds env [] [] bnd_tys in
  let add_bounds bounds add env =
    List.fold bounds ~init:env ~f:(fun env bnd -> add env rigid_tvar_name bnd)
  in
  let env = add_bounds lower_bounds Env.add_lower_bound env in
  let env = add_bounds upper_bounds Env.add_upper_bound env in
  (env, ty)
