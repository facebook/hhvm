open ServerCommandTypes

let debug_describe_t : type a. a t -> string = function
  | STATUS _ -> "STATUS"
  | STATUS_SINGLE _ -> "STATUS_SINGLE"
  | INFER_TYPE _ -> "INFER_TYPE"
  | INFER_TYPE_BATCH _ -> "INFER_TYPE_BATCH"
  | INFER_TYPE_ERROR _ -> "INFER_TYPE_ERROR"
  | IDE_HOVER _ -> "IDE_HOVER"
  | DOCBLOCK_AT _ -> "DOCBLOCK_AT"
  | DOCBLOCK_FOR_SYMBOL _ -> "DOCBLOCK_FOR_SYMBOL"
  | IDE_SIGNATURE_HELP _ -> "SIGNATURE_HELP"
  | COVERAGE_LEVELS _ -> "COVERAGE_LEVELS"
  | COMMANDLINE_AUTOCOMPLETE _ -> "AUTOCOMPLETE"
  | IDENTIFY_FUNCTION _ -> "IDENTIFY_FUNCTION"
  | IDENTIFY_SYMBOL _ -> "IDENTIFY_SYMBOL"
  | METHOD_JUMP _ -> "METHOD_JUMP"
  | METHOD_JUMP_BATCH _ -> "METHOD_JUMP_BATCH"
  | FIND_REFS _ -> "FIND_REFS"
  | GO_TO_IMPL _ -> "GO_TO_IMPL"
  | IDE_FIND_REFS _ -> "IDE_FIND_REFS"
  | IDE_GO_TO_IMPL _ -> "IDE_GO_TO_IMPL"
  | IDE_HIGHLIGHT_REFS _ -> "IDE_HIGHLIGHT_REFS"
  | REFACTOR _ -> "REFACTOR"
  | IDE_REFACTOR _ -> "IDE_REFACTOR"
  | DUMP_SYMBOL_INFO _ -> "DUMP_SYMBOL_INFO"
  | REMOVE_DEAD_FIXMES _ -> "REMOVE_DEAD_FIXMES"
  | REWRITE_LAMBDA_PARAMETERS _ -> "REWRITE_LAMBDA_PARAMETERS"
  | REWRITE_TYPE_PARAMS_TYPE _ -> "REWRITE_TYPE_PARAMS_TYPE"
  | SEARCH _ -> "SEARCH"
  | COVERAGE_COUNTS _ -> "COVERAGE_COUNTS"
  | LINT _ -> "LINT"
  | LINT_STDIN _ -> "LINT_STDIN"
  | LINT_ALL _ -> "LINT_ALL"
  | LINT_XCONTROLLER _ -> "LINT_XCONTROLLER"
  | CREATE_CHECKPOINT _ -> "CREATE_CHECKPOINT"
  | RETRIEVE_CHECKPOINT _ -> "RETRIEVE_CHECKPOINT"
  | DELETE_CHECKPOINT _ -> "DELETE_CHECKPOINT"
  | IN_MEMORY_DEP_TABLE_SIZE -> "IN_MEMORY_DEP_TABLE_SIZE"
  | SAVE_NAMING _ -> "SAVE_NAMING"
  | SAVE_STATE _ -> "SAVE_STATE"
  | STATS -> "STATS"
  | FORMAT _ -> "FORMAT"
  | AI_QUERY _ -> "AI_QUERY"
  | DUMP_FULL_FIDELITY_PARSE _ -> "DUMP_FULL_FIDELITY_PARSE"
  | OPEN_FILE _ -> "OPEN_FILE"
  | CLOSE_FILE _ -> "CLOSE_FILE"
  | EDIT_FILE _ -> "EDIT_FILE"
  | IDE_AUTOCOMPLETE _ -> "IDE_AUTOCOMPLETE"
  | IDE_FFP_AUTOCOMPLETE _ -> "IDE_FFP_AUTOCOMPLETE"
  | DISCONNECT -> "DISCONNECT"
  | SUBSCRIBE_DIAGNOSTIC _ -> "SUBSCRIBE_DIAGNOSTIC"
  | UNSUBSCRIBE_DIAGNOSTIC _ -> "UNSUBSCRIBE_DIAGNOSTIC"
  | OUTLINE _ -> "OUTLINE"
  | IDE_IDLE -> "IDE_IDLE"
  | RAGE -> "RAGE"
  | DYNAMIC_VIEW _ -> "DYNAMIC_VIEW"
  | CST_SEARCH _ -> "CST_SEARCH"
  | NO_PRECHECKED_FILES -> "NO_PRECHECKED_FILES"
  | GEN_HOT_CLASSES _ -> "GEN_HOT_CLASSES"
  | FUN_DEPS_BATCH _ -> "FUN_DEPS_BATCH"
  | LIST_FILES_WITH_ERRORS -> "LIST_FILES_WITH_ERRORS"
  | FILE_DEPENDENTS _ -> "FILE_DEPENDENTS"
  | IDENTIFY_TYPES _ -> "IDENTIFY_TYPES"
  | EXTRACT_STANDALONE _ -> "EXTRACT_STANDALONE"
  | CONCATENATE_ALL _ -> "CONCATENATE_ALL"
  | GO_TO_DEFINITION _ -> "GO_TO_DEFINITION"
  | BIGCODE _ -> "BIGCODE"
  | PAUSE _ -> "PAUSE"
  | GLOBAL_INFERENCE _ -> "GLOBAL_INFERENCE"
  | VERBOSE _ -> "VERBOSE"

let debug_describe_cmd : type a. a command -> string = function
  | Rpc ({ ServerCommandTypes.from; _ }, rpc) ->
    debug_describe_t rpc
    ^
    if String.equal from "" then
      ""
    else
      " --from " ^ from
  | Debug -> "Debug"

(** This returns a string that's shown "hh_server is busy [STATUS]".
The intent is that users understand what command hh_server is currently busy with.
For command-line commands, we show the "--" option that the user used, e.g. --type-at-pos.
For IDE commands like hover, we show a description like "hover". *)
let status_describe_cmd : type a. a command -> string =
 fun cmd ->
  match cmd with
  | Rpc ({ ServerCommandTypes.from; desc }, _rpc) ->
    ( if String.equal from "" then
      ""
    else
      from ^ ":" )
    ^ desc
  | Debug -> "Debug"

let debug_describe_message_type : type a. a message_type -> string = function
  | Push _ -> "Push"
  | Response _ -> "Response"
  | Hello -> "Hello"
  | Ping -> "Ping"

let extract_labelled_file (labelled_file : ServerCommandTypes.labelled_file) :
    Relative_path.t * ServerCommandTypes.file_input =
  match labelled_file with
  | ServerCommandTypes.LabelledFileName filename ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileName filename)
  | ServerCommandTypes.LabelledFileContent { filename; content } ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileContent content)
