(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val assert_nontrivial :
  Pos.t ->
  Ast_defs.bop ->
  Typing_env_types.env ->
  Typing_defs.locl_ty ->
  Typing_defs.locl_ty ->
  unit
