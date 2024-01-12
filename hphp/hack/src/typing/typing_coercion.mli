(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val coerce_type :
  ?coerce_for_op:bool ->
  ?coerce:Typing_logic.coercion_direction option ->
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_defs.enforcement ->
  Typing_error.Callback.t ->
  Typing_env_types.env * Typing_error.t option

val coerce_type_like_strip :
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_error.Callback.t ->
  Typing_env_types.env
  * (Typing_defs.locl_ty * Typing_defs.locl_ty) option
  * bool
  * Typing_defs.locl_ty

val try_coerce :
  ?coerce:Typing_logic.coercion_direction option ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env option
