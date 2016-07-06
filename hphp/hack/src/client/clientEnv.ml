(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


type client_mode =
| MODE_LIST_FILES
| MODE_LIST_MODES
| MODE_TYPE_AT_POS of string
| MODE_AUTO_COMPLETE
| MODE_STATUS
| MODE_SHOW of string
| MODE_COLORING of string
| MODE_COVERAGE of string
| MODE_FIND_REFS of string
| MODE_IDENTIFY_FUNCTION of string
| MODE_OUTLINE
| MODE_OUTLINE2
| MODE_METHOD_JUMP_CHILDREN of string
| MODE_METHOD_JUMP_ANCESTORS of string
| MODE_REFACTOR of string * string * string
| MODE_FIND_CLASS_REFS of string
| MODE_ARGUMENT_INFO of string
(* TODO figure out why we can't reference FuzzySearchService from here *)
| MODE_SEARCH of string * string
| MODE_LINT of string list
| MODE_LINT_ALL of int
| MODE_DUMP_SYMBOL_INFO of string
| MODE_DUMP_AI_INFO of string
| MODE_CREATE_CHECKPOINT of string
| MODE_RETRIEVE_CHECKPOINT of string
| MODE_DELETE_CHECKPOINT of string
| MODE_STATS
| MODE_FIND_LVAR_REFS of string
| MODE_GET_METHOD_NAME of string
| MODE_FORMAT of int * int
| MODE_FIND_DEPENDENT_FILES of string
| MODE_GET_DEFINITION of string
| MODE_GET_DEFINITION_BY_ID of string
| MODE_TRACE_AI of string
| MODE_REMOVE_DEAD_FIXMES of int list
| MODE_IDE_FIND_REFS of string

type client_check_env = {
  mode: client_mode;
  root: Path.t;
  from: string;
  output_json: bool;
  retry_if_init: bool;
  retries: int;
  timeout: float option;
  autostart: bool;
  no_load: bool;
  ai_mode: string option;
}

let mode_to_string = function
  | MODE_LIST_FILES -> "MODE_LIST_FILES"
  | MODE_LIST_MODES -> "MODE_LIST_MODES"
  | MODE_TYPE_AT_POS _ -> "MODE_TYPE_AT_POS"
  | MODE_AUTO_COMPLETE -> "MODE_AUTO_COMPLETE"
  | MODE_STATUS -> "MODE_STATUS"
  | MODE_SHOW _ -> "MODE_SHOW"
  | MODE_COLORING _ -> "MODE_COLORING"
  | MODE_COVERAGE _ -> "MODE_COVERAGE"
  | MODE_FIND_REFS _ -> "MODE_FIND_REFS"
  | MODE_IDENTIFY_FUNCTION _ -> "MODE_IDENTIFY_FUNCTION"
  | MODE_OUTLINE -> "MODE_OUTLINE"
  | MODE_OUTLINE2 -> "MODE_OUTLINE2"
  | MODE_METHOD_JUMP_CHILDREN _ -> "MODE_METHOD_JUMP_CHILDREN"
  | MODE_METHOD_JUMP_ANCESTORS _ -> "MODE_METHOD_JUMP_ANCESTORS"
  | MODE_REFACTOR _ -> "MODE_REFACTOR"
  | MODE_FIND_CLASS_REFS _ -> "MODE_FIND_CLASS_REFS"
  | MODE_ARGUMENT_INFO _ -> "MODE_ARGUMENT_INFO"
  | MODE_SEARCH _ -> "MODE_SEARCH"
  | MODE_LINT _ -> "MODE_LINT"
  | MODE_LINT_ALL _ -> "MODE_LINT_ALL"
  | MODE_DUMP_SYMBOL_INFO _ -> "MODE_DUMP_SYMBOL_INFO"
  | MODE_DUMP_AI_INFO _ -> "MODE_DUMP_AI_INFO"
  | MODE_CREATE_CHECKPOINT _ -> "MODE_CREATE_CHECKPOINT"
  | MODE_RETRIEVE_CHECKPOINT _ -> "MODE_RETRIEVE_CHECKPOINT"
  | MODE_DELETE_CHECKPOINT _ -> "MODE_DELETE_CHECKPOINT"
  | MODE_STATS -> "MODE_STATS"
  | MODE_FIND_LVAR_REFS _ -> "MODE_FIND_LVAR_REFS"
  | MODE_GET_METHOD_NAME _ -> "MODE_GET_METHOD_NAME"
  | MODE_FORMAT _ -> "MODE_FORMAT"
  | MODE_FIND_DEPENDENT_FILES _ -> "MODE_FIND_DEPENDENT_FILES"
  | MODE_GET_DEFINITION _ -> "MODE_GET_DEFINITION"
  | MODE_TRACE_AI _ -> "MODE_TRACE_AI"
  | MODE_REMOVE_DEAD_FIXMES _ -> "MODE_REMOVE_DEAD_FIXMES"
  | MODE_IDE_FIND_REFS _ -> "MODE_IDE_FIND_REFS"
  | MODE_GET_DEFINITION_BY_ID _ -> "MODE_GET_DEFINITION_BY_ID"
