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
      ( env,
        Some
          (mk
             ( r,
               Tshape
                 {
                   s_origin = Missing_origin;
                   s_unknown_value = shape_kind;
                   s_fields = TShapeMap.add field_name sft fields;
                 } )) )
    | Some _ -> (env, Some ty)
  end
  | _ -> (env, None)

let tshape_field_name_with_ty_err env (_, pos, field) =
  let resolve_self env =
    let this =
      match Env.get_self_ty env with
      | None -> None
      | Some c_ty ->
        (match get_node c_ty with
        | Tclass (sid, _, _) -> Some sid
        | _ -> None)
    in
    match this with
    | Some sid -> Ok sid
    | None ->
      Error
        Typing_error.(primary @@ Primary.Expected_class { pos; suffix = None })
  in
  let p = Pos_or_decl.of_raw_pos pos in
  let map_pos = Tuple.T2.map_fst ~f:Pos_or_decl.of_raw_pos in
  match field with
  | Aast.Int name -> Ok (TSFregex_group (p, name))
  | Aast.String name -> Ok (TSFlit_str (p, name))
  | Aast.Class_const ((_, _, ((Aast.CI _ | Aast.CIself) as cid)), (_, "class"))
    ->
    Error
      Typing_error.(
        primary
        @@ Primary.Class_const_to_string
             { pos; cls_name = Typing_class_pointers.string_of_class_id_ cid })
  | Aast.Class_const ((_, _, Aast.CI x), y) ->
    Ok (TSFclass_const (map_pos x, map_pos y))
  | Aast.Nameof (_, _, Aast.CI x) ->
    Ok (TSFclass_const (map_pos x, (p, "class")))
  | Aast.Class_const ((_, _, Aast.CIself), y) ->
    let self_id = resolve_self env in
    Result.map ~f:(fun sid -> TSFclass_const (sid, map_pos y)) self_id
  | Aast.Nameof (_, _, Aast.CIself) ->
    let self_id = resolve_self env in
    Result.map ~f:(fun sid -> TSFclass_const (sid, (p, "class"))) self_id
  | _ ->
    let err =
      Typing_error.Primary.Shape.(
        Invalid_shape_field_name { pos; is_empty = false })
    in
    Error (Typing_error.shape err)

let do_with_field_expr
    (type res)
    env
    (field : ('a, 'b) Aast.expr)
    ~(with_error : res)
    (do_f : tshape_field_name -> res) : res =
  match tshape_field_name_with_ty_err env field with
  | Error ty_err ->
    Typing_error_utils.add_typing_error ~env ty_err;
    with_error
  | Ok field_name -> do_f field_name

let mixed_for_refinement env r ty =
  let mixed = MakeType.mixed r in
  if
    TUtils.is_supportdyn env ty
    && TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env)
  then
    MakeType.supportdyn r mixed
  else
    mixed

(** Refine a shape with the knowledge that field_name
  exists. We do this by intersecting with
 shape(field_name => mixed, ...) *)
let refine_key_exists field_name pos env shape =
  let ((env, e1), shape) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:pos field_name)
      pos
      shape
  in
  let mixed =
    let r =
      Reason.missing_optional_field
        (get_pos shape, TUtils.get_printable_shape_field_name field_name)
    in
    mixed_for_refinement env r shape
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e1;
  Typing_helpers.refine_and_simplify_intersection
    ~hint_first:false
    env
    ~is_class:false
    (Reason.witness pos)
    shape
    (MakeType.open_shape
       Reason.none
       ~kind:mixed
       (TShapeMap.singleton field_name { sft_optional = false; sft_ty = mixed }))

(** Returns the shape map value for `?'x' => nothing`,
  which is what we use to signify that x is unset. *)
let unset_field_shape_ty_entry pos field_name =
  {
    sft_optional = true;
    sft_ty =
      MakeType.nothing
        (Reason.unset_field
           (pos, TUtils.get_printable_shape_field_name field_name));
  }

(** Refine a shape with the knowledge that field_name
  does not exists. We do this by intersecting with
  shape(?field_name => nothing, ...) *)
