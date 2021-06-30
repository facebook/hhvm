(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val capability_id : Local_id.t

val local_capability_id : Local_id.t

val register_capabilities :
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val get_type : Typing_defs.locl_ty Typing_defs.capability -> Typing_defs.locl_ty

val validate_capability :
  Typing_env_types.env -> Pos.t -> Typing_defs.locl_ty -> unit

val pretty : Typing_env_types.env -> Typing_defs.locl_ty -> string
