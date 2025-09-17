(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type rename_mode =
  | Function
  | Class
  | Method
  | Unspecified

type status_single = {
  filenames: string list;
  show_tast: bool;
  preexisting_warnings: bool;
}

type client_mode =
  | MODE_CST_SEARCH of string list option
  | MODE_DUMP_SYMBOL_INFO of string
  | MODE_FIND_CLASS_REFS of string
  | MODE_FIND_REFS of string
  | MODE_FIND_MY_TESTS of string list
  | MODE_FULL_FIDELITY_PARSE of string
  | MODE_FULL_FIDELITY_SCHEMA
  | MODE_GO_TO_IMPL_CLASS of string
  | MODE_GO_TO_IMPL_METHOD of string
  | MODE_HACK_TO_NOTEBOOK
  | MODE_IDE_FIND_REFS_BY_SYMBOL of FindRefsWireFormat.CliArgs.t
  | MODE_IDE_GO_TO_IMPL_BY_SYMBOL of FindRefsWireFormat.CliArgs.t
  | MODE_IDE_RENAME_BY_SYMBOL of string
  | MODE_IDENTIFY_SYMBOL1 of string
  | MODE_IDENTIFY_SYMBOL2 of string
  | MODE_IDENTIFY_SYMBOL3 of string
  | MODE_IDENTIFY_SYMBOL of string
  | MODE_IN_MEMORY_DEP_TABLE_SIZE
  | MODE_LINT
  | MODE_LINT_ALL of int
  | MODE_LINT_STDIN of string
  | MODE_LIST_FILES
  | MODE_METHOD_JUMP_ANCESTORS of string * string
  | MODE_METHOD_JUMP_ANCESTORS_BATCH of string list * string
  | MODE_METHOD_JUMP_CHILDREN of string
  | MODE_NOTEBOOK_TO_HACK of {
      notebook_number: string;
      notebook_header: string;
    }
  | MODE_OUTLINE
  | MODE_OUTLINE2
  | MODE_RENAME of rename_mode * string * string
  | MODE_REMOVE_DEAD_FIXMES of int list
  | MODE_REMOVE_DEAD_UNSAFE_CASTS
  | MODE_REWRITE_DECLARATIONS
  | MODE_REWRITE_LAMBDA_PARAMETERS of string list
  | MODE_SAVE_NAMING of string
  | MODE_SAVE_STATE of string
  | MODE_SEARCH of string
  | MODE_SERVER_RAGE
  | MODE_STATS
  | MODE_STATUS
  | MODE_STATUS_SINGLE of status_single
  | MODE_TYPE_AT_POS of string
  | MODE_TYPE_AT_POS_BATCH of string list
  | MODE_TYPE_ERROR_AT_POS of string
  | MODE_IS_SUBTYPE
  | MODE_TAST_HOLES of string
  | MODE_TAST_HOLES_BATCH of string
  | MODE_FUN_DEPS_AT_POS_BATCH of string list
  | MODE_FILE_LEVEL_DEPENDENCIES
  | MODE_VERBOSE of bool
  | MODE_DEPS_OUT_AT_POS_BATCH of string list
  | MODE_DEPS_IN_AT_POS_BATCH of string list
[@@deriving variants]

type client_check_env = {
  autostart: bool;
  config: (string * string) list;
  custom_hhi_path: string option;
  custom_telemetry_data: (string * string) list;
  error_format: Errors.format option;
  force_dormant_start: bool;
  from: string;
  show_spinner: bool;
  gen_saved_ignore_type_errors: bool;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  paths: string list;
  max_errors: int option;
  preexisting_warnings: bool;
      (** Whether to show preexisint warnings in typechecked files *)
  mode: client_mode;
  no_load: bool;
  save_64bit: string option;
  save_human_readable_64bit_dep_map: string option;
  output_json: bool;
  prechecked: bool option;
  mini_state: string option;
  root: Path.t;
  sort_results: bool;
  stdin_name: string option;
  deadline: float option;
  watchman_debug_logging: bool;
  allow_non_opt_build: bool;
      (** desc is a human-readable string description, to appear in "hh_server busy [desc]" *)
  desc: string;
  is_interactive: bool;
      (** Determined based on the --from option. Affects UI behaviour in a
      number of places, e.g., error formatting and spinners. *)
  warning_switches: Filter_errors.switch list;
  dump_config: bool;
}

let string_to_rename_mode = function
  | "Function" -> Function
  | "Class" -> Class
  | "Method" -> Method
  | _ ->
    Printf.fprintf
      stderr
      "Error: please provide one of the following rename modes: Function, Class, or Method. \n%!";
    exit 1
