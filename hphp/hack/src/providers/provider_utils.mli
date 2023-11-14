(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Construct a [Provider_context.t] from the configuration information
contained within a [ServerEnv.env]. *)
val ctx_from_server_env : ServerEnv.env -> Provider_context.t

(** Load the declarations of [t] into any global-memory storage, then call
[f], then unload those declarations. Quarantine is REQUIRED in clientIdeDaemon and
hh_server scenarios because it embodies local-file-changes and the naming-
table updates therein, and if you try to typecheck a local files without
those updates them it will often fail. Quarantine is INAPPROPRIATE in
mapreduce or other bulk-checking scenarios which operate solely off
files-on-disk and have no concept of unsaved-file-changes.
TODO: It's a bit confusing that quarantining is predicated upon ctx, and
hopefully we'll remove that dependency in future. *)
val respect_but_quarantine_unsaved_changes :
  ctx:Provider_context.t -> f:(unit -> 'a) -> 'a

val invalidate_upon_change :
  ctx:Provider_context.t ->
  local_memory:Provider_backend.local_memory ->
  changes:FileInfo.change list ->
  entries:Provider_context.entries ->
  unit
