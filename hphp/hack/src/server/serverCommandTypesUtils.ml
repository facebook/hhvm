open ServerCommandTypes

let debug_describe_t : type a. a t -> string = function
  | STATUS                   _ -> "STATUS"
  | STATUS_SINGLE            _ -> "STATUS_SINGLE"
  | INFER_TYPE               _ -> "INFER_TYPE"
  | INFER_TYPE_BATCH         _ -> "INFER_TYPE_BATCH"
  | TYPED_AST                _ -> "TYPED_AST"
  | IDE_HOVER                _ -> "IDE_HOVER"
  | DOCBLOCK_AT              _ -> "DOCBLOCK_AT"
  | IDE_SIGNATURE_HELP       _ -> "SIGNATURE_HELP"
  | COVERAGE_LEVELS          _ -> "COVERAGE_LEVELS"
  | AUTOCOMPLETE             _ -> "AUTOCOMPLETE"
  | IDENTIFY_FUNCTION        _ -> "IDENTIFY_FUNCTION"
  | METHOD_JUMP              _ -> "METHOD_JUMP"
  | METHOD_JUMP_BATCH        _ -> "METHOD_JUMP_BATCH"
  | FIND_DEPENDENT_FILES     _ -> "FIND_DEPENDENT_FILES"
  | FIND_REFS                _ -> "FIND_REFS"
  | IDE_FIND_REFS            _ -> "IDE_FIND_REFS"
  | IDE_HIGHLIGHT_REFS       _ -> "IDE_HIGHLIGHT_REFS"
  | REFACTOR                 _ -> "REFACTOR"
  | IDE_REFACTOR             _ -> "IDE_REFACTOR"
  | DUMP_SYMBOL_INFO         _ -> "DUMP_SYMBOL_INFO"
  | DUMP_AI_INFO             _ -> "DUMP_AI_INFO"
  | REMOVE_DEAD_FIXMES       _ -> "REMOVE_DEAD_FIXMES"
  | SEARCH                   _ -> "SEARCH"
  | COVERAGE_COUNTS          _ -> "COVERAGE_COUNTS"
  | LINT                     _ -> "LINT"
  | LINT_STDIN               _ -> "LINT_STDIN"
  | LINT_ALL                 _ -> "LINT_ALL"
  | CREATE_CHECKPOINT        _ -> "CREATE_CHECKPOINT"
  | RETRIEVE_CHECKPOINT      _ -> "RETRIEVE_CHECKPOINT"
  | DELETE_CHECKPOINT        _ -> "DELETE_CHECKPOINT"
  | IN_MEMORY_DEP_TABLE_SIZE   -> "IN_MEMORY_DEP_TABLE_SIZE"
  | SAVE_STATE               _ -> "SAVE_STATE"
  | STATS                      -> "STATS"
  | FORMAT                   _ -> "FORMAT"
  | TRACE_AI                 _ -> "TRACE_AI"
  | AI_QUERY                 _ -> "AI_QUERY"
  | DUMP_FULL_FIDELITY_PARSE _ -> "DUMP_FULL_FIDELITY_PARSE"
  | OPEN_FILE                _ -> "OPEN_FILE"
  | CLOSE_FILE               _ -> "CLOSE_FILE"
  | EDIT_FILE                _ -> "EDIT_FILE"
  | IDE_AUTOCOMPLETE         _ -> "IDE_AUTOCOMPLETE"
  | IDE_FFP_AUTOCOMPLETE     _ -> "IDE_FFP_AUTOCOMPLETE"
  | DISCONNECT                 -> "DISCONNECT"
  | SUBSCRIBE_DIAGNOSTIC     _ -> "SUBSCRIBE_DIAGNOSTIC"
  | UNSUBSCRIBE_DIAGNOSTIC   _ -> "UNSUBSCRIBE_DIAGNOSTIC"
  | OUTLINE                  _ -> "OUTLINE"
  | IDE_IDLE                   -> "IDE_IDLE"
  | INFER_RETURN_TYPE        _ -> "INFER_RETURN_TYPE"
  | RAGE                       -> "RAGE"
  | DYNAMIC_VIEW             _ -> "DYNAMIC_VIEW"
  | CST_SEARCH               _ -> "CST_SEARCH"
  | NO_PRECHECKED_FILES        -> "NO_PRECHECKED_FILES"

let debug_describe_cmd : type a. a command -> string = function
  | Rpc rpc -> debug_describe_t rpc
  | Stream _ -> "Stream"
  | Debug -> "Debug"

let source_tree_of_file_input file_input =
  match file_input with
  | ServerCommandTypes.FileName filename ->
    Full_fidelity_source_text.from_file (Relative_path.create_detect_prefix filename)
  | ServerCommandTypes.FileContent content ->
    Full_fidelity_source_text.make Relative_path.default content
