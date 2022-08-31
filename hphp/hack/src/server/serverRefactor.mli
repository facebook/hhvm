(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_fixme_patches :
  int list -> ServerEnv.env -> ServerRefactorTypes.patch list

val get_lambda_parameter_rewrite_patches :
  Provider_context.t -> string list -> ServerRefactorTypes.patch list

val get_type_params_type_rewrite_patches :
  Provider_context.t -> string list -> ServerRefactorTypes.patch list

val go :
  Provider_context.t ->
  ServerRefactorTypes.action ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ServerEnv.env
  * ServerRefactorTypes.patch list ServerCommandTypes.Done_or_retry.t

val go_sound_dynamic :
  Provider_context.t ->
  ServerRefactorTypes.action ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ServerEnv.env * string ServerCommandTypes.Done_or_retry.t

val go_ide :
  Provider_context.t ->
  string * int * int ->
  string ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ( ServerEnv.env
    * ServerRefactorTypes.patch list ServerCommandTypes.Done_or_retry.t,
    string )
  result
