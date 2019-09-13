open ServerCommandTypes

let debug_describe_t : type a. a t -> string = function
  | STATUS _ -> "STATUS"
  | STATUS_SINGLE _ -> "STATUS_SINGLE"
  | INFER_TYPE _ -> "INFER_TYPE"
  | INFER_TYPE_BATCH _ -> "INFER_TYPE_BATCH"
  | TYPED_AST _ -> "TYPED_AST"
  | IDE_HOVER _ -> "IDE_HOVER"
  | DOCBLOCK_AT _ -> "DOCBLOCK_AT"
  | LOCATE_SYMBOL _ -> "LOCATE_SYMBOL"
  | DOCBLOCK_FOR_SYMBOL _ -> "DOCBLOCK_FOR_SYMBOL"
  | IDE_SIGNATURE_HELP _ -> "SIGNATURE_HELP"
  | COVERAGE_LEVELS _ -> "COVERAGE_LEVELS"
  | AUTOCOMPLETE _ -> "AUTOCOMPLETE"
  | IDENTIFY_FUNCTION _ -> "IDENTIFY_FUNCTION"
  | METHOD_JUMP _ -> "METHOD_JUMP"
  | METHOD_JUMP_BATCH _ -> "METHOD_JUMP_BATCH"
  | FIND_REFS _ -> "FIND_REFS"
  | IDE_FIND_REFS _ -> "IDE_FIND_REFS"
  | IDE_HIGHLIGHT_REFS _ -> "IDE_HIGHLIGHT_REFS"
  | REFACTOR _ -> "REFACTOR"
  | IDE_REFACTOR _ -> "IDE_REFACTOR"
  | DUMP_SYMBOL_INFO _ -> "DUMP_SYMBOL_INFO"
  | REMOVE_DEAD_FIXMES _ -> "REMOVE_DEAD_FIXMES"
  | REWRITE_LAMBDA_PARAMETERS _ -> "REWRITE_LAMBDA_PARAMETERS"
  | REWRITE_RETURN_TYPE _ -> "REWRITE_RETURN_TYPE"
  | REWRITE_PARAMETER_TYPES _ -> "REWRITE_PARAMETER_TYPES"
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
  | FUN_IS_LOCALLABLE_BATCH _ -> "FUN_IS_LOCALLABLE_BATCH"
  | LIST_FILES_WITH_ERRORS -> "LIST_FILES_WITH_ERRORS"
  | FILE_DEPENDENCIES _ -> "FILE_DEPENDENCIES"
  | IDENTIFY_TYPES _ -> "IDENTIFY_TYPES"
  | EXTRACT_STANDALONE _ -> "EXTRACT_STANDALONE"
  | GO_TO_DEFINITION _ -> "GO_TO_DEFINITION"
  | BIGCODE _ -> "BIGCODE"
  | PAUSE _ -> "PAUSE"

let debug_describe_cmd : type a. a command -> string = function
  | Rpc rpc -> debug_describe_t rpc
  | Debug -> "Debug"

let source_tree_of_file_input file_input =
  match file_input with
  | ServerCommandTypes.FileName filename ->
    Full_fidelity_source_text.from_file
      (Relative_path.create_detect_prefix filename)
  | ServerCommandTypes.FileContent content ->
    Full_fidelity_source_text.make Relative_path.default content

let extract_labelled_file (labelled_file : ServerCommandTypes.labelled_file) :
    Relative_path.t * ServerCommandTypes.file_input =
  match labelled_file with
  | ServerCommandTypes.LabelledFileName filename ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileName filename)
  | ServerCommandTypes.LabelledFileContent { filename; content } ->
    let path = Relative_path.create_detect_prefix filename in
    (path, ServerCommandTypes.FileContent content)
