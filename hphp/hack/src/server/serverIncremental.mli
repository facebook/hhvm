(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val remove_defs_from_reverse_naming_table :
  Naming_table.t -> 'a Relative_path.Map.t -> unit

val get_old_and_new_defs_in_files :
  Naming_table.t ->
  Naming_table.t ->
  Relative_path.Set.t ->
  Decl_compare.VersionedNames.t Relative_path.Map.t

(** If the only things that would change about file analysis are positions,
    we're not going to recheck it, and positions in its error list might
    become stale. Look if any of those positions refer to files that have
    actually changed and add them to files to recheck.

  @param reparsed   Set of files that were reparsed (so their ASTs and positions
                    in them could have changed.

  @param errors     Current global error list
*)
val add_files_with_stale_errors :
  Provider_context.t ->
  reparsed:Relative_path.Set.t ->
  Errors.t ->
  Relative_path.Set.t ->
  Relative_path.Set.t

val resolve_files :
  Provider_context.t -> ServerEnv.env -> Fanout.t -> Relative_path.Set.t

val get_files_to_recheck :
  Provider_context.t ->
  ServerEnv.env ->
  Fanout.t ->
  reparsed:Relative_path.Set.t ->
  errors:Errors.t ->
  Relative_path.Set.t
