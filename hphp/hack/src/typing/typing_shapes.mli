(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val refine_shape :
  Typing_defs.TShapeMap.key ->
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val idx :
  Typing_env_types.env ->
  expr_pos:Pos.t ->
  fun_pos:Typing_defs.locl_phase Typing_reason.t_ ->
  shape_pos:Pos.t ->
  Typing_defs.locl_ty ->
  Nast.expr ->
  (Pos.t * Typing_defs.locl_ty) option ->
  Typing_env_types.env * Typing_defs.locl_ty

val shapes_idx_not_null :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.expr ->
  Typing_env_types.env * Typing_defs.locl_ty

val at :
  Typing_env_types.env ->
  expr_pos:Pos.t ->
  shape_pos:Pos.t ->
  Typing_defs.locl_ty ->
  Nast.expr ->
  Typing_env_types.env * Typing_defs.locl_ty

val remove_key :
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.expr ->
  Typing_env_types.env * Typing_reason.locl_phase Typing_defs.ty

val to_array :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_phase Typing_defs.ty ->
  Type_mapper.result

val to_dict :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_phase Typing_defs.ty ->
  Type_mapper.result
