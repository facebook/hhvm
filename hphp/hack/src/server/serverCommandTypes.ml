open Ide_api_types

type connection_type =
  | Persistent
  | Non_persistent

type connection_response =
  | Connected

type status_liveness =
  | Stale_status
  | Live_status

module Server_status = struct
  type t = {
    liveness : status_liveness;
    has_unsaved_changes : bool;
    error_list : Pos.absolute Errors.error_ list;
  }
end

module Identify_symbol = struct
  type single_result = (string SymbolOccurrence.t) * (string SymbolDefinition.t option)
  type result = single_result list
end

module Method_jumps = struct
  type result = {
    orig_name: string;
    orig_pos: Pos.absolute;
    dest_name: string;
    dest_pos: Pos.absolute;
    orig_p_name: string; (* Used for methods to find their parent class *)
    dest_p_name: string;
  }

  type filter =
    | No_filter
    | Class
    | Interface
    | Trait
end

module Done_or_retry = struct
  (* This is an ugly hack to support following case:
   * - client issues a command that requires full recheck. Server knows not to
   *   accept it until full check is completed .
   * - while processing the command, it turns out that we need to recheck even
   *   more files for full check to be "good enough" for this command
   * This can happen when combining prechecked files and find references.
   * We could store the client with its command somwhere, and come back to it
   * after this additional rechecking is done, but in practice this is ugly to
   * implement in current atchitecture. What is easier to do instead, is to
   * return a special "Retry" response to the client, which will cause it to
   * re-issue the same request (which will once again not be accepted until full
   * check is completed. Since command is the same, its second execution should
   * not add even more files to recheck, hence a limit of only two attempts.
   *
   * In other words: the goal of this is to avoid implementing even more
   * server-side client queing mechanisms, but re-use an existing "clients waiting
   * for full check" queue. *)
  exception Two_retries_in_a_row

  type 'a t =
    | Done of 'a
    | Retry

  (* Note: this is designed to work with calls that always will succeed on second try
   * (the reason for retrying is a one time event that is resolved during first call).
   * If this ends up throwing, it's a bug in hh_server. *)
  let rec call ~(f:unit -> 'a t) ~(depth:int) : 'a =
    if depth = 2 then raise Two_retries_in_a_row;
    match f () with
    | Done x -> x
    | Retry -> call ~f ~depth:(depth+1)

  (* Call the function returning Done_or_retry.t with at most one retry, expecting
   * that this is enough to yield a non-Retry value, which is returned *)
  let call ~(f:unit -> 'a t) : 'a = call ~f ~depth:0

  (* Helper function useful when mapping over results from functions that (in addition
   * to Done_or_retry.t result) thread through some kind of environment. *)
  let map_env
    ~(f:'a -> 'b)
    ((env, x) : 'env * 'a t)
  : 'env * 'b t =
    match x with
    | Done x -> env, Done (f x)
    | Retry -> env, Retry
end

module Find_refs = struct
  type member = Ai.ServerFindRefs.member =
    | Method of string
    | Property of string
    | Class_const of string
    | Typeconst of string

  type action = Ai.ServerFindRefs.action =
    | Class of string
    | Member of string * member
    | Function of string
    | GConst of string
    | LocalVar of { filename: Relative_path.t; file_content: string; line: int; char: int }

    type server_result = (string * Pos.t) list
    type result = (string * Pos.absolute) list
    type ide_result = (string * Pos.absolute list) option

    type server_result_or_retry = server_result Done_or_retry.t
    type result_or_retry = result Done_or_retry.t
    type ide_result_or_retry = ide_result Done_or_retry.t
end

module Refactor = struct
  type ide_result = ((ServerRefactorTypes.patch list), string) result
  type result = ServerRefactorTypes.patch list

  type ide_result_or_retry = ide_result Done_or_retry.t
  type result_or_retry = result Done_or_retry.t
end

module Symbol_type = struct
  type t = {
    pos: string Pos.pos;
    type_: string;
    ident_: int;
  }
end

module Symbol_info_service = struct
  type target_type =
    | Function
    | Method
    | Constructor

  type symbol_fun_call = {
    name:  string;
    type_: target_type;
    pos: string Pos.pos;
    caller: string;
  }

  type result = {
    fun_calls: symbol_fun_call list;
    symbol_types: Symbol_type.t list;
  }
end

module Outline = struct
  type outline = string SymbolDefinition.t list
end

module Infer_return_type = struct
  type t =
  | Function of string
  | Method of string * string

  type result = (string, string) Pervasives.result
end

module Ide_refactor_type = struct
  type t = {
    filename: string;
    line: int;
    char: int;
    new_name: string;
  }
end

type file_input =
  | FileName of string
  | FileContent of string

type labelled_file =
  | LabelledFileName of string
  | LabelledFileContent of { filename: string; content: string }

type lint_stdin_input = { filename: string; contents: string }

type cst_search_input = {
  sort_results: bool;
  input: Hh_json.json;
  files_to_search: string list option; (* if None, search all files *)
}

(* The following datatypes can be interpreted as follows:
 * MESSAGE_TAG : Argument type (sent from client to server) -> return type t *)
type _ t =
  | STATUS : bool -> Server_status.t t
  | STATUS_SINGLE : file_input -> Pos.absolute Errors.error_ list t
  | INFER_TYPE : file_input * int * int * bool ->
      InferAtPosService.result t
  | INFER_TYPE_BATCH : (string * int * int * (int * int) option) list * bool -> string list t
  | TYPED_AST : string -> string t
  | IDE_HOVER : file_input * int * int ->
      HoverService.result t
  | DOCBLOCK_AT : (string * int * int * string option) -> DocblockService.result t
  | IDE_SIGNATURE_HELP : (file_input * int * int) -> Lsp.SignatureHelp.result t
  | COVERAGE_LEVELS : file_input -> Coverage_level.result t
  | AUTOCOMPLETE : string -> AutocompleteTypes.result t
  | IDENTIFY_FUNCTION : file_input * int * int ->
      Identify_symbol.result t
  | METHOD_JUMP : (string * Method_jumps.filter * bool) ->
      Method_jumps.result list t
  | METHOD_JUMP_BATCH : (string list * Method_jumps.filter) ->
      Method_jumps.result list t
  | FIND_DEPENDENT_FILES: string list -> string list t
  | FIND_REFS : Find_refs.action -> Find_refs.result_or_retry t
  | IDE_FIND_REFS : labelled_file * int * int * bool ->
      Find_refs.ide_result_or_retry t
  | IDE_HIGHLIGHT_REFS : file_input * int * int ->
      ServerHighlightRefsTypes.result t
  | REFACTOR : ServerRefactorTypes.action -> Refactor.result_or_retry t
  | IDE_REFACTOR : Ide_refactor_type.t -> Refactor.ide_result_or_retry t
  | DUMP_SYMBOL_INFO : string list -> Symbol_info_service.result t
  | DUMP_AI_INFO : string list -> Ai.InfoService.result t
  | REMOVE_DEAD_FIXMES : int list -> [`Ok of ServerRefactorTypes.patch list | `Error of string] t
  | IN_MEMORY_DEP_TABLE_SIZE : ((int, string) Pervasives.result) t
  | SAVE_STATE : string -> ((int, string) Pervasives.result) t
  | SEARCH : string * string -> HackSearchService.result t
  | COVERAGE_COUNTS : string -> ServerCoverageMetricTypes.result t
  | LINT : string list -> ServerLintTypes.result t
  | LINT_STDIN : lint_stdin_input -> ServerLintTypes.result t
  | LINT_ALL : int -> ServerLintTypes.result t
  | CREATE_CHECKPOINT : string -> unit t
  | RETRIEVE_CHECKPOINT : string -> string list option t
  | DELETE_CHECKPOINT : string -> bool t
  | STATS : Stats.t t
  | FORMAT : ServerFormatTypes.action -> ServerFormatTypes.result t
  | TRACE_AI : Ai.TraceService.action -> string t
  | AI_QUERY : string -> string t
  | DUMP_FULL_FIDELITY_PARSE : string -> string t
  | OPEN_FILE : string * string -> unit t
  | CLOSE_FILE : string -> unit t
  | EDIT_FILE : string * (text_edit list) -> unit t
  | IDE_AUTOCOMPLETE : string * position * bool * bool -> AutocompleteTypes.ide_result t
  | IDE_FFP_AUTOCOMPLETE : string * position -> AutocompleteTypes.ide_result t
  | DISCONNECT : unit t
  | SUBSCRIBE_DIAGNOSTIC : int -> unit t
  | UNSUBSCRIBE_DIAGNOSTIC : int -> unit t
  | OUTLINE : string -> Outline.outline t
  | IDE_IDLE : unit t
  | INFER_RETURN_TYPE : Infer_return_type.t ->
      Infer_return_type.result t
  | RAGE : ServerRageTypes.result t
  | DYNAMIC_VIEW: bool -> unit t
  | CST_SEARCH: cst_search_input -> (Hh_json.json, string) result t
  | NO_PRECHECKED_FILES: unit t

let is_disconnect_rpc : type a. a t -> bool = function
  | DISCONNECT -> true
  | _ -> false


let is_critical_rpc : type a. a t -> bool = function
  (* An exception during any critical rpc should shutdown the persistent connection. *)
  (* The critical ones are those that affect the state.                              *)
  | DISCONNECT -> true
  | CREATE_CHECKPOINT _ -> true
  | DELETE_CHECKPOINT _ -> true
  | OPEN_FILE _ -> true
  | CLOSE_FILE _ -> true
  | EDIT_FILE _ -> true
  | SUBSCRIBE_DIAGNOSTIC _ -> true
  | UNSUBSCRIBE_DIAGNOSTIC _ -> true
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
  | BUSY_STATUS of busy_status
  | NEW_CLIENT_CONNECTED
  | FATAL_EXCEPTION of Marshal_tools.remote_exception_data
  | NONFATAL_EXCEPTION of Marshal_tools.remote_exception_data

and busy_status =
  | Needs_local_typecheck
  | Doing_local_typecheck
  | Done_local_typecheck
  | Doing_global_typecheck of bool (* interruptible? *)
  | Done_global_typecheck of {is_truncated: bool; shown: int; total: int;}

type 'a persistent_connection_message_type =
  | Push of push
  | Response of 'a * float (* records the time at which hh_server started handling *)
  | Hello
  (* Hello is the first message sent after handoff. It's used for both *)
  (* persistent and non-persistent connections. It's included in this  *)
  (* type, though, because ocaml typing forces a single type to come   *)
  (* Marshal.from_fd_with_preamble.                                    *)
  | Ping
  (* Pings can be sent to non-persistent connection after Hello and before
   * sending RPC response. *)

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout
