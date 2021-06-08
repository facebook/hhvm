(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_defs

(* Replace unserialized information from the type with dummy information.

For example, we don't currently serialize the arity of function types, so update
the input type to set it to a default arity value. *)
let rec strip_ty ty =
  let (reason, ty) = deref ty in
  let strip_tyl tyl = List.map tyl ~f:strip_ty in
  let strip_possibly_enforced_ty et =
    { et with et_type = strip_ty et.et_type }
  in
  let ty =
    match ty with
    | Tany _
    | Tnonnull
    | Tdynamic
    | Terr ->
      ty
    | Tobject -> ty
    | Tprim _ -> ty
    | Tvar _ -> ty
    | Tgeneric (name, args) -> Tgeneric (name, strip_tyl args)
    | Tdarray (ty1, ty2) -> Tdarray (strip_ty ty1, strip_ty ty2)
    | Tvarray ty -> Tvarray (strip_ty ty)
    | Tvarray_or_darray (ty1, ty2) ->
      Tvarray_or_darray (strip_ty ty1, strip_ty ty2)
    | Tvec_or_dict (ty1, ty2) -> Tvec_or_dict (strip_ty ty1, strip_ty ty2)
    | Ttuple tyl -> Ttuple (strip_tyl tyl)
    | Toption ty -> Toption (strip_ty ty)
    | Tnewtype (name, tparams, ty) ->
      Tnewtype (name, strip_tyl tparams, strip_ty ty)
    | Tdependent (dep, ty) -> Tdependent (dep, strip_ty ty)
    | Tunion tyl -> Tunion (strip_tyl tyl)
    | Tintersection tyl -> Tintersection (strip_tyl tyl)
    | Tclass (sid, exact, tyl) -> Tclass (sid, exact, strip_tyl tyl)
    | Tfun { ft_params; ft_implicit_params = { capability }; ft_ret; _ } ->
      let strip_param ({ fp_type; _ } as fp) =
        let fp_type = strip_possibly_enforced_ty fp_type in
        {
          fp_type;
          fp_flags =
            Typing_defs.make_fp_flags
              ~mode:(get_fp_mode fp)
              ~accept_disposable:false
              ~has_default:false
              ~ifc_external:false
              ~ifc_can_call:false
              ~via_label:false
              ~readonly:false;
          (* Dummy values: these aren't currently serialized. *)
          fp_pos = Pos_or_decl.none;
          fp_name = None;
        }
      in
      let ft_params = List.map ft_params ~f:strip_param in
      let ft_implicit_params =
        {
          capability =
            (match capability with
            | CapTy cap -> CapTy (strip_ty cap)
            | CapDefaults p -> CapDefaults p);
        }
      in
      let ft_ret = strip_possibly_enforced_ty ft_ret in
      Tfun
        {
          ft_params;
          ft_implicit_params;
          ft_ret;
          (* Dummy values: these aren't currently serialized. *)
          ft_arity = Fstandard;
          ft_tparams = [];
          ft_where_constraints = [];
          ft_flags = 0;
          ft_ifc_decl = default_ifc_fun_decl;
        }
    | Tshape (shape_kind, shape_fields) ->
      let strip_field { sft_optional; sft_ty } =
        let sft_ty = strip_ty sft_ty in
        { sft_optional; sft_ty }
      in
      let shape_fields = TShapeMap.map strip_field shape_fields in
      Tshape (shape_kind, shape_fields)
    | Taccess _ -> ty
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | Tneg _ -> ty
  in
  mk (reason, ty)

(*
 * Check that type deserialization works correctly by examining every type of
 * every expression and serializing and deserializing it, and confirming that
 * it's the same type afterward.
 *
 * This is not useful work to run as a part of the typechecker, only when
 * checking the type serialization/deserialization logic. For this reason, this
 * TAST check is not enabled by default.
 *)
let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env ((p, ty), _) =
      try
        let ty = Tast_expand.expand_ty env ~pos:p ty in
        let serialized_ty = Tast_env.ty_to_json env ty in
        let deserialized_ty =
          Tast_env.json_to_locl_ty (Tast_env.get_ctx env) serialized_ty
        in
        match deserialized_ty with
        | Ok deserialized_ty ->
          let ty = strip_ty ty in
          let deserialized_ty = strip_ty deserialized_ty in
          if not (ty_equal ty deserialized_ty) then
            Errors.unserializable_type
              p
              (Printf.sprintf
                 "unequal types: %s vs %s (%s vs %s)"
                 (Tast_env.print_ty env ty)
                 (Tast_env.print_ty env deserialized_ty)
                 (Tast_env.ty_to_json env ty |> Hh_json.json_to_string)
                 ( Tast_env.ty_to_json env deserialized_ty
                 |> Hh_json.json_to_string ))
        | Error (Not_supported _) -> ()
        | Error (Wrong_phase message) ->
          Errors.unserializable_type
            p
            (Printf.sprintf
               "type %s (%s) was not in the locl phase: %s"
               message
               (Tast_env.print_ty env ty)
               (Tast_env.ty_to_json env ty |> Hh_json.json_to_string))
        | Error (Deserialization_error message) ->
          Errors.unserializable_type
            p
            (Printf.sprintf
               "type %s (%s) could not be deserialized to a locl type: %s"
               message
               (Tast_env.print_ty env ty)
               (Tast_env.ty_to_json env ty |> Hh_json.json_to_string))
      with e ->
        Errors.unserializable_type
          p
          (Printf.sprintf "exception: %s" (Exn.to_string e))
  end