let refine_not_key_exists field_name pos env shape_ty =
  let ((env, e1), shape_ty) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:pos field_name)
      pos
      shape_ty
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) e1;
  let r = Reason.witness pos in
  Typing_helpers.refine_and_simplify_intersection
    ~hint_first:false
    env
    ~is_class:false
    r
    shape_ty
    (MakeType.open_shape
       (get_reason shape_ty)
       ~kind:(mixed_for_refinement env r shape_ty)
       (TShapeMap.singleton
          field_name
          (unset_field_shape_ty_entry pos field_name)))

let refine_handle_errors
    env
    (shape_ty : locl_ty)
    (field : ('a, 'b) Aast.expr)
    (refine_f :
      Typing_env_types.env ->
      shape_ty:locl_ty ->
      tshape_field_name ->
      (Typing_env_types.env * Typing_error.t option) * locl_ty) :
    Typing_env_types.env * locl_ty =
  let ((env, ty_err_opt), res) =
    do_with_field_expr
      env
      field
      ~with_error:((env, None), shape_ty)
      (refine_f env ~shape_ty)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, res)

let refine_handle_unions_dyn pos refine_f env ~shape_ty field_name :
    (Typing_env_types.env * Typing_error.t option) * locl_ty =
  let rec go ~supportdyn env shape_ty :
      (Typing_env_types.env * Typing_error.t option) * locl_ty =
    let ((env, e1), shape) =
      Typing_solver.expand_type_and_solve
        ~freshen:false
        ~description_of_expected:"a shape"
        env
        pos
        shape_ty
    in
    let (supportdyn2, env, stripped_shape) =
      TUtils.strip_supportdyn env shape
    in
    let supportdyn = supportdyn || supportdyn2 in
    match get_node stripped_shape with
    | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fields }
      ->
      let result = refine_f stripped_shape shape_kind fields field_name in
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
          ~f:(go ~supportdyn)
      in
      let tyl = List.filter tyl ~f:(fun ty -> not (is_nothing ty)) in
      let result = mk (Reason.witness pos, Tunion tyl) in
      ((env, Option.merge e1 e2 ~f:Typing_error.both), result)
    | _ ->
      ( (env, e1),
        if supportdyn then
          let r = get_reason shape in
          MakeType.supportdyn r shape
        else
          shape )
  in
  go ~supportdyn:false env shape_ty

let refine_handle_errors_unions_dyn env pos shape_ty field refine_f =
  refine_handle_errors env shape_ty field
  @@ refine_handle_unions_dyn pos refine_f

(** Remove a field from a shape. *)
let remove_key pos env shape_ty field =
  refine_handle_errors_unions_dyn env pos shape_ty field
  @@ fun _stripped_shape shape_kind fields field_name ->
  let s_fields =
    if is_nothing shape_kind then
      TShapeMap.remove field_name fields
    else
      TShapeMap.add
        field_name
        (unset_field_shape_ty_entry pos field_name)
        fields
  in
  mk
    ( Reason.witness pos,
      Tshape
        { s_origin = Missing_origin; s_unknown_value = shape_kind; s_fields } )

(** Refine the type of a shape knowing that a call to Shapes::idx is not null.
  This means that the shape now has the field, and that the type for this
  field is not nullable.
  We stay quite liberal here: we add the field to the shape type regardless
  of whether this field can be here at all. Errors will anyway be raised
  elsewhere when typechecking the call to Shapes::idx. This allows for more
  useful typechecking of incomplete code (code in the process of being
  written). *)
