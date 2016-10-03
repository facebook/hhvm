open File_content

type connection_type =
  | Persistent
  | Non_persistent

type connection_response =
  | Persistent_client_connected
  | Persistent_client_alredy_exists

type status_liveness =
  | Stale_status
  | Live_status

(* The following datatypes can be interpreted as follows:
 * MESSAGE_TAG : Argument type (sent from client to server) -> return type t *)
type _ t =
  | STATUS : (status_liveness * Pos.absolute Errors.error_ list) t
  | INFER_TYPE : ServerUtils.file_input * int * int ->
      InferAtPosService.result t
  | COVERAGE_LEVELS : ServerUtils.file_input -> Coverage_level.result t
  | AUTOCOMPLETE : string -> AutocompleteService.result t
  | IDENTIFY_FUNCTION : string * int * int -> IdentifySymbolService.result t
  | OUTLINE : string -> FileOutline.outline t
  | GET_DEFINITION_BY_ID : string -> string SymbolDefinition.t option t
  | METHOD_JUMP : (string * bool) -> MethodJumps.result list t
  | FIND_DEPENDENT_FILES: string list -> string list t
  | FIND_REFS : FindRefsService.action -> FindRefsService.result t
  | IDE_FIND_REFS : string * int * int -> FindRefsService.result t
  | IDE_HIGHLIGHT_REFS : string * int * int -> ServerHighlightRefsTypes.result t
  | REFACTOR : ServerRefactorTypes.action -> ServerRefactorTypes.patch list t
  | DUMP_SYMBOL_INFO : string list -> SymbolInfoServiceTypes.result t
  | DUMP_AI_INFO : string list -> Ai.InfoService.result t
  | REMOVE_DEAD_FIXMES : int list -> ServerRefactorTypes.patch list t
  | ARGUMENT_INFO : string * int * int -> ServerArgumentInfo.result t
  | SEARCH : string * string -> HackSearchService.result t
  | COVERAGE_COUNTS : string -> ServerCoverageMetricTypes.result t
  | LINT : string list -> ServerLintTypes.result t
  | LINT_ALL : int -> ServerLintTypes.result t
  | CREATE_CHECKPOINT : string -> unit t
  | RETRIEVE_CHECKPOINT : string -> string list option t
  | DELETE_CHECKPOINT : string -> bool t
  | STATS : Stats.t t
  | KILL : unit t
  | FIND_LVAR_REFS : string * int * int -> ServerFindLocals.result t
  | FORMAT : string * int * int -> string Format_hack.return t
  | TRACE_AI : Ai.TraceService.action -> string t
  | AI_QUERY : string -> string t
  | DUMP_FULL_FIDELITY_PARSE : string -> string t
  | ECHO_FOR_TEST : string -> string t
  | OPEN_FILE : string -> unit t
  | CLOSE_FILE : string -> unit t
  | EDIT_FILE : string * (code_edit list) -> unit t
  | IDE_AUTOCOMPLETE : string * content_pos -> AutocompleteService.result t
  | IDE_HIGHLIGHT_REF : string * content_pos ->
      ServerHighlightRefsTypes.result t
  | IDE_IDENTIFY_FUNCTION : string * content_pos ->
      IdentifySymbolService.result t
  | DISCONNECT : unit t
  | SUBSCRIBE_DIAGNOSTIC : int -> unit t
  | UNSUBSCRIBE_DIAGNOSTIC : int -> unit t

type 'a command =
  | Rpc of 'a t
  | Stream of streamed
  | Debug

and streamed =
  | SHOW of string
  | LIST_FILES
  | LIST_MODES
  | BUILD of ServerBuild.build_opts

type push =
  | DIAGNOSTIC of int * (Pos.absolute Errors.error_ list) SMap.t

type 'a persistent_connection_message_type =
  | Push of push
  | Response of 'a

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout
