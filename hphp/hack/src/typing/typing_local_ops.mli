(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_assignment : Typing_env_types.env -> Tast.expr -> Typing_env_types.env

val enforce_io : Pos.t -> Typing_env_types.env -> Typing_env_types.env

val enforce_memoize_object :
  Pos.t -> Typing_env_types.env -> Typing_env_types.env

val enforce_enum_class_variant :
  Pos.t -> Typing_env_types.env -> Typing_env_types.env

val check_unset_target :
  Typing_env_types.env ->
  (Ast_defs.param_kind * Tast.expr) list ->
  Typing_env_types.env
