(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Nast
open Typing_defs

module Env          = Typing_env
module Reason       = Typing_reason
module TUtils       = Typing_utils
module Type         = Typing_ops

let rec refine_shape field_name env shape =
  let env, shape = Env.expand_type env shape in
  match shape with
  | shape_r, Tshape (fields_known, fields) ->
    let refine_shape_field_type refined_sft_ty =
      let refined_sft = {sft_optional = false; sft_ty = refined_sft_ty} in
      let refined_fields = ShapeMap.add field_name refined_sft fields in
      env, (shape_r, Tshape (fields_known, refined_fields)) in
    begin match fields_known with
    | FieldsFullyKnown ->
      (* Closed shape *)
      begin match ShapeMap.get field_name fields with
      | None -> env, shape
      | Some {sft_ty; _} -> refine_shape_field_type sft_ty
      end
    | FieldsPartiallyKnown unset_fields ->
      (* Open shape *)
      if ShapeMap.mem field_name unset_fields
      then env, shape
      else
        let refined_sft_ty = match ShapeMap.get field_name fields with
          | None ->
            let printable_field_name =
              TUtils.get_printable_shape_field_name field_name in
            let sft_ty_r = Reason.Rmissing_optional_field
              (Reason.to_pos shape_r, printable_field_name) in
            (sft_ty_r, TUtils.desugar_mixed sft_ty_r)
          | Some {sft_ty; _} -> sft_ty in
        refine_shape_field_type refined_sft_ty
    end
  | r, Tunresolved tyl ->
    let env, tyl = List.map_env env tyl (refine_shape field_name) in
    env, (r, Tunresolved tyl)
  | _ -> env, shape

(*****************************************************************************)
(* Remove a field from all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec shrink_shape pos field_name env shape =
  let _, shape = Env.expand_type env shape in
  match shape with
  | _, Tshape (fields_known, fields) ->
      (* remember that we have unset this field *)
      let fields_known = match fields_known with
        | FieldsFullyKnown ->
            FieldsFullyKnown
        | FieldsPartiallyKnown unset_fields ->
            FieldsPartiallyKnown (ShapeMap.add field_name pos unset_fields) in
      let fields = ShapeMap.remove field_name fields in
      let result = Reason.Rwitness pos, Tshape (fields_known, fields) in
      env, result
  | _, Tunresolved tyl ->
      let env, tyl = List.map_env env tyl(shrink_shape pos field_name) in
      let result = Reason.Rwitness pos, Tunresolved tyl in
      env, result
  | x ->
      env, x

let experiment_enabled env experiment =
  TypecheckerOptions.experimental_feature_enabled
    (Env.get_options env)
    experiment

let make_idx_fake_super_shape field_name field_ty =
  Reason.Rnone,
  Tshape
    (FieldsPartiallyKnown Nast.ShapeMap.empty,
     Nast.ShapeMap.singleton field_name field_ty)

(* Is a given field required in a given shape?
 *
 * sfn is a required field of shape t iff t is a subtype of
 * shape(sfn => mixed, ...) (or shape(sfn => nonnull, ...) if
 * disable_optional_and_unknonw_shape_fields is enabled, as
 * in that case an option-typed field is considered optional).
 *
 * Note that unlike doing a case analysis on the shape type,
 * expressing this check using subtyping successfully deals
 * with the cases where the shape is unresolved or is abstract
 * (e.g., hidden behind a newtype or given by a constrained
 * generic parameter or type constant).
 *)
let is_shape_field_required env field_name shape_ty =
  let field_ty = {
    sft_optional = false;
    sft_ty =
      Reason.Rnone,
      if experiment_enabled env
           TypecheckerOptions.experimental_disable_optional_and_unknown_shape_fields
      then Tnonnull
      else TUtils.desugar_mixed Reason.Rnone
  } in
  Typing_subtype.is_sub_type env
    shape_ty
    (make_idx_fake_super_shape field_name field_ty)

(* Typing rules for Shapes::idx
 *
 *     e : shape(sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : t       if stronger_shape_idx_return is enabled
 *
 *     e : shape(?sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : ?t
 *
 *     e1 : shape(?sfn => t, ...)
 *     e2 : t
 *     ----------------------------
 *     Shapes::idx(e1, sfn, e2) : t
 *
 *)
let idx env _p fty shape_ty field default =
  let env, shape_ty = Env.expand_type env shape_ty in
  let env, res = Env.fresh_unresolved_type env in
  match TUtils.shape_field_name env field with
  | None -> env, (Reason.Rwitness (fst field), TUtils.tany env)
  | Some field_name ->
    let fake_super_shape_ty =
      make_idx_fake_super_shape
        field_name
        {sft_optional = true; sft_ty = res} in
    match default with
    | None ->
      let env =
        Type.sub_type (fst field) Reason.URparam env
          shape_ty
          fake_super_shape_ty in
      env,
      if experiment_enabled env
           TypecheckerOptions.experimental_stronger_shape_idx_ret &&
         is_shape_field_required env field_name shape_ty
      then res
      else TUtils.ensure_option env (fst fty) res
    | Some (default_pos, default_ty) ->
      let env =
        Type.sub_type (fst field) Reason.URparam env
          shape_ty
          fake_super_shape_ty in
      let env =
        Type.sub_type default_pos Reason.URparam env
          default_ty
          res in
      env, res

let remove_key p env shape_ty field  =
  match TUtils.shape_field_name env field with
   | None -> env, (Reason.Rwitness (fst field), TUtils.tany env)
   | Some field_name -> shrink_shape p field_name env shape_ty

let to_array env shape_ty res =
  let mapper = object
    inherit Type_mapper.shallow_type_mapper as super
    inherit! Type_mapper.tunresolved_type_mapper
    inherit! Type_mapper.tvar_expanding_type_mapper

    method! on_tshape env r fields_known fdm =
      match fields_known with
      | FieldsFullyKnown ->
        let keys = ShapeMap.keys fdm in
        let env, keys = List.map_env env keys begin fun env key ->
          match key with
          | Ast.SFlit_int (p, _) -> env, (Reason.Rwitness p, Tprim Tint)
          | Ast.SFlit_str (p, _) -> env, (Reason.Rwitness p, Tprim Tstring)
          | Ast.SFclass_const ((p, cid), (_, mid)) ->
            begin match Env.get_class env cid with
              | Some class_ -> begin match Env.get_const env class_ mid with
                  | Some const ->
                    Typing_phase.localize_with_self env const.cc_type
                  | None -> env, (Reason.Rwitness p, TUtils.tany env)
                end
              | None -> env, (Reason.Rwitness p, TUtils.tany env)
            end
        end in
        let env, key = Typing_arrays.union_keys env keys in
        let values = ShapeMap.values fdm in
        let values = List.map ~f:(fun { sft_ty; _ } -> sft_ty) values in
        let env, value = Typing_arrays.union_values env values in
        env, (r, Tarraykind (AKmap (key, value)))
      | FieldsPartiallyKnown _ ->
        env, res

    method! on_type env (r, ty) = match ty with
      | Tvar _ | Tunresolved _ | Tshape _ ->  super#on_type env (r, ty)
      | _ -> env, res

  end in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty
