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
  match deref ty with
  | (r, Tshape (shape_kind, fields)) ->
    begin
      match TShapeMap.find_opt field_name fields with
      | None ->
        let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
        let sft = { sft_optional = true; sft_ty = element_ty } in
        ( (env, None),
          Some
            (mk (r, Tshape (shape_kind, TShapeMap.add field_name sft fields)))
        )
      | Some _ -> ((env, None), Some ty)
    end
  | _ -> ((env, None), None)

let refine_shape field_name pos env shape =
  let ((env, e1), shape) =
    Typing_solver.expand_type_and_narrow
      ~description_of_expected:"a shape"
      env
      (widen_for_refine_shape ~expr_pos:pos field_name)
      pos
      shape
  in
  let sft_ty =
    let r =
      Reason.Rmissing_optional_field
        (get_pos shape, TUtils.get_printable_shape_field_name field_name)
    in
    let mixed = MakeType.mixed r in
    if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
      MakeType.supportdyn r mixed
    else
      mixed
  in
  let sft = { sft_optional = false; sft_ty } in
  Option.iter ~f:Errors.add_typing_error e1;
  Typing_intersection.intersect
    env
    ~r:(Reason.Rwitness pos)
    shape
    (mk (Reason.Rnone, Tshape (Open_shape, TShapeMap.singleton field_name sft)))

let make_locl_like_type env ty =
  if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
    let dyn = MakeType.dynamic (Reason.Renforceable (get_pos ty)) in
    Typing_union.union env dyn ty
  else
    (env, ty)

(*****************************************************************************)
(* Remove a field from all the shapes found in a given type.
 * The function leaves all the other types (non-shapes) unchanged.
 *)
(*****************************************************************************)

let rec shrink_shape pos field_name env shape =
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
  match get_node shape with
  | Tshape (shape_kind, fields) ->
    let fields =
      match shape_kind with
      | Closed_shape -> TShapeMap.remove field_name fields
      | Open_shape ->
        let printable_name = TUtils.get_printable_shape_field_name field_name in
        let nothing =
          MakeType.nothing (Reason.Runset_field (pos, printable_name))
        in
        TShapeMap.add
          field_name
          { sft_ty = nothing; sft_optional = true }
          fields
    in
    let result = mk (Reason.Rwitness pos, Tshape (shape_kind, fields)) in
    ((env, e1), result)
  | Tunion tyl ->
    let ((env, e2), tyl) =
      List.map_env_ty_err_opt
        env
        tyl
        ~combine_ty_errs:Typing_error.multiple_opt
        ~f:(shrink_shape pos field_name)
    in
    let result = mk (Reason.Rwitness pos, Tunion tyl) in
    ((env, Option.merge e1 e2 ~f:Typing_error.both), result)
  | _ -> ((env, e1), shape)

(* Refine the type of a shape knowing that a call to Shapes::idx is not null.
 * This means that the shape now has the field, and that the type for this
 * field is not nullable.
 * We stay quite liberal here: we add the field to the shape type regardless
 * of whether this field can be here at all. Errors will anyway be raised
 * elsewhere when typechecking the call to Shapes::idx. This allows for more
 * useful typechecking of incomplete code (code in the process of being
 * written). *)
let shapes_idx_not_null_with_ty_err env shape_ty (ty, p, field) =
  match TUtils.shape_field_name env (ty, p, field) with
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
      | (r, Tshape (shape_kind, ftm)) ->
        let (env, field_type) =
          match TShapeMap.find_opt field ftm with
          | Some { sft_ty; _ } ->
            let (env, sft_ty) =
              Typing_solver.non_null env (Pos_or_decl.of_raw_pos p) sft_ty
            in
            (env, { sft_optional = false; sft_ty })
          | None ->
            ( env,
              {
                sft_optional = false;
                sft_ty = MakeType.nonnull (Reason.Rwitness p);
              } )
        in
        let ftm = TShapeMap.add field field_type ftm in
        (env, mk (r, Tshape (shape_kind, ftm)))
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
  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  (env, res)

let make_idx_fake_super_shape shape_pos fun_name field_name field_ty =
  mk
    ( Reason.Rshape (shape_pos, fun_name),
      Tshape (Open_shape, TShapeMap.singleton field_name field_ty) )

(* Typing rules for Shapes::idx
 *
 *     e : ?shape(?sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : ?t
 *
 *     e1 : ?shape(?sfn => t, ...)
 *     e2 : t
 *     ----------------------------
 *     Shapes::idx(e1, sfn, e2) : t
 *
 *  Typing rules when the shape has a like type:
 *
 *     e : ~?shape(?sfn => t, ...)
 *     ----------------------------
 *     Shapes::idx(e, sfn) : ~?t
 *
 *     e1 : ~?shape(?sfn => t, ...)
 *     e2 : t
 *     ----------------------------
 *     Shapes::idx(e1, sfn, e2) : ~t
 *)
let idx
    env
    ~expr_pos
    ~fun_pos
    ~shape_pos
    shape_ty
    ((_, field_p, _) as field)
    default =
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let ((env, ty_err_opt), res) =
    match TUtils.shape_field_name env field with
    | None -> ((env, None), TUtils.mk_tany env field_p)
    | Some field_name ->
      let field_name = TShapeField.of_ast Pos_or_decl.of_raw_pos field_name in
      let fake_super_shape_ty =
        make_idx_fake_super_shape
          shape_pos
          "Shapes::idx"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      let nullable_super_shape =
        mk (Reason.Rnone, Toption fake_super_shape_ty)
      in
      let super_shape =
        if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
          let like_nullable_super_shape =
            MakeType.locl_like (Reason.Rwitness shape_pos) nullable_super_shape
          in
          like_nullable_super_shape
        else
          nullable_super_shape
      in
      (match default with
      | None ->
        let (env, ty_err_opt) =
          Typing_coercion.coerce_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            { et_type = super_shape; et_enforced = Unenforced }
            Typing_error.Callback.unify_error
        in
        let (env, res) = TUtils.union env res (MakeType.null fun_pos) in
        ((env, ty_err_opt), res)
      | Some (default_pos, default_ty) ->
        let (env, e1) =
          Typing_coercion.coerce_type
            shape_pos
            Reason.URparam
            env
            shape_ty
            { et_type = super_shape; et_enforced = Unenforced }
            Typing_error.Callback.unify_error
        in
        let (env, e2) =
          Type.sub_type
            default_pos
            Reason.URparam
            env
            default_ty
            res
            Typing_error.Callback.unify_error
        in
        ((env, Option.merge e1 e2 ~f:Typing_error.both), res))
  in
  let (env, res) =
    match get_node (TUtils.strip_dynamic env shape_ty) with
    | Tnewtype (n, _, _)
      when String.equal n Naming_special_names.Classes.cSupportDyn ->
      let r = get_reason shape_ty in
      TUtils.make_supportdyn r env res
    | _ -> (env, res)
  in

  Option.iter ty_err_opt ~f:Errors.add_typing_error;
  make_locl_like_type env res

let at env ~expr_pos ~shape_pos shape_ty ((_, field_p, _) as field) =
  let (env, shape_ty) = Env.expand_type env shape_ty in
  let (env, res) = Env.fresh_type env expr_pos in
  let (env, res) =
    match TUtils.shape_field_name env field with
    | None -> (env, TUtils.mk_tany env field_p)
    | Some field_name ->
      let field_name = TShapeField.of_ast Pos_or_decl.of_raw_pos field_name in
      let fake_super_shape_ty =
        make_idx_fake_super_shape
          shape_pos
          "Shapes::at"
          field_name
          { sft_optional = true; sft_ty = res }
      in
      let like_fake_super_shape_ty =
        MakeType.locl_like (Reason.Rwitness shape_pos) fake_super_shape_ty
      in
      let super_shape_ty =
        if TypecheckerOptions.pessimise_builtins (Env.get_tcopt env) then
          like_fake_super_shape_ty
        else
          fake_super_shape_ty
      in
      let (env, e1) =
        Typing_coercion.coerce_type
          shape_pos
          Reason.URparam
          env
          shape_ty
          { et_type = super_shape_ty; et_enforced = Unenforced }
          Typing_error.Callback.unify_error
      in
      Option.iter e1 ~f:Errors.add_typing_error;
      (env, res)
  in
  make_locl_like_type env res

let remove_key_with_ty_err p env shape_ty ((_, field_p, _) as field) =
  match TUtils.shape_field_name env field with
  | None -> ((env, None), TUtils.mk_tany env field_p)
  | Some field_name ->
    let field_name = TShapeField.of_ast Pos_or_decl.of_raw_pos field_name in
    shrink_shape p field_name env shape_ty

let remove_key p env shape_ty field =
  let ((env, ty_err_opt), res) = remove_key_with_ty_err p env shape_ty field in
  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  (env, res)

let to_collection env shape_ty res return_type =
  let mapper =
    object (self)
      inherit Type_mapper.shallow_type_mapper as super

      inherit! Type_mapper.tunion_type_mapper

      inherit! Type_mapper.tvar_expanding_type_mapper

      method! on_tshape env r shape_kind fdm =
        match shape_kind with
        | Closed_shape ->
          let keys = TShapeMap.keys fdm in
          let (env, keys) =
            List.map_env env keys ~f:(fun env key ->
                match key with
                | Typing_defs.TSFlit_int (p, _) ->
                  (env, MakeType.int (Reason.Rwitness_from_decl p))
                | Typing_defs.TSFlit_str (p, _) ->
                  (env, MakeType.string (Reason.Rwitness_from_decl p))
                | Typing_defs.TSFclass_const ((p, cid), (_, mid)) ->
                  begin
                    match Env.get_class env cid with
                    | Some class_ ->
                      begin
                        match Env.get_const env class_ mid with
                        | Some const ->
                          let ((env, ty_err_opt), lty) =
                            Typing_phase.localize_no_subst
                              env
                              ~ignore_errors:true
                              const.cc_type
                          in
                          Option.iter ~f:Errors.add_typing_error ty_err_opt;
                          (env, lty)
                        | None -> (env, TUtils.mk_tany_ env p)
                      end
                    | None -> (env, TUtils.mk_tany_ env p)
                  end)
          in
          let (env, key) = Typing_union.union_list env r keys in
          let values = TShapeMap.values fdm in
          let values = List.map ~f:(fun { sft_ty; _ } -> sft_ty) values in
          let (env, value) = Typing_union.union_list env r values in
          return_type env (get_reason res) key value
        | Open_shape -> (env, res)

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
        | _ -> (env, res)
    end
  in
  mapper#on_type (Type_mapper.fresh_env env) shape_ty

let to_array env pos shape_ty res =
  let ((env, e1), shape_ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape_ty
  in
  Option.iter ~f:Errors.add_typing_error e1;
  to_collection env shape_ty res (fun env r key value ->
      make_locl_like_type env (MakeType.darray r key value))

let to_dict env pos shape_ty res =
  let ((env, e1), shape_ty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"a shape"
      env
      pos
      shape_ty
  in
  Option.iter ~f:Errors.add_typing_error e1;
  to_collection env shape_ty res (fun env r key value ->
      make_locl_like_type env (MakeType.dict r key value))

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
        Errors.add_typing_error
        @@ Typing_error.(
             shape
             @@ Primary.Shape.Invalid_shape_field_name
                  { pos = key_pos; is_empty = true }));
      (env, key_pos, None)
    | Ast_defs.SFclass_const ((_p, cls), (p, y)) ->
      begin
        match Env.get_class env cls with
        | None -> (env, key_pos, Some (cls, TUtils.terr env Reason.Rnone))
        | Some cd ->
          (match Env.get_const env cd y with
          | None ->
            Errors.add_typing_error
            @@ Typing_object_get.smember_not_found
                 p
                 ~is_const:true
                 ~is_method:false
                 ~is_function_pointer:false
                 cd
                 y
                 Typing_error.Callback.unify_error;
            (env, key_pos, Some (cls, TUtils.terr env Reason.Rnone))
          | Some { cc_type; _ } ->
            let ((env, ty_err_opt), ty) =
              Typing_phase.localize_no_subst ~ignore_errors:true env cc_type
            in
            Option.iter ~f:Errors.add_typing_error ty_err_opt;
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
            Option.iter ~f:Errors.add_typing_error e2;
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
            (if String.( <> ) cls1 cls2 then
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
            (let (ty1_sub_ty2, e1) = Typing_solver.is_sub_type env ty1 ty2
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
    Option.iter ~f:Errors.add_typing_error ty_err_opt;
    env
