(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val add_typing_error : Typing_error.t -> env:Typing_env_types.env -> unit

val ambiguous_inheritance :
  Pos_or_decl.t ->
  string ->
  string ->
  (Pos.t, Pos_or_decl.t) User_error.t ->
  Typing_error.Reasons_callback.t ->
  env:Typing_env_types.env ->
  unit
