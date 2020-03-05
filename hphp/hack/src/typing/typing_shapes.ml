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
open Aast
open Typing_defs
module Env = Typing_env
module Reason = Typing_reason
module TUtils = Typing_utils
module Type = Typing_ops
module MakeType = Typing_make_type

let widen_for_refine_shape ~expr_pos field_name env ty =
  match deref ty with
  | (r, Tshape (shape_kind, fields)) ->
    begin
      match ShapeMap.find_opt field_name fields with
      | None ->
        let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
        let sft = { sft_optional = true; sft_ty = element_ty } in
        ( env,
          Some (mk (r, Tshape (shape_kind, ShapeMap.add field_name sft fields)))
        )
      | Some _ -> (env, Some ty)
    end
  | _ -> (env, None)

let refine_shape field_name pos env shape =
  let (env, shape) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:pos field_name)
      pos
      shape
      Errors.unify_error
  in
  let sft_ty =
    MakeType.mixed
      (Reason.Rmissing_optional_field
         (get_pos shape, TUtils.get_printable_shape_field_name field_name))
  in
  let sft = { sft_optional = false; sft_ty } in
  Typing_intersection.intersect
    env
    (Reason.Rwitness pos)
    shape
    (mk (Reason.Rnone, Tshape (Open_shape, ShapeMap.singleton field_name sft)))

