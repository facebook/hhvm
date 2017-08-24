(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Nast
open Typing_defs

module Env          = Typing_env
module Reason       = Typing_reason
module TUtils       = Typing_utils
module Type         = Typing_ops

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

(* Helper function to create a temporary "fake" type for use by Shapes::idx,
  which will be used as the supertype of the first argument passed to
  Shapes::idx (arg_ty). In most cases, the returned supertype will be
    shape(?field_name: res, ...)

  If experimental_shape_idx_relaxed is enabled, then we will return shape(...)
  for the case where field_name does not exist in the arg_ty and arg_ty is
  partial but does not unset field_name.

  If experimental_optional_shape_field is disabled, then a nullable type will be
  used instead of an optional type in the returned supertype.
*)
let make_idx_fake_super_shape env (arg_r, arg_ty) field_name res =
  let optional_shape_field_enabled = experiment_enabled env
    TypecheckerOptions.experimental_optional_shape_field in
  let shape_idx_relaxed = experiment_enabled env
    TypecheckerOptions.experimental_shape_idx_relaxed in
  let fake_shape_field = {
    sft_optional = optional_shape_field_enabled;
    sft_ty = Reason.Rnone, Toption res;
  } in
  if shape_idx_relaxed then
    match arg_ty with
    | Tshape (FieldsPartiallyKnown unset_fields, fdm)
        when not (ShapeMap.mem field_name fdm
                  || ShapeMap.mem field_name unset_fields) ->
      (* Special logic for when arg_ty does not have field and does not
        explicitly unset field. We want to relax Shapes::idx to allow accessing
        field in this case, so we will only require that arg_ty be a shape. *)
      (* This is dangerous because the shape may later be instantiated with a
        field that conflicts with the return type of Shapes::idx. Programmers
        should instead use direct accessing (i.e. shape[field]) when possible to
        get stricter behavior. *)
      Lint.shape_idx_access_unknown_field (Reason.to_pos arg_r)
        (Env.get_shape_field_name field_name);
      (* But we allow it anyhow *)
      Nast.ShapeMap.empty
    | _ -> Nast.ShapeMap.singleton field_name fake_shape_field
  else
    Nast.ShapeMap.singleton field_name fake_shape_field

let apply_on_field ~f ~default field_name (_, ty) =
  match ty with
  | Tshape (_, fdm) ->
    begin match ShapeMap.get field_name fdm with
    | Some field -> f field
    | None -> default
    end
  | _ ->  default

let has_non_optional_field env =
  apply_on_field
    ~f:(fun field_ty -> not (TUtils.is_shape_field_optional env field_ty))
    ~default: false

let field_has_nullable_type env =
  apply_on_field
    ~f:(fun field_ty -> TUtils.is_option env field_ty.sft_ty)
    ~default: false

(* Typing rule for Shapes::idx($s, field, [default])

  Shapes::idx has type res (or Toption res, if res is not already Toption _ and
  default is not provided), where res is the inferred type of field (provided by
  $s, or else Tmixed). If default is provided, then res must be a subtype of
  Tunresolved[default].

  Ensures that $s is a shape. $s must be a subtype of:
  shape(...) -- if experimental_shape_idx_relaxed and $s does not contain field
                and does not unset field; i.e. it is possible for an instance of
                $s to provide the field with an arbitrary type.
                (This will emit a lint warning)
  shape(?field => Tmixed, ...)  -- if experimental_optional_shape_field
  shape(field => ?Tmixed, ...)  -- otherwise
*)
let idx env fty shape_ty field default =
  let env, shape_ty = Env.expand_type env shape_ty in
  let env, res = Env.fresh_unresolved_type env in
  match TUtils.shape_field_name env (fst field) (snd field) with
  | None -> env, (Reason.Rwitness (fst field), Tany)
  | Some field_name ->
    let fake_shape = (
      (* Rnone because we don't want the fake shape to show up in messages about
       * field non existing. Errors.missing_optional_field filters them out *)
      Reason.Rnone,
      Tshape (
        FieldsPartiallyKnown Nast.ShapeMap.empty,
        make_idx_fake_super_shape env shape_ty field_name res
      )
    ) in
    let env =
      Type.sub_type (fst field) Reason.URparam env shape_ty fake_shape in
    let stronger_shape_idx_ret = experiment_enabled env
      TypecheckerOptions.experimental_stronger_shape_idx_ret in
    match default with
      | None when TUtils.is_option env res -> env, res
      | None when stronger_shape_idx_ret
          && has_non_optional_field env field_name shape_ty ->
          Lint.shape_idx_access_required_field (fst field)
            (Env.get_shape_field_name field_name);
          let res =
            if field_has_nullable_type env field_name shape_ty
            then (fst fty, Toption res)
            else res in
          env, res
      | None ->
        (* no default and we can't guarantee that the shape contains field:
         * result is nullable, point to Shapes::idx definition as reason *)
        env, (fst fty, Toption res)
      | Some (default_pos, default_ty) ->
        let env, default_ty = Typing_utils.unresolved env default_ty in
        let env, res = Type.sub_type default_pos Reason.URparam env res default_ty, res in
        let res =
          if field_has_nullable_type env field_name shape_ty
          then (fst fty, Toption res)
          else res in
        env, res

let remove_key p env shape_ty field  =
  match TUtils.shape_field_name env (fst field) (snd field) with
   | None -> env, (Reason.Rwitness (fst field), Tany)
   | Some field_name -> shrink_shape p field_name env shape_ty

let to_array env shape_ty res =
  let mapper = object
    inherit Type_mapper.shallow_type_mapper as super
    inherit! Type_mapper.tunresolved_type_mapper
    inherit! Type_mapper.tvar_expanding_type_mapper

    method! on_tshape env r fields_known fdm =
      match fields_known with
      | FieldsFullyKnown ->
        let env, values =
          ShapeFieldList.map_env
            env (ShapeMap.values fdm) (Typing_utils.unresolved) in
        let keys = ShapeMap.keys fdm in
        let env, keys = List.map_env env keys begin fun env key ->
          let env, ty = match key with
          | Ast.SFlit (p, _) -> env, (Reason.Rwitness p, Tprim Tstring)
          | Ast.SFclass_const ((_, cid), (_, mid)) ->
            begin match Env.get_class env cid with
              | Some class_ -> begin match Env.get_const env class_ mid with
                  | Some const ->
                    Typing_phase.localize_with_self env const.cc_type
                  | None -> env, (Reason.Rnone, Tany)
                end
              | None -> env, (Reason.Rnone, Tany)
            end in
          Typing_utils.unresolved env ty
        end in
        let env, key =
          Typing_arrays.array_type_list_to_single_type env keys in
        let values = List.map ~f:(fun { sft_ty; _ } -> sft_ty) values in
        let env, value =
          Typing_arrays.array_type_list_to_single_type env values in
        env, (r, Tarraykind (AKmap (key, value)))
      | FieldsPartiallyKnown _ ->
        env, res

    method! on_type env (r, ty) = match ty with
      | Tvar _ | Tunresolved _ | Tshape _ ->  super#on_type env (r, ty)
      | _ -> env, res

  end in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty
