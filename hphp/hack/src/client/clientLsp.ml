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

(* TODO: this file has a few things from ClientIde and Ide_api_types.
   It didn't seem worth refactoring them into a common place shared by both
   IDE and LSP, given that we hope to phase out IDE. *)


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
(** Protocol                                                           **)
(************************************************************************)

type conn = Timeout.in_channel * out_channel

let rpc (conn: conn option) (command: 'a ServerCommandTypes.t) : 'a =
  let conn = match conn with None -> failwith "expected conn" | Some c -> c in
  ClientIde.rpc conn command

let do_shutdown (conn: conn option) : Shutdown.result =
  rpc conn (ServerCommandTypes.UNSUBSCRIBE_DIAGNOSTIC 0);
  rpc conn (ServerCommandTypes.DISCONNECT);
  ()

let do_did_open (conn: conn option) (params: Did_open.params) : unit =
  let open Did_open in
  let open Text_document_item in
  let filename = params.text_document.uri in
  let text = params.text_document.text in
  let command = ServerCommandTypes.OPEN_FILE (filename, text) in
  rpc conn command;
  ()

let do_did_close (conn: conn option) (params: Did_close.params) : unit =
  let open Did_close in
  let open Text_document_identifier in
  let filename = params.text_document.uri in
  let command = ServerCommandTypes.CLOSE_FILE filename in
  rpc conn command;
  ()

let do_did_change (conn: conn option) (params: Did_change.params) : unit =
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

let do_hover (conn: conn option) (params: Hover.params) : Hover.result =
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

let do_definition (conn: conn option) (params: Definition.params)
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

let do_completion (conn: conn option) (params: Completion.params)
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

let do_workspace_symbol (conn: conn option) (params: Workspace_symbol.params)
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

let do_document_symbol (conn: conn option) (params: Document_symbol.params)
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

let do_find_references (conn: conn option) (params: Find_references.params)
  : Find_references.result =
  let open Find_references in

  let {Ide_api_types.line; column;} = lsp_position_to_ide params.position in
  let filename = lsp_text_document_identifier_to_hack params.text_document in
  let command = ServerCommandTypes.IDE_FIND_REFS (filename, line, column) in
  let results = rpc conn command in
  (* TODO: respect params.context.include_declaration *)
  match results with
  | None -> []
  | Some (_name, positions) -> List.map positions ~f:hack_pos_to_lsp_location


let do_document_highlights
  (conn: conn option)
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


let do_initialize (params: Initialize.params)
  : (conn option * Initialize.result) =
  let open Initialize in
  let root = if Option.is_some params.root_uri
             then ClientArgsUtils.get_root params.root_uri
             else ClientArgsUtils.get_root params.root_path in
  let env = { ClientIde.root = root }
  in
  try
    let conn = ClientIde.connect_persistent env ~retries:800 in
    rpc (Some conn) (ServerCommandTypes.SUBSCRIBE_DIAGNOSTIC 0);
    let result = {
      server_capabilities = {
        text_document_sync = {
          want_open_close = true;
          want_change = IncrementalSync;
          want_will_save = false;
          want_will_save_wait_until = false;
          want_did_save = { include_text = false }
        };
        hover_provider = false;
        completion_provider = None;
        signature_help_provider = None;
        definition_provider = false;
        references_provider = true;
        document_highlight_provider = true;
        document_symbol_provider = true;
        workspace_symbol_provider = true;
        code_action_provider = false;
        code_lens_provider = None;
        document_formatting_provider = false;
        document_range_formatting_provider = false;
        document_on_type_formatting_provider = None;
        rename_provider = false;
        document_link_provider = None;
        execute_command_provider = None;
      }
    }
    in
    (Some conn, result)
  with
  | e ->
    let message = (Printexc.to_string e) ^ ": " ^ (Printexc.get_backtrace ()) in
    raise (Error.Server_error_start (message, {retry = false;}))


(************************************************************************)
(** Protocol orchestration & helpers                                   **)
(************************************************************************)

type lsp_state_machine =
  | Pre_init
  | Main_loop
  | Post_shutdown

and event =
  | Server of ServerCommandTypes.push
  | Server_exit of Exit_status.t
  | Client of client_message

and client_message = {
  method_ : string;
  id : Hh_json.json option;
  params : Hh_json.json option;
  is_request : bool;
}

(* respond: produces either a Response or an Error message, according
   to whether the json has an error-code or not. Note that JsonRPC and LSP
   mandate id to be present. *)
let respond (outchan: out_channel) (c: client_message) (json: Hh_json.json)
  : unit =
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

(* get_next_event: picks up the next available message from either client or
   server. The way it's implemented, at the first character of a message
   from either client or server, we block until that message is completely
   received. Note: if server is None (meaning we haven't yet established
   connection with server) then we'll just block waiting for client. *)
let get_next_event
  ~(server: conn option)
  ~(client: Buffered_line_reader.t)
  : event =
  let from_server (server: Unix.file_descr) =
    try Server (ClientIde.get_next_push_message server)
    with _ -> Server_exit Exit_status.No_error (* TODO: failure? *)
  in
  let from_client (client: Buffered_line_reader.t) =
    let json = try Some (Http_lite.read_message_utf8 client
                         |> Hh_json.json_of_string)
               with Http_lite.Malformed _ | Hh_json.Syntax_error _ -> None in
    let id = Jget.val_opt json "id" in
    Client {
      method_ = Jget.string_exn json "method";
      id = id;
      params = Jget.obj_opt json "params";
      is_request = (id <> None);
    }
  in
  let rec from_either server client =
  (* TODO: avoid use of global variable ClientIde.stdin_reader in following. *)
    match ClientIde.get_ready_message server with
    | `None -> from_either server client
    | `Stdin -> from_client client
    | `Server -> from_server server
  in
  match server with
  | None -> from_client client
  | Some conn -> let server = Unix.descr_of_out_channel (snd conn) in
                 from_either server client

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. *)
let main () : unit =
  Printexc.record_backtrace true;
  let state: lsp_state_machine ref = ref Pre_init in
  let conn: conn option ref = ref None in
  while true do
    let event = get_next_event ~server:!conn ~client:ClientIde.stdin_reader in
    try
      match !state, event with

      (* exit notification *)
      | _, Client c when c.method_ = "exit" ->
        exit (if !state = Post_shutdown then 0 else 1)

      (* initialize request*)
      | Pre_init, Client c when c.method_ = "initialize" -> begin
        let (conn2, result) = parse_initialize c.params |> do_initialize in
        print_initialize result |> respond stdout c;
        state := Main_loop;
        conn := conn2
        end

      (* any request/notification if we haven't yet initialized *)
      | Pre_init, Client _c ->
        raise (Error.Server_not_initialized "init");

      (* textDocument/hover request *)
      | Main_loop, Client c when c.method_ = "textDocument/hover" ->
        parse_hover c.params |> do_hover !conn |> print_hover
          |> respond stdout c;

      (* textDocument/definition request *)
      | Main_loop, Client c when c.method_ = "textDocument/definition" ->
        parse_definition c.params |> do_definition !conn
          |> print_definition |> respond stdout c

      (* textDocument/completion request *)
      | Main_loop, Client c when c.method_ = "textDocument/completion" ->
        parse_completion c.params |> do_completion !conn |> print_completion
          |> respond stdout c

      (* workspace/symbol request *)
      | Main_loop, Client c when c.method_ = "workspace/symbol" ->
        parse_workspace_symbol c.params |> do_workspace_symbol !conn
          |> print_workspace_symbol |> respond stdout c

      (* textDocument/documentSymbol request *)
      | Main_loop, Client c when c.method_ = "textDocument/documentSymbol" ->
        parse_document_symbol c.params |> do_document_symbol !conn
          |> print_document_symbol |> respond stdout c

      (* textDocument/references requeset *)
      | Main_loop, Client c when c.method_ = "textDocument/references" ->
        parse_find_references c.params |> do_find_references !conn
          |> print_find_references |> respond stdout c

      (* textDocument/documentHighlight *)
      | Main_loop, Client c when c.method_ = "textDocument/documentHighlight" ->
        parse_document_highlights c.params |> do_document_highlights !conn
          |> print_document_highlights |> respond stdout c

      (* textDocument/didOpen notification *)
      | Main_loop, Client c when c.method_ = "textDocument/didOpen" ->
        parse_did_open c.params |> do_did_open !conn;

      (* textDocument/didClose notification *)
      | Main_loop, Client c when c.method_ = "textDocument/didClose" ->
        parse_did_close c.params |> do_did_close !conn;

      (* textDocument/didChange notification *)
      | Main_loop, Client c when c.method_ = "textDocument/didChange" ->
        parse_did_change c.params |> do_did_change !conn;

      (* shutdown request *)
      | Main_loop, Client c when c.method_ = "shutdown" ->
        do_shutdown !conn |> print_shutdown |> respond stdout c;
        state := Post_shutdown;
        conn := None;

      (* textDocument/publishDiagnostics notification *)
      | Main_loop, Server ServerCommandTypes.DIAGNOSTIC (_, errors) ->
        let per_file uri errors = hack_errors_to_lsp_diagnostic uri errors
            |> print_diagnostics
            |> notify stdout "textDocument/publishDiagnostics"
        in
        SMap.iter per_file errors

      (* any server diagnostics that come after we've shut down *)
      | _, Server ServerCommandTypes.DIAGNOSTIC _ ->
        ()

      (* catch-all for client reqs/notifications we haven't yet implemented *)
      | Main_loop, Client _c ->
        raise (Error.Method_not_found "not implemented")

      (* catch-all for requests/notifications after shutdown request *)
      | Post_shutdown, Client _c ->
        raise (Error.Invalid_request "already received shutdown request")

      (* TODO message from server *)
      | _, Server ServerCommandTypes.NEW_CLIENT_CONNECTED -> () (* todo *)
      | _, Server_exit _ -> () (* todo *)

    with
    | e -> match event with
           | Client c when c.is_request -> print_error e |> respond stdout c
           | _ -> () (* todo: log this? *)
  done
