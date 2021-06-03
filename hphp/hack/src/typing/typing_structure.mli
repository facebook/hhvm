(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val transform_shapemap :
  ?nullable:bool ->
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_phase Typing_defs.shape_field_type Typing_defs.TShapeMap.t ->
  Typing_env_types.env
  * Typing_defs.locl_phase Typing_defs.shape_field_type Typing_defs.TShapeMap.t
