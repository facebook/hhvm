(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_param_local_ty :
  dynamic_mode:bool ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option ->
  Nast.fun_param ->
  Typing_env_types.env * Typing_defs.locl_ty

val make_param_local_tys :
  dynamic_mode:bool ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option list ->
  Nast.fun_param list ->
  Typing_env_types.env * Typing_defs.locl_ty list