let shapes_idx_not_null_with_ty_err field_p env ~shape_ty field =
  let ((env, e1), shape_ty) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:field_p field)
      field_p
      shape_ty
  in
  let refine_type env shape_ty =
    Typing_utils.map_supportdyn env shape_ty (fun env shape_ty ->
        let (env, shape_ty) = Env.expand_type env shape_ty in
        match deref shape_ty with
        | ( r,
            Tshape
              { s_origin = _; s_fields = ftm; s_unknown_value = shape_kind } )
          ->
          let (env, field_type) =
            let sft_ty =
              match TShapeMap.find_opt field ftm with
              | Some { sft_ty; _ } -> sft_ty
              | None -> shape_kind
            in
            let (env, sft_ty) =
              Typing_intersection.intersect_with_nonnull
                env
                (Pos_or_decl.of_raw_pos field_p)
                sft_ty
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
          (env, shape_ty))
  in
  match deref shape_ty with
  | (r, Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl ~f:refine_type in
    let (env, ty) = Typing_union.union_list env r tyl in
    ((env, e1), ty)
  | _ ->
    let (env, ty) = refine_type env shape_ty in
    ((env, None), ty)

let shapes_idx_not_null env shape_ty (fld : Nast.expr) =
  let (_, p, _) = fld in
  refine_handle_errors env shape_ty fld (shapes_idx_not_null_with_ty_err p)

(* /!\ This is not used fur typing Shapes::idx anymore,
 * only to type field access on the LHS of ??
 *
 * Typing rules for Shapes::idx
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
  Typing_log.(
    log_with_level env "shapes" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [
            Log_head
              ( "Typing_shapes.idx_without_default",
                [
                  Log_type ("Shape type", shape_ty);
                  Log_head
                    ( Printf.sprintf "Field: %s" (TShapeField.name field_name),
                      [] );
                ] );
          ]));
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let ((env, ty_err_opt), res) =
    let fake_super_shape_ty =
      let r = Reason.shape (shape_pos, "Shapes::idx") in
      (* Since this shape is only used as a supertype, it is safe to use mixed
         as the kind, regardless of --everything-sdt *)
      MakeType.open_shape
        r
        ~kind:(MakeType.mixed r)
        (TShapeMap.singleton field_name { sft_optional = true; sft_ty = res })
    in
    let nullable_super_shape = mk (Reason.none, Toption fake_super_shape_ty) in
    let super_shape =
      if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
        MakeType.locl_like (Reason.witness shape_pos) nullable_super_shape
      else
        nullable_super_shape
    in
    let (env, ty_err_opt) =
      Typing_coercion.coerce_type
        shape_pos
        Reason.URparam
        env
        shape_ty
        super_shape
        Unenforced
        Typing_error.Callback.unify_error
    in
    let (env, res) = TUtils.union env res (MakeType.null Reason.none) in
    ((env, ty_err_opt), res)
  in
  Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
  let (env, stripped_shape_ty) =
    Typing_dynamic_utils.strip_dynamic env shape_ty
  in
  match get_node stripped_shape_ty with
  | Tnewtype (n, _, _)
    when String.equal n Naming_special_names.Classes.cSupportDyn ->
    let r = get_reason shape_ty in
    TUtils.make_supportdyn r env res
  | _ -> (env, res)

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
                  | Typing_defs.TSFregex_group (p, _) ->
                    (env, MakeType.int (Reason.witness_from_decl p))
                  | Typing_defs.TSFlit_str (p, _) ->
                    (env, MakeType.string (Reason.witness_from_decl p))
                  | Typing_defs.TSFclass_const ((_, cid), (_, mid)) -> begin
                    match Env.get_class env cid with
                    | Decl_entry.Found class_ -> begin
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
                    | Decl_entry.DoesNotExist
                    | Decl_entry.NotYetAvailable ->
                      Env.fresh_type_error env pos
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
        let ((env, _e1), ty) =
          Typing_solver.expand_type_and_solve
            ~description_of_expected:"a shape"
            env
            pos
            ty
        in
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
        | Tnewtype (c, tyl, _) ->
          let (env, ty) =
            Typing_utils.get_newtype_super env (get_reason ty) c tyl
          in
          super#on_type env ty
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
  let (env, shape_ty) =
    TUtils.get_base_type ~expand_supportdyn:true env shape_ty
  in
  to_collection env pos shape_ty res (fun env r key value ->
      (env, MakeType.dict r key value))

let shape_field_pos = function
  | Ast_defs.SFlit_str (p, _)
  | Ast_defs.SFclassname (p, _) ->
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
    | Ast_defs.SFlit_str (_, key_name) ->
      (if Int.equal 0 (String.length key_name) then
        Typing_error_utils.add_typing_error ~env
        @@ Typing_error.(
             shape
             @@ Primary.Shape.Invalid_shape_field_name
                  { pos = key_pos; is_empty = true }));
      (env, key_pos, None)
    | Ast_defs.SFclassname (p, cls) -> begin
      match Env.get_class env cls with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        let (env, ty) = Env.fresh_type_error env p in
        (env, key_pos, Some (cls, ty))
      | Decl_entry.Found cd ->
        (* Match the sketchy Tany logic of C::class keys *)
        (match Env.get_const env cd "class" with
        | None ->
          let (env, ty) = Env.fresh_type_error env p in
          (env, key_pos, Some (cls, ty))
        | Some { cc_type; _ } ->
          let ((env, ty_err_opt), ty) =
            Typing_phase.localize_no_subst ~ignore_errors:true env cc_type
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          (env, key_pos, Some (cls, ty)))
    end
    | Ast_defs.SFclass_const ((_p, cls), (p, y)) -> begin
      match Env.get_class env cls with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        let (env, ty) = Env.fresh_type_error env p in
        (env, key_pos, Some (cls, ty))
      | Decl_entry.Found cd ->
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
          let r = Reason.witness key_pos in
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
  | [] -> (env, [])
  | witness :: rest_keys ->
    let (env, pos, info) = get_field_info env witness in
    List.fold_left rest_keys ~init:(env, []) ~f:(fun (env, ty_errs) k ->
        match check_field pos info env k with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))

