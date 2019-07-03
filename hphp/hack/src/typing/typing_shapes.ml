(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Nast
open Typing_defs

module Env          = Typing_env
module Reason       = Typing_reason
module TUtils       = Typing_utils
module Type         = Typing_ops
module MakeType     = Typing_make_type


let widen_for_refine_shape ~expr_pos field_name env ty =
  match ty with
  | r, Tshape (fields_known, fields) ->
    begin match ShapeMap.get field_name fields with
    | None ->
      let env, element_ty = Env.fresh_invariant_type_var env expr_pos in
      let sft = {sft_optional = true; sft_ty = element_ty} in
      env, Some (r, Tshape (fields_known, ShapeMap.add field_name sft fields))
    | Some _ ->
      env, Some ty
    end
  | _ ->
    env, None

let rec refine_shape field_name pos env shape =
  let env, shape =
    Typing_subtype.expand_type_and_narrow ~description_of_expected:"a shape" env
      (widen_for_refine_shape ~expr_pos:pos field_name) pos shape in
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
    | FieldsPartiallyKnown _ ->
      let refined_sft_ty = match ShapeMap.get field_name fields with
        | None ->
          let printable_field_name =
            TUtils.get_printable_shape_field_name field_name in
          let sft_ty_r = Reason.Rmissing_optional_field
            (Reason.to_pos shape_r, printable_field_name) in
          MakeType.mixed sft_ty_r
        | Some {sft_ty; _} -> sft_ty in
      refine_shape_field_type refined_sft_ty
    end
  | r, Tunion tyl ->
    let env, tyl = List.map_env env tyl (refine_shape field_name pos) in
    env, (r, Tunion tyl)
  | _ -> env, shape

(*****************************************************************************)
(* Remove a field from all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec shrink_shape ~seen_tyvars pos field_name env shape =
  let env, shape =
    Typing_subtype.expand_type_and_solve ~description_of_expected:"a shape" env pos shape in
  match shape with
  | _, Tshape (fields_known, fields) ->
      (* remember that we have unset this field *)
      let fields = match fields_known with
        | FieldsFullyKnown ->
          ShapeMap.remove field_name fields
        | FieldsPartiallyKnown _ ->
          let printable_name = TUtils.get_printable_shape_field_name field_name in
          let nothing = MakeType.nothing (Reason.Runset_field (pos, printable_name)) in
          ShapeMap.add field_name {sft_ty = nothing; sft_optional = true} fields in
      let result = Reason.Rwitness pos, Tshape (fields_known, fields) in
      env, result
  | _, Tunion tyl ->
      let env, tyl =
        List.map_env env tyl (shrink_shape ~seen_tyvars pos field_name) in
      let result = Reason.Rwitness pos, Tunion tyl in
      env, result
  | x ->
      env, x

(* Refine the type of a shape knowing that a call to Shapes::idx is not null.
 * This means that the shape now has the field, and that the type for this
 * field is not nullable.
 * We stay quite liberal here: we add the field to the shape type regardless
 * of whether this field can be here at all. Errors will anyway be raised
 * elsewhere when typechecking the call to Shapes::idx. This allows for more
 * useful typechecking of incomplete code (code in the process of being
 * written). *)
let shapes_idx_not_null env shape_ty (p, field) =
  let env, (r, shape_ty) = Env.expand_type env shape_ty in
  match TUtils.shape_field_name env (p, field) with
  | None -> env, (r, shape_ty)
  | Some field ->
   let env, (r, shape_ty) =
    Typing_subtype.expand_type_and_narrow ~description_of_expected:"a shape" env
      (widen_for_refine_shape ~expr_pos:p field) p (r, shape_ty) in
    begin match shape_ty with
    | Tshape (fieldsknown, ftm) ->
      let env, field_type =
        begin match ShapeMap.find_opt field ftm with
        | Some { sft_ty; _ } ->
          let env, sft_ty = TUtils.non_null env p sft_ty in
          env, { sft_optional = false; sft_ty }
        | None ->
          env,
          { sft_optional = false
          ; sft_ty = MakeType.nonnull (Reason.Rwitness p)
          }
        end in
      let ftm = ShapeMap.add field field_type ftm in
      env, (r, Tshape (fieldsknown, ftm))
    | _ -> (* This should be an error, but it is already raised when
      typechecking the call to Shapes::idx *)
      env, (r, shape_ty)
    end

let experiment_enabled env experiment =
  TypecheckerOptions.experimental_feature_enabled
    (Env.get_tcopt env)
    experiment

let make_idx_fake_super_shape shape_pos fun_name field_name field_ty =
  Reason.Rshape (shape_pos, fun_name),
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
let is_shape_field_required env shape_pos fun_name field_name shape_ty =
  let field_ty = {
    sft_optional = false;
    sft_ty = MakeType.mixed Reason.Rnone
  } in
  Typing_subtype.is_sub_type env
    shape_ty
    (make_idx_fake_super_shape shape_pos fun_name field_name field_ty)

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
let idx env ~expr_pos ~fun_pos ~shape_pos shape_ty field default =
  let env, shape_ty = Env.expand_type env shape_ty in
  let env, res = Env.fresh_type env expr_pos in
  match TUtils.shape_field_name env field with
  | None -> env, (Reason.Rwitness (fst field), TUtils.tany env)
  | Some field_name ->
    let fake_super_shape_ty =
      make_idx_fake_super_shape
        shape_pos
        "Shapes::idx"
        field_name
        {sft_optional = true; sft_ty = res} in
    match default with
    | None ->
      let env =
        Type.sub_type shape_pos Reason.URparam env
          shape_ty
          fake_super_shape_ty in
      env,
      (if experiment_enabled env
           TypecheckerOptions.experimental_stronger_shape_idx_ret &&
         is_shape_field_required env shape_pos "Shapes::idx" field_name shape_ty
      then res
      else TUtils.ensure_option env fun_pos res)
    | Some (default_pos, default_ty) ->
      let env =
        Type.sub_type shape_pos Reason.URparam env
          shape_ty
          fake_super_shape_ty in
      let env =
        Type.sub_type default_pos Reason.URparam env
          default_ty
          res in
      env, res

let at env ~expr_pos ~shape_pos shape_ty field =
  let env, shape_ty = Env.expand_type env shape_ty in
  let env, res = Env.fresh_type env expr_pos in
  match TUtils.shape_field_name env field with
   | None ->
     env, (Reason.Rwitness (fst field), TUtils.tany env)
   | Some field_name ->
     let fake_super_shape_ty =
       make_idx_fake_super_shape
         shape_pos
         "Shapes::at"
         field_name
         {sft_optional = true; sft_ty = res} in
     let env =
       Type.sub_type shape_pos Reason.URparam env
         shape_ty
         fake_super_shape_ty in
     env, res

let remove_key p env shape_ty field  =
  match TUtils.shape_field_name env field with
   | None ->
     env, (Reason.Rwitness (fst field), TUtils.tany env)
   | Some field_name ->
     shrink_shape ~seen_tyvars:IMap.empty p field_name env shape_ty

let to_collection env shape_ty res return_type =
  let mapper = object
    inherit Type_mapper.shallow_type_mapper as super
    inherit! Type_mapper.tunion_type_mapper
    inherit! Type_mapper.tvar_expanding_type_mapper

    method! on_tshape env _r fields_known fdm =
      match fields_known with
      | FieldsFullyKnown ->
        let keys = ShapeMap.keys fdm in
        let env, keys = List.map_env env keys begin fun env key ->
          match key with
          | Ast.SFlit_int (p, _) -> env, MakeType.int (Reason.Rwitness p)
          | Ast.SFlit_str (p, _) -> env, MakeType.string (Reason.Rwitness p)
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
        env, return_type (fst res) key value
      | FieldsPartiallyKnown _ ->
        env, res

    method! on_type env (r, ty) = match ty with
      | Tvar _ | Tunion _ | Tshape _ ->  super#on_type env (r, ty)
      | _ -> env, res

  end in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty

let to_array env pos shape_ty res =
  let env, shape_ty =
    Typing_subtype.expand_type_and_solve ~description_of_expected:"a shape" env pos shape_ty in
  to_collection env shape_ty res (fun r key value ->
    (r, Tarraykind (AKmap (key, value))))

let to_dict env pos shape_ty res =
  let env, shape_ty =
    Typing_subtype.expand_type_and_solve ~description_of_expected:"a shape" env pos shape_ty in
  to_collection env shape_ty res (fun r key value ->
    MakeType.dict r key value)
