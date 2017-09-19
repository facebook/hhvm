(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This module implements the typing for type_structure. *)
open Core
open Nast
open Typing_defs

module Env = Typing_env
module Phase = Typing_phase
module Reason = Typing_reason
module SN = Naming_special_names
module Subst = Decl_subst
module TUtils = Typing_utils

let make_ts env ty =
  match Env.get_typedef env SN.FB.cTypeStructure with
  | Some {td_tparams; _} ->
      (* Typedef parameters can not have constraints *)
      let params = List.map ~f:begin fun (_, (p, x), _) ->
        Reason.Rwitness p, Tgeneric x
      end td_tparams in
      let ts = fst ty, Tapply ((Pos.none, SN.FB.cTypeStructure), params) in
      let ety_env = { (Phase.env_with_self env) with
                      substs = Subst.make td_tparams [ty] } in
      Phase.localize ~ety_env env ts
  | _ ->
      (* Should not hit this because TypeStructure should always be defined *)
      env, (fst ty, Tany)

let rec transform_shapemap ?(nullable = false) env ty shape =
  let env, ty = TUtils.fold_unresolved env ty in
  let env, ety = Env.expand_type env ty in
  (* If there are Tanys, be conservative and don't try to represent the
   * type more precisely
   *)
  if TUtils.HasTany.check ty then env, shape else
  match ety with
  | _, Toption ty ->
      transform_shapemap ~nullable:true env ty shape
  | _ ->
      (* If the abstract type is unbounded we do not specialize at all *)
      let is_unbound = match ety |> TUtils.get_base_type env |> snd with
        (* An enum is considered a valid bound *)
        | Tabstract (AKenum _, _) -> false
        | Tabstract (_, None) -> true
        | _ -> false in
      if is_unbound then (env, shape) else
      let is_generic =
        match snd ety with Tabstract (AKgeneric _, _) -> true | _ -> false in
      let transform_shape_field field { sft_ty; _ } (env, shape) =
        let open Ast in

        (* Accumulates the provided type for this iteration of the fold, adding
           it to the accumulation ShapeMap for the current field. Since the
           field must have been explicitly set, we set sft_optional to true. *)
        let acc_field_with_type sft_ty =
          ShapeMap.add field { sft_optional = false; sft_ty } shape in

        match field, sft_ty, TUtils.get_base_type env ety with
        | SFlit (_, "nullable"), (_, Toption (fty)), _ when nullable ->
            env, acc_field_with_type fty
        | SFlit (_, "nullable"), (_, Toption (fty)), (_, Toption _) ->
            env, acc_field_with_type fty
        | SFlit (_, "classname"), (_, Toption (fty)),
          (_, (Tclass _ | Tabstract (AKenum _, _))) ->
            env, acc_field_with_type fty
        | SFlit (_, "elem_types"), _, (r, Ttuple tyl) ->
            let env, tyl = List.map_env env tyl make_ts in
            env, acc_field_with_type (r, Ttuple tyl)
        | SFlit (_, "param_types"), _, (r, (Tfun funty)) ->
            let tyl = List.map funty.ft_params (fun x -> x.fp_type) in
            let env, tyl = List.map_env env tyl make_ts in
            env, acc_field_with_type (r, Ttuple tyl)
        | SFlit (_, "return_type"), _, (r, Tfun funty) ->
            let env, ty = make_ts env funty.ft_ret in
            env, acc_field_with_type (r, Ttuple [ty])
        | SFlit (_, "fields"), _, (r, Tshape (fk, fields)) ->
            let env, fields = ShapeFieldMap.map_env make_ts env fields in
            env, acc_field_with_type (r, Tshape (fk, fields))
        (* For generics we cannot specialize the generic_types field. Consider:
         *
         *  class C<T> {}
         *  class D extends C<int> {}
         *
         *  function test<T as C<int>>(TypeStructure<T> $ts): TypeStructure<int> {
         *    return $ts['generic_types'][0];
         *  }
         *
         * For test(TypeStructure<D>) there will not be a generic_types field
         *)
        | SFlit (_, "generic_types"), _, _ when is_generic ->
            env, acc_field_with_type sft_ty
        | SFlit (_, "generic_types"), _, (r, Tarraykind (AKvec ty))
              when not is_generic ->
            let env, ty = make_ts env ty in
            env, acc_field_with_type (r, Ttuple [ty])
        | SFlit (_, "generic_types"), _,
          (r, Tarraykind (AKmap (ty1, ty2)))
              when not is_generic ->
            let tyl = [ty1; ty2] in
            let env, tyl = List.map_env env tyl make_ts in
            env, acc_field_with_type (r, Ttuple tyl)
        | SFlit (_, "generic_types"), _, (r, Tclass (_, tyl))
              when List.length tyl > 0 ->
            let env, tyl = List.map_env env tyl make_ts in
            env, acc_field_with_type (r, Ttuple tyl)
        | SFlit (_, ("kind" | "name" | "alias")), _, _ ->
            env, acc_field_with_type sft_ty
        | _, _, _ ->
            env, shape in
      ShapeMap.fold transform_shape_field shape (env, ShapeMap.empty)
