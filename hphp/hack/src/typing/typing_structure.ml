(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type
module Phase = Typing_phase
module Reason = Typing_reason
module SN = Naming_special_names
module Subst = Decl_subst
module TUtils = Typing_utils

(** This module implements the typing for type_structure. *)

let make_ts : Typing_env_types.env -> locl_ty -> Typing_env_types.env * locl_ty
    =
 fun env ty ->
  let r = get_reason ty in
  match Env.get_typedef env SN.FB.cTypeStructure with
  | Some { td_tparams; _ } ->
    (* Typedef parameters can not have constraints *)
    let params =
      List.map
        ~f:
          begin
            fun { tp_name = (p, x); _ } ->
              mk (Reason.Rwitness_from_decl p, Tgeneric (x, []))
          end
        td_tparams
    in
    let ts =
      mk
        (Reason.Rnone, Tapply ((Pos_or_decl.none, SN.FB.cTypeStructure), params))
    in
    let ety_env =
      {
        (empty_expand_env_with_on_error
           (Typing_error.Reasons_callback.invalid_type_hint
              (Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos)))
        with
        substs = Subst.make_locl td_tparams [ty];
      }
    in
    let ((env, ty_err), ty) = Phase.localize ~ety_env env ts in
    let ty = with_reason ty r in
    Option.iter ~f:Errors.add_typing_error ty_err;
    (env, ty)
  | _ ->
    (* Should not hit this because TypeStructure should always be defined *)
    (env, MakeType.dynamic r)

let rec transform_shapemap ?(nullable = false) env pos ty shape =
  let ((env, ty_err_opt), ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      ty
  in
  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  (* If there are Tanys, be conservative and don't try to represent the
   * type more precisely
   *)
  if TUtils.HasTany.check ty then
    (env, shape)
  else
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Toption ty -> transform_shapemap ~nullable:true env pos ty shape
    | _ ->
      let base_type = TUtils.get_base_type env ty in
      (* If the abstract type is unbounded we do not specialize at all *)
      let is_unbound =
        match get_node base_type with
        | Tgeneric _ -> true
        | _ -> false
      in
      let is_generic =
        match get_node ty with
        | Tgeneric _ -> true
        | _ -> false
      in
      let (supportdyn, env, ty) = TUtils.strip_supportdyn env ty in
      let base_type = TUtils.get_base_type ~expand_supportdyn:false env ty in
      let (supportdyn_bound, env, base_type) =
        TUtils.strip_supportdyn env base_type
      in
      let base_type =
        if is_unbound then
          MakeType.mixed (get_reason base_type)
        else
          base_type
      in
      (* Does this type contain only enum or object instances?
       * These are the types that can be represented using classname
       *)
      let rec is_enum_or_classish ty =
        match get_node ty with
        | Tnewtype (cid, _, _) when Env.is_enum env cid -> true
        | Tintersection tys -> List.exists tys ~f:is_enum_or_classish
        | Tunion tys -> List.for_all tys ~f:is_enum_or_classish
        | Tclass ((_, x), _, _)
          when not
                 (String.equal x SN.Collections.cVec
                 || String.equal x SN.Collections.cDict
                 || String.equal x SN.Collections.cKeyset) ->
          true
        | Tgeneric _ ->
          let (_env, tyl) =
            Typing_utils.get_concrete_supertypes
              ~expand_supportdyn:true
              ~abstract_enum:false
              env
              ty
          in
          List.exists tyl ~f:is_enum_or_classish
        | _ -> false
      in
      let supportdyn = supportdyn || supportdyn_bound in
      let transform_shape_field field { sft_ty; _ } (env, shape) =
        (* Accumulates the provided type for this iteration of the fold, adding
           it to the accumulation ShapeMap for the current field. Since the
           field must have been explicitly set, we set sft_optional to true. *)
        let acc_field_with_type sft_ty =
          TShapeMap.add field { sft_optional = false; sft_ty } shape
        in
        let (env, sft_ty) = Env.expand_type env sft_ty in
        match (field, deref sft_ty, deref base_type) with
        | (TSFlit_str (_, "nullable"), (_, Toption fty), _) when nullable ->
          (env, acc_field_with_type fty)
        | (TSFlit_str (_, "nullable"), (_, Toption fty), (_, Toption _)) ->
          (env, acc_field_with_type fty)
        | (TSFlit_str (_, "classname"), (_, Toption fty), _)
          when is_enum_or_classish ty ->
          (env, acc_field_with_type fty)
        | (TSFlit_str (_, "elem_types"), _, (r, Ttuple tyl)) ->
          let (env, tyl) = List.map_env env tyl ~f:make_ts in
          (env, acc_field_with_type (mk (r, Ttuple tyl)))
        | (TSFlit_str (_, "param_types"), _, (r, Tfun funty)) ->
          let tyl = List.map funty.ft_params ~f:(fun x -> x.fp_type.et_type) in
          let (env, tyl) = List.map_env env tyl ~f:make_ts in
          (env, acc_field_with_type (mk (r, Ttuple tyl)))
        | (TSFlit_str (_, "return_type"), _, (r, Tfun funty)) ->
          let (env, ty) = make_ts env funty.ft_ret.et_type in
          (env, acc_field_with_type (mk (r, Ttuple [ty])))
        | (TSFlit_str (_, "fields"), _, (r, Tshape (_, shape_kind, fields))) ->
          let (env, fields) = ShapeFieldMap.map_env make_ts env fields in
          let ty = mk (r, Tshape (Missing_origin, shape_kind, fields)) in
          let ty =
            if supportdyn then
              MakeType.supportdyn r ty
            else
              ty
          in
          (env, acc_field_with_type ty)
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
        | (TSFlit_str (_, "generic_types"), _, _) when is_generic ->
          (env, acc_field_with_type sft_ty)
        | (TSFlit_str (_, "generic_types"), _, (r, Tclass (_, _, tyl)))
          when List.length tyl > 0 ->
          let (env, tyl) = List.map_env env tyl ~f:make_ts in
          (env, acc_field_with_type (mk (r, Ttuple tyl)))
        | (TSFlit_str (_, ("kind" | "name" | "alias")), _, _) ->
          (env, acc_field_with_type sft_ty)
        | (_, _, _) -> (env, shape)
      in
      TShapeMap.fold transform_shape_field shape (env, TShapeMap.empty)
