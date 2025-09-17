(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type connection_type = Non_persistent

type connection_response = Connected

type status_liveness =
  | Stale_status
  | Live_status

module Server_status = struct
  type t = {
    liveness: status_liveness;
    error_list: Errors.finalized_error list;
    dropped_count: int;
    last_recheck_stats: Telemetry.t option;
    file_watcher_clock: ServerNotifierTypes.clock option;
  }
end

module Identify_symbol = struct
  type single_result =
    string SymbolOccurrence.t * string SymbolDefinition.t option

  type result = single_result list
end

module Method_jumps = struct
  type result = {
    orig_name: string;
    orig_pos: Pos.absolute;
    dest_name: string;
    dest_pos: Pos.absolute;
    orig_p_name: string;
    (* Used for methods to find their parent class *)
    dest_p_name: string;
  }

  type filter =
    | No_filter
    | Class
    | Interface
    | Trait
  [@@deriving show]

  let readable_place name pos p_name =
    let readable = Pos.string pos in
    if String.length p_name <> 0 then
      readable ^ " " ^ Utils.strip_ns p_name ^ "::" ^ Utils.strip_ns name
    else
      readable ^ " " ^ Utils.strip_ns name

  let print_readable res ~find_children =
    List.iter res ~f:(fun res ->
        let origin_readable =
          readable_place res.orig_name res.orig_pos res.orig_p_name
        in
        let dest_readable =
          readable_place res.dest_name res.dest_pos res.dest_p_name
        in
        let extended =
          "inherited "
          ^
          if find_children then
            "by"
          else
            "from"
        in
        print_endline
          (origin_readable ^ "\n    " ^ extended ^ " " ^ dest_readable));
    ()
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
  let rec call ~(f : unit -> 'a t Lwt.t) ~(depth : int) : _ Lwt.t =
    Lwt.Infix.(
      (if depth = 2 then
        Lwt.fail Two_retries_in_a_row
      else
        Lwt.return_unit)
      >>= fun () ->
      f () >>= fun result ->
      match result with
      | Done x -> Lwt.return x
      | Retry -> call ~f ~depth:(depth + 1))

  (* Call the function returning Done_or_retry.t with at most one retry, expecting
   * that this is enough to yield a non-Retry value, which is returned *)
  let call ~(f : unit -> 'a t Lwt.t) : _ Lwt.t = call ~f ~depth:0

  (* Helper function useful when mapping over results from functions that (in addition
   * to Done_or_retry.t result) thread through some kind of environment. *)
  let map_env ~(f : 'a -> 'b) ((env, x) : 'env * 'a t) : 'env * 'b t =
    match x with
    | Done x -> (env, Done (f x))
    | Retry -> (env, Retry)
end

module Find_refs = struct
  include SearchTypes.Find_refs

  type server_result = (string * Pos.t) list

  type result = (string * Pos.absolute) list

  type server_result_or_retry = server_result Done_or_retry.t

  type result_or_retry = result Done_or_retry.t
end

module Find_my_tests = struct
  type result_entry = {
    file_path: string;
    distance: int;
  }

  type result = (result_entry list, string) Result.t
end

module Rename = struct
  type ide_result = (ServerRenameTypes.patch list, string) result

  type result = ServerRenameTypes.patch list

  type ide_result_or_retry = ide_result Done_or_retry.t

  type result_or_retry = result Done_or_retry.t

  (** Returns a string in the form of
   * NEW_NAME|COMMA_SEPARATED_FIND_REFS_ACTION|OCaml_marshalled_SymbolDefinition.T
  *)
  let arguments_to_string_exn
      (new_name : string)
      (action : Find_refs.action)
      (symbol_def : Relative_path.t SymbolDefinition.t) : string =
    let symbol_and_action =
      FindRefsWireFormat.CliArgs.to_string
        {
          FindRefsWireFormat.CliArgs.symbol_name = new_name;
          action;
          stream_file = None;
          hint_suffixes = [];
        }
    in
    let marshalled_def = Marshal.to_string symbol_def [] in
    let encoded = Base64.encode_exn marshalled_def in
    Printf.sprintf "%s|%s" symbol_and_action encoded

  (** Expects a string in the form of
   * NEW_NAME|COMMA_SEPARATED_FIND_REFS_ACTION|OCaml_marshalled_SymbolDefinition.T
   * For example, a valid entry is
   * HackTypecheckerQueryBase::WWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir|<byte_string>
  *)
  let string_to_args arg :
      string * Find_refs.action * Relative_path.t SymbolDefinition.t =
    let split_arg = Str.split (Str.regexp "|") arg in
    let (symbol_name, action_arg, marshalled_def) =
      match split_arg with
      | [symbol_name; action_arg; marshalled_def] ->
        (symbol_name, action_arg, marshalled_def)
      | _ ->
        Printf.eprintf "Invalid input\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let str = Printf.sprintf "%s|%s" symbol_name action_arg in
    let { FindRefsWireFormat.CliArgs.symbol_name = new_name; action; _ } =
      FindRefsWireFormat.CliArgs.from_string_exn str
    in
    let decoded_str = Base64.decode_exn marshalled_def in
    let symbol_definition : Relative_path.t SymbolDefinition.t =
      Marshal.from_string decoded_str 0
    in
    (new_name, action, symbol_definition)
end

module Symbol_type = struct
  type t = {
    pos: string Pos.pos;
    type_: string;
    ident_: int;
  }
  [@@deriving show]
end

module Symbol_info_service = struct
  type target_type =
    | Function
    | Method
    | Constructor
  [@@deriving ord, show]

  type symbol_fun_call = {
    name: string;
    type_: target_type;
    pos: string Pos.pos;
    caller: string;
  }
  [@@deriving show]

  type result = {
    fun_calls: symbol_fun_call list;
    symbol_types: Symbol_type.t list;
  }

  let fun_call_to_json fun_call_results =
    let open Hh_json in
    List.map fun_call_results ~f:(fun item ->
        let item_type =
          match item.type_ with
          | Function -> "Function"
          | Method -> "Method"
          | Constructor -> "Constructor"
        in
        JSON_Object
          [
            ("name", JSON_String item.name);
            ("type", JSON_String item_type);
            ("pos", Pos.json item.pos);
            ("caller", JSON_String item.caller);
          ])

  let symbol_type_to_json symbol_type_results =
    let open Hh_json in
    Symbol_type.(
      List.rev_map symbol_type_results ~f:(fun item ->
          JSON_Object
            [
              ("pos", Pos.json item.pos);
              ("type", JSON_String item.type_);
              ("ident", int_ item.ident_);
            ]))

  let to_json result =
    let open Hh_json in
    let fun_call_json = fun_call_to_json result.fun_calls in
    let symbol_type_json = symbol_type_to_json result.symbol_types in
    JSON_Object
      [
        ("function_calls", JSON_Array fun_call_json);
        ("symbol_types", JSON_Array symbol_type_json);
      ]
end

module Outline = struct
  type outline = string SymbolDefinition.t list
end

module Infer_return_type = struct
  type t =
    | Function of string
    | Method of string * string

  type result = (string, string) Stdlib.result
end

module Ide_rename_type = struct
  type t = {
    filename: Relative_path.t;
    line: int;
    char: int;
    new_name: string;
  }
  [@@deriving show]
end

module Go_to_definition = struct
  type result = (string SymbolOccurrence.t * string SymbolDefinition.t) list
end

module Go_to_type_definition = struct
  type result = (Pos.absolute * string) list
end

module Tast_hole = struct
  type filter =
    | Typing
    | Cast
    | Any
  [@@deriving show]
end

type file_input =
  | FileName of string
  | FileContent of string
[@@deriving show]

type lint_stdin_input = {
  filename: string;
  contents: string;
}
[@@deriving show]

type cst_search_input = {
  sort_results: bool;
  input: Hh_json.json;
  files_to_search: string list option; (* if None, search all files *)
}
[@@deriving show]

(* The following datatypes can be interpreted as follows:
 * MESSAGE_TAG : Argument type (sent from client to server) -> return type t *)
type _ t =
  | STATUS : {
      max_errors: int option;
      error_filter: Filter_errors.Filter.t;
    }
      -> Server_status.t t
  | STATUS_SINGLE : {
      file_names: file_input list;
      max_errors: int option;
      error_filter: Filter_errors.Filter.t;
      return_expanded_tast: bool;
      preexisting_warnings: bool;
          (** Whether to show preexisting warnings
            (the ones that were there prior to saved state revision) *)
    }
      -> ((Errors.finalized_error list * int)
         * Tast.program Tast_with_dynamic.t Relative_path.Map.t option)
         t
  | INFER_TYPE :
      file_input * File_content.Position.t
      -> InferAtPosService.result t
  | INFER_TYPE_BATCH :
      (string * File_content.Position.t * File_content.Position.t option) list
      -> string list t
  | INFER_TYPE_ERROR : file_input * int * int -> InferErrorAtPosService.result t
  | IS_SUBTYPE : string -> (string, string) result t
  | TAST_HOLES : file_input * Tast_hole.filter -> TastHolesService.result t
  | TAST_HOLES_BATCH : string list -> TastHolesService.result t
  | IDENTIFY_SYMBOL : string -> string SymbolDefinition.t list t
  | IDENTIFY_FUNCTION :
      string * file_input * File_content.Position.t
      -> Identify_symbol.result t
  | METHOD_JUMP :
      (string * Method_jumps.filter * bool)
      -> Method_jumps.result list t
  | METHOD_JUMP_BATCH :
      (string list * Method_jumps.filter)
      -> Method_jumps.result list t
  | FIND_REFS : Find_refs.action -> Find_refs.result_or_retry t
  | GO_TO_IMPL : Find_refs.action -> Find_refs.result_or_retry t
  | IDE_FIND_REFS_BY_SYMBOL :
      FindRefsWireFormat.CliArgs.t
      -> Find_refs.result_or_retry t
  | IDE_GO_TO_IMPL_BY_SYMBOL :
      FindRefsWireFormat.CliArgs.t
      -> Find_refs.result_or_retry t
  | RENAME : ServerRenameTypes.action -> Rename.result_or_retry t
  | IDE_RENAME_BY_SYMBOL :
      Find_refs.action * string * Relative_path.t SymbolDefinition.t
      -> Rename.ide_result_or_retry t
  | DUMP_SYMBOL_INFO : string list -> Symbol_info_service.result t
  | REMOVE_DEAD_FIXMES :
      int list
      -> [ `Ok of ServerRenameTypes.patch list | `Error of string ] t
  | REMOVE_DEAD_UNSAFE_CASTS
      : [ `Ok of ServerRenameTypes.patch list | `Error of string ] t
  | REWRITE_LAMBDA_PARAMETERS : string list -> ServerRenameTypes.patch list t
  | IN_MEMORY_DEP_TABLE_SIZE : (int, string) Stdlib.result t
  | SAVE_NAMING :
      string
      -> (SaveStateServiceTypes.save_naming_result, string) Stdlib.result t
  | SAVE_STATE :
      (string * bool)
      -> (SaveStateServiceTypes.save_state_result, string) Stdlib.result t
  | CHECK_LIVENESS : unit t
  | LINT : string list -> ServerLintTypes.result t
  | LINT_STDIN : lint_stdin_input -> ServerLintTypes.result t
  | LINT_ALL : int -> ServerLintTypes.result t
  | STATS : Stats.t t
  | DUMP_FULL_FIDELITY_PARSE : string -> string t
  | RAGE : ServerRageTypes.result t
  | CST_SEARCH : cst_search_input -> (Hh_json.json, string) result t
  | NO_PRECHECKED_FILES : unit t
  | FUN_DEPS_BATCH : (string * int * int) list -> string list t
  | LIST_FILES_WITH_ERRORS : string list t
  | FILE_DEPENDENTS : string list -> string list t
  | VERBOSE : bool -> unit t
  | DEPS_OUT_BATCH : (string * int * int) list -> string list t
  | DEPS_IN_BATCH :
      (string * int * int) list
      -> Find_refs.result_or_retry list t
  | FIND_MY_TESTS : (int * string list) -> Find_my_tests.result t

type cmd_metadata = {
  from: string;
  (* a short human-readable string, used in "hh_server is busy [desc]" *)
  desc: string;
}

type 'a command = cmd_metadata * 'a t

and streamed =
  | SHOW of string
  | LIST_MODES

type errors = Errors.finalized_error list [@@deriving show]

let equal_errors errors1 errors2 =
  let errors1 = Errors.FinalizedErrorSet.of_list errors1 in
  let errors2 = Errors.FinalizedErrorSet.of_list errors2 in
  Errors.FinalizedErrorSet.equal errors1 errors2

type diagnostic_errors = errors SMap.t [@@deriving eq, show]

type 'a message_type =
  | Hello  (** Hello is the first message sent to the client by the server *)
  | Monitor_failed_to_handoff
      (** However, if the handoff failed, this will be sent instead of Hello, and the connection terminated. *)
  | Ping
      (** Server sometimes sends these, after Hello and before Response, to check if client fd is still open *)
  | Response of 'a * Connection_tracker.t
      (** Response message is the response to an RPC. The server will close fd after this. *)

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout

(** Invariant: The server_finale_file is created by Exit.exit and left there. *)
type server_specific_files = {
  server_finale_file: string;  (** just before exit, server will write here *)
}

(** Some client commands require full check to be run in order to update global
  state that they depend on *)
let rpc_command_needs_full_check : type a. a t -> bool =
 fun msg ->
  match msg with
  (* global error list is not updated during small checks *)
  | STATUS _ -> true
  | LIST_FILES_WITH_ERRORS
  | REMOVE_DEAD_FIXMES _
  | REMOVE_DEAD_UNSAFE_CASTS ->
    true (* need same information as STATUS *)
  | REWRITE_LAMBDA_PARAMETERS _ -> true
  (* Finding references/implementations uses global dependency table *)
  | FIND_REFS _ -> true
  | GO_TO_IMPL _ -> true
  | IDE_FIND_REFS_BY_SYMBOL _ -> true
  | IDE_GO_TO_IMPL_BY_SYMBOL _ -> true
  | METHOD_JUMP (_, _, find_children) -> find_children (* uses find refs *)
  | FIND_MY_TESTS _ -> true
  | SAVE_NAMING _ -> false
  | SAVE_STATE _ -> true
  (* Codebase-wide rename, uses find references *)
  | RENAME _ -> true
  | IDE_RENAME_BY_SYMBOL _ -> true
  (* Same case as Ai commands *)
  | IN_MEMORY_DEP_TABLE_SIZE -> true
  | NO_PRECHECKED_FILES -> true
  | STATS -> false
  | STATUS_SINGLE _ -> false
  | INFER_TYPE _ -> false
  | INFER_TYPE_BATCH _ -> false
  | INFER_TYPE_ERROR _ -> false
  | IS_SUBTYPE _ -> false
  | TAST_HOLES _ -> false
  | TAST_HOLES_BATCH _ -> false
  | IDENTIFY_FUNCTION _ -> false
  | IDENTIFY_SYMBOL _ -> false
  | METHOD_JUMP_BATCH _ -> false
  | DUMP_SYMBOL_INFO _ -> false
  | LINT _ -> false
  | LINT_STDIN _ -> false
  | LINT_ALL _ -> false
  | DUMP_FULL_FIDELITY_PARSE _ -> false
  | RAGE -> false
  | CST_SEARCH _ -> false
  | CHECK_LIVENESS -> false
  | FUN_DEPS_BATCH _ -> false
  | DEPS_OUT_BATCH _ -> false
  | FILE_DEPENDENTS _ -> true
  | VERBOSE _ -> false
  | DEPS_IN_BATCH _ -> true

let use_priority_pipe (command : 'result t) : bool =
  not (rpc_command_needs_full_check command)
