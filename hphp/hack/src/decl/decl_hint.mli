(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val hint : Decl_env.env -> Aast.hint -> Typing_defs.decl_ty

val aast_tparam_to_decl_tparam :
  Decl_env.env -> Nast.tparam -> Typing_defs.decl_ty Typing_defs.tparam

val aast_user_attribute_to_decl_user_attribute :
  Decl_env.env -> Nast.user_attribute -> Typing_defs.user_attribute

val aast_contexts_to_decl_capability :
  Decl_env.env ->
  Aast.contexts option ->
  Aast.pos ->
  Aast.pos * Typing_defs.decl_ty Typing_defs.capability
