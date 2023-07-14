(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ide_api_types

type connection_type =
  | Persistent
  | Non_persistent

type connection_response = Connected

type status_liveness =
  | Stale_status
  | Live_status

module Server_status = struct
  type t = {
    liveness: status_liveness;
    has_unsaved_changes: bool;
    error_list: Errors.finalized_error list;
    dropped_count: int;
    last_recheck_stats: Telemetry.t option;
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
  type member =
    | Method of string
    | Property of string
    | Class_const of string
    | Typeconst of string
  [@@deriving show]

  type action =
    | Class of string
        (** When user does find-all-refs on Foo::f or self::f or parent::f, it produces `Class "Foo"`
        and passes this to serverFindRefs. As expected, it will find all references to the class Foo,
        including self:: and parent:: references.
        Class is used for all typelikes -- typedefs, enums, enum classes, classes, interfaces, traits *)
    | ExplicitClass of string
        (** When user does rename of Foo to Bar, then serverRename produces `ExplicitClass "Foo"`
        and passes this to serverFindRefs. This will only find the explicit references to the class Foo,
        not including self:: and parent::, since they shouldn't be renamed. *)
    | Member of string * member
    | Function of string
    | GConst of string
    | LocalVar of {
        filename: Relative_path.t;
        file_content: string;
        line: int;
        char: int;
      }
  [@@deriving show]

  type server_result = (string * Pos.t) list

  type result = (string * Pos.absolute) list

  type ide_result = (string * Pos.absolute list) option

  type server_result_or_retry = server_result Done_or_retry.t

  type result_or_retry = result Done_or_retry.t

  type ide_result_or_retry = ide_result Done_or_retry.t

  let dep_member_of member : Typing_deps.Dep.Member.t list =
    let open Typing_deps.Dep.Member in
    match member with
    | Method n -> [method_ n; smethod n]
    | Property n -> [prop n; sprop n]
    | Class_const n -> [const n]
    | Typeconst n -> [const n]

  let member_to_string member =
    match member with
    | Method s -> "Method " ^ s
    | Property s -> "Property " ^ s
    | Class_const s -> "Class_consts " ^ s
    | Typeconst s -> "Typeconst " ^ s

  (**
   * Output strings in the form of
   * SYMBOL_NAME|COMMA_SEPARATED_ACTION_STRING
   * For example,
   * HackTypecheckerQueryBase::getWWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir
   * Must be manually kept in sync with string_to_symbol_and_action_exn.
  *)
  let symbol_and_action_to_string_exn symbol_name action =
    let action_serialized =
      match action with
      | Class str -> Printf.sprintf "Class,%s" str
      | ExplicitClass str -> Printf.sprintf "ExplicitClass,%s" str
      | Function str -> Printf.sprintf "Function,%s" str
      | GConst str -> Printf.sprintf "GConst,%s" str
      | Member (str, Method s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Method" s
      | Member (str, Property s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Property" s
      | Member (str, Class_const s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Class_const" s
      | Member (str, Typeconst s) ->
        Printf.sprintf "Member,%s,%s,%s" str "Typeconst" s
      | LocalVar _ ->
        Printf.eprintf "Invalid action\n";
        raise Exit_status.(Exit_with Input_error)
    in
    Printf.sprintf "%s|%s" symbol_name action_serialized

  (**
   * Expects input strings in the form of
   * SYMBOL_NAME|COMMA_SEPARATED_ACTION_STRING
   * For example,
   * HackTypecheckerQueryBase::getWWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir
   * The implementation explicitly is tied to whatever is generated from symbol_and_action_to_string_exn
   * and should be kept in sync manually by anybody editing.
  *)
  let string_to_symbol_and_action_exn arg =
    let pair = Str.split (Str.regexp "|") arg in
    let (symbol_name, action_arg) =
      match pair with
      | [symbol_name; action_arg] -> (symbol_name, action_arg)
      | _ ->
        Printf.eprintf "Invalid input\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let action =
      (* Explicitly ignoring localvar support *)
      match Str.split (Str.regexp ",") action_arg with
      | ["Class"; str] -> Class str
      | ["ExplicitClass"; str] -> ExplicitClass str
      | ["Function"; str] -> Function str
      | ["GConst"; str] -> GConst str
      | ["Member"; str; "Method"; member_name] ->
        Member (str, Method member_name)
      | ["Member"; str; "Property"; member_name] ->
        Member (str, Property member_name)
      | ["Member"; str; "Class_const"; member_name] ->
        Member (str, Class_const member_name)
      | ["Member"; str; "Typeconst"; member_name] ->
        Member (str, Typeconst member_name)
      | _ ->
        Printf.eprintf "Invalid input for action, got %s\n" action_arg;
        raise Exit_status.(Exit_with Input_error)
    in
    (symbol_name, action)
end

module Rename = struct
  type ide_result = (ServerRenameTypes.patch list, string) result

  type result = ServerRenameTypes.patch list

  type ide_result_or_retry = ide_result Done_or_retry.t

  type result_or_retry = result Done_or_retry.t

  (** Returns a string in the form of
   * NEW_NAME|COMMA_SEPARATED_FIND_REFS_ACTION|FILENAME|OCaml_marshalled_SymbolDefinition.T
  *)
  let arguments_to_string_exn
      (new_name : string)
      (action : Find_refs.action)
      (filename : string)
      (symbol_def : Relative_path.t SymbolDefinition.t) : string =
    let symbol_and_action =
      Find_refs.symbol_and_action_to_string_exn new_name action
    in
    let marshalled_def = Marshal.to_string symbol_def [] in
    let encoded = Base64.encode_exn marshalled_def in
    Printf.sprintf "%s|%s|%s" symbol_and_action filename encoded

  (** Expects a string in the form of
   * NEW_NAME|COMMA_SEPARATED_FIND_REFS_ACTION|filename|OCaml_marshalled_SymbolDefinition.T
   * For example, a valid entry is
   * HackTypecheckerQueryBase::WWWDir|Member,\HackTypecheckerQueryBase,Method,getWWWDir|foo.php|<byte_string>
  *)
  let string_to_args arg :
      string * Find_refs.action * string * Relative_path.t SymbolDefinition.t =
    let split_arg = Str.split (Str.regexp "|") arg in
    let (symbol_name, action_arg, filename, marshalled_def) =
      match split_arg with
      | [symbol_name; action_arg; filename; marshalled_def] ->
        (symbol_name, action_arg, filename, marshalled_def)
      | _ ->
        Printf.eprintf "Invalid input\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let str = Printf.sprintf "%s|%s" symbol_name action_arg in
    let (new_name, action) = Find_refs.string_to_symbol_and_action_exn str in
    let decoded_str = Base64.decode_exn marshalled_def in
    let symbol_definition : Relative_path.t SymbolDefinition.t =
      Marshal.from_string decoded_str 0
    in
    (new_name, action, filename, symbol_definition)
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

module Extract_standalone = struct
  type target =
    | Function of string
    | Method of string * string
  [@@deriving show]
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

type labelled_file =
  | LabelledFileName of string
  | LabelledFileContent of {
      filename: string;
      content: string;
    }
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
      ignore_ide: bool;
      remote: bool;
      max_errors: int option;
    }
      -> Server_status.t t
  | STATUS_SINGLE : {
      file_names: file_input list;
      max_errors: int option;
    }
      -> (Errors.finalized_error list * int) t
  | INFER_TYPE : file_input * int * int -> InferAtPosService.result t
  | INFER_TYPE_BATCH :
      (string * int * int * (int * int) option) list
      -> string list t
  | INFER_TYPE_ERROR : file_input * int * int -> InferErrorAtPosService.result t
  | IS_SUBTYPE : string -> (string, string) result t
  | TAST_HOLES : file_input * Tast_hole.filter -> TastHolesService.result t
  | TAST_HOLES_BATCH : string list -> TastHolesService.result t
  | IDE_HOVER : string * int * int -> HoverService.result t
  | DOCBLOCK_AT :
      (string * int * int * string option * SearchUtils.si_kind)
      -> DocblockService.result t
  | DOCBLOCK_FOR_SYMBOL :
      (string * SearchUtils.si_kind)
      -> DocblockService.result t
  | IDE_SIGNATURE_HELP : (string * int * int) -> Lsp.SignatureHelp.result t
  | XHP_AUTOCOMPLETE_SNIPPET : string -> string option t
  | IDENTIFY_SYMBOL : string -> string SymbolDefinition.t list t
  | IDENTIFY_FUNCTION :
      string * file_input * int * int
      -> Identify_symbol.result t
  | METHOD_JUMP :
      (string * Method_jumps.filter * bool)
      -> Method_jumps.result list t
  | METHOD_JUMP_BATCH :
      (string list * Method_jumps.filter)
      -> Method_jumps.result list t
  | FIND_REFS : Find_refs.action -> Find_refs.result_or_retry t
  | GO_TO_IMPL : Find_refs.action -> Find_refs.result_or_retry t
  | IDE_FIND_REFS :
      labelled_file * int * int * bool
      -> Find_refs.ide_result_or_retry t
  | IDE_FIND_REFS_BY_SYMBOL :
      Find_refs.action * string
      -> Find_refs.ide_result_or_retry t
  | IDE_GO_TO_IMPL :
      labelled_file * int * int
      -> Find_refs.ide_result_or_retry t
  | IDE_GO_TO_IMPL_BY_SYMBOL :
      Find_refs.action * string
      -> Find_refs.ide_result_or_retry t
  | IDE_HIGHLIGHT_REFS :
      string * file_input * int * int
      -> ServerHighlightRefsTypes.result t
  | RENAME : ServerRenameTypes.action -> Rename.result_or_retry t
  | IDE_RENAME : Ide_rename_type.t -> Rename.ide_result_or_retry t
  | IDE_RENAME_BY_SYMBOL :
      Find_refs.action
      * string
      * Relative_path.t
      * Relative_path.t SymbolDefinition.t
      -> Rename.ide_result_or_retry t
  | CODEMOD_SDT :
      string
      -> (ServerRenameTypes.patch list
         * string list
         * [ `ClassLike | `Function ])
         t
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
  | SEARCH : string * string -> SearchUtils.result t
  | LINT : string list -> ServerLintTypes.result t
  | LINT_STDIN : lint_stdin_input -> ServerLintTypes.result t
  | LINT_ALL : int -> ServerLintTypes.result t
  | CREATE_CHECKPOINT : string -> unit t
  | RETRIEVE_CHECKPOINT : string -> string list option t
  | DELETE_CHECKPOINT : string -> bool t
  | STATS : Stats.t t
  | FORMAT : ServerFormatTypes.action -> ServerFormatTypes.result t
  | DUMP_FULL_FIDELITY_PARSE : string -> string t
  | OPEN_FILE : string * string -> unit t
  | CLOSE_FILE : string -> unit t
  | EDIT_FILE : string * text_edit list -> unit t
  | IDE_AUTOCOMPLETE :
      string * position * bool
      -> AutocompleteTypes.ide_result t
  | CODE_ACTION : {
      path: string;
      range: range;
    }
      -> Lsp.CodeAction.command_or_action list t
  | CODE_ACTION_RESOLVE : {
      path: string;
      range: range;
      resolve_title: string;
    }
      -> Lsp.CodeActionResolve.result t
  | DISCONNECT : unit t
  | OUTLINE : string -> Outline.outline t
  | IDE_IDLE : unit t
  | RAGE : ServerRageTypes.result t
  | CST_SEARCH : cst_search_input -> (Hh_json.json, string) result t
  | NO_PRECHECKED_FILES : unit t
  | POPULATE_REMOTE_DECLS : Relative_path.t list option -> unit t
  | FUN_DEPS_BATCH : (string * int * int) list -> string list t
  | LIST_FILES_WITH_ERRORS : string list t
  | FILE_DEPENDENTS : string list -> string list t
  | IDENTIFY_TYPES : labelled_file * int * int -> (Pos.absolute * string) list t
  | EXTRACT_STANDALONE : Extract_standalone.target -> string t
  | CONCATENATE_ALL : string list -> string t
  | GO_TO_DEFINITION : labelled_file * int * int -> Go_to_definition.result t
  | PREPARE_CALL_HIERARCHY :
      labelled_file * int * int
      -> Lsp.PrepareCallHierarchy.result t
  | CALL_HIERARCHY_INCOMING_CALLS :
      Lsp.CallHierarchyItem.t
      -> Lsp.CallHierarchyIncomingCalls.callHierarchyIncomingCall list
         Done_or_retry.t
         list
         t
  | CALL_HIERARCHY_OUTGOING_CALLS :
      Lsp.CallHierarchyItem.t
      -> Lsp.CallHierarchyOutgoingCalls.result t
  | PAUSE : bool -> unit t
  | VERBOSE : bool -> unit t
  | DEPS_OUT_BATCH : (string * int * int) list -> string list t
  | DEPS_IN_BATCH :
      (string * int * int) list
      -> Find_refs.result_or_retry list t

type cmd_metadata = {
  from: string;
  (* a short human-readable string, used in "hh_server is busy [desc]" *)
  desc: string;
}

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
  | _ -> false

let is_idle_rpc : type a. a t -> bool = function
  | IDE_IDLE -> true
  | _ -> false

type 'a command =
  | Rpc of cmd_metadata * 'a t
  | Debug_DO_NOT_USE
      (** this unused constructor is part of the binary protocol
      between client and server; removing it would alter the protocol. *)

and streamed =
  | SHOW of string
  | LIST_MODES

type errors = Errors.finalized_error list [@@deriving show]

let equal_errors errors1 errors2 =
  let errors1 = Errors.FinalizedErrorSet.of_list errors1 in
  let errors2 = Errors.FinalizedErrorSet.of_list errors2 in
  Errors.FinalizedErrorSet.equal errors1 errors2

type diagnostic_errors = errors SMap.t [@@deriving eq, show]

type push =
  | DIAGNOSTIC of {
      errors: diagnostic_errors;
      is_truncated: int option;
          (** Whether the list of errors has been truncated
              to preserve IDE perf. *)
    }
  | BUSY_STATUS of busy_status
  | NEW_CLIENT_CONNECTED
  | FATAL_EXCEPTION of (Marshal_tools.remote_exception_data[@opaque])
  | NONFATAL_EXCEPTION of (Marshal_tools.remote_exception_data[@opaque])

and busy_status =
  | Needs_local_typecheck
  | Doing_local_typecheck
  | Done_local_typecheck
  | Doing_global_typecheck of global_typecheck_kind
  | Done_global_typecheck

and global_typecheck_kind =
  | Blocking
  | Interruptible
  | Remote_blocking of string
[@@deriving eq, show]

type pushes = push list [@@deriving eq, show]

type 'a message_type =
  | Hello
      (** Hello is the first message sent to the client by the server, for both persistent and non-persistent *)
  | Monitor_failed_to_handoff
      (** However, if the handoff failed, this will be sent instead of Hello, and the connection terminated. *)
  | Ping
      (** Server sometimes sends these, after Hello and before Response, to check if client fd is still open *)
  | Response of 'a * Connection_tracker.t
      (** Response message is the response to an RPC. For non-persistent, the server will close fd after this. *)
  | Push of push
      (** This is how errors are sent; only sent to persistent connections. *)

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout

(** Invariant: The server_finale_file is created by Exit.exit and left there. *)
type server_specific_files = {
  server_finale_file: string;  (** just before exit, server will write here *)
}
