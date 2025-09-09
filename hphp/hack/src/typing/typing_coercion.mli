(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val coerce_type :
  ?coerce_for_op:bool ->
  ?is_dynamic_aware:bool ->
  ?ignore_readonly:bool ->
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
