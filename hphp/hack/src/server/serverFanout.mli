(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
