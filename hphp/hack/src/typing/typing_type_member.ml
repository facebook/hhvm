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

(* This module is meant to eventually replace Typing_taccess,
 * with drastically simpler logic. The idea here is to only
 * have *lookup* logic on class types, unlike Typing_taccess
 * that aims to work on any locl type. *)

type type_member =
  | Error of Typing_error.t option
  | Exact of locl_ty
  | Abstract of {
      lower: locl_ty;
      upper: locl_ty;
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
        let rnone = Reason.Rnone in
        let ((env, err_opt_1), lty_as) =
          match tca.atc_as_constraint with
          | Some decl_ty -> localize env decl_ty
          | None -> ((env, None), Typing_make_type.mixed rnone)
        in
        let ((env, err_opt_2), lty_super) =
          match tca.atc_super_constraint with
          | Some decl_ty -> localize env decl_ty
          | None -> ((env, None), Typing_make_type.nothing rnone)
        in
        ( env,
          (match Option.first_some err_opt_1 err_opt_2 with
          | Some _ as err_opt -> Error err_opt
          | None -> Abstract { lower = lty_super; upper = lty_as }) )
    end

(* Lookups a type member in a class definition, or in refinement
 * information (`exact`). *)
let lookup_type_member env ~on_error ~this_ty (cls_id, exact) type_id =
  let refined_type_member =
    match exact with
    | Nonexact cr ->
      begin
        match Class_refinement.get_type_ref type_id cr with
        | Some (Texact ty) -> Exact ty
        (* TODO(refinements): For `Tloose _` we will return `Abstract _` *)
        | None -> Error None
      end
    | _ -> Error None
  in
  match refined_type_member with
  | Exact _ -> (env, refined_type_member)
  (* TODO(refinements): `Abstract _` will lookup the type member in the
   * class and combine the two results. *)
  | _ -> lookup_class_decl_type_member env ~on_error ~this_ty cls_id type_id
