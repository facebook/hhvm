(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Common
open Typing_defs
module Env = Typing_env
module Reason = Typing_reason
module TUtils = Typing_utils
module Type = Typing_ops
module MakeType = Typing_make_type

let widen_for_refine_shape ~expr_pos field_name env ty =
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [Log_head ("widen_for_refine_shape", [Log_type ("ty", ty)])]));
  match deref ty with
  | (r, Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fields })
    -> begin
    match TShapeMap.find_opt field_name fields with
    | None ->
      let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
      let sft = { sft_optional = true; sft_ty = element_ty } in
      ( (env, None),
        Some
          (mk
             ( r,
               Tshape
                 {
                   s_origin = Missing_origin;
                   s_unknown_value = shape_kind;
                   s_fields = TShapeMap.add field_name sft fields;
                 } )) )
    | Some _ -> ((env, None), Some ty)
  end
  | _ -> ((env, None), None)

(* Refine a shape with the knowledge that field_name
 * exists. We do this by intersecting with
 * shape(field_name => mixed, ...)
 *)
let refine_shape field_name pos env shape =
  let ((env, e1), shape) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:pos field_name)
      pos
      shape
  in
  let r =
    Reason.Rmissing_optional_field
      (get_pos shape, TUtils.get_printable_shape_field_name field_name)
  in
  let sft_ty = MakeType.mixed r in
  let sft_ty =
    if
      TUtils.is_dynamic env shape
      && TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env)
    then
      MakeType.supportdyn r sft_ty
    else
      sft_ty
  in
  let sft = { sft_optional = false; sft_ty } in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e1;
  Typing_intersection.intersect
    env
    ~r:(Reason.Rwitness pos)
    shape
    (MakeType.open_shape Reason.Rnone (TShapeMap.singleton field_name sft))

(*****************************************************************************)
(* Remove a field from all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec shrink_shape pos ~supportdyn field_name env shape =
  (* Make sure we have a shape type in our hands.
   * Note that we don't want to freshen any types inside the shape
   * e.g. turn shape('a' => C) into shape('a' => #1) with a subtype constraint on #1,
   * because we know that the types of the fields don't change
   *)
  let ((env, e1), shape) =
    Typing_solver.expand_type_and_solve
      ~freshen:false
      ~description_of_expected:"a shape"
      env
      pos
      shape
  in
  let (supportdyn2, env, stripped_shape) = TUtils.strip_supportdyn env shape in
  let supportdyn = supportdyn || supportdyn2 in
  match get_node stripped_shape with
  | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fields } ->
    let fields =
      if is_nothing shape_kind then
        TShapeMap.remove field_name fields
      (* TODO akenn: check this *)
      else
        let printable_name = TUtils.get_printable_shape_field_name field_name in
        let nothing =
          MakeType.nothing (Reason.Runset_field (pos, printable_name))
        in
        TShapeMap.add
          field_name
          { sft_ty = nothing; sft_optional = true }
          fields
    in
    let result =
      mk
        ( Reason.Rwitness pos,
          Tshape
            {
              s_origin = Missing_origin;
              s_unknown_value = shape_kind;
              s_fields = fields;
            } )
    in
    ( (env, e1),
      if supportdyn then
        let r = get_reason result in
        MakeType.supportdyn r result
      else
        result )
  | Tunion tyl ->
    let ((env, e2), tyl) =
      List.map_env_ty_err_opt
        env
        tyl
        ~combine_ty_errs:Typing_error.multiple_opt
        ~f:(shrink_shape pos ~supportdyn field_name)
    in
    let result = mk (Reason.Rwitness pos, Tunion tyl) in
    ((env, Option.merge e1 e2 ~f:Typing_error.both), result)
  | _ ->
    ( (env, e1),
      if supportdyn then
        let r = get_reason shape in
        MakeType.supportdyn r shape
      else
        shape )

(* Refine the type of a shape knowing that a call to Shapes::idx is not null.
 * This means that the shape now has the field, and that the type for this
 * field is not nullable.
 * We stay quite liberal here: we add the field to the shape type regardless
 * of whether this field can be here at all. Errors will anyway be raised
 * elsewhere when typechecking the call to Shapes::idx. This allows for more
 * useful typechecking of incomplete code (code in the process of being
 * written). *)
let shapes_idx_not_null_with_ty_err env shape_ty (ty, p, field) =
  let (fld_opt, ty_err_opt) =
    TUtils.shape_field_name_with_ty_err env (ty, p, field)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  match fld_opt with
  | None -> ((env, None), shape_ty)
  | Some field ->
    let field = TShapeField.of_ast Pos_or_decl.of_raw_pos field in
    let ((env, e1), shape_ty) =
      Typing_solver.expand_type_and_narrow
        ~description_of_expected:"a shape"
        env
        (widen_for_refine_shape ~expr_pos:p field)
        p
        shape_ty
    in
    let rec refine_type env shape_ty =
      let (env, shape_ty) = Env.expand_type env shape_ty in
      match deref shape_ty with
      | (r, Tnewtype (n, _, ty))
        when String.equal n Naming_special_names.Classes.cSupportDyn ->
        let (env, ty) = refine_type env ty in
        TUtils.make_supportdyn r env ty
      | ( r,
          Tshape { s_origin = _; s_fields = ftm; s_unknown_value = shape_kind }
        ) ->
        let (env, field_type) =
          let sft_ty =
            match TShapeMap.find_opt field ftm with
            | Some { sft_ty; _ } -> sft_ty
            | None -> shape_kind
          in
          let (env, sft_ty) =
            Typing_solver.non_null env (Pos_or_decl.of_raw_pos p) sft_ty
          in
          (env, { sft_optional = false; sft_ty })
        in
        let ftm = TShapeMap.add field field_type ftm in
        ( env,
          mk
            ( r,
              Tshape
                {
                  s_origin = Missing_origin;
                  s_fields = ftm;
                  s_unknown_value = shape_kind;
                } ) )
      | _ ->
        (* This should be an error, but it is already raised when
           typechecking the call to Shapes::idx *)
        (env, shape_ty)
    in
    (match deref shape_ty with
    | (r, Tunion tyl) ->
      let (env, tyl) = List.map_env env tyl ~f:refine_type in
      let (env, ty) = Typing_union.union_list env r tyl in
      ((env, e1), ty)
    | _ ->
      let (env, ty) = refine_type env shape_ty in
      ((env, None), ty))

let shapes_idx_not_null env shape_ty fld =
  let ((env, ty_err_opt), res) =
    shapes_idx_not_null_with_ty_err env shape_ty fld
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)

