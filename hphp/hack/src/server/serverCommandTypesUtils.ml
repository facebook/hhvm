open Hh_prelude
open ServerCommandTypes

let debug_describe_t : type a. a t -> string = function
  | STATUS _ -> "STATUS"
  | STATUS_SINGLE _ -> "STATUS_SINGLE"
  | INFER_TYPE _ -> "INFER_TYPE"
  | INFER_TYPE_BATCH _ -> "INFER_TYPE_BATCH"
  | INFER_TYPE_ERROR _ -> "INFER_TYPE_ERROR"
  | IS_SUBTYPE _ -> "IS_SUBTYPE"
  | TAST_HOLES _ -> "TAST_HOLES"
  | TAST_HOLES_BATCH _ -> "TAST_HOLES_BATCH"
  | XHP_AUTOCOMPLETE_SNIPPET _ -> "XHP_AUTOCOMPLETE_SNIPPET"
  | IDENTIFY_FUNCTION _ -> "IDENTIFY_FUNCTION"
  | IDENTIFY_SYMBOL _ -> "IDENTIFY_SYMBOL"
  | METHOD_JUMP _ -> "METHOD_JUMP"
  | METHOD_JUMP_BATCH _ -> "METHOD_JUMP_BATCH"
  | FIND_REFS _ -> "FIND_REFS"
  | GO_TO_IMPL _ -> "GO_TO_IMPL"
  | IDE_FIND_REFS_BY_SYMBOL _ -> "IDE_FIND_REFS_BY_SYMBOL"
  | IDE_GO_TO_IMPL_BY_SYMBOL _ -> "IDE_GO_TO_IMPL_BY_SYMBOL"
  | RENAME _ -> "RENAME"
  | IDE_RENAME_BY_SYMBOL _ -> "IDE_RENAME_BY_SYMBOL"
  | DUMP_SYMBOL_INFO _ -> "DUMP_SYMBOL_INFO"
  | REMOVE_DEAD_FIXMES _ -> "REMOVE_DEAD_FIXMES"
  | CODEMOD_SDT _ -> "CODEMOD_SDT"
  | REMOVE_DEAD_UNSAFE_CASTS -> "REMOVE_DEAD_UNSAFE_CASTS"
  | REWRITE_LAMBDA_PARAMETERS _ -> "REWRITE_LAMBDA_PARAMETERS"
  | SEARCH _ -> "SEARCH"
  | LINT _ -> "LINT"
  | LINT_STDIN _ -> "LINT_STDIN"
  | LINT_ALL _ -> "LINT_ALL"
  | CREATE_CHECKPOINT _ -> "CREATE_CHECKPOINT"
  | RETRIEVE_CHECKPOINT _ -> "RETRIEVE_CHECKPOINT"
  | DELETE_CHECKPOINT _ -> "DELETE_CHECKPOINT"
  | IN_MEMORY_DEP_TABLE_SIZE -> "IN_MEMORY_DEP_TABLE_SIZE"
  | SAVE_NAMING _ -> "SAVE_NAMING"
  | SAVE_STATE _ -> "SAVE_STATE"
  | STATS -> "STATS"
  | FORMAT _ -> "FORMAT"
  | DUMP_FULL_FIDELITY_PARSE _ -> "DUMP_FULL_FIDELITY_PARSE"
  | OUTLINE _ -> "OUTLINE"
  | RAGE -> "RAGE"
  | CST_SEARCH _ -> "CST_SEARCH"
  | NO_PRECHECKED_FILES -> "NO_PRECHECKED_FILES"
  | POPULATE_REMOTE_DECLS _ -> "POPULATE_REMOTE_DECLS"
  | FUN_DEPS_BATCH _ -> "FUN_DEPS_BATCH"
  | LIST_FILES_WITH_ERRORS -> "LIST_FILES_WITH_ERRORS"
  | FILE_DEPENDENTS _ -> "FILE_DEPENDENTS"
  | IDENTIFY_TYPES _ -> "IDENTIFY_TYPES"
  | EXTRACT_STANDALONE _ -> "EXTRACT_STANDALONE"
  | CONCATENATE_ALL _ -> "CONCATENATE_ALL"
  | PAUSE _ -> "PAUSE"
  | VERBOSE _ -> "VERBOSE"
  | DEPS_OUT_BATCH _ -> "DEPS_OUT_BATCH"
  | DEPS_IN_BATCH _ -> "DEPS_IN_BATCH"

let debug_describe_cmd : type a. a command -> string = function
  | Rpc ({ ServerCommandTypes.from; _ }, rpc) ->
    debug_describe_t rpc
    ^
    if String.equal from "" then
      ""
    else
      " --from " ^ from
  | Debug_DO_NOT_USE -> failwith "Debug_DO_NOT_USE"

(** This returns a string that's shown "hh_server is busy [STATUS]".
The intent is that users understand what command hh_server is currently busy with.
For command-line commands, we show the "--" option that the user used, e.g. --type-at-pos.
For IDE commands like hover, we show a description like "hover". *)
let status_describe_cmd : type a. a command -> string =
 fun cmd ->
  match cmd with
  | Rpc ({ ServerCommandTypes.from; desc }, _rpc) ->
    (if String.equal from "" then
      ""
    else
      from ^ ":")
    ^ desc
  | Debug_DO_NOT_USE -> failwith "Debug_DO_NOT_USE"

let debug_describe_message_type : type a. a message_type -> string = function
  | Hello -> "Hello"
  | Monitor_failed_to_handoff -> "Monitor_failed_to_handoff"
  | Ping -> "Ping"
  | Response _ -> "Response"
  | Push _ -> "Push"

let extract_labelled_file (labelled_file : ServerCommandTypes.labelled_file) :
    Relative_path.t * ServerCommandTypes.file_input =
  match labelled_file with
  | ServerCommandTypes.LabelledFileName filename ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileName filename)
  | ServerCommandTypes.LabelledFileContent { filename; content } ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileContent content)
