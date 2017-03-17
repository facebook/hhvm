open Ide_api_types

type connection_type =
  | Persistent
  | Non_persistent

type connection_response =
  | Connected

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
  | IDENTIFY_FUNCTION : ServerUtils.file_input * int * int ->
      IdentifySymbolService.result t
  | GET_DEFINITION_BY_ID : string -> string SymbolDefinition.t option t
  | METHOD_JUMP : (string * bool) -> MethodJumps.result list t
  | FIND_DEPENDENT_FILES: string list -> string list t
  | FIND_REFS : FindRefsService.action -> FindRefsService.result t
  | IDE_FIND_REFS : ServerUtils.file_input * int * int ->
      FindRefsService.ide_result t
  | IDE_HIGHLIGHT_REFS : ServerUtils.file_input * int * int ->
      ServerHighlightRefsTypes.result t
  | REFACTOR : ServerRefactorTypes.action -> ServerRefactorTypes.patch list t
  | DUMP_SYMBOL_INFO : string list -> SymbolInfoServiceTypes.result t
  | DUMP_AI_INFO : string list -> Ai.InfoService.result t
  | REMOVE_DEAD_FIXMES : int list -> ServerRefactorTypes.patch list t
  | IGNORE_FIXMES : string list -> Pos.absolute Errors.error_ list t
  | SEARCH : string * string -> HackSearchService.result t
  | COVERAGE_COUNTS : string -> ServerCoverageMetricTypes.result t
  | LINT : string list -> ServerLintTypes.result t
  | LINT_ALL : int -> ServerLintTypes.result t
  | CREATE_CHECKPOINT : string -> unit t
  | RETRIEVE_CHECKPOINT : string -> string list option t
  | DELETE_CHECKPOINT : string -> bool t
  | STATS : Stats.t t
  | KILL : unit t
  | FORMAT : ServerFormatTypes.action -> ServerFormatTypes.result t
  | IDE_FORMAT : ServerFormatTypes.ide_action -> ServerFormatTypes.ide_result t
  | TRACE_AI : Ai.TraceService.action -> string t
  | AI_QUERY : string -> string t
  | DUMP_FULL_FIDELITY_PARSE : string -> string t
  | OPEN_FILE : string * string -> unit t
  | CLOSE_FILE : string -> unit t
  | EDIT_FILE : string * (text_edit list) -> unit t
  | IDE_AUTOCOMPLETE : string * position -> AutocompleteService.result t
  | DISCONNECT : unit t
  | SUBSCRIBE_DIAGNOSTIC : int -> unit t
  | UNSUBSCRIBE_DIAGNOSTIC : int -> unit t
  | OUTLINE : string -> FileOutline.outline t

let is_disconnect_rpc : type a. a t -> bool = function
  | DISCONNECT -> true
  | _ -> false

let is_kill_rpc : type a. a t -> bool = function
  | KILL -> true
  | _ -> false

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
  | NEW_CLIENT_CONNECTED

type 'a persistent_connection_message_type =
  | Push of push
  | Response of 'a

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout
