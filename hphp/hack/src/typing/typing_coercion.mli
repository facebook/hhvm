(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val coerce_type :
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Errors.typing_error_callback ->
  Typing_env_types.env

val coerce_type_res :
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Errors.typing_error_callback ->
  (Typing_env_types.env, Typing_env_types.env) result

val try_coerce :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Typing_env_types.env option
