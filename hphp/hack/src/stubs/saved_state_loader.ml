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
    naming_sqlite_table_path: Path.t;
    dep_table_path: Path.t;
    errors_path: Path.t;
  }

  type dirty_files = {
    master_changes: Relative_path.Set.t;
    local_changes: Relative_path.Set.t;
  }

  type additional_info = {
    mergebase_global_rev: Hg.global_rev option;
    dirty_files_promise: dirty_files Future.t;
    saved_state_distance: int option;
    saved_state_age: int option;
  }
end

module Naming_table_info = struct
  type main_artifacts = { naming_table_path: Path.t }

  type additional_info = unit
end

module Shallow_decls_info = struct
  type main_artifacts = { shallow_decls_path: Path.t }

  type additional_info = unit
end

type _ saved_state_type =
  | Naming_and_dep_table_distc
      : (Naming_and_dep_table_info.main_artifacts
        * Naming_and_dep_table_info.additional_info)
        saved_state_type
  | Naming_table
      : (Naming_table_info.main_artifacts * Naming_table_info.additional_info)
        saved_state_type
  | Shallow_decls
      : (Shallow_decls_info.main_artifacts * Shallow_decls_info.additional_info)
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
  corresponding_rev: Hg.Rev.t;
  mergebase_rev: Hg.Rev.t;
  is_cached: bool;
}

module LoadError = struct
  type t = string

  (* Please do not throw an exception here; it breaks hack for open source users *)
  let short_user_message_of_error _ =
    "Saved states are not supported in this build."

  let medium_user_message_of_error _ =
    "Saved states are not supported in this build."

  let long_user_message_of_error _ =
    "Saved states are not supported in this build."

  let saved_state_manifold_api_key_of_error _ = None

  let debug_details_of_error _ = ""

  let category_of_error _ = ""

  let is_error_actionable _ = false
end

let get_project_name _ = ""

let ignore_saved_state_version_mismatch ~ignore_hh_version = ignore_hh_version

let get_query_for_root ~root:_ ~relative_root:_ _ _ = ""
