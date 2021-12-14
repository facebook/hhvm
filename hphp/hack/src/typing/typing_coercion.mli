(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val coerce_type :
  ?coerce_for_op:bool ->
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Typing_error.Callback.t ->
  Typing_env_types.env

val coerce_type_res :
  ?coerce_for_op:bool ->
  Pos.t ->
  Typing_defs.Reason.ureason ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Typing_error.Callback.t ->
  (Typing_env_types.env, Typing_env_types.env) result

val try_coerce :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty Typing_defs.possibly_enforced_ty ->
  Typing_env_types.env option
