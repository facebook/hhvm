(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val sub_string_err :
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * (Typing_defs.locl_ty * Typing_defs.locl_ty) option

val sub_string :
  Pos.t -> Typing_env_types.env -> Typing_defs.locl_ty -> Typing_env_types.env
