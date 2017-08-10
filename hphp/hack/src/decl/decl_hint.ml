(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Converts a type hint into a type  *)
(*****************************************************************************)
open Core
open Nast
open Typing_defs

(* Unpacking a hint for typing *)

let rec hint env (p, h) =
  let h = hint_ p env h in
  Typing_reason.Rhint p, h

and shape_field_info_to_shape_field_type env { sfi_optional; sfi_hint } =
  { sft_optional = sfi_optional; sft_ty = hint env sfi_hint }

and hint_ p env = function
  | Hany -> Tany
  | Hmixed -> Tmixed
  | Hthis -> Tthis
  | Harray (h1, h2) ->
    if env.Decl_env.mode = FileInfo.Mstrict && h1 = None
    then Errors.generic_array_strict p;
    let h1 = Option.map h1 (hint env) in
    let h2 = Option.map h2 (hint env) in
    Tarray (h1, h2)
  | Hdarray (h1, h2) ->
    Tdarray (hint env h1, hint env h2)
  | Hvarray (h) ->
    Tvarray (hint env h)
  | Hdarray_or_varray h ->
    Tdarray_or_varray (hint env h)
  | Hprim p -> Tprim p
  | Habstr x ->
    Tgeneric x
  | Hoption (_, Hprim Tvoid) ->
    Errors.option_return_only_typehint p `void;
    Terr
  | Hoption (_, Hprim Tnoreturn) ->
    Errors.option_return_only_typehint p `noreturn;
    Terr
  | Hoption (_, Hmixed) ->
    Errors.option_mixed p;
    Terr
  | Hoption h ->
    let h = hint env h in
    Toption h
  | Hfun (hl, b, h) ->
    let paraml = List.map hl (hint env) in
    let paraml = List.map paraml (fun x -> None, x) in
    let ret = hint env h in
    let arity_min = List.length paraml in
    let arity = if b
      then Fellipsis arity_min
      else Fstandard (arity_min, arity_min)
    in
    Tfun {
      ft_pos = p;
      ft_deprecated = None;
      ft_abstract = false;
      ft_arity = arity;
      ft_tparams = [];
      ft_where_constraints = [];
      ft_params = paraml;
      ft_ret = ret;
    }
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    Errors.tuple_syntax p;
    Terr
  | Happly (((_p, c) as id), argl) ->
    Decl_hooks.dispatch_class_id_hook id None;
    Decl_env.add_wclass env c;
    let argl = List.map argl (hint env) in
    Tapply (id, argl)
  | Haccess (root_ty, ids) ->
    let root_ty = hint env root_ty in
    Taccess (root_ty, ids)
  | Htuple hl ->
    let tyl = List.map hl (hint env) in
    Ttuple tyl
  | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
    let optional_shape_fields_enabled =
      TypecheckerOptions.experimental_feature_enabled
        env.Decl_env.decl_tcopt
        TypecheckerOptions.experimental_optional_shape_field in
    let shape_fields_known =
      match optional_shape_fields_enabled, nsi_allows_unknown_fields with
        | _, true
        | false, false ->
          (* Fields are only partially known, because this shape type comes from
           * type hint - shapes that contain listed fields can be passed here,
           * but due to structural subtyping they can also contain other fields,
           * that we don't know about. *)
          FieldsPartiallyKnown ShapeMap.empty
        | true, false ->
          FieldsFullyKnown in
    let fdm =
      ShapeMap.map (shape_field_info_to_shape_field_type env) nsi_field_map in
    Tshape (shape_fields_known, fdm)
