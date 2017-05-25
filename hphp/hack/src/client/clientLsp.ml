(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Lsp
open Lsp_fmt

(* All hack-specific code relating to LSP goes in here. *)

(************************************************************************)
(** Conversions - ad-hoc ones written as needed them, not systematic   **)
(************************************************************************)

let lsp_text_document_identifier_to_hack
  (identifier: Lsp.Text_document_identifier.t)
  : ServerUtils.file_input =
  let open Lsp.Text_document_identifier in
  ServerUtils.FileName identifier.uri

let lsp_position_to_ide (position: Lsp.position) : Ide_api_types.position =
  { Ide_api_types.
    line = position.line + 1;
    column = position.character + 1;
  }

let lsp_file_position_to_hack (params: Lsp.Text_document_position_params.t)
  : ServerUtils.file_input * int * int =
  let open Lsp.Text_document_position_params in
  let {Ide_api_types.line; column;} = lsp_position_to_ide params.position in
  let filename = lsp_text_document_identifier_to_hack params.text_document
  in
  (filename, line, column)

let hack_pos_to_lsp_range (pos: 'a Pos.pos) : Lsp.range =
  let line1, col1, line2, col2 = Pos.destruct_range pos in
  {
    start = {line = line1 - 1; character = col1 - 1;};
    end_ = {line = line2 - 1; character = col2 - 1;};
  }

let hack_pos_to_lsp_location (pos: string Pos.pos) : Lsp.Location.t =
  let open Lsp.Location in
  {
    uri = Pos.filename pos;
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
  : Lsp.Location.t =
  let open SymbolDefinition in
  hack_pos_to_lsp_location symbol.pos

let hack_errors_to_lsp_diagnostic (filename: string)
  (errors: Pos.absolute Errors.error_ list)
  : Publish_diagnostics.params =
  let open Lsp.Location in
  let hack_error_to_lsp_diagnostic error =
    let all_locations = Errors.to_list error in
    let pos, message = List.hd_exn all_locations in
    (* TODO: investigate whether Hack ever gives multiline locations *)
    (* TODO: add to LSP protocol for multiple error locations *)
    let {uri = _; range;} = hack_pos_to_lsp_location pos in
    { Lsp.Publish_diagnostics.
      range = range;
      severity = Some Publish_diagnostics.Error;
      code = Some (Errors.get_code error);
      source = Some "Hack";
      message = message;
    }
  in
  { Lsp.Publish_diagnostics.
    uri = filename;
    diagnostics = List.map errors ~f:hack_error_to_lsp_diagnostic;
  }

(************************************************************************)
(** Protocol orchestration & helpers                                   **)
(************************************************************************)

type server_conn = {
  ic : Timeout.in_channel;
  oc : out_channel;

  (* Pending messages sent from the server. They need to be relayed to the
     client. *)
  pending_messages : ServerCommandTypes.push Queue.t;
}

type message_source =
  | No_source
  | From_client
  | From_server

type in_init_env = {
  is_retry : bool;
  start_time : float;
  last_progress_report_time : float;
  has_shown_dialog : bool;
  file_edits : ClientMessageQueue.client_message list;
  tail_env: Tail.env;
}

type lsp_state_machine =
  (* Pre_init: we haven't yet responded to the initialize request. The   *)
  (* argument "Pre_init is_retry" says whether we're already dealt with  *)
  (* (and rejected) a previous initialize request.                       *)
  | Pre_init of bool
  (* In_init: we did respond to the initialize request, and now we're    *)
  (* waiting for a "Hello" from the server. When that comes we'll        *)
  (* request a permanent connection from the server, and process the     *)
  (* file_changes backlog, and switch to Main_loop.                      *)
  | In_init of in_init_env
  (* Main_loop: we have a working connection to both server and client.  *)
  | Main_loop
  (* Post_shutdown: we received a shutdown request from the client, and  *)
  (* therefore shut down our connection to the server. We can't handle   *)
  (* any more requests from the client and will close as soon as it      *)
  (* notifies us that we can exit.                                       *)
  | Post_shutdown

type state = {
  mutable lsp_state : lsp_state_machine;
  client : ClientMessageQueue.t;
  mutable server_conn : server_conn option;
  mutable needs_idle : bool;
}

type event =
  | Server_hello
  | Server_message of ServerCommandTypes.push
  | Server_exit of Exit_status.t
  | Client_message of ClientMessageQueue.client_message
  | Client_error
  | Client_exit (* client exited unceremoniously, without an exit message *)
  | Tick (* once per second, on idle, only generated when server is connected *)

exception Denied_due_to_existing_persistent_connection

(* To handle requests, we use a global list of callbacks for when the *)
(* response is received, and a global id counter for correlation...   *)
module Callback = struct
  type t = {
    method_: string; (* name of the request *)
    on_result: Hh_json.json option -> unit; (* takes the result *)
    on_error: int -> string -> Hh_json.json option -> unit;
    (* code, message, data *)
  }
end
let requests_counter: IMap.key ref = ref 0
let requests_outstanding: Callback.t IMap.t ref = ref IMap.empty


let rpc
  (server_conn: server_conn option)
  (command: 'a ServerCommandTypes.t)
  : 'a =
  match server_conn with
  | None -> failwith "expected server_conn"
  | Some server_conn ->
    let res, pending_messages =
      ServerCommand.rpc_persistent (server_conn.ic, server_conn.oc) command in
    List.iter pending_messages
      ~f:(fun x -> Queue.push x server_conn.pending_messages);
    res

(* Determine whether to read a message from the client (the editor) or the
   server (hh_server). *)
let get_message_source
  (server: server_conn)
  (client: ClientMessageQueue.t)
  : message_source =
  (* Take action on server messages in preference to client messages, because
     server messages are very easy and quick to service (just send a message to
     the client), while client messages require us to launch a potentially
     long-running RPC command. *)
  let has_server_messages = not (Queue.is_empty server.pending_messages) in
  if has_server_messages then From_server else
  if ClientMessageQueue.has_message client then From_client else

  (* If no immediate messages are available, then wait up to 1 second. *)
  let server_read_fd = Unix.descr_of_out_channel server.oc in
  let client_read_fd = ClientMessageQueue.get_read_fd client in
  let readable, _, _ = Unix.select [server_read_fd; client_read_fd] [] [] 1.0 in
  if readable = [] then No_source
  else if List.mem readable server_read_fd then From_server
  else From_client

(*  Read a message unmarshaled from the server's out_channel. *)
let read_message_from_server (server: server_conn) : event =
  let open ServerCommandTypes in
  let fd = Unix.descr_of_out_channel server.oc in
  match Marshal_tools.from_fd_with_preamble fd with
    | Response _ ->
      raise Lsp.Error.(Internal_error "unexpected response without a request")
    | Push m -> Server_message m
    | Hello -> Server_hello

(* get_next_event: picks up the next available message from either client or
   server. The way it's implemented, at the first character of a message
   from either client or server, we block until that message is completely
   received. Note: if server is None (meaning we haven't yet established
   connection with server) then we'll just block waiting for client. *)
let get_next_event (state: state) : event =
  let from_server (server: server_conn) =
    if Queue.is_empty server.pending_messages
      then read_message_from_server server
      else Server_message (Queue.take server.pending_messages)
  in

  let from_client (client: ClientMessageQueue.t) =
    match ClientMessageQueue.get_message client with
    | ClientMessageQueue.Message message -> Client_message message
    | ClientMessageQueue.Error -> Client_error
    | ClientMessageQueue.Exit -> Client_exit
  in

  let from_either server client =
    match get_message_source server client with
    | No_source -> Tick
    | From_client -> from_client client
    | From_server -> from_server server
  in

  match state.server_conn with
  | Some server -> from_either server state.client
  | None -> from_client state.client

(* respond: produces either a Response or an Error message, according
   to whether the json has an error-code or not. Note that JsonRPC and LSP
   mandate id to be present. *)
let respond
  (outchan: out_channel)
  (c: ClientMessageQueue.client_message)
  (json: Hh_json.json)
  : unit =
  let open ClientMessageQueue in
  let open Hh_json in
  let is_error = (Jget.val_opt (Some json) "code" <> None) in
  let response = JSON_Object (
    ["jsonrpc", JSON_String "2.0"]
    @
    ["id", match c.id with Some id -> id | None -> JSON_Null]
    @
    (if is_error then ["error", json] else ["result", json])
    )
  in
  response |> Hh_json.json_to_string |> Http_lite.write_message outchan

(* notify: produces a Notify message *)
let notify (outchan: out_channel) (method_: string) (json: Hh_json.json)
  : unit =
  let open Hh_json in
  let message = JSON_Object [
    "jsonrpc", JSON_String "2.0";
    "method", JSON_String method_;
    "params", json;
  ]
  in
  message |> Hh_json.json_to_string |> Http_lite.write_message outchan

(* request: produce a Request message *)
let request
  (outchan: out_channel)
  (on_result: Hh_json.json option -> unit)
  (on_error: int -> string -> Hh_json.json option -> unit)
  (method_: string)
  (json: Hh_json.json)
  : unit =
  incr requests_counter;
  let callback = { Callback.method_; on_result; on_error; } in
  let request_id = !requests_counter in
  requests_outstanding := IMap.add request_id callback !requests_outstanding;

  let open Hh_json in
  let message = JSON_Object [
    "jsonrpc", string_ "2.0";
    "id", int_ request_id;
    "method", string_ method_;
    "params", json;
  ]
  in
  message |> Hh_json.json_to_string |> Http_lite.write_message outchan

let get_outstanding_request (id: Hh_json.json option) =
  match id with
  | Some (Hh_json.JSON_Number s) -> begin
    try
      let id = int_of_string s in
      Option.map (IMap.get id !requests_outstanding) ~f:(fun v -> (id, v))
    with Failure _ -> None
    end
  | _ -> None

let get_outstanding_method_name (id: Hh_json.json option) : string =
  let open Callback in
  match (get_outstanding_request id) with
  | Some (_, callback) -> callback.method_
  | None -> ""

let do_response
  (id: Hh_json.json option)
  (result: Hh_json.json option)
  (error: Hh_json.json option)
  : unit =
  let open Callback in
  let id, on_result, on_error = match (get_outstanding_request id) with
    | Some (id, callback) -> (id, callback.on_result, callback.on_error)
    | None -> raise (Error.Invalid_request "response to non-existent id")
  in
  requests_outstanding := IMap.remove id !requests_outstanding;
  if Option.is_some error then
    let code = Jget.int_exn error "code" in
    let message = Jget.string_exn error "message" in
    let data = Jget.val_opt error "data" in
    on_error code message data
  else
    on_result result


(* cancel_if_stale: If a message is stale, throw the necessary exception to
   cancel it. A message is considered stale if it's sufficiently old and there
   are other messages in the queue that are newer than it. *)
let short_timeout = 2.5
let long_timeout = 15.0

let cancel_if_stale
  (state: state)
  (message: ClientMessageQueue.client_message)
  (timeout: float)
  : unit =
  let message_received_time = message.ClientMessageQueue.timestamp in
  let time_elapsed = (Unix.gettimeofday ()) -. message_received_time in
  if time_elapsed >= timeout && ClientMessageQueue.has_message state.client
  then raise (Error.Request_cancelled "request timed out")


(* respond_to_error: if we threw an exception during the handling of a request,
   report the exception to the client as the response their request. *)
let respond_to_error (event: event option) (e: exn) : unit =
  match event with
  | Some (Client_message c)
    when c.ClientMessageQueue.kind = ClientMessageQueue.Request ->
    print_error e |> respond stdout c;
  | _ -> ()


let log_error (event: event option) (e: exn) : unit =
  let (error_code, _message, _data) = get_error_info e in
  let (command, kind) = match event with
    | Some (Client_message c) ->
      let open ClientMessageQueue in
      (Some c.method_, Some (kind_to_string c.kind))
    | _ -> (None, None)
  in
  HackEventLogger.client_command_exception
    ~command
    ~kind
    ~e
    ~error_code


(************************************************************************)
(** Protocol                                                           **)
(************************************************************************)

let do_shutdown (conn: server_conn option) : Shutdown.result =
  rpc conn (ServerCommandTypes.UNSUBSCRIBE_DIAGNOSTIC 0);
  rpc conn (ServerCommandTypes.DISCONNECT);
  ()

let do_did_open (conn: server_conn option) (params: Did_open.params) : unit =
  let open Did_open in
  let open Text_document_item in
  let filename = params.text_document.uri in
  let text = params.text_document.text in
  let command = ServerCommandTypes.OPEN_FILE (filename, text) in
  rpc conn command;
  ()

let do_did_close (conn: server_conn option) (params: Did_close.params) : unit =
  let open Did_close in
  let open Text_document_identifier in
  let filename = params.text_document.uri in
  let command = ServerCommandTypes.CLOSE_FILE filename in
  rpc conn command;
  ()

let do_did_change
  (conn: server_conn option)
  (params: Did_change.params)
  : unit =
  let open Versioned_text_document_identifier in
  let open Lsp.Did_change in
  let lsp_change_to_ide (lsp: Did_change.text_document_content_change_event)
      : Ide_api_types.text_edit =
    { Ide_api_types.
      range = Option.map lsp.range lsp_range_to_ide;
      text = lsp.text;
    }
  in
  let filename = params.text_document.uri in
  let changes = List.map params.content_changes ~f:lsp_change_to_ide in
  let command = ServerCommandTypes.EDIT_FILE (filename, changes) in
  rpc conn command;
  ()

let do_hover (conn: server_conn option) (params: Hover.params) : Hover.result =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.INFER_TYPE (file, line, column) in
  let inferred_type = rpc conn command in
  match inferred_type with
  | None -> { Hover.
      contents = [Marked_string "nothing found"];
      range = None;
    }
  | Some (s, _) -> { Hover.
      contents = [Marked_string s];
      range = None;
    }

let do_definition (conn: server_conn option) (params: Definition.params)
  : Definition.result =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDENTIFY_FUNCTION (file, line, column) in
  let results = rpc conn command in
  let rec hack_to_lsp = function
    | [] -> []
    | (_occurrence, None) :: l -> hack_to_lsp l
    | (_occurrence, Some def) :: l ->
        (hack_symbol_definition_to_lsp_location def) :: (hack_to_lsp l)
  in
  hack_to_lsp results

let do_completion (conn: server_conn option) (params: Completion.params)
  : Completion.result =
  let open Text_document_position_params in
  let open Text_document_identifier in
  let open AutocompleteService in
  let open Completion
  in
  let pos = lsp_position_to_ide params.position in
  let filename = params.text_document.uri in
  let command = ServerCommandTypes.IDE_AUTOCOMPLETE (filename, pos) in
  let results = rpc conn command
  in
  let rec hack_completion_to_lsp (result: complete_autocomplete_result)
    : Completion.completion_item =
    {
      label = result.res_name;
      kind = None;
      detail = Some (hack_to_string result);
      documentation = None;
      sort_text = None;
      filter_text = None;
      insert_text = None;
      insert_text_format = PlainText;
      text_edits = [];
      command = None;
      data = None;
    }
  and hack_to_string (result: complete_autocomplete_result) : string =
    match result.func_details with
    | None -> result.res_ty
    | Some f -> let pp = List.map f.params ~f:hack_param_to_string in
                result.res_ty ^ " - " ^
                  (String.concat ", " pp) ^ " -> " ^ f.return_ty
  and hack_param_to_string (p: func_param_result) : string =
    p.param_name ^ ": " ^ p.param_ty
  in
  {
    is_incomplete = false;
    items = List.map results ~f:hack_completion_to_lsp;
  }

let do_workspace_symbol
  (conn: server_conn option)
  (params: Workspace_symbol.params)
  : Workspace_symbol.result =
  let open Workspace_symbol in
  let open SearchUtils in

  let query = params.query in
  let query_type = "" in
  let command = ServerCommandTypes.SEARCH (query, query_type) in
  let results = rpc conn command in

  let hack_to_lsp_kind = function
    | HackSearchService.Class (Some Ast.Cabstract) -> Symbol_information.Class
    | HackSearchService.Class (Some Ast.Cnormal) -> Symbol_information.Class
    | HackSearchService.Class (Some Ast.Cinterface) ->
        Symbol_information.Interface
    | HackSearchService.Class (Some Ast.Ctrait) -> Symbol_information.Interface
        (* LSP doesn't have traits, so we approximate with interface *)
    | HackSearchService.Class (Some Ast.Cenum) -> Symbol_information.Enum
    | HackSearchService.Class (None) -> assert false (* should never happen *)
    | HackSearchService.Method _ -> Symbol_information.Method
    | HackSearchService.ClassVar _ -> Symbol_information.Property
    | HackSearchService.Function -> Symbol_information.Function
    | HackSearchService.Typedef -> Symbol_information.Class
        (* LSP doesn't have typedef, so we approximate with class *)
    | HackSearchService.Constant -> Symbol_information.Constant
  in
  let hack_to_lsp_container = function
    | HackSearchService.Method (_, scope) -> Some scope
    | HackSearchService.ClassVar (_, scope) -> Some scope
    | _ -> None
  in
  let hack_symbol_to_lsp (symbol: HackSearchService.symbol) =
    { Symbol_information.
      name = (Utils.strip_ns symbol.name);
      kind = hack_to_lsp_kind symbol.result_type;
      location = hack_pos_to_lsp_location symbol.pos;
      container_name = hack_to_lsp_container symbol.result_type;
    }
  in
  List.map results ~f:hack_symbol_to_lsp

let do_document_symbol
  (conn: server_conn option)
  (params: Document_symbol.params)
  : Document_symbol.result =
  let open Document_symbol in
  let open Text_document_identifier in
  let open SymbolDefinition in

  let filename = params.text_document.uri in
  let command = ServerCommandTypes.OUTLINE filename in
  let results = rpc conn command in

  let hack_to_lsp_kind = function
    | SymbolDefinition.Function -> Symbol_information.Function
    | SymbolDefinition.Class -> Symbol_information.Class
    | SymbolDefinition.Method -> Symbol_information.Method
    | SymbolDefinition.Property -> Symbol_information.Property
    | SymbolDefinition.Const -> Symbol_information.Constant
    | SymbolDefinition.Enum -> Symbol_information.Enum
    | SymbolDefinition.Interface -> Symbol_information.Interface
    | SymbolDefinition.Trait -> Symbol_information.Interface
        (* LSP doesn't have traits, so we approximate with interface *)
    | SymbolDefinition.LocalVar -> Symbol_information.Variable
    | SymbolDefinition.Typeconst -> Symbol_information.Class
        (* e.g. "const type Ta = string;" -- absent from LSP *)
    | SymbolDefinition.Typedef -> Symbol_information.Class
        (* e.g. top level type alias -- absent from LSP *)
    | SymbolDefinition.Param -> Symbol_information.Variable
        (* We never return a param from a document-symbol-search *)
  in
  let hack_symbol_to_lsp def container_name =
    { Symbol_information.
      name = def.name;
      kind = hack_to_lsp_kind def.kind;
      location = hack_symbol_definition_to_lsp_location def;
      container_name;
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

let do_find_references
  (conn: server_conn option)
  (params: Find_references.params)
  : Find_references.result =
  let open Find_references in

  let {Ide_api_types.line; column;} = lsp_position_to_ide params.position in
  let filename = lsp_text_document_identifier_to_hack params.text_document in
  let include_defs = params.context.include_declaration in
  let command = ServerCommandTypes.IDE_FIND_REFS
    (filename, line, column, include_defs) in
  let results = rpc conn command in
  (* TODO: respect params.context.include_declaration *)
  match results with
  | None -> []
  | Some (_name, positions) -> List.map positions ~f:hack_pos_to_lsp_location


let do_document_highlights
  (conn: server_conn option)
  (params: Document_highlights.params)
  : Document_highlights.result =
  let open Document_highlights in

  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDE_HIGHLIGHT_REFS (file, line, column) in
  let results = rpc conn command in

  let hack_range_to_lsp_highlight range =
    {
      range = ide_range_to_lsp range;
      kind = None;
    }
  in
  List.map results ~f:hack_range_to_lsp_highlight


let do_type_coverage (conn: server_conn option) (params: Type_coverage.params)
  : Type_coverage.result =
  let open Type_coverage in

  let filename = lsp_text_document_identifier_to_hack params.text_document in
  let command = ServerCommandTypes.COVERAGE_LEVELS filename in
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
  let covered_percent = if ntotal = 0 then 100
    else ((nchecked * 100) + (npartial * 50)) / ntotal in

  let hack_coverage_to_lsp (pos, level) =
    let range = hack_pos_to_lsp_range pos in
    match level with
    | Ide_api_types.Checked -> None
    | Ide_api_types.Unchecked -> Some {range; message =
        "Un-type checked code. Consider adding type annotations.";}
    | Ide_api_types.Partial -> Some {range; message =
        "Partially type checked code. Consider adding type annotations.";}
  in
  {
    covered_percent;
    uncovered_ranges = List.filter_map results ~f:hack_coverage_to_lsp;
  }


let do_formatting_common
  (conn: server_conn option)
  (args: ServerFormatTypes.ide_action)
  : Text_edit.t list =
  let open ServerFormatTypes in
  let command = ServerCommandTypes.IDE_FORMAT args in
  let response: ServerFormatTypes.ide_result = rpc conn command in
  match response with
  | Result.Error message ->
      raise (Error.Internal_error message)
  | Result.Ok r ->
      let range = ide_range_to_lsp r.range in
      let new_text = r.new_text in
      [{Text_edit.range; new_text;}]


let do_document_range_formatting
  (conn: server_conn option)
  (params: Document_range_formatting.params)
  : Document_range_formatting.result =
  let open Document_range_formatting in
  let open Text_document_identifier in
  let action = ServerFormatTypes.Range { Ide_api_types.
    range_filename = params.text_document.uri;
    file_range = lsp_range_to_ide params.range;
  }
  in
  do_formatting_common conn action


let do_document_on_type_formatting
  (conn: server_conn option)
  (params: Document_on_type_formatting.params)
  : Document_on_type_formatting.result =
  let open Document_on_type_formatting in
  let open Text_document_identifier in
  let action = ServerFormatTypes.Position { Ide_api_types.
    filename = params.text_document.uri;
    position = lsp_position_to_ide params.position;
  } in
  do_formatting_common conn action


let do_document_formatting
  (conn: server_conn option)
  (params: Document_formatting.params)
  : Document_formatting.result =
  let open Document_formatting in
  let open Text_document_identifier in
  let action = ServerFormatTypes.Document params.text_document.uri in
  do_formatting_common conn action


let report_progress
  (ienv: in_init_env)
  : in_init_env =
  (* Our goal behind progress reporting is to let the user know when things   *)
  (* won't be instantaneous, and to show that things are working as expected. *)
  (* We eagerly show a "please wait" dialog box after just 5 seconds into     *)
  (* progress reporting. And we'll log to the console every 10 seconds. When  *)
  (* it completes, if any progress has so far been shown to the user, then    *)
  (* we'll log completion to the console. Except if it took a long time, like *)
  (* 60 seconds or more, we'll show completion with a dialog box instead.     *)
  (*                                                                          *)
  let time = Unix.time () in
  let ienv = ref ienv in

  if (not !ienv.has_shown_dialog) then begin
    ienv := {!ienv with has_shown_dialog = true};
    print_show_message Message_type.InfoMessage
      "Waiting for Hack server to be ready. See console for further details."
      |> notify stdout "window/showMessage"
  end;

  let delay_in_secs = int_of_float (time -. !ienv.start_time) in
  if (delay_in_secs mod 10 = 0 &&
    time -. !ienv.last_progress_report_time >= 5.0) then begin
    ienv := {!ienv with last_progress_report_time = time};
    let _load_state_not_found, tail_msg =
      ClientConnect.open_and_get_tail_msg !ienv.start_time !ienv.tail_env in
    let msg = Printf.sprintf "Still waiting after %i seconds: %s..."
      delay_in_secs tail_msg in
    print_log_message Message_type.LogMessage msg
      |> notify stdout "window/logMessage"
  end;
  !ienv


let report_progress_end
  (ienv: in_init_env)
  : unit =
  let time = Unix.time () in
  let msg = Printf.sprintf "Hack is now ready, after %i seconds."
    (int_of_float (time -. ienv.start_time)) in
  if (time -. ienv.start_time >= 60.0) then begin
    print_show_message Message_type.InfoMessage msg
      |> notify stdout "window/showMessage"
  end else begin
    print_log_message Message_type.InfoMessage msg
      |> notify stdout "window/logMessage";
  end


let do_initialize_after_hello
  (server_conn: server_conn option)
  ~(is_retry: bool)
  (file_edits: ClientMessageQueue.client_message list)
  : unit =
  let oc = match server_conn with
    | None -> failwith "expected server_conn"
    | Some server_conn -> server_conn.oc in
  let connection_type = if is_retry
    then ServerCommandTypes.Persistent_hard
    else ServerCommandTypes.Persistent_soft in
  ServerCommand.send_connection_type oc connection_type;
  let fd = Unix.descr_of_out_channel oc in
  let response = Marshal_tools.from_fd_with_preamble fd in
  if response = ServerCommandTypes.Denied_due_to_existing_persistent_connection
    then raise Denied_due_to_existing_persistent_connection;

  let handle_file_edit (c: ClientMessageQueue.client_message) =
    let open ClientMessageQueue in
    match c.method_ with
    | "textDocument/didOpen" ->
      parse_did_open c.params |> do_did_open server_conn
    | "textDocument/didChange" ->
      parse_did_change c.params |> do_did_change server_conn
    | "textDocument/didClose" ->
      parse_did_close c.params |> do_did_close server_conn
    | _ ->
      failwith "should only buffer up didOpen/didChange/didClose"
  in
  List.iter file_edits ~f:handle_file_edit;

  rpc server_conn (ServerCommandTypes.SUBSCRIBE_DIAGNOSTIC 0)


let do_initialize_start
  (root: Path.t)
  : Exit_status.t =
  (* This basically does "hh_client start": a single attempt to open the     *)
  (* socket, send+read version and compare for mismatch, send handoff and    *)
  (* read response. It will print information to stderr. If the server is in *)
  (* an unresponsive or invalid state then it will kill the server. Next if  *)
  (* necessary it tries to spawn the server and wait until the monitor is    *)
  (* responsive enough to print "ready". It will do a hard program exit if   *)
  (* there were spawn problems.                                              *)
  let env_start = { ClientStart.
    root;
    no_load = false;
    ai_mode = None;
    silent = true;
    debug_port = None;
  } in
  ClientStart.main env_start


let do_initialize_connect
  (root: Path.t)
  : server_conn =
  let open Exit_status in
  let open Lsp.Error in
  (* This basically does the same connection attempt as "hh_client check":  *)
  (* it makes repeated attempts to connect; it prints useful messages to    *)
  (* stderr; in case of failure it will raise an exception. Below we're     *)
  (* catching the main exceptions so we can give a good user-facing error   *)
  (* text. For other exceptions, they'll end up showing to the user just    *)
  (* "internal error" with the error code.                                  *)
  let env_connect = { ClientConnect.
    root;
    autostart = false; (* we already did a more aggressive start *)
    force_dormant_start = false;
    retries = Some 3; (* each retry takes up to 1 second *)
    expiry = None; (* we can limit retries by time as well as by count *)
    retry_if_init = true; (* not actually used *)
    no_load = false; (* only relevant when autostart=true *)
    ai_mode = None; (* only relevant when autostart=true *)
    progress_callback = ClientConnect.null_progress_reporter; (* we're fast! *)
    do_post_handoff_handshake = false;
  } in
  try
    let (ic, oc) = ClientConnect.connect env_connect in
    let pending_messages = Queue.create () in
    { ic; oc; pending_messages; }
  with
  | Exit_with No_server_running ->
      (* Raised when (1) the connection was refused/timed-out and no lockfile *)
      (* is present; (2)) server was dormant and had already received too     *)
      (* many pending connection requests. In all cases more detail has       *)
      (* been printed to stderr.                                              *)
      (* How should the user react? -- they can read the console to find out  *)
      (* more; they can try to restart at the command-line; or they can try   *)
      (* to restart within their editor.                                      *)
      raise (Server_error_start (
        "Attempts to start Hack server have failed; see console for details.",
        { Lsp.Initialize.retry = true; }))
  | Exit_with Out_of_retries
  | Exit_with Out_of_time ->
      (* Raised when we couldn't complete the entire handshake despite        *)
      (* repeated attempts. Most likely because hh_server was busy loading    *)
      (* saved-state, or failed to load saved-state and had to initialize     *)
      (* everything itself.                                                   *)
      (* How should the user react? -- as above, by reading the console, by   *)
      (* attempting restart at the command-line or in editor.                 *)
      raise (Server_error_start (
        "The Hack server is busy and unable to provide language services.",
        { Lsp.Initialize.retry = true; }))


let do_initialize_hello_attempt
  (server_conn: server_conn)
  : bool =
  (* This waits up to 3 seconds for the server to send "Hello", the first     *)
  (* message it sends after handoff. It might take some time if the server    *)
  (* has to finish typechecking first. Returns whether it got the "Hello".    *)
  try
    let retries = Some 3 in  (* it does one retry per second *)
    let tail_env = None in  (* don't report progress to stderr *)
    let time = Unix.time () in  (* dummy; not used because of tail_env=None *)
    ClientConnect.wait_for_server_hello server_conn.ic retries
      ClientConnect.null_progress_reporter time tail_env;
    true
  with
  | Exit_status.Exit_with Exit_status.Out_of_retries ->
      false
  | ClientConnect.Server_hung_up ->
      (* Raised by wait_for_server_hello, if someone killed the server while  *)
      (* it was busy doing its typechecking or other work.                    *)
      (* How should user react? - by attempting to re-connect.                *)
      raise (Lsp.Error.Server_error_start (
        "hh_server died unexpectedly. Maybe you recently launched a " ^
        "different version of hh_server.",
        { Lsp.Initialize.retry = true; }))


let do_initialize ~(is_retry: bool) (params: Initialize.params)
  : (server_conn option * Initialize.result * lsp_state_machine) =
  let open Initialize in
  let start_time = Unix.time () in
  let root = if Option.is_some params.root_uri
             then ClientArgsUtils.get_root params.root_uri
             else ClientArgsUtils.get_root params.root_path in
  let local_config = ServerLocalConfig.load () in

  (* This code does the connection protocol. *)
  let _result_code = do_initialize_start root in
  let server_conn  = do_initialize_connect root in
  let got_hello    = do_initialize_hello_attempt server_conn in
  let new_state =
    if got_hello then begin
      try
        do_initialize_after_hello (Some server_conn) ~is_retry [];
        Main_loop
        with Denied_due_to_existing_persistent_connection ->
          (* How should the user react? - by retrying, to kick off our rival. *)
          raise (Lsp.Error.Server_error_start (
            "Cannot provide Hack language services while this project is " ^
            "is active in another window.",
            { Lsp.Initialize.retry = true; }))
    end else begin
      let log_file = Sys_utils.readlink_no_fail (ServerFiles.log_link root) in
      let ienv = {
        is_retry;
        start_time;
        last_progress_report_time = start_time;
        has_shown_dialog = false;
        file_edits = [];
        tail_env = Tail.create_env log_file;
      } in
      In_init ienv
    end

  in
  let result = {
    server_capabilities = {
      text_document_sync = {
        want_open_close = true;
        want_change = IncrementalSync;
        want_will_save = false;
        want_will_save_wait_until = false;
        want_did_save = { include_text = false }
      };
      hover_provider = true;
      completion_provider = Some {
        resolve_provider = true;
        completion_trigger_characters = ["-"; ">"; "\\"];
      };
      signature_help_provider = None;
      definition_provider = true;
      references_provider = true;
      document_highlight_provider = true;
      document_symbol_provider = true;
      workspace_symbol_provider = true;
      code_action_provider = false;
      code_lens_provider = None;
      document_formatting_provider = true;
      document_range_formatting_provider = true;
      document_on_type_formatting_provider =
        Option.some_if local_config.ServerLocalConfig.use_hackfmt
        {
          first_trigger_character = ";";
          more_trigger_characters = ["}"];
        };
      rename_provider = false;
      document_link_provider = None;
      execute_command_provider = None;
      type_coverage_provider = true;
    }
  }
  in
  (Some server_conn, result, new_state)


(************************************************************************)
(** Message handling                                                   **)
(************************************************************************)

(* handle_event: Process and respond to a message, and update the LSP state
   machine accordingly. *)
let handle_event (state: state) (event: event) : unit =
  let open ClientMessageQueue in
  let exit () = exit (if state.lsp_state = Post_shutdown then 0 else 1) in
  match state.lsp_state, event with
  (* response *)
  | _, Client_message c when c.kind = ClientMessageQueue.Response ->
    do_response c.id c.result c.error

  (* exit notification *)
  | _, Client_message c when c.method_ = "exit" -> exit ()
  | _, Client_exit -> exit ()

  (* initialize request*)
  | Pre_init is_retry, Client_message c when c.method_ = "initialize" -> begin
    (* We can't directly determine whether the initialization request is a *)
    (* retry, since that's not a part of the protocol. However, if we went *)
    (* through initialize once and failed (with an exception), then the    *)
    (* next time we're asked to initialize will necessarily be a retry.    *)
    (* We set the retry flag to 'true' here now, so the next               *)
    (* initialization request will be treated as a retry.                  *)
    state.lsp_state <- Pre_init true;
    let (server_conn', result, new_state) =
      parse_initialize c.params |> do_initialize ~is_retry in
    print_initialize result |> respond stdout c;
    state.lsp_state <- new_state;
    state.server_conn <- server_conn'
    end

  (* any request/notification if we haven't yet initialized *)
  | Pre_init _, Client_message _c ->
    raise (Error.Server_not_initialized "Server not yet initialized");

  (* any request/notification if we're not yet ready *)
  | In_init ienv, Client_message c ->
    begin match c.method_ with
      | "textDocument/didOpen"
      | "textDocument/didChange"
      | "textDocument/didClose" ->
        let ienv' = { ienv with file_edits = c :: ienv.file_edits } in
        state.lsp_state <- In_init ienv'
        (* These three crucial-for-correctness notifications will be buffered *)
        (* up so we'll be able to handle them when we're ready.               *)
      | "shutdown" ->
        state.lsp_state <- Post_shutdown;
        state.server_conn <- None
      | _ ->
        raise (Error.Request_cancelled "Server busy")
        (* We deny all other requests. Operation_cancelled is the only *)
        (* error-response that won't produce logs/warnings on most clients. *)
    end

  (* idle tick while waiting for server to complete initialization *)
  | In_init ienv, Tick ->
    let ienv' = report_progress ienv in
    state.lsp_state <- In_init ienv'

  (* server completes initialization *)
  | In_init ienv, Server_hello ->
    (* As above, if we get an exception while handling this, then we'll want *)
    (* to be in the "pre_init / no_connection" state for future messages.    *)
    let conn = state.server_conn in
    state.lsp_state <- Pre_init true;
    state.server_conn <- None;
    begin try
      do_initialize_after_hello conn ienv.is_retry (List.rev ienv.file_edits)
    with
      Denied_due_to_existing_persistent_connection -> exit ()
      (* If we're denied at this stage, we can't recover: we'll just exit *)
      (* and trust the client to restart us. *)
    end;
    report_progress_end ienv;
    state.lsp_state <- Main_loop;
    state.server_conn <- conn

  (* any "hello" from the server when we weren't expecting it. This is so *)
  (* egregious that we can't trust anything more from the server.         *)
  | Pre_init _, Server_hello
  | Main_loop, Server_hello
  | Post_shutdown, Server_hello ->
    log_error
      (Some event)
      (Error.Internal_error "Received unexpected 'hello' from hh_server");
    exit ()

  (* Tick when we're connected to the server and have empty queue *)
  | Main_loop, Tick when state.needs_idle ->
    state.needs_idle <- false;
    rpc state.server_conn ServerCommandTypes.IDE_IDLE
    (* If we're connected to a server and have no more messages in the queue, *)
    (* then we must let the server know we're idle, so it will be free to     *)
    (* handle command-line requests.                                          *)

  (* textDocument/hover request *)
  | Main_loop, Client_message c when c.method_ = "textDocument/hover" ->
    cancel_if_stale state c short_timeout;
    parse_hover c.params |> do_hover state.server_conn |> print_hover
      |> respond stdout c;

  (* textDocument/definition request *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/definition" ->
    cancel_if_stale state c short_timeout;
    parse_definition c.params |> do_definition state.server_conn
      |> print_definition |> respond stdout c

  (* textDocument/completion request *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/completion" ->
    cancel_if_stale state c short_timeout;
    parse_completion c.params |> do_completion state.server_conn
      |> print_completion |> respond stdout c

  (* workspace/symbol request *)
  | Main_loop, Client_message c when c.method_ = "workspace/symbol" ->
    parse_workspace_symbol c.params |> do_workspace_symbol state.server_conn
      |> print_workspace_symbol |> respond stdout c

  (* textDocument/documentSymbol request *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/documentSymbol" ->
    parse_document_symbol c.params |> do_document_symbol state.server_conn
      |> print_document_symbol |> respond stdout c

  (* textDocument/references requeset *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/references" ->
    cancel_if_stale state c long_timeout;
    parse_find_references c.params |> do_find_references state.server_conn
      |> print_find_references |> respond stdout c

  (* textDocument/documentHighlight *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/documentHighlight" ->
    cancel_if_stale state c short_timeout;
    parse_document_highlights c.params
      |> do_document_highlights state.server_conn |> print_document_highlights
      |> respond stdout c

  (* textDocument/typeCoverage *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/typeCoverage" ->
    parse_type_coverage c.params |> do_type_coverage state.server_conn
      |> print_type_coverage |> respond stdout c

  (* textDocument/formatting *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/formatting" ->
    parse_document_formatting c.params
      |> do_document_formatting state.server_conn |> print_document_formatting
      |> respond stdout c

  (* textDocument/formatting *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/rangeFormatting" ->
    parse_document_range_formatting c.params
      |> do_document_range_formatting state.server_conn
      |> print_document_range_formatting |> respond stdout c

  (* textDocument/onTypeFormatting *)
  | Main_loop, Client_message c
    when c.method_ = "textDocument/onTypeFormatting" ->
    cancel_if_stale state c short_timeout;
    parse_document_on_type_formatting c.params
      |> do_document_on_type_formatting state.server_conn
      |> print_document_on_type_formatting |> respond stdout c

  (* textDocument/didOpen notification *)
  | Main_loop, Client_message c when c.method_ = "textDocument/didOpen" ->
    parse_did_open c.params |> do_did_open state.server_conn;

  (* textDocument/didClose notification *)
  | Main_loop, Client_message c when c.method_ = "textDocument/didClose" ->
    parse_did_close c.params |> do_did_close state.server_conn;

  (* textDocument/didChange notification *)
  | Main_loop, Client_message c when c.method_ = "textDocument/didChange" ->
    parse_did_change c.params |> do_did_change state.server_conn;

  (* shutdown request *)
  | Main_loop, Client_message c when c.method_ = "shutdown" ->
    do_shutdown state.server_conn |> print_shutdown |> respond stdout c;
    state.lsp_state <- Post_shutdown;
    state.server_conn <- None;

  (* textDocument/publishDiagnostics notification *)
  | Main_loop, Server_message ServerCommandTypes.DIAGNOSTIC (_, errors) ->
    let per_file uri errors = hack_errors_to_lsp_diagnostic uri errors
        |> print_diagnostics
        |> notify stdout "textDocument/publishDiagnostics"
    in
    SMap.iter per_file errors

  (* any server diagnostics that come after we've shut down *)
  | _, Server_message ServerCommandTypes.DIAGNOSTIC _ ->
    ()

  (* ignore parsing errors *)
  | _, Client_error -> ()

  (* catch-all for client reqs/notifications we haven't yet implemented *)
  | Main_loop, Client_message c ->
    let message = Printf.sprintf "not implemented: %s" c.method_ in
    raise (Error.Method_not_found message)

  (* catch-all for requests/notifications after shutdown request *)
  | Post_shutdown, Client_message _c ->
    raise (Error.Invalid_request "already received shutdown request")

  (* server shut-down request *)
  | _, Server_message ServerCommandTypes.NEW_CLIENT_CONNECTED ->
    print_log_message Message_type.ErrorMessage "Another client has connected"
      |> notify stdout "window/logMessage";
    exit ()

  (* TODO message from server *)
  | _, Server_exit _ -> () (* todo *)

  (* idle tick. No-op. *)
  | _, Tick -> ()


(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. *)
let main () : unit =
  Printexc.record_backtrace true;
  let state = {
    lsp_state = Pre_init false;
    client = ClientMessageQueue.make ();
    server_conn = None;
    needs_idle = false;
  } in
  while true do
    let event = get_next_event state in
    try
      let event = get_next_event state in
      if event <> Tick then state.needs_idle <- true;
      handle_event state event;
      match event with
      | Client_message c -> begin
          let open ClientMessageQueue in
          (* Log to scuba the time the message arrived at hh_client and the *)
          (* time we finished handling it. This includes any time it spent  *)
          (* waiting in the queue...                                        *)
          let command =
            if c.kind = Response
            then get_outstanding_method_name c.id
            else c.method_ in
          HackEventLogger.client_handled_command
            ~command
            ~start_t:c.timestamp
            ~kind:(kind_to_string c.kind);
        end
      | _ -> ()
    with e ->
      respond_to_error (Some event) e;
      log_error (Some event) e;
      if e = Sys_error "Broken pipe" then exit 1
      (* The reading/writing we do here is with hh_server, so this exception *)
      (* means that our connection to hh_server has gone down, which is      *)
      (* unrecoverable by us. We'll trust the LSP client to restart.         *)
  done
