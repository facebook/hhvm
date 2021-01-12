(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
  * Copyright (c) Facebook, Inc. and its affiliates.
  *
  * This source code is licensed under the MIT license found in the
  * LICENSE file in the "hack" directory of this source tree.
  *
  *)

type process_success = {
  command_line: string;
  stdout: string;
}

type process_failure = string

type config = string * string SMap.t

module Watchman_options = struct
  type t = {
    root: Path.t;
    sockname: Path.t option;
  }
end

module Naming_and_dep_table_info = struct
  type main_artifacts = {
    naming_table_path: Path.t;
    dep_table_path: Path.t;
    legacy_hot_decls_path: Path.t;
    shallow_hot_decls_path: Path.t;
    errors_path: Path.t;
  }

  type dirty_files = {
    master_changes: Relative_path.Set.t;
    local_changes: Relative_path.Set.t;
  }

  type additional_info = {
    mergebase_global_rev: Hg.global_rev option;
    dep_table_is_64bit: bool;
    dirty_files_promise: dirty_files Future.t;
  }
end

module Naming_table_info = struct
  type main_artifacts = { naming_table_path: Path.t }

  type additional_info = unit
end

module Symbol_index_info = struct
  type main_artifacts = { symbol_index_path: Path.t }

  type additional_info = unit
end

type _ saved_state_type =
  | Naming_and_dep_table : {
      is_64bit: bool;
    }
      -> ( Naming_and_dep_table_info.main_artifacts
         * Naming_and_dep_table_info.additional_info )
         saved_state_type
  | Naming_table
      : (Naming_table_info.main_artifacts * Naming_table_info.additional_info)
        saved_state_type
  | Symbol_index
      : (Symbol_index_info.main_artifacts * Symbol_index_info.additional_info)
        saved_state_type

(** List of files changed since the saved-state's commit. This list of files may
include files other than Hack files, so the caller should filter the given list
as necessary. *)
type changed_files = Relative_path.t list

type ('main_artifacts, 'additional_info) load_result = {
  main_artifacts: 'main_artifacts;
  additional_info: 'additional_info;
  manifold_path: string;
  changed_files: changed_files;
  corresponding_rev: Hg.hg_rev;
  mergebase_rev: Hg.hg_rev;
  is_cached: bool;
}

type load_error = string

(* Please do not throw an exception here; it breaks hack for open source users *)
let short_user_message_of_error _ =
  "Saved states are not supported in this build."

let medium_user_message_of_error _ =
  "Saved states are not supported in this build."

let long_user_message_of_error _ =
  "Saved states are not supported in this build."

let debug_details_of_error _ = ""

let is_error_actionable _ = false
