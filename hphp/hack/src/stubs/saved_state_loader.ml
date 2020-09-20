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
  type t = {
    naming_table_path: Path.t;
    dep_table_path: Path.t;
    hot_decls_path: Path.t;
    errors_path: Path.t;
  }
end

module Naming_table_info = struct
  type t = { naming_table_path: Path.t }
end

module Symbol_index_info = struct
  type t = { symbol_index_path: Path.t }
end

type _ saved_state_type =
  | Naming_and_dep_table : Naming_and_dep_table_info.t saved_state_type
  | Naming_table : Naming_table_info.t saved_state_type
  | Symbol_index : Symbol_index_info.t saved_state_type

(** List of files changed since the saved-state's commit. This list of files may
include files other than Hack files, so the caller should filter the given list
as necessary. *)
type changed_files = Relative_path.t list

type 'info load_result = {
  saved_state_info: 'info;
  manifold_path: string;
  changed_files: changed_files;
}

type load_error

(* Please do not throw an exception here; it breaks hack for open source users *)
let short_user_message_of_error _ =
  "Saved states are not supported in this build."

let medium_user_message_of_error _ =
  "Saved states are not supported in this build."

let long_user_message_of_error _ =
  "Saved states are not supported in this build."

let debug_details_of_error _ = ""

let is_error_actionable _ = false