let make_idx_fake_super_shape shape_pos fun_name field_name field_ty =
  MakeType.open_shape
    (Reason.Rshape (shape_pos, fun_name))
    (TShapeMap.singleton field_name field_ty)

(* Typing rules for Shapes::idx
 *
 *     e : ?shape(?sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : ?t
 *
 *  Typing rules when the shape has a like type:
 *
 *     e : ~?shape(?sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : ~?t
 *)
let idx_without_default env ~expr_pos ~shape_pos shape_ty field_name =
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let ((env, ty_err_opt), res) =
    let fake_super_shape_ty =
      make_idx_fake_super_shape
        shape_pos
        "Shapes::idx"
        field_name
        { sft_optional = true; sft_ty = res }
    in
    let nullable_super_shape = mk (Reason.Rnone, Toption fake_super_shape_ty) in
    let super_shape =
      if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
        MakeType.locl_like (Reason.Rwitness shape_pos) nullable_super_shape
      else
        nullable_super_shape
    in
    let (env, ty_err_opt) =
      Typing_coercion.coerce_type
        shape_pos
        Reason.URparam
        env
        shape_ty
        { et_type = super_shape; et_enforced = Unenforced }
        Typing_error.Callback.unify_error
    in
    let (env, res) = TUtils.union env res (MakeType.null Reason.Rnone) in
    ((env, ty_err_opt), res)
  in
  Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
  match get_node (TUtils.strip_dynamic env shape_ty) with
  | Tnewtype (n, _, _)
    when String.equal n Naming_special_names.Classes.cSupportDyn ->
    let r = get_reason shape_ty in
    TUtils.make_supportdyn r env res
  | _ -> (env, res)

let remove_key_with_ty_err p env shape_ty ((_, field_p, _) as field) =
  let (fld_opt, ty_err_opt) = TUtils.shape_field_name_with_ty_err env field in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  match fld_opt with
  | None ->
    let (env, ty) = Env.fresh_type_error env field_p in
    ((env, None), ty)
  | Some field_name ->
    let field_name = TShapeField.of_ast Pos_or_decl.of_raw_pos field_name in
    shrink_shape ~supportdyn:false p field_name env shape_ty

let remove_key p env shape_ty field =
  let ((env, ty_err_opt), res) = remove_key_with_ty_err p env shape_ty field in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)

let to_collection env pos shape_ty res return_type =
  let mapper =
    object (self)
      inherit Type_mapper.tvar_expanding_type_mapper as super

      method! on_tshape env r s =
        let { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } =
          s
        in
        (* The key type is the union of the types of the known fields,
         * or arraykey if there may be unknown fields (open shape)
         *)
        let (env, key) =
          if not (TUtils.is_nothing env shape_kind) then
            (env, MakeType.arraykey r)
          else
            let keys = TShapeMap.keys fdm in
            let (env, keys) =
              List.map_env env keys ~f:(fun env key ->
                  match key with
                  | Typing_defs.TSFlit_int (p, _) ->
                    (env, MakeType.int (Reason.Rwitness_from_decl p))
                  | Typing_defs.TSFlit_str (p, _) ->
                    (env, MakeType.string (Reason.Rwitness_from_decl p))
                  | Typing_defs.TSFclass_const ((_, cid), (_, mid)) -> begin
                    match Env.get_class env cid with
                    | Some class_ -> begin
                      match Env.get_const env class_ mid with
                      | Some const ->
                        let ((env, ty_err_opt), lty) =
                          Typing_phase.localize_no_subst
                            env
                            ~ignore_errors:true
                            const.cc_type
                        in
                        Option.iter
                          ~f:(Typing_error_utils.add_typing_error ~env)
                          ty_err_opt;
                        (env, lty)
                      | None -> Env.fresh_type_error env pos
                    end
                    | None -> Env.fresh_type_error env pos
                  end)
            in
            Typing_union.union_list env r keys
        in
        (* The value type is the union of the types of the known fields together
         * with the type of the unknown fields (open shape, typically mixed or supportdyn<mixed>)
         *)
        let (env, value) =
          (* If the unknown fields have type mixed then that's the type of values: no need for union *)
          if TUtils.is_mixed env shape_kind then
            (env, shape_kind)
          else
            (* Otherwise first filter out subtypes (common case,
             * as unknown fields are likely supportdyn<mixed>)
             * and then construct the union.
             *)
            let values = TShapeMap.values fdm in
            let values =
              List.filter_map
                ~f:(fun { sft_ty; _ } ->
                  if TUtils.is_sub_type env sft_ty shape_kind then
                    None
                  else
                    Some sft_ty)
                values
            in
            Typing_union.union_list env r (shape_kind :: values)
        in
        return_type env (get_reason res) key value

      method! on_tunion env r tyl =
        let (env, tyl) = List.fold_map tyl ~init:env ~f:self#on_type in
        Typing_union.union_list env r tyl

      method! on_tintersection env r tyl =
        let (env, tyl) = List.fold_map tyl ~init:env ~f:self#on_type in
        Typing_intersection.intersect_list env r tyl

      method! on_type env ty =
        match get_node ty with
        | Tdynamic ->
          (* This makes it so that to_collection on a dynamic value returns a dynamic
           * value instead of the standard dict<arraykey, mixed> declared in the HHI,
           * which would otherwise subsume any other inferred type due to covariance. *)
          (env, ty)
        | Tvar _
        | Tshape _
        | Tunion _
        | Tintersection _ ->
          super#on_type env ty
          (* Look through bound on newtype, including supportdyn. We assume that
           * supportdyn has already been pushed into the type of unknown fields *)
        | Tnewtype (_, _, ty) -> super#on_type env ty
        | _ -> (env, res)
    end
  in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty

let to_dict env pos shape_ty res =
  let ((env, e1), shape_ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape_ty
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e1;
  let shape_ty = TUtils.get_base_type ~expand_supportdyn:true env shape_ty in
  to_collection env pos shape_ty res (fun env r key value ->
      (env, MakeType.dict r key value))

let shape_field_pos = function
  | Ast_defs.SFlit_int (p, _)
  | Ast_defs.SFlit_str (p, _) ->
    p
  | Ast_defs.SFclass_const ((cls_pos, _), (mem_pos, _)) ->
    Pos.btw cls_pos mem_pos

let check_shape_keys_validity env keys =
  (* If the key is a class constant, get its class name and type. *)
  let get_field_info env key =
    let key_pos = shape_field_pos key in
    (* Empty strings or literals that start with numbers are not
         permitted as shape field names. *)
    match key with
    | Ast_defs.SFlit_int _ -> (env, key_pos, None)
    | Ast_defs.SFlit_str (_, key_name) ->
      (if Int.equal 0 (String.length key_name) then
        Typing_error_utils.add_typing_error ~env
        @@ Typing_error.(
             shape
             @@ Primary.Shape.Invalid_shape_field_name
                  { pos = key_pos; is_empty = true }));
      (env, key_pos, None)
    | Ast_defs.SFclass_const ((_p, cls), (p, y)) -> begin
      match Env.get_class env cls with
      | None ->
        let (env, ty) = Env.fresh_type_error env p in
        (env, key_pos, Some (cls, ty))
      | Some cd ->
        (match Env.get_const env cd y with
        | None ->
          Typing_error_utils.add_typing_error ~env
          @@ Typing_object_get.smember_not_found
               p
               ~is_const:true
               ~is_method:false
               ~is_function_pointer:false
               cd
               y
               Typing_error.Callback.unify_error;
          let (env, ty) = Env.fresh_type_error env p in

          (env, key_pos, Some (cls, ty))
        | Some { cc_type; _ } ->
          let ((env, ty_err_opt), ty) =
            Typing_phase.localize_no_subst ~ignore_errors:true env cc_type
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          let r = Reason.Rwitness key_pos in
          let (env, e2) =
            Type.sub_type key_pos Reason.URnone env ty (MakeType.arraykey r)
            @@ Typing_error.(
                 Callback.always
                   Primary.(
                     Shape
                       (Shape.Invalid_shape_field_type
                          {
                            pos = key_pos;
                            ty_pos = get_pos ty;
                            ty_name = lazy (Typing_print.error env ty);
                            trail = [];
                          })))
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e2;
          (env, key_pos, Some (cls, ty)))
    end
  in
  let check_field witness_pos witness_info env key =
    let (env, key_pos, key_info) = get_field_info env key in
    let ty_errs =
      let open Typing_error in
      match (witness_info, key_info) with
      | (Some _, None) ->
        [
          shape
          @@ Primary.Shape.Invalid_shape_field_literal
               { pos = key_pos; witness_pos };
        ]
      | (None, Some _) ->
        [
          shape
          @@ Primary.Shape.Invalid_shape_field_const
               { pos = key_pos; witness_pos };
        ]
      | (None, None) -> []
      | (Some (cls1, ty1), Some (cls2, ty2)) ->
        List.filter_map
          ~f:Fn.id
          [
            (if TUtils.is_tyvar_error env ty1 || TUtils.is_tyvar_error env ty2
            then
              None
            else if String.( <> ) cls1 cls2 then
              Some
                (shape
                   (Primary.Shape.Shape_field_class_mismatch
                      {
                        pos = key_pos;
                        witness_pos;
                        class_name = Utils.strip_ns cls2;
                        witness_class_name = Utils.strip_ns cls1;
                      }))
            else
              None);
            (if TUtils.is_tyvar_error env ty1 || TUtils.is_tyvar_error env ty2
            then
              None
            else
              let (ty1_sub_ty2, e1) = Typing_solver.is_sub_type env ty1 ty2
              and (ty2_sub_ty1, e2) = Typing_solver.is_sub_type env ty2 ty1 in
              let e3 =
                if not (ty1_sub_ty2 && ty2_sub_ty1) then
                  Some
                    (shape
                       (Primary.Shape.Shape_field_type_mismatch
                          {
                            pos = key_pos;
                            witness_pos;
                            ty_name = lazy (Typing_print.error env ty2);
                            witness_ty_name = lazy (Typing_print.error env ty1);
                          }))
                else
                  None
              in
              Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3]);
          ]
    in
    let ty_err_opt = Typing_error.multiple_opt ty_errs in
    (env, ty_err_opt)
  in
  (* Sort the keys by their positions since the error messages will make
   * more sense if we take the one that appears first as canonical and if
   * they are processed in source order. *)
  let cmp_keys x y = Pos.compare (shape_field_pos x) (shape_field_pos y) in
  let keys = List.sort ~compare:cmp_keys keys in
  match keys with
  | [] -> env
  | witness :: rest_keys ->
    let (env, pos, info) = get_field_info env witness in
    let (env, ty_errs) =
      List.fold_left rest_keys ~init:(env, []) ~f:(fun (env, ty_errs) k ->
          match check_field pos info env k with
          | (env, Some ty_err) -> (env, ty_err :: ty_errs)
          | (env, _) -> (env, ty_errs))
    in
    let ty_err_opt = Typing_error.multiple_opt ty_errs in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    env

let update_param : decl_fun_param -> decl_ty -> decl_fun_param =
 (fun param ty -> { param with fp_type = { param.fp_type with et_type = ty } })

(* For function Shapes::idx called with a literal
 * field name, transform the decl function type from the hhi file
 * into a type that is specific to the field.
*
 * In the hhi file, the function has type
 *    Shapes::idx<Tv>(
 *      ?shape(...) $shape,
 *      arraykey $index,
 *      ?Tv,
 *    )[]: ?Tv;
 *
 * If there are two arguments, transform it to
 *    Shapes::idx<Tv>(
 *      ?shape('field_name' => Tv, ...) $shape,
 *      arraykey $index,
 *    )[]: ?Tv;
 *
 * If there are three arguments, transform it to
 *    Shapes::idx<Tv>(
 *      ?shape('field_name' => Tv, ...) $shape,
 *      arraykey $index,
 *      Tv $default,
 *    )[]: Tv;
 *)
let transform_idx_fun_ty (field_name : tshape_field_name) nargs fty =
  let (param1, param2, param3) =
    match fty.ft_params with
    | [param1; param2; param3] -> (param1, param2, param3)
    | _ -> failwith "Expected 3 parameters for Shapes::idx in hhi file"
  in
  let rret = get_reason fty.ft_ret.et_type in
  let field_ty : decl_ty =
    MakeType.generic (Reason.Rwitness_from_decl param1.fp_pos) "Tv"
  in
  let (params, ret) =
    let param1 =
      update_param
        param1
        (mk
           ( Reason.Rnone,
             Toption
               (MakeType.open_shape
                  (Reason.Rwitness_from_decl param1.fp_pos)
                  (TShapeMap.singleton
                     field_name
                     { sft_optional = true; sft_ty = field_ty })) ))
    in
    match nargs with
    | 2 ->
      (* Return type should be ?Tv *)
      let ret = MakeType.nullable rret (MakeType.generic rret "Tv") in
      ([param1; param2], ret)
    | 3 ->
      (* Third parameter should have type Tv *)
      let param3 =
        let r3 = get_reason param1.fp_type.et_type in
        update_param param3 (MakeType.generic r3 "Tv")
      in
      (* Return type should be Tv *)
      let ret = MakeType.generic rret "Tv" in
      ([param1; param2; param3], ret)
    (* Shouldn't happen! *)
    | _ -> (fty.ft_params, fty.ft_ret.et_type)
  in
  { fty with ft_params = params; ft_ret = { fty.ft_ret with et_type = ret } }

(* For function Shapes::at called with a literal
 * field name, transform the decl function type from the hhi file
 * into a type that is specific to the field.
 *
 * In the hhi file, the function has type
 *    Shapes::at<Tv>(
 *      shape(...) $shape,
 *      arraykey $index,
 *    )[]: Tv;
 *
 * Transform it to
 *    Shapes::at<Tv>(
 *      shape('field_name' => Tv, ...) $shape,
 *      arraykey $index,
 *    )[]: Tv;
 *)
let transform_at_fun_ty (field_name : tshape_field_name) fty =
  let (param1, param2) =
    match fty.ft_params with
    | [param1; param2] -> (param1, param2)
    | _ -> failwith "Expected 2 parameters for Shapes::at in hhi file"
  in
  let params =
    (* Return type should be Tv already, but first parameter is just shape(...) *)
    let field_ty : decl_ty =
      MakeType.generic (Reason.Rwitness_from_decl param1.fp_pos) "Tv"
    in
    let param1 =
      update_param
        param1
        (MakeType.open_shape
           (Reason.Rwitness_from_decl param1.fp_pos)
           (TShapeMap.singleton
              field_name
              { sft_optional = true; sft_ty = field_ty }))
    in
    [param1; param2]
  in
  { fty with ft_params = params }

(* For functions Shapes::idx or Shapes::at called with a literal
 * field name, transform the decl function type from the hhi file
 * into a type that is specific to the field.
 *)
let transform_special_shapes_fun_ty field_name id nargs fty =
  if String.equal (snd id) Naming_special_names.Shapes.at then
    transform_at_fun_ty field_name fty
  else if String.equal (snd id) Naming_special_names.Shapes.idx then
    transform_idx_fun_ty field_name nargs fty
  else
    fty
