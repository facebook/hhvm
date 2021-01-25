(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val validate_hint :
  Tast_env.t -> Aast.hint -> Type_validator.error_emitter -> unit

val validate_type :
  Tast_env.t ->
  Pos.t ->
  Typing_defs.decl_ty ->
  Type_validator.error_emitter ->
  unit
