(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type client_mode =
  | MODE_AI_QUERY of string
  | MODE_AUTO_COMPLETE
  | MODE_BIGCODE of string
  | MODE_COLORING of string
  | MODE_COVERAGE of string
  | MODE_CREATE_CHECKPOINT of string
  | MODE_CST_SEARCH of string list option
  | MODE_DELETE_CHECKPOINT of string
  | MODE_DUMP_SYMBOL_INFO of string
  | MODE_EXTRACT_STANDALONE of string
  | MODE_CONCATENATE_ALL
  | MODE_FIND_CLASS_REFS of string
  | MODE_FIND_REFS of string
  | MODE_FORMAT of int * int
  | MODE_FULL_FIDELITY_PARSE of string
  | MODE_FULL_FIDELITY_SCHEMA
  | MODE_GEN_HOT_CLASSES of int * string
  | MODE_GO_TO_IMPL_CLASS of string
  | MODE_GO_TO_IMPL_CLASS_REMOTE of string
  | MODE_GO_TO_IMPL_METHOD of string
  | MODE_IDE_FIND_REFS of string
  | MODE_IDE_HIGHLIGHT_REFS of string
  | MODE_IDE_REFACTOR of string
  | MODE_IDENTIFY_SYMBOL1 of string
  | MODE_IDENTIFY_SYMBOL2 of string
  | MODE_IDENTIFY_SYMBOL3 of string
  | MODE_IDENTIFY_SYMBOL of string
  | MODE_IN_MEMORY_DEP_TABLE_SIZE
  | MODE_LINT
  | MODE_LINT_ALL of int
  | MODE_LINT_STDIN of string
  | MODE_LINT_XCONTROLLER of string
  | MODE_LIST_FILES
  | MODE_METHOD_JUMP_ANCESTORS of string * string
  | MODE_METHOD_JUMP_ANCESTORS_BATCH of string list * string
  | MODE_METHOD_JUMP_CHILDREN of string
  | MODE_OUTLINE
  | MODE_OUTLINE2
  | MODE_PAUSE of bool
  | MODE_REFACTOR of string * string * string
  | MODE_REMOVE_DEAD_FIXMES of int list
  | MODE_REWRITE_LAMBDA_PARAMETERS of string list
  | MODE_REWRITE_TYPE_PARAMS_TYPE of string list
  | MODE_RETRIEVE_CHECKPOINT of string
  | MODE_SAVE_NAMING of string
  | MODE_SAVE_STATE of string
  (* TODO figure out why we can't reference FuzzySearchService from here *)
  | MODE_SEARCH of string * string
  | MODE_SERVER_RAGE
  | MODE_STATS
  | MODE_STATUS
  | MODE_STATUS_SINGLE of string (* filename *)
  | MODE_TYPE_AT_POS of string
  | MODE_TYPE_AT_POS_BATCH of string list
  | MODE_TYPE_ERROR_AT_POS of string
  | MODE_FUN_DEPS_AT_POS_BATCH of string list
  | MODE_FILE_DEPENDENTS
  | MODE_GLOBAL_INFERENCE of ServerGlobalInferenceTypes.mode * string list
  | MODE_VERBOSE of bool

type client_check_env = {
  ai_mode: string option;
  autostart: bool;
  config: (string * string) list;
  custom_telemetry_data: (string * string) list;
  dynamic_view: bool;
  error_format: Errors.format;
  force_dormant_start: bool;
  from: string;
  show_spinner: bool;
  gen_saved_ignore_type_errors: bool;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  paths: string list;
  log_inference_constraints: bool;
  max_errors: int option;
  mode: client_mode;
  no_load: bool;
  save_64bit: string option;
  output_json: bool;
  prechecked: bool option;
  mini_state: string option;
  profile_log: bool;
  remote: bool;
  replace_state_after_saving: bool;
  root: Path.t;
  sort_results: bool;
  stdin_name: string option;
  deadline: float option;
  watchman_debug_logging: bool;
  allow_non_opt_build: bool;
      (** desc is a human-readable string description, to appear in "hh_server busy [desc]" *)
  desc: string;
}

let mode_to_string = function
  | MODE_AI_QUERY _ -> "MODE_AI_QUERY"
  | MODE_AUTO_COMPLETE -> "MODE_AUTO_COMPLETE"
  | MODE_BIGCODE _ -> "MODE_BIGCODE"
  | MODE_COLORING _ -> "MODE_COLORING"
  | MODE_COVERAGE _ -> "MODE_COVERAGE"
  | MODE_CREATE_CHECKPOINT _ -> "MODE_CREATE_CHECKPOINT"
  | MODE_CST_SEARCH _ -> "MODE_CST_SEARCH"
  | MODE_DELETE_CHECKPOINT _ -> "MODE_DELETE_CHECKPOINT"
  | MODE_DUMP_SYMBOL_INFO _ -> "MODE_DUMP_SYMBOL_INFO"
  | MODE_EXTRACT_STANDALONE _ -> "MODE_EXTRACT_STANDALONE"
  | MODE_CONCATENATE_ALL -> "MODE_CONCATENATE_ALL"
  | MODE_FIND_CLASS_REFS _ -> "MODE_FIND_CLASS_REFS"
  | MODE_FIND_REFS _ -> "MODE_FIND_REFS"
  | MODE_FORMAT _ -> "MODE_FORMAT"
  | MODE_FULL_FIDELITY_PARSE _ -> "MODE_FULL_FIDELITY_PARSE"
  | MODE_FULL_FIDELITY_SCHEMA -> "MODE_FULL_FIDELITY_SCHEMA"
  | MODE_GEN_HOT_CLASSES _ -> "MODE_GEN_HOT_CLASSES"
  | MODE_GO_TO_IMPL_CLASS _ -> "MODE_GO_TO_IMPL_CLASS"
  | MODE_GO_TO_IMPL_CLASS_REMOTE _ -> "MODE_GO_TO_IMPL_CLASS_REMOTE"
  | MODE_GO_TO_IMPL_METHOD _ -> "MODE_GO_TO_IMPL_METHOD"
  | MODE_IDE_FIND_REFS _ -> "MODE_IDE_FIND_REFS"
  | MODE_IDE_HIGHLIGHT_REFS _ -> "MODE_IDE_HIGHLIGHT_REFS"
  | MODE_IDE_REFACTOR _ -> "MODE_IDE_REFACTOR"
  | MODE_IDENTIFY_SYMBOL _ -> "MODE_IDENTIFY_SYMBOL"
  | MODE_IDENTIFY_SYMBOL1 _ -> "MODE_IDENTIFY_SYMBOL1"
  | MODE_IDENTIFY_SYMBOL2 _ -> "MODE_IDENTIFY_SYMBOL2"
  | MODE_IDENTIFY_SYMBOL3 _ -> "MODE_IDENTIFY_SYMBOL3"
  | MODE_IN_MEMORY_DEP_TABLE_SIZE -> "MODE_IN_MEMORY_DEP_TABLE_SIZE"
  | MODE_LINT -> "MODE_LINT"
  | MODE_LINT_ALL _ -> "MODE_LINT_ALL"
  | MODE_LINT_STDIN _ -> "MODE_LINT_STDIN"
  | MODE_LINT_XCONTROLLER _ -> "MODE_LINT_XCONTROLLER"
  | MODE_LIST_FILES -> "MODE_LIST_FILES"
  | MODE_METHOD_JUMP_ANCESTORS _ -> "MODE_METHOD_JUMP_ANCESTORS"
  | MODE_METHOD_JUMP_ANCESTORS_BATCH _ -> "MODE_METHOD_JUMP_ANCESTORS_BATCH"
  | MODE_METHOD_JUMP_CHILDREN _ -> "MODE_METHOD_JUMP_CHILDREN"
  | MODE_OUTLINE -> "MODE_OUTLINE"
  | MODE_OUTLINE2 -> "MODE_OUTLINE2"
  | MODE_PAUSE _ -> "MODE_PAUSE"
  | MODE_REFACTOR _ -> "MODE_REFACTOR"
  | MODE_REMOVE_DEAD_FIXMES _ -> "MODE_REMOVE_DEAD_FIXMES"
  | MODE_REWRITE_LAMBDA_PARAMETERS _ -> "MODE_REWRITE_LAMBDA_PARAMETERS"
  | MODE_REWRITE_TYPE_PARAMS_TYPE _ -> "MODE_REWRITE_TYPE_PARAMS_TYPE"
  | MODE_RETRIEVE_CHECKPOINT _ -> "MODE_RETRIEVE_CHECKPOINT"
  | MODE_SAVE_NAMING _ -> "MODE_SAVE_NAMING"
  | MODE_SAVE_STATE _ -> "MODE_SAVE_STATE"
  | MODE_SEARCH _ -> "MODE_SEARCH"
  | MODE_SERVER_RAGE -> "MODE_SERVER_RAGE"
  | MODE_STATS -> "MODE_STATS"
  | MODE_STATUS -> "MODE_STATUS"
  | MODE_STATUS_SINGLE _ -> "MODE_STATUS_SINGLE"
  | MODE_TYPE_AT_POS _ -> "MODE_TYPE_AT_POS"
  | MODE_TYPE_AT_POS_BATCH _ -> "MODE_TYPE_AT_POS_BATCH"
  | MODE_TYPE_ERROR_AT_POS _ -> "MODE_TYPE_ERROR_AT_POS"
  | MODE_FUN_DEPS_AT_POS_BATCH _ -> "MODE_FUN_DEPS_AT_POS_BATCH"
  | MODE_FILE_DEPENDENTS -> "MODE_FILE_LEVEL_DEPENDENCIES"
  | MODE_GLOBAL_INFERENCE _ -> "MODE_GLOBAL_INFERENCE"
  | MODE_VERBOSE _ -> "MODE_VERBOSE"
