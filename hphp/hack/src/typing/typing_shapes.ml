(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Aast
open Typing_defs
module Env = Typing_env
module Reason = Typing_reason
module TUtils = Typing_utils
module Type = Typing_ops
module MakeType = Typing_make_type

let widen_for_refine_shape ~expr_pos field_name env ty =
  match ty with
  | (r, Tshape (shape_kind, fields)) ->
    begin
      match ShapeMap.get field_name fields with
      | None ->
        let (env, element_ty) = Env.fresh_invariant_type_var env expr_pos in
        let sft = { sft_optional = true; sft_ty = element_ty } in
        (env, Some (r, Tshape (shape_kind, ShapeMap.add field_name sft fields)))
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
         ( Reason.to_pos (fst shape),
           TUtils.get_printable_shape_field_name field_name ))
  in
  let sft = { sft_optional = false; sft_ty } in
  Typing_intersection.intersect
    env
    (Reason.Rwitness pos)
    shape
    (Reason.Rnone, Tshape (Open_shape, ShapeMap.singleton field_name sft))

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
  match shape with
  | (_, Tshape (shape_kind, fields)) ->
    let fields =
      match shape_kind with
      | Closed_shape -> ShapeMap.remove field_name fields
      | Open_shape ->
        let printable_name =
          TUtils.get_printable_shape_field_name field_name
        in
        let nothing =
          MakeType.nothing (Reason.Runset_field (pos, printable_name))
        in
        ShapeMap.add
          field_name
          { sft_ty = nothing; sft_optional = true }
          fields
    in
    let result = (Reason.Rwitness pos, Tshape (shape_kind, fields)) in
    (env, result)
  | (_, Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl (shrink_shape pos field_name) in
    let result = (Reason.Rwitness pos, Tunion tyl) in
    (env, result)
  | x -> (env, x)

(* Refine the type of a shape knowing that a call to Shapes::idx is not null.
 * This means that the shape now has the field, and that the type for this
 * field is not nullable.
 * We stay quite liberal here: we add the field to the shape type regardless
 * of whether this field can be here at all. Errors will anyway be raised
 * elsewhere when typechecking the call to Shapes::idx. This allows for more
 * useful typechecking of incomplete code (code in the process of being
 * written). *)
let shapes_idx_not_null env shape_ty (p, field) =
  let (env, (r, shape_ty)) = Env.expand_type env shape_ty in
  match TUtils.shape_field_name env (p, field) with
  | None -> (env, (r, shape_ty))
  | Some field ->
    let (env, (r, shape_ty)) =
      Typing_solver.expand_type_and_narrow
        ~description_of_expected:"a shape"
        env
        (widen_for_refine_shape ~expr_pos:p field)
        p
        (r, shape_ty)
        Errors.unify_error
    in
    begin
      match shape_ty with
      | Tshape (shape_kind, ftm) ->
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
        (env, (r, Tshape (shape_kind, ftm)))
      | _ ->
        (* This should be an error, but it is already raised when
      typechecking the call to Shapes::idx *)
        (env, (r, shape_ty))
    end

let experiment_enabled env experiment =
  TypecheckerOptions.experimental_feature_enabled
    (Env.get_tcopt env)
    experiment

let make_idx_fake_super_shape env shape_pos fun_name field_name field_ty =
  let ty =
    ( Reason.Rshape (shape_pos, fun_name),
      Tshape (Open_shape, Nast.ShapeMap.singleton field_name field_ty) )
  in
  Typing_enforceability.make_locl_like_type env ty

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
  let (env, super_shape_ty) =
    make_idx_fake_super_shape env shape_pos fun_name field_name field_ty
  in
  Typing_solver.is_sub_type env shape_ty super_shape_ty

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
    | None -> (env, (Reason.Rwitness (fst field), TUtils.tany env))
    | Some field_name ->
      let (env, fake_super_shape_ty) =
        make_idx_fake_super_shape
          env
          shape_pos
          "Shapes::idx"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      (match default with
      | None ->
        let env =
          Type.sub_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            fake_super_shape_ty
            Errors.unify_error
        in
        ( env,
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
            res
          else
            TUtils.ensure_option env fun_pos res )
      | Some (default_pos, default_ty) ->
        let env =
          Type.sub_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            fake_super_shape_ty
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
    | None -> (env, (Reason.Rwitness (fst field), TUtils.tany env))
    | Some field_name ->
      let (env, fake_super_shape_ty) =
        make_idx_fake_super_shape
          env
          shape_pos
          "Shapes::at"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      let env =
        Type.sub_type
          shape_pos
          Reason.URparam
          env
          shape_ty
          fake_super_shape_ty
          Errors.unify_error
      in
      (env, res)
  in
  Typing_enforceability.make_locl_like_type env res

let remove_key p env shape_ty field =
  match TUtils.shape_field_name env field with
  | None -> (env, (Reason.Rwitness (fst field), TUtils.tany env))
  | Some field_name -> shrink_shape p field_name env shape_ty

let to_collection env shape_ty res return_type =
  let mapper =
    object
      inherit Type_mapper.shallow_type_mapper as super

      inherit! Type_mapper.tunion_type_mapper

      inherit! Type_mapper.tvar_expanding_type_mapper

      method! on_tshape env _r shape_kind fdm =
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
                        | None -> (env, (Reason.Rwitness p, TUtils.tany env))
                      end
                    | None -> (env, (Reason.Rwitness p, TUtils.tany env))
                  end)
          in
          let (env, key) = Typing_arrays.union_keys env keys in
          let values = ShapeMap.values fdm in
          let values = List.map ~f:(fun { sft_ty; _ } -> sft_ty) values in
          let (env, value) = Typing_arrays.union_values env values in
          return_type env (fst res) key value
        | Open_shape -> (env, res)

      method! on_type env (r, ty) =
        match ty with
        | Tvar _
        | Tunion _
        | Tshape _ ->
          super#on_type env (r, ty)
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
        (r, Tarraykind (AKmap (key, value))))

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
