(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_param_local_tys :
  dynamic_mode:bool ->
  no_auto_likes:bool ->
  Typing_env_types.env ->
  Typing_defs.decl_ty option list ->
  Nast.fun_param list ->
  Typing_env_types.env * Typing_defs.locl_ty option list
