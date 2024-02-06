(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_fixme_patches :
  int list -> ServerEnv.env -> ServerRenameTypes.patch list

val get_dead_unsafe_cast_patches : ServerEnv.env -> ServerRenameTypes.patch list

val get_lambda_parameter_rewrite_patches :
  Provider_context.t -> string list -> ServerRenameTypes.patch list

(** Does the rename. [definition_for_wrapper] is the definition where a deprecated-wrapper may
be generated, or None to suppress any possible generation. *)
val go :
  Provider_context.t ->
  ServerRenameTypes.action ->
  ServerEnv.genv ->
  ServerEnv.env ->
  definition_for_wrapper:Relative_path.t SymbolDefinition.t option ->
  ServerEnv.env
  * ServerRenameTypes.patch list ServerCommandTypes.Done_or_retry.t

val go_for_single_file :
  Provider_context.t ->
  find_refs_action:ServerCommandTypes.Find_refs.action ->
  new_name:string ->
  filename:Relative_path.t ->
  symbol_definition:Relative_path.t SymbolDefinition.t ->
  naming_table:Naming_table.t ->
  (ServerRenameTypes.patch list, 'a) result

val go_ide_with_find_refs_action :
  Provider_context.t ->
  find_refs_action:ServerCommandTypes.Find_refs.action ->
  new_name:string ->
  symbol_definition:Relative_path.t SymbolDefinition.t ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ( ServerEnv.env
    * ServerRenameTypes.patch list ServerCommandTypes.Done_or_retry.t,
    string )
  result

val go_for_localvar :
  Provider_context.t ->
  ServerCommandTypes.Find_refs.action ->
  string ->
  ( ServerRenameTypes.patch list option,
    ServerCommandTypes.Find_refs.action )
  result
