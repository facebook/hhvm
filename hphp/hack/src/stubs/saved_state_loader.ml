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

module Regular_saved_state_info = struct
  type t = {
    naming_table_path: Path.t;
    deptable_path: Path.t;
    hot_decls_path: Path.t;
  }
end

module Naming_table_saved_state_info = struct
  type t = { naming_table_path: Path.t }
end

module Symbol_index_saved_state_info = struct
  type t = { symbol_index_path: Path.t }
end

type _ saved_state_type =
  | Regular : Regular_saved_state_info.t saved_state_type
  | Naming_table : Naming_table_saved_state_info.t saved_state_type
  | Symbol_index : Symbol_index_saved_state_info.t saved_state_type

(** List of files changed since the saved-state's commit. This list of files may
include files other than Hack files, so the caller should filter the given list
as necessary. *)
type changed_files = Path.t list

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
