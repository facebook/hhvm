(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val validate_hint :
  Typing_env_types.env -> Aast.hint -> Type_validator.error_emitter -> unit

val validate_type :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.decl_ty ->
  Type_validator.error_emitter ->
  unit
