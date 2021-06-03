(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val binop :
  Pos.t ->
  Typing_env_types.env ->
  Ast_defs.bop ->
  Pos.t ->
  Tast.expr ->
  Typing_defs.locl_ty ->
  Pos.t ->
  Tast.expr ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Tast.expr * Tast.ty

val unop :
  Pos.t ->
  Typing_env_types.env ->
  Ast_defs.uop ->
  Tast.expr ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Tast.expr * Tast.ty
