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

val idx_without_default :
  Typing_env_types.env ->
  expr_pos:Pos.t ->
  shape_pos:Pos.t ->
  Typing_defs.locl_ty ->
  Typing_defs.tshape_field_name ->
  Typing_env_types.env * Typing_defs.locl_ty

val shapes_idx_not_null :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Nast.expr ->
  Typing_env_types.env * Typing_defs.locl_ty

val remove_key :
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  ('a, 'b) Aast.expr ->
  Typing_env_types.env * Typing_reason.locl_phase Typing_defs.ty

val to_dict :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_phase Typing_defs.ty ->
  Type_mapper.result

val check_shape_keys_validity :
  Typing_env_types.env -> Ast_defs.shape_field_name list -> Typing_env_types.env

val transform_special_shapes_fun_ty :
  Typing_defs.tshape_field_name ->
  Aast.sid ->
  int ->
  Typing_defs.decl_fun_type ->
  Typing_defs.decl_fun_type