(*****************************************************************************)
(* Remove a field from all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec shrink_shape pos field_name env shape =
  let (env, shape) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape
      Errors.unify_error
  in
  match get_node shape with
  | Tshape (shape_kind, fields) ->
    let fields =
      match shape_kind with
      | Closed_shape -> ShapeMap.remove field_name fields
      | Open_shape ->
        let printable_name = TUtils.get_printable_shape_field_name field_name in
        let nothing =
          MakeType.nothing (Reason.Runset_field (pos, printable_name))
        in
        ShapeMap.add field_name { sft_ty = nothing; sft_optional = true } fields
    in
    let result = mk (Reason.Rwitness pos, Tshape (shape_kind, fields)) in
    (env, result)
  | Tunion tyl ->
    let (env, tyl) = List.map_env env tyl (shrink_shape pos field_name) in
    let result = mk (Reason.Rwitness pos, Tunion tyl) in
    (env, result)
  | _ -> (env, shape)

(* Refine the type of a shape knowing that a call to Shapes::idx is not null.
 * This means that the shape now has the field, and that the type for this
 * field is not nullable.
 * We stay quite liberal here: we add the field to the shape type regardless
 * of whether this field can be here at all. Errors will anyway be raised
 * elsewhere when typechecking the call to Shapes::idx. This allows for more
 * useful typechecking of incomplete code (code in the process of being
 * written). *)
let shapes_idx_not_null env shape_ty (p, field) =
  match TUtils.shape_field_name env (p, field) with
  | None -> (env, shape_ty)
  | Some field ->
    let (env, shape_ty) =
      Typing_solver.expand_type_and_narrow
        ~description_of_expected:"a shape"
        env
        (widen_for_refine_shape ~expr_pos:p field)
        p
        shape_ty
        Errors.unify_error
    in
    let refine_type env shape_ty =
      let (env, shape_ty) = Env.expand_type env shape_ty in
      match deref shape_ty with
      | (r, Tshape (shape_kind, ftm)) ->
        let (env, field_type) =
          match ShapeMap.find_opt field ftm with
          | Some { sft_ty; _ } ->
            let (env, sft_ty) = Typing_solver.non_null env p sft_ty in
            (env, { sft_optional = false; sft_ty })
          | None ->
            ( env,
              {
                sft_optional = false;
                sft_ty = MakeType.nonnull (Reason.Rwitness p);
              } )
        in
        let ftm = ShapeMap.add field field_type ftm in
        (env, mk (r, Tshape (shape_kind, ftm)))
      | _ ->
        (* This should be an error, but it is already raised when
      typechecking the call to Shapes::idx *)
        (env, shape_ty)
    in
    (match deref shape_ty with
    | (_, Tshape _) -> refine_type env shape_ty
    | (r, Tunion tyl) ->
      let (env, tyl) = List.map_env env tyl refine_type in
      Typing_union.union_list env r tyl
    | _ -> (env, shape_ty))

let experiment_enabled env experiment =
  TypecheckerOptions.experimental_feature_enabled (Env.get_tcopt env) experiment

let make_idx_fake_super_shape shape_pos fun_name field_name field_ty =
  mk
    ( Reason.Rshape (shape_pos, fun_name),
      Tshape (Open_shape, Nast.ShapeMap.singleton field_name field_ty) )

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
  let field_ty =
    { sft_optional = false; sft_ty = MakeType.mixed Reason.Rnone }
  in
  let super_shape_ty =
    make_idx_fake_super_shape shape_pos fun_name field_name field_ty
  in
  let (env, ty1) =
    Typing_solver.expand_type_and_solve_eq env shape_ty Errors.unify_error
  in
  let (env, ty2) =
    Typing_solver.expand_type_and_solve_eq env super_shape_ty Errors.unify_error
  in
  Typing_subtype.is_sub_type_for_coercion env ty1 ty2

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
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let (env, res) =
    match TUtils.shape_field_name env field with
    | None -> (env, TUtils.mk_tany env (fst field))
    | Some field_name ->
      let fake_super_shape_ty =
        make_idx_fake_super_shape
          shape_pos
          "Shapes::idx"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      (match default with
      | None ->
        let env =
          Typing_coercion.coerce_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            { et_type = fake_super_shape_ty; et_enforced = false }
            Errors.unify_error
        in
        if
          experiment_enabled
            env
            TypecheckerOptions.experimental_stronger_shape_idx_ret
          && is_shape_field_required
               env
               shape_pos
               "Shapes::idx"
               field_name
               shape_ty
        then
          (env, res)
        else
          TUtils.union env res (MakeType.null fun_pos)
      | Some (default_pos, default_ty) ->
        let env =
          Typing_coercion.coerce_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            { et_type = fake_super_shape_ty; et_enforced = false }
            Errors.unify_error
        in
        let env =
          Type.sub_type
            default_pos
            Reason.URparam
            env
            default_ty
            res
            Errors.unify_error
        in
        (env, res))
  in
  Typing_enforceability.make_locl_like_type env res

let at env ~expr_pos ~shape_pos shape_ty field =
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let (env, res) =
    match TUtils.shape_field_name env field with
    | None -> (env, TUtils.mk_tany env (fst field))
    | Some field_name ->
      let fake_super_shape_ty =
        make_idx_fake_super_shape
          shape_pos
          "Shapes::at"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      let env =
        Typing_coercion.coerce_type
          shape_pos
          Reason.URparam
          env
          shape_ty
          { et_type = fake_super_shape_ty; et_enforced = false }
          Errors.unify_error
      in
      (env, res)
  in
  Typing_enforceability.make_locl_like_type env res

let remove_key p env shape_ty field =
  match TUtils.shape_field_name env field with
  | None -> (env, TUtils.mk_tany env (fst field))
  | Some field_name -> shrink_shape p field_name env shape_ty

let to_collection env shape_ty res return_type =
  let mapper =
    object (self)
      inherit Type_mapper.shallow_type_mapper as super

      inherit! Type_mapper.tunion_type_mapper

      inherit! Type_mapper.tvar_expanding_type_mapper

      method! on_tshape env r shape_kind fdm =
        match shape_kind with
        | Closed_shape ->
          let keys = ShapeMap.keys fdm in
          let (env, keys) =
            List.map_env env keys (fun env key ->
                match key with
                | Ast_defs.SFlit_int (p, _) ->
                  (env, MakeType.int (Reason.Rwitness p))
                | Ast_defs.SFlit_str (p, _) ->
                  (env, MakeType.string (Reason.Rwitness p))
                | Ast_defs.SFclass_const ((p, cid), (_, mid)) ->
                  begin
                    match Env.get_class env cid with
                    | Some class_ ->
                      begin
                        match Env.get_const env class_ mid with
                        | Some const ->
                          Typing_phase.localize_with_self env const.cc_type
                        | None -> (env, TUtils.mk_tany env p)
                      end
                    | None -> (env, TUtils.mk_tany env p)
                  end)
          in
          let (env, key) = Typing_union.union_list env r keys in
          let values = ShapeMap.values fdm in
          let values = List.map ~f:(fun { sft_ty; _ } -> sft_ty) values in
          let (env, value) = Typing_union.union_list env r values in
          return_type env (get_reason res) key value
        | Open_shape -> (env, res)

      method! on_tunion env r tyl =
        let (env, tyl) = List.fold_map tyl ~init:env ~f:self#on_type in
        Typing_union.union_list env r tyl

      method! on_type env ty =
        match get_node ty with
        | Tdynamic ->
          (* This makes it so that to_collection on a dynamic value returns a dynamic
           * value instead of the standard dict<arraykey, mixed> declared in the HHI,
           * which would otherwise subsume any other inferred type due to covariance. *)
          (env, ty)
        | Tvar _
        | Tshape _ ->
          super#on_type env ty
        | _ -> (env, res)
    end
  in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty

let to_array env pos shape_ty res =
  let (env, shape_ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape_ty
      Errors.unify_error
  in
  to_collection env shape_ty res (fun env r key value ->
      Typing_enforceability.make_locl_like_type
        env
        (mk (r, Tarraykind (AKdarray (key, value)))))

let to_dict env pos shape_ty res =
  let (env, shape_ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape_ty
      Errors.unify_error
  in
  to_collection env shape_ty res (fun env r key value ->
      Typing_enforceability.make_locl_like_type env (MakeType.dict r key value))
