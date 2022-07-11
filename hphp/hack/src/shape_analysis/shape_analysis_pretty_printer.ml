(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types

let mk_shape field_map =
  T.(mk (Typing_reason.Rnone, Tshape (Closed_shape, field_map)))

let show_entity = function
  | Literal pos -> Format.asprintf "%a" Pos.pp pos
  | Variable var -> Format.sprintf "?%d" var

let show_ty env = Typing_print.full env

let show_constraint_ env =
  let show_ty = show_ty env in
  function
  | Marks (kind, pos) ->
    Format.asprintf "%s at %a" (show_marker_kind kind) Pos.pp pos
  | Has_static_key (entity, key, ty) ->
    let field_map =
      T.TShapeMap.singleton key T.{ sft_ty = ty; sft_optional = false }
    in
    let shape = mk_shape field_map in
    Format.asprintf "SK %s : %s" (show_entity entity) (show_ty shape)
  | Has_dynamic_key entity -> "DK " ^ show_entity entity ^ " : dyn"
  | Subsets (sub, sup) -> show_entity sub ^ " ⊆ " ^ show_entity sup
  | Joins { left; right; join } ->
    show_entity left ^ " ∪ " ^ show_entity right ^ " = " ^ show_entity join

let show_shape_result env = function
  | Shape_like_dict (pos, kind, keys_and_types) ->
    let show_ty = show_ty env in
    let shape = mk_shape keys_and_types in
    Format.asprintf
      "%s [%s]:\n  %s"
      (Format.asprintf "%a" Pos.pp pos)
      (show_marker_kind kind)
      (show_ty shape)
  | Dynamically_accessed_dict entity ->
    Format.asprintf "%s : dynamic" (show_entity entity)
