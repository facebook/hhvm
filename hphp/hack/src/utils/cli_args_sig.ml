(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Types = struct
  type saved_state_target_info = {
    changes: Relative_path.t list;
        [@printer Utils.pp_large_list Relative_path.pp]
    naming_changes: Relative_path.t list;
        [@printer Utils.pp_large_list Relative_path.pp]
    corresponding_base_revision: Hg.Rev.t;
    deptable_fn: string;
    compressed_deptable_fn: string option;
    prechecked_changes: Relative_path.t list;
        [@printer Utils.pp_large_list Relative_path.pp]
    naming_table_path: string;
  }
  [@@deriving show]

  (* The idea of a file range necessarily means that the hypothetical list
     of them is sorted in some way. It is valid to have None as either endpoint
     because that simply makes it open-ended. For example, a range of files
     { None - "/some/path" } includes all files with path less than /some/path *)
  type files_to_check_range = {
    from_prefix_incl: Relative_path.t option;
    to_prefix_excl: Relative_path.t option;
  }

  type files_to_check_spec =
    | Range of files_to_check_range
    | Prefix of Relative_path.t

  type save_state_spec_info = {
    files_to_check: files_to_check_spec list;
    (* The base name of the file into which we should save the naming table. *)
    filename: string;
    (* Indicates whether we should generate a state in the presence of errors. *)
    gen_with_errors: bool;
  }
end

module type S = sig
  include module type of Types

  val save_state_spec_json_descr : string

  val get_save_state_spec :
    string option -> (save_state_spec_info option, string) result

  val get_save_state_spec_json : save_state_spec_info -> string

  val saved_state_json_descr : string

  val get_saved_state_spec :
    string option -> (saved_state_target_info option, string) result

  val legacy_hot_decls_path_for_target_info : saved_state_target_info -> string

  val shallow_hot_decls_path_for_target_info : saved_state_target_info -> string

  val naming_sqlite_path_for_target_info : saved_state_target_info -> string

  val errors_path_for_target_info : saved_state_target_info -> string
end
