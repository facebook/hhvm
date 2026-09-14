(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_fixme_patches :
  int list -> Server_env.env -> Server_rename_types.patch list

val get_dead_unsafe_cast_patches :
  Server_env.env -> Server_rename_types.patch list

val get_lambda_parameter_rewrite_patches :
  Provider_context.t -> string list -> Server_rename_types.patch list

(** Does the rename. [definition_for_wrapper] is the definition where a deprecated-wrapper may
be generated, or None to suppress any possible generation. *)
val go :
  Provider_context.t ->
  Server_rename_types.action ->
  Server_env.genv ->
  Server_env.env ->
  definition_for_wrapper:Relative_path.t Symbol_definition.t option ->
  Server_env.env
  * Server_rename_types.patch list Server_command_types.Done_or_retry.t

val go_for_single_file :
  Provider_context.t ->
  find_refs_action:Server_command_types.Find_refs.action ->
  new_name:string ->
  filename:Relative_path.t ->
  symbol_definition:Relative_path.t Symbol_definition.t ->
  (Server_rename_types.patch list, 'a) result

val go_ide_with_find_refs_action :
  Provider_context.t ->
  find_refs_action:Server_command_types.Find_refs.action ->
  new_name:string ->
  symbol_definition:Relative_path.t Symbol_definition.t ->
  Server_env.genv ->
  Server_env.env ->
  ( Server_env.env
    * Server_rename_types.patch list Server_command_types.Done_or_retry.t,
    string )
  result

val go_for_localvar :
  Provider_context.t ->
  Server_command_types.Find_refs.action ->
  string ->
  ( Server_rename_types.patch list option,
    Server_command_types.Find_refs.action )
  result
