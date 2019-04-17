(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Typing_defs

(* Replace unserialized information from the with dummy information.

For example, we don't currently serialize the arity of function types, so update
the input type to set it to a default arity value. *)
let rec strip_ty : type a. a ty -> a ty = fun ty ->
  let (reason, ty) = ty in
  let strip_tyl tyl = List.map tyl ~f:strip_ty in
  let strip_opt ty_opt = Option.map ty_opt ~f:strip_ty in
  let ty =
    match ty with
    | Tmixed ->
      ty
    | Tnothing ->
      ty
    | Tany
    | Tnonnull
    | Tdynamic
    | Terr ->
      ty
    | Tobject ->
      ty
    | Tthis -> ty
    | Tgeneric _ -> ty
    | Tprim _ -> ty
    | Tvar _ -> ty
    | Tanon _ -> ty

    | Tapply (s, tyl) ->
      Tapply (s, strip_tyl tyl)
    | Taccess (ty, sids) ->
      Taccess (strip_ty ty, sids)

    | Tarray (ty1_opt, ty2_opt) ->
      Tarray (strip_opt ty1_opt, strip_opt ty2_opt)
    | Tarraykind (AKany | AKempty) ->
      ty

    | Tarraykind (AKdarray (ty1, ty2)) ->
      Tarraykind (AKdarray (strip_ty ty1, strip_ty ty2))
    | Tarraykind (AKmap (ty1, ty2)) ->
      Tarraykind (AKmap (strip_ty ty1, strip_ty ty2))
    | Tdarray (ty1, ty2) ->
      Tdarray (strip_ty ty1, strip_ty ty2)

    | Tarraykind (AKvarray ty) ->
      Tarraykind (AKvarray (strip_ty ty))
    | Tarraykind (AKvec ty) ->
      Tarraykind (AKvec (strip_ty ty))
    | Tvarray ty ->
      Tvarray (strip_ty ty)

    | Tvarray_or_darray ty ->
      Tvarray_or_darray (strip_ty ty)
    | Tarraykind (AKvarray_or_darray ty) ->
      Tarraykind (AKvarray_or_darray (strip_ty ty))

    | Ttuple tyl ->
      Ttuple (strip_tyl tyl)

    | Toption ty ->
      Toption (strip_ty ty)
    | Tlike ty ->
      Tlike (strip_ty ty)
    | Tabstract (abstract_kind, ty_opt) ->
      let abstract_kind = match abstract_kind with
        | AKnewtype (name, tparams) ->
          AKnewtype (name, strip_tyl tparams)
        | AKenum _
        | AKgeneric _
        | AKdependent _ -> abstract_kind
      in
      Tabstract (abstract_kind, strip_opt ty_opt)
    | Tunresolved tyl ->
      Tunresolved (strip_tyl tyl)
    | Tclass (sid, exact, tyl) ->
      Tclass (sid, exact, strip_tyl tyl)

    | Tfun {
        ft_is_coroutine;
        ft_params;
        ft_ret;
        _
      } ->
      let strip_param { fp_type; fp_kind; _ } =
        let fp_type = strip_ty fp_type in
        {
          fp_type;
          fp_kind;

          (* Dummy values: these aren't currently serialized. *)
          fp_pos = Pos.none;
          fp_name = None;
          fp_accept_disposable = false;
          fp_mutability = None;
          fp_rx_annotation = None;
      } in
      let ft_params = List.map ft_params ~f:strip_param in
      let ft_ret = strip_ty ft_ret in
      Tfun {
        ft_is_coroutine;
        ft_params;
        ft_ret;

        (* Dummy values: these aren't currently serialized. *)
        ft_pos = Pos.none;
        ft_deprecated = None;
        ft_abstract = false;
        ft_arity = Fstandard (0, 0);
        ft_tparams = ([], FTKtparams);
        ft_where_constraints = [];
        ft_reactive = Nonreactive;
        ft_return_disposable = false;
        ft_mutability = None;
        ft_returns_mutable = false;
        ft_decl_errors = None;
        ft_returns_void_to_rx = false;
      }

    | Tshape (shape_fields_known, shape_fields) ->
      let shape_fields_known = match shape_fields_known with
        | FieldsFullyKnown -> FieldsFullyKnown
        | FieldsPartiallyKnown _fields ->
          (* Dummy value; the partially-known fields aren't currently
          serialized. *)
          FieldsPartiallyKnown (Nast.ShapeMap.empty)
      in
      let strip_field { sft_optional; sft_ty } =
        let sft_ty = strip_ty sft_ty in
        { sft_optional; sft_ty }
      in
      let shape_fields = Nast.ShapeMap.map strip_field shape_fields in
      Tshape (shape_fields_known, shape_fields)
  in
  (reason, ty)

(*
 * Check that type deserialization works correctly by examining every type of
 * every expression and serializing and deserializing it, and confirming that
 * it's the same type afterward.
 *
 * This is not useful work to run as a part of the typechecker, only when
 * checking the type serialization/deserialization logic. For this reason, this
 * TAST check is not enabled by default.
 *)
let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env ((p, ty), _) =
    try
      let ty = Tast_expand.expand_ty env ty in
      let serialized_ty = Tast_env.ty_to_json env ty in
      let deserialized_ty = Tast_env.json_to_locl_ty serialized_ty in
      match deserialized_ty with
      | Ok deserialized_ty ->
        let ty = strip_ty ty in
        let deserialized_ty = strip_ty deserialized_ty in
        if not (ty_equal ty deserialized_ty)
        then Errors.unserializable_type p
          (Printf.sprintf "unequal types: %s vs %s (%s vs %s)"
            (Tast_env.print_ty env ty)
            (Tast_env.print_ty env deserialized_ty)
            (Tast_env.ty_to_json env ty |> Hh_json.json_to_string)
            (Tast_env.ty_to_json env deserialized_ty |> Hh_json.json_to_string))
      | Error (Not_supported _) ->
        ()
      | Error (Wrong_phase message) ->
        Errors.unserializable_type p (Printf.sprintf
          "type %s (%s) was not in the locl phase: %s"
          message
          (Tast_env.print_ty env ty)
          (Tast_env.ty_to_json env ty |> Hh_json.json_to_string))
      | Error (Deserialization_error message) ->
        Errors.unserializable_type p (Printf.sprintf
          "type %s (%s) could not be deserialized to a locl type: %s"
          message
          (Tast_env.print_ty env ty)
          (Tast_env.ty_to_json env ty |> Hh_json.json_to_string))
    with e ->
      Errors.unserializable_type p
        (Printf.sprintf "exception: %s" (Exn.to_string e))
end
