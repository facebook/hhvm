(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_param_has_hint :
  Typing_env_types.env -> Nast.fun_param -> Typing_defs.locl_ty -> unit

val make_param_local_ty :
  Typing_env_types.env ->
  Typing_defs.decl_ty option ->
  Nast.fun_param ->
  Typing_env_types.env * Typing_defs.locl_ty
