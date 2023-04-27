(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module HT = Hips_types

type 'constraint_ show_constraint_ =
  Typing_env_types.env -> 'constraint_ -> string

let mk_shape field_map =
  Typing_make_type.closed_shape Typing_reason.Rnone field_map

let show_entity = function
  | Literal pos -> Format.asprintf "%a" Pos.pp pos
  | Variable var -> Format.sprintf "?%d" var
  | Inter ent -> HT.show_entity ent

let show_ty env = Typing_print.full env

let show_variety = function
  | Has -> "!"
  | Needs -> "?"

let show_constraint env =
  let show_ty = show_ty env in
  function
  | Marks (kind, pos) ->
    Format.asprintf "%s at %a" (show_marker_kind kind) Pos.pp pos
  | Static_key (variety, certainty, entity, key, ty) ->
    let sft_optional =
      match certainty with
      | Maybe -> true
      | Definite -> false
    in
    let field_map = T.TShapeMap.singleton key T.{ sft_ty = ty; sft_optional } in
    let shape = mk_shape field_map in
    Format.asprintf
      "%s SK %s : %s"
      (show_variety variety)
      (show_entity entity)
      (show_ty shape)
  | Has_dynamic_key entity -> "DK " ^ show_entity entity ^ " : dyn"
  | Subsets (sub, sup) -> show_entity sub ^ " ⊆ " ^ show_entity sup

let show_inter_constraint _ = function
  | HT.ArgLike (((_, f_id), arg_idx), ent) ->
    Format.asprintf
      "ArgLike(%s, %s, %s)"
      f_id
      (HT.show_param_like_index arg_idx)
      (show_entity ent)
  | HT.Constant const ->
    Format.asprintf "Constant %s" (HT.show_const_entity const)
  | HT.ConstantInitial inst ->
    Format.asprintf "Constant initialization at %s" (show_entity inst)
  | HT.ConstantIdentifier ident ->
    Format.asprintf
      "ConstantIdentifier %s"
      (HT.show_constant_identifier_entity ident)
  | HT.ParamLike param ->
    Format.asprintf "ParamLike %s" (HT.show_param_like_entity param)
  | HT.ClassExtends ident ->
    Format.asprintf "Extends %s " (HT.show_class_identifier_entity ident)

let show_decorated_constraint_general
    ~verbosity
    env
    ~show_constr
    ({ hack_pos; origin; constraint_ } : 'constraint_ decorated) =
  let line = Pos.line hack_pos in
  let constraint_ = show_constr env constraint_ in
  if verbosity > 0 then
    Format.asprintf "%4d: %4d: %s" line origin constraint_
  else
    Format.asprintf "%4d: %s" line constraint_

let show_decorated_constraint =
  show_decorated_constraint_general ~show_constr:show_constraint

let show_decorated_inter_constraint =
  show_decorated_constraint_general ~show_constr:show_inter_constraint

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
