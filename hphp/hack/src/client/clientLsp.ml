(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hh_core
open Lsp
open Lsp_fmt

(* All hack-specific code relating to LSP goes in here. *)

(* The environment for hh_client with LSP *)
type env = {
  from: string; (* The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  use_ffp_autocomplete: bool; (* Flag to turn on the (experimental) FFP based autocomplete *)
}


(************************************************************************)
(** Protocol orchestration & helpers                                   **)
(************************************************************************)

type server_conn = {
  ic: Timeout.in_channel;
  oc: out_channel;

  (* Pending messages sent from the server. They need to be relayed to the
     client. *)
  pending_messages: ServerCommandTypes.push Queue.t;
}

module Main_env = struct
  type t = {
    conn: server_conn;
    needs_idle: bool;
    uris_with_diagnostics: SSet.t;
    uris_with_unsaved_changes: SSet.t; (* see comment in get_uris_with_unsaved_changes *)
    dialog: ShowMessageRequest.t; (* "hack server is now ready" *)
    progress: Progress.t; (* "typechecking..." *)
    actionRequired: ActionRequired.t; (* "save any file to trigger a global recheck" *)
  }
end

module In_init_env = struct
  type t = {
    conn: server_conn;
    start_time: float;
    file_edits: Hh_json.json ImmQueue.t;
    uris_with_unsaved_changes: SSet.t; (* see comment in get_uris_with_unsaved_changes *)
    tail_env: Tail.env;
    has_reported_progress: bool;
    dialog: ShowMessageRequest.t; (* "hack server is busy" *)
    progress: Progress.t; (* "hh_server is initializing [naming]" *)
  }
end

module Lost_env = struct
  type t = {
    p: params;
    uris_with_unsaved_changes: SSet.t; (* see comment in get_uris_with_unsaved_changes *)
    lock_file: string;
    dialog: ShowMessageRequest.t; (* "hh_server stopped" *)
    actionRequired: ActionRequired.t; (* "hh_server stopped" *)
    progress: Progress.t; (* "hh_server monitor is waiting for a rebase to settle" *)
  }

  and how_to_explain_loss_to_user =
    | Action_required of string (* explain via dialog and actionRequired *)
    | Wait_required of string (* explain via progress *)

  and params = {
    explanation: how_to_explain_loss_to_user;
    start_on_click: bool; (* if user clicks Restart, do we ClientStart before reconnecting? *)
    trigger_on_lsp: bool; (* reconnect if we receive any LSP request/notification *)
    trigger_on_lock_file: bool; (* reconnect if lockfile is created *)
  }
end

type state =
  (* Pre_init: we haven't yet received the initialize request.           *)
  | Pre_init
  (* In_init: we did respond to the initialize request, and now we're    *)
  (* waiting for a "Hello" from the server. When that comes we'll        *)
  (* request a permanent connection from the server, and process the     *)
  (* file_changes backlog, and switch to Main_loop.                      *)
  | In_init of In_init_env.t
  (* Main_loop: we have a working connection to both server and client.  *)
  | Main_loop of Main_env.t
  (* Lost_server: someone stole the persistent connection from us.       *)
  (* We might choose to grab it back if prompted...                      *)
  | Lost_server of Lost_env.t
  (* Post_shutdown: we received a shutdown request from the client, and  *)
  (* therefore shut down our connection to the server. We can't handle   *)
  (* any more requests from the client and will close as soon as it      *)
  (* notifies us that we can exit.                                       *)
  | Post_shutdown

let initialize_params: Hh_json.json option ref = ref None
let hhconfig_version: string ref = ref "[NotYetInitialized]"

module Jsonrpc = Jsonrpc.Make (struct type t = state end)
module Lsp_helpers = Lsp_helpers.Make (struct type t = state let get () = !initialize_params end)

let can_autostart_after_mismatch: bool ref = ref true

type event =
  | Server_hello
  | Server_message of ServerCommandTypes.push
  | Client_message of Jsonrpc.message
  | Tick (* once per second, on idle *)

(* Here are some exit points. *)
let exit_ok () = exit 0
let exit_fail () = exit 1

(* The following connection exceptions inform the main LSP event loop how to  *)
(* respond to an exception: was the exception a connection-related exception  *)
(* (one of these) or did it arise during other logic (not one of these)? Can  *)
(* we report the exception to the LSP client? Can we continue handling        *)
(* further LSP messages or must we quit? If we quit, can we do so immediately *)
(* or must we delay?  --  Separately, they also help us marshal callstacks    *)
(* across daemon- and process-boundaries.                                     *)
exception Client_fatal_connection_exception of Marshal_tools.remote_exception_data
exception Client_recoverable_connection_exception of Marshal_tools.remote_exception_data
exception Server_fatal_connection_exception of Marshal_tools.remote_exception_data


let event_to_string (event: event) : string =
  let open Jsonrpc in
  match event with
  | Server_hello -> "Server hello"
  | Server_message ServerCommandTypes.DIAGNOSTIC _ -> "Server DIAGNOSTIC"
  | Server_message ServerCommandTypes.BUSY_STATUS _ -> "Server BUSY_STATUS"
  | Server_message ServerCommandTypes.NEW_CLIENT_CONNECTED -> "Server NEW_CLIENT_CONNECTED"
  | Server_message ServerCommandTypes.FATAL_EXCEPTION _ -> "Server FATAL_EXCEPTION"
  | Client_message c -> Printf.sprintf "Client %s %s" (kind_to_string c.kind) c.method_
  | Tick -> "Tick"


let state_to_string (state: state) : string =
  match state with
  | Pre_init -> "Pre_init"
  | In_init _ienv -> "In_init"
  | Main_loop _menv -> "Main_loop"
  | Lost_server _lenv -> "Lost_server"
  | Post_shutdown -> "Post_shutdown"


let get_root () : Path.t option =
  let path_opt = Lsp_helpers.get_root () in
  if Option.is_none path_opt then
    None (* None for us means "haven't yet received initialize so we don't know" *)
  else
    Some (ClientArgsUtils.get_root path_opt) (* None for ClientArgsUtils means "." *)


let read_hhconfig_version () : string =
  match get_root () with
  | None ->
    "[NoRoot]"
  | Some root ->
    let file = Filename.concat (Path.to_string root) ".hhconfig" in
    try
      let contents = Sys_utils.cat file in
      let config = Config_file.parse_contents contents in
      let version = SMap.get "version" config in
      Option.value version ~default:"[NoVersion]"
    with e ->
      Printf.sprintf "[NoHhconfig:%s]" (Printexc.to_string e)


(* get_uris_with_unsaved_changes is the set of files for which we've          *)
(* received didChange but haven't yet received didSave/didOpen. It is purely  *)
(* a description of what we've heard of the editor, and is independent of     *)
(* whether or not they've yet been synced with hh_server.                     *)
(* As it happens: in Main_loop state all these files will already have been   *)
(* sent to hh_server; in In_init state all these files will have been queued  *)
(* up inside file_edits ready to be sent when we receive the hello; in        *)
(* Lost_server state they're not even queued up, and if ever we see hh_server *)
(* ready then we'll terminate the LSP server and trust the client to relaunch *)
(* us and resend a load of didOpen/didChange events.                          *)
let get_uris_with_unsaved_changes (state: state): SSet.t =
  match state with
  | Main_loop menv -> menv.Main_env.uris_with_unsaved_changes
  | In_init ienv -> ienv.In_init_env.uris_with_unsaved_changes
  | Lost_server lenv -> lenv.Lost_env.uris_with_unsaved_changes
  | _ -> SSet.empty


let rpc
    (server_conn: server_conn)
    (command: 'a ServerCommandTypes.t)
  : 'a =
  try
    let res, pending_messages =
      ServerCommand.rpc_persistent (server_conn.ic, server_conn.oc) command in
    List.iter pending_messages
      ~f:(fun x -> Queue.push x server_conn.pending_messages);
    res
  with
  | ServerCommand.Remote_exception remote_e_data ->
    raise (Server_fatal_connection_exception remote_e_data)
  | e ->
    let message = Printexc.to_string e in
    let stack = Printexc.get_backtrace () in
    raise (Server_fatal_connection_exception { Marshal_tools.message; stack; })


(* Determine whether to read a message from the client (the editor) or the
   server (hh_server), or whether neither is ready within 1s. *)
let get_message_source
    (server: server_conn)
    (client: Jsonrpc.queue)
    : [> `From_server | `From_client | `No_source ] =
  (* Take action on server messages in preference to client messages, because
     server messages are very easy and quick to service (just send a message to
     the client), while client messages require us to launch a potentially
     long-running RPC command. *)
  let has_server_messages = not (Queue.is_empty server.pending_messages) in
  if has_server_messages then `From_server else
  if Jsonrpc.has_message client then `From_client else

  (* If no immediate messages are available, then wait up to 1 second. *)
  let server_read_fd = Unix.descr_of_out_channel server.oc in
  let client_read_fd = Jsonrpc.get_read_fd client in
  let readable, _, _ = Unix.select [server_read_fd; client_read_fd] [] [] 1.0 in
  if readable = [] then `No_source
  else if List.mem readable server_read_fd then `From_server
  else `From_client


(* A simplified version of get_message_source which only looks at client *)
let get_client_message_source
    (client: Jsonrpc.queue)
  : [> `From_client | `No_source ] =
  if Jsonrpc.has_message client then `From_client else
  let client_read_fd = Jsonrpc.get_read_fd client in
  let readable, _, _ = Unix.select [client_read_fd] [] [] 1.0 in
  if readable = [] then `No_source
  else `From_client


(*  Read a message unmarshaled from the server's out_channel. *)
let read_message_from_server (server: server_conn) : event =
  let open ServerCommandTypes in
  try
    let fd = Unix.descr_of_out_channel server.oc in
    match Marshal_tools.from_fd_with_preamble fd with
    | Response _ ->
      failwith "unexpected response without request"
    | Push m -> Server_message m
    | Hello -> Server_hello
  with e ->
    let message = Printexc.to_string e in
    let stack = Printexc.get_backtrace () in
    raise (Server_fatal_connection_exception { Marshal_tools.message; stack; })

(* get_next_event: picks up the next available message from either client or
   server. The way it's implemented, at the first character of a message
   from either client or server, we block until that message is completely
   received. Note: if server is None (meaning we haven't yet established
   connection with server) then we'll just block waiting for client. *)
let get_next_event (state: state) (client: Jsonrpc.queue) : event =
  let from_server (server: server_conn) =
    if Queue.is_empty server.pending_messages
    then read_message_from_server server
    else Server_message (Queue.take server.pending_messages)
  in

  let from_client (client: Jsonrpc.queue) =
    match Jsonrpc.get_message client with
    | `Message message -> Client_message message
    | `Fatal_exception edata -> raise (Client_fatal_connection_exception edata)
    | `Recoverable_exception edata -> raise (Client_recoverable_connection_exception edata)
  in

  match state with
  | Main_loop { Main_env.conn; _ } | In_init { In_init_env.conn; _ } -> begin
      match get_message_source conn client with
      | `From_client -> from_client client
      | `From_server -> from_server conn
      | `No_source -> Tick
    end
  | _ -> begin
      match get_client_message_source client with
      | `From_client -> from_client client
      | `No_source -> Tick
    end


(* respond_to_error: if we threw an exception during the handling of a request,
   report the exception to the client as the response to their request. *)
let respond_to_error (event: event option) (e: exn) (stack: string): unit =
  match event with
  | Some (Client_message c)
    when c.Jsonrpc.kind = Jsonrpc.Request ->
    print_error e stack |> Jsonrpc.respond c
  | _ ->
    let (code, message, _original_data) = get_error_info e in
    Lsp_helpers.telemetry_error (Printf.sprintf "%s [%i]\n%s" message code stack)


(* dismiss_ui: dismisses all dialogs, progress- and action-required           *)
(* indicators and diagnostics in a state.                                     *)
let dismiss_ui (state: state) : state =
  match state with
  | In_init ienv ->
    let open In_init_env in
    In_init { ienv with
      dialog = Lsp_helpers.dismiss_showMessageRequest ienv.dialog;
      progress = Lsp_helpers.notify_progress ienv.progress None;
    }
  | Main_loop menv ->
    let open Main_env in
    Main_loop { menv with
      uris_with_diagnostics = Lsp_helpers.dismiss_diagnostics menv.uris_with_diagnostics;
      dialog = Lsp_helpers.dismiss_showMessageRequest menv.dialog;
      progress = Lsp_helpers.notify_progress menv.progress None;
      actionRequired = Lsp_helpers.notify_actionRequired menv.actionRequired None;
    }
  | Lost_server lenv ->
    let open Lost_env in
    Lost_server { lenv with
      dialog = Lsp_helpers.dismiss_showMessageRequest lenv.dialog;
      actionRequired = Lsp_helpers.notify_actionRequired lenv.actionRequired None;
      progress = Lsp_helpers.notify_progress lenv.progress None;
    }
  | Pre_init -> Pre_init
  | Post_shutdown -> Post_shutdown


(************************************************************************)
(** Conversions - ad-hoc ones written as needed them, not systematic   **)
(************************************************************************)

let lsp_uri_to_path = Lsp_helpers.lsp_uri_to_path
let path_to_lsp_uri = Lsp_helpers.path_to_lsp_uri

let lsp_position_to_ide (position: Lsp.position) : Ide_api_types.position =
  { Ide_api_types.
    line = position.line + 1;
    column = position.character + 1;
  }

let lsp_file_position_to_hack (params: Lsp.TextDocumentPositionParams.t)
  : string * int * int =
  let open Lsp.TextDocumentPositionParams in
  let {Ide_api_types.line; column;} = lsp_position_to_ide params.position in
  let filename = Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument
  in
  (filename, line, column)

let hack_pos_to_lsp_range (pos: 'a Pos.pos) : Lsp.range =
  let line1, col1, line2, col2 = Pos.destruct_range pos in
  {
    start = {line = line1 - 1; character = col1 - 1;};
    end_ = {line = line2 - 1; character = col2 - 1;};
  }

let hack_pos_to_lsp_location (pos: string Pos.pos) ~(default_path: string): Lsp.Location.t =
  let open Lsp.Location in
  {
    uri = path_to_lsp_uri (Pos.filename pos) ~default_path;
    range = hack_pos_to_lsp_range pos;
  }

let ide_range_to_lsp (range: Ide_api_types.range) : Lsp.range =
  { Lsp.
    start = { Lsp.
              line = range.Ide_api_types.st.Ide_api_types.line - 1;
              character = range.Ide_api_types.st.Ide_api_types.column - 1;
            };
    end_ = { Lsp.
             line = range.Ide_api_types.ed.Ide_api_types.line - 1;
             character = range.Ide_api_types.ed.Ide_api_types.column - 1;
           };
  }

let lsp_range_to_ide (range: Lsp.range) : Ide_api_types.range =
  let open Ide_api_types in
  {
    st = lsp_position_to_ide range.start;
    ed = lsp_position_to_ide range.end_;
  }

let hack_symbol_definition_to_lsp_location
    (symbol: string SymbolDefinition.t)
    ~(default_path: string)
  : Lsp.Location.t =
  let open SymbolDefinition in
  hack_pos_to_lsp_location symbol.pos ~default_path

let hack_errors_to_lsp_diagnostic
    (filename: string)
    (errors: Pos.absolute Errors.error_ list)
  : PublishDiagnostics.params =
  let open Lsp.Location in
  let location_message (error: Pos.absolute * string) : (Lsp.Location.t * string) =
    let (pos, message) = error in
    let {uri; range;} = hack_pos_to_lsp_location pos ~default_path:filename in
    ({Location.uri; range;}, message)
  in
  let hack_error_to_lsp_diagnostic (error: Pos.absolute Errors.error_) =
    let all_messages = Errors.to_list error |> List.map ~f:location_message in
    let (first_message, additional_messages) = match all_messages with
      | hd :: tl -> (hd, tl)
      | [] -> failwith "Expected at least one error in the error list"
    in
    let ({range; _}, message) = first_message in
    let relatedLocations = additional_messages |> List.map ~f:(fun (location, message) ->
      { PublishDiagnostics.
        relatedLocation = location;
        relatedMessage = message;
      }) in
    { Lsp.PublishDiagnostics.
      range;
      severity = Some PublishDiagnostics.Error;
      code = Some (Errors.get_code error);
      source = Some "Hack";
      message;
      relatedLocations;
    }
  in
  (* The caller is required to give us a non-empty filename. If it is empty,  *)
  (* the following path_to_lsp_uri will fall back to the default path - which *)
  (* is also empty - and throw, logging appropriate telemetry.                *)
  { Lsp.PublishDiagnostics.
    uri = path_to_lsp_uri filename ~default_path:"";
    diagnostics = List.map errors ~f:hack_error_to_lsp_diagnostic;
  }


(************************************************************************)
(** Protocol                                                           **)
(************************************************************************)

let do_shutdown (state: state) : state =
  let state = dismiss_ui state in
  begin match state with
  | Main_loop menv ->
    (* In Main_loop state, we're expected to unsubscribe diagnostics and tell *)
    (* server to disconnect so it can revert the state of its unsaved files.  *)
    let open Main_env in
    rpc menv.conn (ServerCommandTypes.UNSUBSCRIBE_DIAGNOSTIC 0);
    rpc menv.conn (ServerCommandTypes.DISCONNECT)
  | In_init _ienv ->
    (* In In_init state, even though we have a 'conn', it's still waiting for *)
    (* the server to become responsive, so there's no use sending any rpc     *)
    (* messages to the server over it.                                        *)
    ()
  | _ ->
    (* No other states have a 'conn' to send any disconnect messages over.    *)
    ()
  end;
  Post_shutdown


let do_rage (state: state) : Rage.result =
  let open Rage in
  let stateItem: rageItem = {
    title = None;
    data = "LSP adapter state: " ^ (state_to_string state);
  } in
  let hack_to_lsp (item: ServerRageTypes.rageItem) : Lsp.Rage.rageItem =
    {
      title = item.ServerRageTypes.title;
      data = item.ServerRageTypes.data;
    } in
  match state with
  | Main_loop menv ->
    let open Main_env in
    let items = rpc menv.conn ServerCommandTypes.RAGE in
    stateItem :: (List.map items ~f:hack_to_lsp)
  | _ ->
    [stateItem]


let do_didOpen (conn: server_conn) (params: DidOpen.params) : unit =
  let open DidOpen in
  let open TextDocumentItem in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let text = params.textDocument.text in
  let command = ServerCommandTypes.OPEN_FILE (filename, text) in
  rpc conn command;
  ()

let do_didClose (conn: server_conn) (params: DidClose.params) : unit =
  let open DidClose in
  let open TextDocumentIdentifier in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let command = ServerCommandTypes.CLOSE_FILE filename in
  rpc conn command;
  ()

let do_didChange
    (conn: server_conn)
    (params: DidChange.params)
  : unit =
  let open VersionedTextDocumentIdentifier in
  let open Lsp.DidChange in
  let lsp_change_to_ide (lsp: DidChange.textDocumentContentChangeEvent)
    : Ide_api_types.text_edit =
    { Ide_api_types.
      range = Option.map lsp.range lsp_range_to_ide;
      text = lsp.text;
    }
  in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let changes = List.map params.contentChanges ~f:lsp_change_to_ide in
  let command = ServerCommandTypes.EDIT_FILE (filename, changes) in
  rpc conn command;
  ()

let do_hover (conn: server_conn) (params: Hover.params) : Hover.result =
  (* TODO: should return MarkedCode, once Nuclide supports it *)
  (* TODO: should return doc-comment as well *)
  (* TODO: should return signature of what we hovered on, not just type. *)
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.INFER_TYPE (ServerUtils.FileName file, line, column) in
  let inferred_type = rpc conn command in
  match inferred_type with
  (* Hack server uses both None and "_" to indicate absence of a result. *)
  (* We're also catching the non-result "" just in case...               *)
  | None
  | Some ("_", _)
  | Some ("", _) -> { Hover.contents = []; range = None; }
  | Some (s, _) -> { Hover.contents = [MarkedString s]; range = None; }

let do_definition (conn: server_conn) (params: Definition.params)
  : Definition.result =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDENTIFY_FUNCTION (ServerUtils.FileName file, line, column) in
  let results = rpc conn command in
  (* What's it like when we return multiple definitions? For instance, if you ask *)
  (* for the definition of "new C()" then we've now got the definition of the     *)
  (* class "\C" and also of the constructor "\\C::__construct". I think that      *)
  (* users would be happier to only have the definition of the constructor, so    *)
  (* as to jump straight to it without the fuss of clicking to select which one.  *)
  (* That indeed is what Typescript does -- it only gives the constructor.        *)
  (* (VSCode displays multiple definitions with a peek view of them all;          *)
  (*  Atom displays them with a small popup showing just file+line of each).      *)
  (* There's one subtlety. If you declare a base class "B" with a constructor,    *)
  (* and a derived class "C" without a constructor, and click on "new C()", then  *)
  (* both Hack and Typescript will take you to the constructor of B. As desired!  *)
  (* Conclusion: given a class+method, we'll return only the method.              *)
  let result_is_method (result: IdentifySymbolService.single_result): bool =
    match result with
    | { SymbolOccurrence.type_ = SymbolOccurrence.Method _; _ }, _ -> true
    | _ -> false in
  let result_is_class (result: IdentifySymbolService.single_result): bool =
    match result with
    | { SymbolOccurrence.type_ = SymbolOccurrence.Class; _ }, _ -> true
    | _ -> false in
  let has_class = List.exists results ~f:result_is_class in
  let has_method = List.exists results ~f:result_is_method in
  let filtered_results = if has_class && has_method then
    List.filter results ~f:result_is_method
  else
    results
  in
  let rec hack_to_lsp = function
    | [] -> []
    | (_occurrence, None) :: l -> hack_to_lsp l
    | (_occurrence, Some definition) :: l ->
      (hack_symbol_definition_to_lsp_location definition ~default_path:file) :: (hack_to_lsp l)
  in
  hack_to_lsp filtered_results

let make_ide_completion_response (result:AutocompleteTypes.ide_result) =
  let open AutocompleteTypes in
  let open Completion in
  (* We use snippets to provide parentheses+arguments when autocompleting     *)
  (* method calls e.g. "$c->|" ==> "$c->foo($arg1)". But we'll only do this   *)
  (* there's nothing after the caret: no "$c->|(1)" -> "$c->foo($arg1)(1)"    *)
  let is_caret_followed_by_lparen = result.char_at_pos = '(' in

  let rec hack_completion_to_lsp (completion: complete_autocomplete_result)
    : Completion.completionItem =
    let (insertText, insertTextFormat) = hack_to_insert completion in
    {
      label = completion.res_name ^ (if completion.res_kind = Namespace_kind then "\\" else "");
      kind = hack_to_kind completion;
      detail = Some (hack_to_detail completion);
      inlineDetail = Some (hack_to_inline_detail completion);
      itemType = hack_to_itemType completion;
      documentation = None; (* TODO: provide doc-comments *)
      sortText = None;
      filterText = None;
      insertText = Some insertText;
      insertTextFormat = insertTextFormat;
      textEdits = [];
      command = None;
      data = None;
    }
  and hack_to_kind (completion: complete_autocomplete_result)
    : Completion.completionItemKind option =
    match completion.res_kind with
    | Abstract_class_kind
    | Class_kind -> Some Completion.Class
    | Method_kind -> Some Completion.Method
    | Function_kind -> Some Completion.Function
    | Variable_kind -> Some Completion.Variable
    | Property_kind -> Some Completion.Property
    | Class_constant_kind -> Some Completion.Value (* a bit off, but the best we can do *)
    | Interface_kind
    | Trait_kind -> Some Completion.Interface
    | Enum_kind -> Some Completion.Enum
    | Namespace_kind -> Some Completion.Module
    | Constructor_kind -> Some Completion.Constructor
    | Keyword_kind -> Some Completion.Keyword
  and hack_to_itemType (completion: complete_autocomplete_result) : string option =
    (* TODO: we're using itemType (left column) for function return types, and *)
    (* the inlineDetail (right column) for variable/field types. Is that good? *)
    Option.map completion.func_details ~f:(fun details -> details.return_ty)
  and hack_to_detail (completion: complete_autocomplete_result) : string =
    (* TODO: retrieve the actual signature including name+modifiers     *)
    (* For now we just return the type of the completion. In the case   *)
    (* of functions, their function-types have parentheses around them  *)
    (* which we want to strip. In other cases like tuples, no strip.    *)
    match completion.func_details with
    | None -> completion.res_ty
    | Some _ -> String_utils.rstrip (String_utils.lstrip completion.res_ty "(") ")"
  and hack_to_inline_detail (completion: complete_autocomplete_result) : string =
    match completion.func_details with
    | None -> hack_to_detail completion
    | Some details ->
      (* "(type1 $param1, ...)" *)
      let f param = Printf.sprintf "%s %s" param.param_ty param.param_name in
      let params = String.concat ", " (List.map details.params ~f) in
      Printf.sprintf "(%s)" params
  and hack_to_insert (completion: complete_autocomplete_result) : (string * insertTextFormat) =
    match completion.func_details with
    | Some details when Lsp_helpers.supports_snippets () && not is_caret_followed_by_lparen ->
      (* "method(${1:arg1}, ...)" but for args we just use param names. *)
      let f i param = Printf.sprintf "${%i:%s}" (i + 1) param.param_name in
      let params = String.concat ", " (List.mapi details.params ~f) in
      (Printf.sprintf "%s(%s)" completion.res_name params, SnippetFormat)
    | _ ->
      (completion.res_name, PlainText)
  in
  {
    isIncomplete = not result.is_complete;
    items = List.map result.completions ~f:hack_completion_to_lsp;
  }

let do_completion_ffp (conn: server_conn) (params: Completion.params) : Completion.result =
  let open TextDocumentIdentifier in
  let pos = lsp_position_to_ide params.TextDocumentPositionParams.position in
  let filename = lsp_uri_to_path params.TextDocumentPositionParams.textDocument.uri in
  let command = ServerCommandTypes.IDE_FFP_AUTOCOMPLETE (filename, pos) in
  let result = rpc conn command in
  make_ide_completion_response result

let do_completion_legacy (conn: server_conn) (params: Completion.params)
  : Completion.result =
  let open TextDocumentIdentifier in
  let pos = lsp_position_to_ide params.TextDocumentPositionParams.position in
  let filename = lsp_uri_to_path params.TextDocumentPositionParams.textDocument.uri in
  let delimit_on_namespaces = true in
  let command = ServerCommandTypes.IDE_AUTOCOMPLETE (filename, pos, delimit_on_namespaces) in
  let result = rpc conn command in
  make_ide_completion_response result

let do_workspaceSymbol
    (conn: server_conn)
    (params: WorkspaceSymbol.params)
  : WorkspaceSymbol.result =
  let open WorkspaceSymbol in
  let open SearchUtils in

  let query = params.query in
  let query_type = "" in
  let command = ServerCommandTypes.SEARCH (query, query_type) in
  let results = rpc conn command in

  let hack_to_lsp_kind = function
    | HackSearchService.Class (Some Ast.Cabstract) -> SymbolInformation.Class
    | HackSearchService.Class (Some Ast.Cnormal) -> SymbolInformation.Class
    | HackSearchService.Class (Some Ast.Cinterface) -> SymbolInformation.Interface
    | HackSearchService.Class (Some Ast.Ctrait) -> SymbolInformation.Interface
    (* LSP doesn't have traits, so we approximate with interface *)
    | HackSearchService.Class (Some Ast.Cenum) -> SymbolInformation.Enum
    | HackSearchService.Class (None) -> assert false (* should never happen *)
    | HackSearchService.Method _ -> SymbolInformation.Method
    | HackSearchService.ClassVar _ -> SymbolInformation.Property
    | HackSearchService.Function -> SymbolInformation.Function
    | HackSearchService.Typedef -> SymbolInformation.Class
    (* LSP doesn't have typedef, so we approximate with class *)
    | HackSearchService.Constant -> SymbolInformation.Constant
  in
  let hack_to_lsp_container = function
    | HackSearchService.Method (_, scope) -> Some scope
    | HackSearchService.ClassVar (_, scope) -> Some scope
    | _ -> None
  in
  (* Hack sometimes gives us back items with an empty path, by which it       *)
  (* intends "whichever path you asked me about". That would be meaningless   *)
  (* here. If it does, then it'll pick up our default path (also empty),      *)
  (* which will throw and go into our telemetry. That's the best we can do.   *)
  let hack_symbol_to_lsp (symbol: HackSearchService.symbol) =
    { SymbolInformation.
      name = (Utils.strip_ns symbol.name);
      kind = hack_to_lsp_kind symbol.result_type;
      location = hack_pos_to_lsp_location symbol.pos ~default_path:"";
      containerName = hack_to_lsp_container symbol.result_type;
    }
  in
  List.map results ~f:hack_symbol_to_lsp

let do_documentSymbol
    (conn: server_conn)
    (params: DocumentSymbol.params)
  : DocumentSymbol.result =
  let open DocumentSymbol in
  let open TextDocumentIdentifier in
  let open SymbolDefinition in

  let filename = lsp_uri_to_path params.textDocument.uri in
  let command = ServerCommandTypes.OUTLINE filename in
  let results = rpc conn command in

  let hack_to_lsp_kind = function
    | SymbolDefinition.Function -> SymbolInformation.Function
    | SymbolDefinition.Class -> SymbolInformation.Class
    | SymbolDefinition.Method -> SymbolInformation.Method
    | SymbolDefinition.Property -> SymbolInformation.Property
    | SymbolDefinition.Const -> SymbolInformation.Constant
    | SymbolDefinition.Enum -> SymbolInformation.Enum
    | SymbolDefinition.Interface -> SymbolInformation.Interface
    | SymbolDefinition.Trait -> SymbolInformation.Interface
    (* LSP doesn't have traits, so we approximate with interface *)
    | SymbolDefinition.LocalVar -> SymbolInformation.Variable
    | SymbolDefinition.Typeconst -> SymbolInformation.Class
    (* e.g. "const type Ta = string;" -- absent from LSP *)
    | SymbolDefinition.Typedef -> SymbolInformation.Class
    (* e.g. top level type alias -- absent from LSP *)
    | SymbolDefinition.Param -> SymbolInformation.Variable
    (* We never return a param from a document-symbol-search *)
  in
  let hack_symbol_to_lsp definition containerName =
    { SymbolInformation.
      name = definition.name;
      kind = hack_to_lsp_kind definition.kind;
      location = hack_symbol_definition_to_lsp_location definition ~default_path:filename;
      containerName;
    }
  in
  let rec hack_symbol_tree_to_lsp ~accu ~container_name = function
    (* Flattens the recursive list of symbols *)
    | [] -> List.rev accu
    | def :: defs ->
      let children = Option.value def.children ~default:[] in
      let accu = (hack_symbol_to_lsp def container_name) :: accu in
      let accu = hack_symbol_tree_to_lsp accu (Some def.name) children in
      hack_symbol_tree_to_lsp accu container_name defs
  in
  hack_symbol_tree_to_lsp ~accu:[] ~container_name:None results

let do_findReferences
    (conn: server_conn)
    (params: FindReferences.params)
  : FindReferences.result =
  let open FindReferences in

  let {Ide_api_types.line; column;} = lsp_position_to_ide params.position in
  let filename = Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument in
  let include_defs = params.context.includeDeclaration in
  let command = ServerCommandTypes.IDE_FIND_REFS
      (ServerUtils.FileName filename, line, column, include_defs) in
  let results = rpc conn command in
  (* TODO: respect params.context.include_declaration *)
  match results with
  | None -> []
  | Some (_name, positions) ->
    List.map positions ~f:(hack_pos_to_lsp_location ~default_path:filename)


let do_documentHighlights
    (conn: server_conn)
    (params: DocumentHighlights.params)
  : DocumentHighlights.result =
  let open DocumentHighlights in

  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDE_HIGHLIGHT_REFS (ServerUtils.FileName file, line, column) in
  let results = rpc conn command in

  let hack_range_to_lsp_highlight range =
    {
      range = ide_range_to_lsp range;
      kind = None;
    }
  in
  List.map results ~f:hack_range_to_lsp_highlight


let do_typeCoverage (conn: server_conn) (params: TypeCoverage.params)
  : TypeCoverage.result =
  let open TypeCoverage in

  let filename = Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument in
  let command = ServerCommandTypes.COVERAGE_LEVELS (ServerUtils.FileName filename) in
  let results: Coverage_level.result = rpc conn command in
  let results = Coverage_level.merge_adjacent_results results in

  (* We want to get a percentage-covered number. We could do that with an *)
  (* additional server round trip to ServerCommandTypes.COVERAGE_COUNTS. *)
  (* But to avoid that, we'll instead use this rough approximation: *)
  (* Count how many checked/unchecked/partial "regions" there are, where *)
  (* a "region" is like results_merged, but counting each line separately. *)
  let count_region (nchecked, nunchecked, npartial) (pos, level) =
    let nlines = (Pos.end_line pos) - (Pos.line pos) + 1 in
    match level with
    | Ide_api_types.Checked -> (nchecked + nlines, nunchecked, npartial)
    | Ide_api_types.Unchecked -> (nchecked, nunchecked + nlines, npartial)
    | Ide_api_types.Partial -> (nchecked, nunchecked, npartial + nlines)
  in
  let (nchecked, nunchecked, npartial) =
    List.fold results ~init:(0,0,0) ~f:count_region in

  let ntotal = nchecked + nunchecked + npartial in
  let coveredPercent = if ntotal = 0 then 100
    else ((nchecked * 100) + (npartial * 50)) / ntotal in

  let hack_coverage_to_lsp (pos, level) =
    let range = hack_pos_to_lsp_range pos in
    match level with
    | Ide_api_types.Checked -> None
    | Ide_api_types.Unchecked -> Some
        { range;
          message = "Un-type checked code. Consider adding type annotations.";
        }
    | Ide_api_types.Partial -> Some
        { range;
          message = "Partially type checked code. Consider adding type annotations.";
        }
  in
  {
    coveredPercent;
    uncoveredRanges = List.filter_map results ~f:hack_coverage_to_lsp;
  }


let do_formatting_common
    (conn: server_conn)
    (args: ServerFormatTypes.ide_action)
  : TextEdit.t list =
  let open ServerFormatTypes in
  let command = ServerCommandTypes.IDE_FORMAT args in
  let response: ServerFormatTypes.ide_result = rpc conn command in
  match response with
  | Error message ->
    raise (Error.InternalError message)
  | Ok r ->
    let range = ide_range_to_lsp r.range in
    let newText = r.new_text in
    [{TextEdit.range; newText;}]


let do_documentRangeFormatting
    (conn: server_conn)
    (params: DocumentRangeFormatting.params)
  : DocumentRangeFormatting.result =
  let open DocumentRangeFormatting in
  let open TextDocumentIdentifier in
  let action = ServerFormatTypes.Range
      { Ide_api_types.
        range_filename = lsp_uri_to_path params.textDocument.uri;
        file_range = lsp_range_to_ide params.range;
      }
  in
  do_formatting_common conn action


let do_documentOnTypeFormatting
    (conn: server_conn)
    (params: DocumentOnTypeFormatting.params)
  : DocumentOnTypeFormatting.result =
  let open DocumentOnTypeFormatting in
  let open TextDocumentIdentifier in
  let action = ServerFormatTypes.Position
      { Ide_api_types.
        filename = lsp_uri_to_path params.textDocument.uri;
        position = lsp_position_to_ide params.position;
      } in
  do_formatting_common conn action


let do_documentFormatting
    (conn: server_conn)
    (params: DocumentFormatting.params)
  : DocumentFormatting.result =
  let open DocumentFormatting in
  let open TextDocumentIdentifier in
  let action = ServerFormatTypes.Document (lsp_uri_to_path params.textDocument.uri) in
  do_formatting_common conn action


(* do_server_busy: controls the progress / action-required indicator          *)
let do_server_busy (state: state) (status: ServerCommandTypes.busy_status) : state =
  let open ServerCommandTypes in
  let open Main_env in
  let (progress, action) = match status with
    | Needs_local_typecheck -> (Some "Hack: preparing to check edits", None)
    | Doing_local_typecheck -> (Some "Hack: checking edits", None)
    | Done_local_typecheck -> (None, Some "Hack: save any file to do a whole-program check")
    | Doing_global_typecheck -> (Some "Hack: checking entire project", None)
    | Done_global_typecheck -> (None, None)
  in
  (* Following code is subtle. Thanks to the magic of the notify_ functions,  *)
  (* it will either create a new progress/action notification, or update an   *)
  (* an existing one, or close an existing one, or just no-op, as appropriate *)
  match state with
  | Main_loop menv ->
    Main_loop { menv with
      progress = Lsp_helpers.notify_progress menv.progress progress;
      actionRequired = Lsp_helpers.notify_actionRequired menv.actionRequired action;
    }
  | _ ->
    state


(* do_diagnostics: sends notifications for all reported diagnostics; also     *)
(* returns an updated "files_with_diagnostics" set of all files for which     *)
(* our client currently has non-empty diagnostic reports.                     *)
let do_diagnostics
    (uris_with_diagnostics: SSet.t)
    (file_reports: Pos.absolute Errors.error_ list SMap.t)
  : SSet.t =
  (* Hack sometimes reports a diagnostic on an empty file when it can't       *)
  (* figure out which file to report. In this case we'll report on the root.  *)
  (* Nuclide and VSCode both display this fine, though they obviously don't   *)
  (* let you click-to-go-to-file on it.                                       *)
  let default_path = match get_root () with
    | None -> failwith "expected root"
    | Some root -> Path.to_string root in
  let file_reports = match SMap.get "" file_reports with
    | None -> file_reports
    | Some errors -> SMap.remove "" file_reports |> SMap.add ~combine:(@) default_path errors
  in

  let per_file file errors =
    hack_errors_to_lsp_diagnostic file errors
    |> print_diagnostics
    |> Jsonrpc.notify "textDocument/publishDiagnostics"
  in
  SMap.iter per_file file_reports;

  let is_error_free _uri errors = List.is_empty errors in
  (* reports_without/reports_with are maps of filename->ErrorList. *)
  let (reports_without, reports_with) = SMap.partition is_error_free file_reports in
  (* files_without/files_with are sets of filenames *)
  let files_without = SMap.bindings reports_without |> List.map ~f:fst in
  let files_with = SMap.bindings reports_with |> List.map ~f:fst in
  (* uris_without/uris_with are sets of uris *)
  let uris_without = List.map files_without ~f:(path_to_lsp_uri ~default_path) |> SSet.of_list in
  let uris_with = List.map files_with ~f:(path_to_lsp_uri ~default_path) |> SSet.of_list
  in
  (* this is "(uris_with_diagnostics \ uris_without) U uris_with" *)
  SSet.union (SSet.diff uris_with_diagnostics uris_without) uris_with


let report_connect_start
    (ienv: In_init_env.t)
  : state =
  let open In_init_env in
  assert (not ienv.has_reported_progress);
  assert (ienv.dialog = ShowMessageRequest.None);
  assert (ienv.progress = Progress.None);
  (* Our goal behind progress reporting is to let the user know when things   *)
  (* won't be instantaneous, and to show that things are working as expected. *)
  (* Upon connection, if it connects immediately (before we've had 1s idle)   *)
  (* then nothing will have been displayed. Otherwise, at that first 1s idle, *)
  (* which is implemented here, we put up a progress indicator and a dialog   *)
  (* saying "initializing..."... When it's done, if it took too long, then in *)
  (* report_progress_end we put up a "ready" dialog.                          *)

  (* dialog... *)
  let handle_result ~result:_ state = match state with
    | In_init ienv -> In_init {ienv with In_init_env.dialog = ShowMessageRequest.None}
    | _ -> state in
  let handle_error ~code:_ ~message:_ ~data:_ state = handle_result "" state in
  let dialog = Lsp_helpers.request_showMessage handle_result handle_error
    MessageType.InfoMessage "Waiting for hh_server to be ready..." [] in

  (* progress indicator... *)
  let progress = Lsp_helpers.notify_progress Progress.None (Some "hh_server initializing") in

  In_init { ienv with has_reported_progress = true; dialog; progress; }


let report_connect_progress
    (ienv: In_init_env.t)
  : state =
  let open In_init_env in
  assert ienv.has_reported_progress;
  let time = Unix.time () in
  let delay_in_secs = int_of_float (time -. ienv.start_time) in
  (* TODO: better to report time that hh_server has spent initializing *)
  let load_state_not_found, tail_msg =
    ClientConnect.open_and_get_tail_msg ienv.start_time ienv.tail_env in
  let msg = if load_state_not_found then
    Printf.sprintf
      "hh_server initializing (load-state not found - will take a while): %s [%i seconds]"
      tail_msg delay_in_secs
  else
    Printf.sprintf
      "hh_server initializing: %s [%i seconds]"
      tail_msg delay_in_secs
  in
  In_init { ienv with
    progress = Lsp_helpers.notify_progress ienv.progress (Some msg);
  }


let report_connect_end
    (ienv: In_init_env.t)
  : state =
  let open In_init_env in
  let _state = dismiss_ui (In_init ienv) in
  let menv =
    { Main_env.
      conn = ienv.In_init_env.conn;
      needs_idle = true;
      uris_with_diagnostics = SSet.empty;
      uris_with_unsaved_changes = ienv.In_init_env.uris_with_unsaved_changes;
      dialog = ShowMessageRequest.None;
      progress = Progress.None;
      actionRequired = ActionRequired.None;
    }
  in
  (* alert the user that hack is ready, either by console log or by dialog *)
  let time = Unix.time () in
  let seconds = int_of_float (time -. ienv.start_time) in
  let msg = Printf.sprintf "hh_server is now ready, after %i seconds." seconds in
  if (time -. ienv.start_time > 30.0) then
    let handle_result ~result:_ state = match state with
      | Main_loop menv -> Main_loop {menv with Main_env.dialog = ShowMessageRequest.None}
      | _ -> state in
    let handle_error ~code:_ ~message:_ ~data:_ state = handle_result "" state in
    let dialog = Lsp_helpers.request_showMessage handle_result handle_error
      MessageType.InfoMessage msg [] in
    Main_loop {menv with Main_env.dialog;}
  else
    Main_loop menv


(* After the server has sent 'hello', it means the persistent connection is   *)
(* ready, so we can send our backlog of file-edits to the server.             *)
let connect_after_hello
    (server_conn: server_conn)
    (file_edits: Hh_json.json ImmQueue.t)
  : unit =
  let open Marshal_tools in
  begin try
      let oc = server_conn.oc in
      ServerCommand.send_connection_type oc ServerCommandTypes.Persistent;
      let fd = Unix.descr_of_out_channel oc in
      let response = Marshal_tools.from_fd_with_preamble fd in
      if response <> ServerCommandTypes.Connected then
        failwith "Didn't get server Connected response";

      let handle_file_edit (json: Hh_json.json) =
        let open Jsonrpc in
        let c = Jsonrpc.parse_message ~json ~timestamp:0.0 in
        match c.method_ with
        | "textDocument/didOpen" -> parse_didOpen c.params |> do_didOpen server_conn
        | "textDocument/didChange" -> parse_didChange c.params |> do_didChange server_conn
        | "textDocument/didClose" -> parse_didClose c.params |> do_didClose server_conn
        | _ -> failwith "should only buffer up didOpen/didChange/didClose"
      in
      ImmQueue.iter file_edits ~f:handle_file_edit;
    with e ->
      let message = Printexc.to_string e in
      let stack = Printexc.get_backtrace () in
      raise (Server_fatal_connection_exception { message; stack; })
  end;

  rpc server_conn (ServerCommandTypes.SUBSCRIBE_DIAGNOSTIC 0)


let rec connect_client
    (root: Path.t)
    ~(autostart: bool)
  : server_conn =
  let open Exit_status in
  (* This basically does the same connection attempt as "hh_client check":  *)
  (* it makes repeated attempts to connect; it prints useful messages to    *)
  (* stderr; in case of failure it will raise an exception. Below we're     *)
  (* catching the main exceptions so we can give a good user-facing error   *)
  (* text. For other exceptions, they'll end up showing to the user just    *)
  (* "internal error" with the error code.                                  *)
  let env_connect =
    { ClientConnect.
      root;
      autostart;
      force_dormant_start = false;
      retries = Some 3; (* each retry takes up to 1 second *)
      expiry = None; (* we can limit retries by time as well as by count *)
      no_load = false; (* only relevant when autostart=true *)
      profile_log = false; (* irrelevant *)
      ai_mode = None; (* only relevant when autostart=true *)
      progress_callback = ClientConnect.null_progress_reporter; (* we're fast! *)
      do_post_handoff_handshake = false;
    } in
  try
    let (ic, oc) = ClientConnect.connect env_connect in
    can_autostart_after_mismatch := false;
    let pending_messages = Queue.create () in
    { ic; oc; pending_messages; }
  with
  | Exit_with Build_id_mismatch when !can_autostart_after_mismatch ->
    (* Raised when the server was running an old version. We'll retry once.   *)
    can_autostart_after_mismatch := false;
    connect_client root ~autostart:true


let do_initialize () : Initialize.result =
  let open Initialize in
  let local_config = ServerLocalConfig.load ~silent:true in
  {
    server_capabilities = {
      textDocumentSync = {
        want_openClose = true;
        want_change = IncrementalSync;
        want_willSave = false;
        want_willSaveWaitUntil = false;
        want_didSave = Some { includeText = false }
      };
      hoverProvider = true;
      completionProvider = Some {
        resolveProvider = false;
        completion_triggerCharacters = ["$"; ">"; "\\"; ":"];
      };
      signatureHelpProvider = None;
      definitionProvider = true;
      referencesProvider = true;
      documentHighlightProvider = true;
      documentSymbolProvider = true;
      workspaceSymbolProvider = true;
      codeActionProvider = false;
      codeLensProvider = None;
      documentFormattingProvider = true;
      documentRangeFormattingProvider = true;
      documentOnTypeFormattingProvider =
        Option.some_if local_config.ServerLocalConfig.use_hackfmt
          {
            firstTriggerCharacter = ";";
            moreTriggerCharacter = ["}"];
          };
      renameProvider = false;
      documentLinkProvider = None;
      executeCommandProvider = None;
      typeCoverageProvider = true;
      rageProvider = true;
    }
  }


let start_server (root: Path.t) : unit =
  (* This basically does "hh_client start": a single attempt to open the     *)
  (* socket, send+read version and compare for mismatch, send handoff and    *)
  (* read response. It will print information to stderr. If the server is in *)
  (* an unresponsive or invalid state then it will kill the server. Next if  *)
  (* necessary it tries to spawn the server and wait until the monitor is    *)
  (* responsive enough to print "ready". It will do a hard program exit if   *)
  (* there were spawn problems.                                              *)
  let env_start =
    { ClientStart.
      root;
      no_load = false;
      profile_log = false;
      ai_mode = None;
      silent = true;
      exit_on_failure = false;
      debug_port = None;
    } in
  let _exit_status = ClientStart.main env_start in
  ()


(* connect: this method either connects to the monitor and leaves in an       *)
(* In_init state waiting for the server hello, or it fails to connect and     *)
(* leaves in a Lost_server state.                                             *)
let rec connect (state: state) : state =
  let start_time = Unix.time () in
  let root = match get_root () with
    | None -> failwith "we should have root after an initialize request"
    | Some root -> root
  in
  try
    let conn = connect_client root ~autostart:false in
    let tail_env = Tail.create_env (Sys_utils.readlink_no_fail (ServerFiles.log_link root)) in
    let _state = dismiss_ui state in
    In_init { In_init_env.
      conn;
      start_time;
      uris_with_unsaved_changes = SSet.empty;
      file_edits = ImmQueue.empty;
      tail_env;
      has_reported_progress = false;
      dialog = ShowMessageRequest.None;
      progress = Progress.None;
    }
  with e ->
    (* Exit_with Out_of_retries, Exit_with Out_of_time: raised when we        *)
    (*   couldn't complete the handshake up to handoff within 3 attempts over *)
    (*   3 seconds. Maybe the informant is stopping anything from happening   *)
    (*   until a rebase has settled?                                          *)
    (* Exit_with No_server_running: raised when (1) the server's simply not   *)
    (*   running, or there's some other reason why the connection was refused *)
    (*   or timed-out and no lockfile is present; (2) the server was dormant  *)
    (*   and had already received too many pending connection requests.       *)
    let stack = Printexc.get_backtrace () in
    let (code, message, _data) = Lsp_fmt.get_error_info e in
    let longMessage = Printf.sprintf "connect failed: %s [%i]\n%s" message code stack in
    let () = Lsp_helpers.telemetry_error longMessage in
    let open Exit_status in
    let explanation = match e with
      | Exit_with Out_of_retries
      | Exit_with Out_of_time ->
        Lost_env.Wait_required "hh_server is waiting for things to settle"
      | _ ->
        Lost_env.Action_required ("hh_server: " ^ message)
    in
    do_lost_server state ~allow_immediate_reconnect:false
      { Lost_env.
        explanation;
        start_on_click = true;
        trigger_on_lock_file = true;
        trigger_on_lsp = false;
      }


and reconnect_from_lost_if_necessary
    (state: state)
    (reason: [> `Event of event | `Force_regain ])
  : state =
  let open Lost_env in
  let should_reconnect = match state, reason with
    | Lost_server _, `Force_regain -> true
    | Lost_server lenv, `Event Client_message c
      when lenv.p.trigger_on_lsp && c.Jsonrpc.kind <> Jsonrpc.Response -> true
    | Lost_server lenv, `Event Tick when lenv.p.trigger_on_lock_file ->
      MonitorConnection.server_exists lenv.lock_file
    | _, _ -> false
  in
  if should_reconnect then
    let has_unsaved_changes = not (SSet.is_empty (get_uris_with_unsaved_changes state)) in
    let current_version = read_hhconfig_version () in
    let needs_to_terminate = has_unsaved_changes || !hhconfig_version <> current_version in
    if needs_to_terminate then
      (* In these cases we have to terminate our LSP server, and trust the    *)
      (* client to restart us. Note that we can't do clientStart because that *)
      (* would start our (old) version of hh_server, not the new one!         *)
      let unsaved = get_uris_with_unsaved_changes state |> SSet.elements in
      let unsaved_str = if unsaved = [] then "[None]" else String.concat "\n" unsaved in
      let message = "Unsaved files:\n" ^ unsaved_str ^
        "\nVersion in hhconfig that spawned the current hh_client: " ^ !hhconfig_version ^
        "\nVersion in hhconfig currently: " ^ current_version ^
        "\n" in
      Lsp_helpers.telemetry_log message;
      exit_fail ()
    else
      connect state
  else
    state


(* do_lost_server: handles the various ways we might lose hh_server. We keep  *)
(* the LSP server alive, and will (elsewhere) listen for the various triggers *)
(* of getting the server back.                                                *)
and do_lost_server (state: state) ?(allow_immediate_reconnect = true) (p: Lost_env.params) : state =
  let open Lost_env in

  let no_op = match p.explanation, state with
    | Wait_required _, Lost_server { p = { explanation = Wait_required _; _ }; _ } -> true
    | _ -> false in
  if no_op then state else

  let state = dismiss_ui state in
  let uris_with_unsaved_changes = get_uris_with_unsaved_changes state in

  let lock_file = match get_root () with
    | None -> assert false
    | Some root -> ServerFiles.lock_file root
  in
  let reconnect_immediately = allow_immediate_reconnect &&
    p.trigger_on_lock_file && MonitorConnection.server_exists lock_file
  in

  (* These helper functions are for the dialog *)
  let clear_dialog_flag (state: state) : state =
    match state with
    | Lost_server lenv -> Lost_server { lenv with dialog = ShowMessageRequest.None; }
    | _ -> state
  in
  let handle_error ~code:_ ~message:_ ~data:_ state =
    state |> clear_dialog_flag
  in
  let handle_result ~result state =
    let state = state |> clear_dialog_flag in
    let result = Option.value_map result ~default:""
      ~f:(Hh_json_helpers.get_string_val "title" ~default:"") in
    match result, state with
    | "Restart", Lost_server _ ->
      if p.start_on_click then begin
        let root = match get_root () with
          | None -> failwith "we should have root by now"
          | Some root -> root
        in
        start_server root
      end;
      reconnect_from_lost_if_necessary state `Force_regain
    | _ -> state
  in

  if reconnect_immediately then
    let lost_state = Lost_server { Lost_env.
      p;
      uris_with_unsaved_changes;
      lock_file;
      dialog = ShowMessageRequest.None;
      actionRequired = ActionRequired.None;
      progress = Progress.None;
    } in
    Lsp_helpers.telemetry_log "Reconnecting immediately to hh_server";
    let new_state = reconnect_from_lost_if_necessary lost_state `Force_regain in
    new_state
  else
    let progress, actionRequired, dialog = match p.explanation with
      | Wait_required msg ->
        let progress = Lsp_helpers.notify_progress Progress.None (Some msg) in
        progress, ActionRequired.None, ShowMessageRequest.None
      | Action_required msg ->
        let actionRequired = Lsp_helpers.notify_actionRequired ActionRequired.None (Some msg) in
        let dialog = Lsp_helpers.request_showMessage handle_result handle_error
          MessageType.ErrorMessage msg ["Restart"] in
        Progress.None, actionRequired, dialog
    in
    Lost_server { Lost_env.
      p;
      uris_with_unsaved_changes;
      lock_file;
      dialog;
      actionRequired;
      progress;
    }



let dismiss_ready_dialog_if_necessary (state: state) (event: event) : state =
  (* We'll auto-dismiss the ready dialog if it was up, in response to user    *)
  (* actions like typing or hover, and in response to a lost server.          *)
  let open Jsonrpc in
  let open Main_env in
  match state with
  | Main_loop menv -> begin
      match event with
      | Client_message {kind = Jsonrpc.Response; _} ->
        state
      | Client_message _
      | Server_message ServerCommandTypes.NEW_CLIENT_CONNECTED ->
        let dialog = Lsp_helpers.dismiss_showMessageRequest menv.dialog in
        Main_loop { menv with dialog; }
      | _ ->
        state
    end
  | _ -> state


let handle_idle_if_necessary (state: state) (event: event) : state =
  match state, event with
  | Main_loop menv, Tick -> Main_loop { menv with Main_env.needs_idle = true; }
  | _ -> state


let track_edits_if_necessary (state: state) (event: event) : state =
  let open Jsonrpc in
  (* We'll keep track of which files have unsaved edits. Note that not all    *)
  (* clients send didSave messages; for those we only rely on didClose.       *)
  let previous = get_uris_with_unsaved_changes state in
  let uris_with_unsaved_changes = match event with
    | Client_message ({ method_ = "textDocument/didChange"; _ } as c) ->
      let params = parse_didChange c.params in
      let uri = params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri in
      SSet.add uri previous
    | Client_message ({ method_ = "textDocument/didClose"; _ } as c) ->
      let params = parse_didClose c.params in
      let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
      SSet.remove uri previous
    | Client_message ({ method_ = "textDocument/didSave"; _ } as c) ->
      let params = parse_didSave c.params in
      let uri = params.DidSave.textDocument.TextDocumentIdentifier.uri in
      SSet.remove uri previous
    | _ ->
      previous
  in
  match state with
  | Main_loop menv -> Main_loop { menv with Main_env.uris_with_unsaved_changes; }
  | In_init ienv -> In_init { ienv with In_init_env.uris_with_unsaved_changes; }
  | Lost_server lenv -> Lost_server { lenv with Lost_env.uris_with_unsaved_changes; }
  | _ -> state


let log_response_if_necessary
    (event: event)
    (response: Hh_json.json option)
    (start_handle_t: float)
  : unit =
  let open Jsonrpc in
  match event with
  | Client_message c ->
    let json = c.json |> Hh_json.json_truncate |> Hh_json.json_to_string in
    let json_response = match response with
      | None -> ""
      | Some json -> json |> Hh_json.json_truncate |> Hh_json.json_to_string
    in
    HackEventLogger.client_lsp_method_handled
      ~root:(get_root ())
      ~method_:(if c.kind = Response then get_method_for_response c else c.method_)
      ~kind:(kind_to_string c.kind)
      ~start_queue_t:c.timestamp
      ~start_handle_t
      ~json
      ~json_response
  | _ -> ()


let hack_log_error
    (event: event option)
    (message: string)
    (stack: string)
    (source: string)
    (start_handle_t: float)
  : unit =
  let root = get_root () in
  match event with
  | Some Client_message c ->
    let open Jsonrpc in
    let json = c.json |> Hh_json.json_truncate |> Hh_json.json_to_string in
    HackEventLogger.client_lsp_method_exception
      root c.method_ (kind_to_string c.kind) c.timestamp start_handle_t json message stack source
  | _ ->
    HackEventLogger.client_lsp_exception root message stack source


(* cancel_if_stale: If a message is stale, throw the necessary exception to
   cancel it. A message is considered stale if it's sufficiently old and there
   are other messages in the queue that are newer than it. *)
let short_timeout = 2.5
let long_timeout = 15.0

let cancel_if_stale (client: Jsonrpc.queue) (message: Jsonrpc.message) (timeout: float) : unit =
  let message_received_time = message.Jsonrpc.timestamp in
  let time_elapsed = (Unix.gettimeofday ()) -. message_received_time in
  if time_elapsed >= timeout && Jsonrpc.has_message client
  then raise (Error.RequestCancelled "request timed out")


(************************************************************************)
(** Message handling                                                   **)
(************************************************************************)

(* handle_event: Process and respond to a message, and update the LSP state
   machine accordingly. In case the message was a request, it returns the
   json it responded with, so the caller can log it. *)
let handle_event
    ~(env: env)
    ~(state: state ref)
    ~(client: Jsonrpc.queue)
    ~(event: event)
  : unit =
  let open Jsonrpc in
  let open Main_env in
  match !state, event with
  (* response *)
  | _, Client_message c when c.kind = Jsonrpc.Response ->
    state := Jsonrpc.dispatch_response c !state

  (* shutdown request *)
  | _, Client_message c when c.method_ = "shutdown" ->
    state := do_shutdown !state;
    print_shutdown () |> Jsonrpc.respond c;

  (* cancel notification *)
  | _, Client_message c when c.method_ = "$/cancelRequest" ->
    (* For now, we'll ignore it. *)
    ()

  (* exit notification *)
  | _, Client_message c when c.method_ = "exit" ->
    if !state = Post_shutdown then exit_ok () else exit_fail ()

  (* rage request *)
  | _, Client_message c when c.method_ = "telemetry/rage" ->
    do_rage !state |> print_rage |> Jsonrpc.respond c

  (* initialize request *)
  | Pre_init, Client_message c when c.method_ = "initialize" ->
    initialize_params := c.params;
    hhconfig_version := read_hhconfig_version ();
    state := connect !state;
    do_initialize () |> print_initialize |> Jsonrpc.respond c;
    Lsp_helpers.telemetry_log ("Version in hhconfig=" ^ !hhconfig_version)

  (* any request/notification if we haven't yet initialized *)
  | Pre_init, Client_message _c ->
    raise (Error.ServerNotInitialized "Server not yet initialized")

  (* any request/notification if we're not yet ready *)
  | In_init ienv, Client_message c ->
    let open In_init_env in
    begin match c.method_ with
      | "textDocument/didOpen"
      | "textDocument/didChange"
      | "textDocument/didClose" ->
        (* These three crucial-for-correctness notifications will be buffered *)
        (* up so we'll be able to handle them when we're ready.               *)
        state := In_init { ienv with file_edits = ImmQueue.push ienv.file_edits c.json }
      | _ ->
        raise (Error.RequestCancelled "Server busy")
        (* We deny all other requests. Operation_cancelled is the only *)
        (* error-response that won't produce logs/warnings on most clients. *)
    end

  (* idle tick while waiting for server to complete initialization *)
  | In_init ienv, Tick ->
    let open In_init_env in
    if ienv.has_reported_progress then
      state := report_connect_progress ienv
    else
      state := report_connect_start ienv

  (* server completes initialization *)
  | In_init ienv, Server_hello ->
    connect_after_hello ienv.In_init_env.conn ienv.In_init_env.file_edits;
    state := report_connect_end ienv

  (* any "hello" from the server when we weren't expecting it. This is so *)
  (* egregious that we can't trust anything more from the server.         *)
  | _, Server_hello ->
    let message = "Unexpected hello" in
    let stack = "" in
    raise (Server_fatal_connection_exception { Marshal_tools.message; stack; })

  (* Tick when we're connected to the server and have empty queue *)
  | Main_loop menv, Tick when menv.needs_idle ->
    (* If we're connected to a server and have no more messages in the queue, *)
    (* then we must let the server know we're idle, so it will be free to     *)
    (* handle command-line requests.                                          *)
    state := Main_loop { menv with needs_idle = false; };
    rpc menv.conn ServerCommandTypes.IDE_IDLE

  (* textDocument/hover request *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/hover" ->
    cancel_if_stale client c short_timeout;
    parse_hover c.params |> do_hover menv.conn |> print_hover |> Jsonrpc.respond c

  (* textDocument/definition request *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/definition" ->
    cancel_if_stale client c short_timeout;
    parse_definition c.params |> do_definition menv.conn |> print_definition |> Jsonrpc.respond c

  (* textDocument/completion request *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/completion" ->
    let do_completion =
      if env.use_ffp_autocomplete then do_completion_ffp else do_completion_legacy in
    cancel_if_stale client c short_timeout;
    parse_completion c.params |> do_completion menv.conn |> print_completion |> Jsonrpc.respond c

  (* workspace/symbol request *)
  | Main_loop menv, Client_message c when c.method_ = "workspace/symbol" ->
    parse_workspaceSymbol c.params |> do_workspaceSymbol menv.conn
    |> print_workspaceSymbol |> Jsonrpc.respond c

  (* textDocument/documentSymbol request *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/documentSymbol" ->
    parse_documentSymbol c.params |> do_documentSymbol menv.conn
    |> print_documentSymbol |> Jsonrpc.respond c

  (* textDocument/references request *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/references" ->
    cancel_if_stale client c long_timeout;
    parse_findReferences c.params |> do_findReferences menv.conn
    |> print_findReferences |> Jsonrpc.respond c

  (* textDocument/documentHighlight *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/documentHighlight" ->
    cancel_if_stale client c short_timeout;
    parse_documentHighlights c.params |> do_documentHighlights menv.conn
    |> print_documentHighlights |> Jsonrpc.respond c

  (* textDocument/typeCoverage *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/typeCoverage" ->
    parse_typeCoverage c.params |> do_typeCoverage menv.conn
    |> print_typeCoverage |> Jsonrpc.respond c

  (* textDocument/formatting *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/formatting" ->
    parse_documentFormatting c.params |> do_documentFormatting menv.conn
    |> print_documentFormatting |> Jsonrpc.respond c

  (* textDocument/formatting *)
  | Main_loop menv, Client_message c
    when c.method_ = "textDocument/rangeFormatting" ->
    parse_documentRangeFormatting c.params |> do_documentRangeFormatting menv.conn
    |> print_documentRangeFormatting |> Jsonrpc.respond c

  (* textDocument/onTypeFormatting *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/onTypeFormatting" ->
    cancel_if_stale client c short_timeout;
    parse_documentOnTypeFormatting c.params |> do_documentOnTypeFormatting menv.conn
    |> print_documentOnTypeFormatting |> Jsonrpc.respond c

  (* textDocument/didOpen notification *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/didOpen" ->
    parse_didOpen c.params |> do_didOpen menv.conn

  (* textDocument/didClose notification *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/didClose" ->
    parse_didClose c.params |> do_didClose menv.conn

  (* textDocument/didChange notification *)
  | Main_loop menv, Client_message c when c.method_ = "textDocument/didChange" ->
    parse_didChange c.params |> do_didChange menv.conn

  (* textDocument/didSave notification *)
  | Main_loop _menv, Client_message c when c.method_ = "textDocument/didSave" ->
    ()

  (* server busy status *)
  | _, Server_message ServerCommandTypes.BUSY_STATUS status ->
    state := do_server_busy !state status

  (* textDocument/publishDiagnostics notification *)
  | Main_loop menv, Server_message ServerCommandTypes.DIAGNOSTIC (_, errors) ->
    let uris_with_diagnostics = do_diagnostics menv.uris_with_diagnostics errors in
    state := Main_loop { menv with uris_with_diagnostics; }

  (* any server diagnostics that come after we've shut down *)
  | _, Server_message ServerCommandTypes.DIAGNOSTIC _ ->
    ()

  (* catch-all for client reqs/notifications we haven't yet implemented *)
  | Main_loop _menv, Client_message c ->
    let message = Printf.sprintf "not implemented: %s" c.method_ in
    raise (Error.MethodNotFound message)

  (* catch-all for requests/notifications after shutdown request *)
  | Post_shutdown, Client_message _c ->
    raise (Error.InvalidRequest "already received shutdown request")

  (* server shut-down request *)
  | Main_loop _menv, Server_message ServerCommandTypes.NEW_CLIENT_CONNECTED ->
    state := dismiss_ready_dialog_if_necessary !state event;
    state := do_lost_server !state { Lost_env.
      explanation = Lost_env.Action_required "hh_server is active in another window.";
      start_on_click = false;
      trigger_on_lock_file = false;
      trigger_on_lsp = true;
    }

  (* server shut-down request, unexpected *)
  | _, Server_message ServerCommandTypes.NEW_CLIENT_CONNECTED ->
    let open Marshal_tools in
    let message = "unexpected close of absent server" in
    let stack = "" in
    raise (Server_fatal_connection_exception { message; stack; })

  (* server fatal shutdown *)
  | _, Server_message ServerCommandTypes.FATAL_EXCEPTION e ->
    raise (Server_fatal_connection_exception e)

  (* idle tick. No-op. *)
  | _, Tick ->
    EventLogger.flush ()

  (* client message when we've lost the server *)
  | Lost_server lenv, Client_message _c ->
    let open Lost_env in
    (* if trigger_on_lsp_method is set, our caller should already have        *)
    (* transitioned away from this state.                                     *)
    assert (not lenv.p.trigger_on_lsp);
    (* We deny all other requests. This is the only response that won't       *)
    (* produce logs/warnings on most clients.                                 *)
    raise (Error.RequestCancelled "Server busy")

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. Never returns. *)
let main (env: env) : 'a =
  let open Marshal_tools in
  Printexc.record_backtrace true;
  HackEventLogger.client_set_from env.from;
  let client = Jsonrpc.make_queue () in
  let deferred_action = ref None in
  let state = ref Pre_init in
  while true do
    let ref_event = ref None in
    let start_handle_t = Unix.gettimeofday () in
    (* TODO: we should log how much of the "handling" time was spent *)
    (* idle just waiting for an RPC response from hh_server.         *)
    try
      Option.call () !deferred_action;
      deferred_action := None;
      let event = get_next_event !state client in
      ref_event := Some event;

      (* maybe set a flag to indicate that we'll need to send an idle message *)
      state := handle_idle_if_necessary !state event;
      (* if we're in a lost-server state, some triggers cause us to reconnect *)
      state := reconnect_from_lost_if_necessary !state (`Event event);
      (* if the user does any interaction, then dismiss the "ready" dialog *)
      state := dismiss_ready_dialog_if_necessary !state event;
      (* we keep track of all files that have unsaved changes in them *)
      state := track_edits_if_necessary !state event;

      (* this is the main handler for each message*)
      Jsonrpc.clear_last_sent ();
      handle_event ~env ~state ~client ~event;
      let response = Jsonrpc.last_sent () in
      (* for LSP requests and notifications, we keep a log of what+when we responded *)
      log_response_if_necessary event response start_handle_t
    with
    | Server_fatal_connection_exception edata ->
      if !state <> Post_shutdown then begin
        let stack = edata.stack ^ "---\n" ^ (Printexc.get_backtrace ()) in
        hack_log_error !ref_event edata.message stack "from_server" start_handle_t;
        Lsp_helpers.telemetry_error (edata.message ^ ", from_server\n" ^ stack);
        (* The server never tells us why it closed the connection - it simply   *)
        (* closes. We don't have privilege to inspect its exit status.          *)
        (* The monitor is responsible for detecting server closure and exit     *)
        (* status, and restarting the server if necessary (that's not our job). *)
        (* All we'll do is put up a dialog telling the user that the server is  *)
        (* down and giving them a button to restart. We use a heuristic hint    *)
        (* for when would be a good time to auto-dismiss the dialog and attempt *)
        (* a proper re-connection (it's not our job to ascertain with certainty *)
        (* whether that re-connection will succeed - it's impossible to know,   *)
        (* but also our re-connection attempt is pretty forceful. Our heurstic  *)
        (* is to sleep for 1 second, and then look for the presence of the lock *)
        (* file. The sleep is because typically if you do "hh stop" then the    *)
        (* persistent connection shuts down instantly but the monitor takes a   *)
        (* short time to release its lockfile.                                  *)
        Unix.sleep 1;
        (* We're right now inside an exception handler. We don't want to do     *)
        (* work that might itself throw. So instead we'll leave that to the     *)
        (* next time around the loop.                                           *)
        deferred_action := Some (fun () ->
          state := do_lost_server !state { Lost_env.
            explanation = Lost_env.Action_required "hh_server has stopped";
            start_on_click = true;
            trigger_on_lock_file = true;
            trigger_on_lsp = false;
          })
        end
    | Client_fatal_connection_exception edata ->
      let stack = edata.stack ^ "---\n" ^ (Printexc.get_backtrace ()) in
      hack_log_error !ref_event edata.message stack "from_client" start_handle_t;
      Lsp_helpers.telemetry_error (edata.message ^ ", from_client\n" ^ stack);
      exit_fail ()
    | Client_recoverable_connection_exception edata ->
      let stack = edata.stack ^ "---\n" ^ (Printexc.get_backtrace ()) in
      hack_log_error !ref_event edata.message stack "from_client" start_handle_t;
      Lsp_helpers.telemetry_error (edata.message ^ ", from_client\n" ^ stack);
    | e ->
      let message = Printexc.to_string e in
      let stack = Printexc.get_backtrace () in
      respond_to_error !ref_event e stack;
      hack_log_error !ref_event message stack "from_lsp" start_handle_t;
  done;
  failwith "unreachable"
