(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type changed_file_results = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
}

(** Updates the reverse-naming-table (which is inside ctx for the local
memory backend, and is a sharedmem heap for the sharedmem backend).
Returns an updated forward-naming-table in 'naming_table', and updated
symbol-search index in 'sienv'. Also invalidates various caches inside
ctx backend (but not the ctx.entry caches).

It does this by by parsing the file at the given path and reading their
declarations. If the file could not be read, it's assumed to be deleted,
and so the old forward-naming-table indicates which caches must be deleted. *)
val process_changed_file :
  ctx:Provider_context.t ->
  naming_table:Naming_table.t ->
  sienv:SearchUtils.si_env ->
  path:Path.t ->
  changed_file_results Lwt.t