let update_param : decl_fun_param -> decl_ty -> decl_fun_param =
 (fun param ty -> { param with fp_type = ty })

(** For function Shapes::idx called with a literal
  field name, transform the decl function type from the hhi file
  into a type that is specific to the field.

  In the hhi file, the function has type
    Shapes::idx<Tv>(
      ?shape(...) $shape,
      arraykey $index,
      ?Tv,
    )[]: ?Tv;

  If there are two arguments, transform it to
    Shapes::idx<Tv>(
      ?shape('field_name' => Tv, ...) $shape,
      arraykey $index,
    )[]: ?Tv;

  If there are three arguments, transform it to
    Shapes::idx<Tv>(
      ?shape('field_name' => Tv, ...) $shape,
      arraykey $index,
      Tv $default,
    )[]: Tv;
 *)
let transform_idx_fun_ty (field_name : tshape_field_name) nargs fty =
  let (param1, param2, param3) =
    match fty.ft_params with
    | [param1; param2; param3] -> (param1, param2, param3)
    | _ -> failwith "Expected 3 parameters for Shapes::idx in hhi file"
  in
  let rret = get_reason fty.ft_ret in
  let field_ty : decl_ty =
    MakeType.generic (Reason.witness_from_decl param1.fp_pos) "Tv"
  in
  let (params, ret) =
    let r = Reason.witness_from_decl param1.fp_pos in
    let param1 =
      update_param
        param1
        (mk
           ( Reason.none,
             Toption
               (* It is safe to use mixed as the kind, regardless of --everything-sdt, since
                  the idx function doesn't look at any of the other fields besides the explicitly
                  specified one *)
               (MakeType.open_shape
                  r
                  ~kind:(MakeType.mixed r)
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
        let r3 = get_reason param1.fp_type in
        update_param param3 (MakeType.generic r3 "Tv")
      in
      (* Return type should be Tv *)
      let ret = MakeType.generic rret "Tv" in
      ([param1; param2; param3], ret)
    (* Shouldn't happen! *)
    | _ -> (fty.ft_params, fty.ft_ret)
  in
  { fty with ft_params = params; ft_ret = ret }

(** For function Shapes::at called with a literal
  field name, transform the decl function type from the hhi file
  into a type that is specific to the field.

  In the hhi file, the function has type
     Shapes::at<Tv>(
       shape(...) $shape,
       arraykey $index,
     )[]: Tv;

  Transform it to
     Shapes::at<Tv>(
       shape('field_name' => Tv, ...) $shape,
       arraykey $index,
     )[]: Tv;
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
      MakeType.generic (Reason.witness_from_decl param1.fp_pos) "Tv"
    in
    let r = Reason.witness_from_decl param1.fp_pos in
    let param1 =
      update_param
        param1
        (* It is safe to use mixed as the kind, regardless of --everything-sdt, since
           the at function doesn't look at any of the other fields besides the explicitly
           specified one *)
        (MakeType.open_shape
           r
           ~kind:(MakeType.mixed r)
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
