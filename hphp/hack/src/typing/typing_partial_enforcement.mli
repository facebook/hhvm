(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_enforced_type :
  Typing_env_types.env ->
  Decl_provider.Class.t option ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty
