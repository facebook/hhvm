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
  old_file_info: FileInfo.t option;
  new_file_info: FileInfo.t option;
}

(** Updates the reverse-naming-table (which is inside ctx for the local
memory backend, and is a sharedmem heap for the sharedmem backend).
Returns an updated forward-naming-table in 'naming_table', and updated
symbol-search index in 'sienv'. It does this by by parsing the file at
the given path and reading their declarations. If the file could not be read,
it's assumed to be deleted, and so the old forward-naming-table indicates
which caches will have to be deleted by the caller.

Note: this function ignores non-root files,
and those that fail FindUtils.path_filter.

IO: this function uses File_provider to read the file. *)
val update_naming_tables_for_changed_file :
  ctx:Provider_context.t ->
  naming_table:Naming_table.t ->
  sienv:SearchUtils.si_env ->
  path:Relative_path.t ->
  changed_file_results

module Batch : sig
  type changed_file_info = {
    path: Relative_path.t;
    old_file_info: FileInfo.t option;
    new_file_info: FileInfo.t option;
  }

  type update_result = {
    naming_table: Naming_table.t;
    sienv: SearchUtils.si_env;
    changes: changed_file_info list;
  }

  (** Like [update_naming_tables_for_changed_file], but uses a Rust multi-threaded
  indexer to parse files. TODO: will replace
  [update_naming_tables_for_changed_file] when fully tested. *)
  val update_naming_tables_and_si :
    ctx:Provider_context.t ->
    naming_table:Naming_table.t ->
    sienv:SearchUtils.si_env ->
    changes:Relative_path.Set.t ->
    update_result
end
