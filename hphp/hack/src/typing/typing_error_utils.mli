(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val add_typing_error : Typing_error.t -> env:Typing_env_types.env -> unit

val apply_error_from_reasons_callback :
  ?code:Error_codes.Typing.t ->
  ?claim:Pos.t Message.t Lazy.t ->
  ?reasons:Pos_or_decl.t Message.t list Lazy.t ->
  ?flags:User_error_flags.t ->
  ?quickfixes:Pos.t Quickfix.t list ->
  Typing_error.Reasons_callback.t ->
  env:Typing_env_types.env ->
  unit

val apply_callback_to_errors :
  Errors.t ->
  Typing_error.Reasons_callback.t ->
  env:Typing_env_types.env ->
  unit

val ambiguous_inheritance :
  Pos_or_decl.t ->
  string ->
  string ->
  (Pos.t, Pos_or_decl.t) User_error.t ->
  Typing_error.Reasons_callback.t ->
  env:Typing_env_types.env ->
  unit
