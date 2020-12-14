(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val member_type :
  Typing_env_types.env -> Typing_defs.class_elt -> Typing_defs.decl_ty

val enum_class_check :
  Typing_env_types.env ->
  Decl_provider.Class.t ->
  Nast.class_const list ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env
