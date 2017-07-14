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
open Ide_api_types
open Ide_message
open Ide_rpc_protocol_parser_types

module Cmd = ServerCommand
module Rpc = ServerCommandTypes
module SMUtils = ServerMonitorUtils

type env = {
  root: Path.t;
}

(** For slow function call "f x", we want to consume some retries
 * and return the number of retries remaning.*)
let consume_retries ~retries f x =
  let start_t = Unix.gettimeofday () in
  let result = f x in
  let elapsed_t = int_of_float (Unix.gettimeofday () -. start_t) in
  let retries = retries - elapsed_t in
  if retries < 0
  then raise Exit_status.(Exit_with Out_of_retries)
  else retries, result

(* Configuration to use before / in absence of init request *)
let did_init = ref false
let init_version = ref Ide_rpc_protocol_parser_types.V0
let init_protocol = ref Ide_rpc_protocol_parser_types.Nuclide_rpc

let rec connect_persistent env retries start_time =
  if retries < 0 then raise Exit_status.(Exit_with Out_of_retries);
  let connect_once_start_t = Unix.time () in
  let handoff_options = {
    MonitorRpc.server_name = HhServerMonitorConfig.Program.hh_server;
    force_dormant_start = false;
  } in
  let retries, conn = consume_retries ~retries
    (ServerUtils.connect_to_monitor ~timeout:retries env.root) handoff_options
  in
  HackEventLogger.client_connect_once connect_once_start_t;
  match conn with
  | Result.Ok (ic, oc) ->
      (try
        ClientConnect.wait_for_server_hello ic (Some retries)
          ClientConnect.tty_progress_reporter start_time None;
      with
      | ClientConnect.Server_hung_up ->
        Exit_status.exit Exit_status.No_server_running
      );
      (ic, oc)
  | Result.Error e ->
    match e with
    | SMUtils.Monitor_connection_failure
    | SMUtils.Monitor_socket_not_ready
      when retries > 0 -> connect_persistent env (retries-1) start_time
    | SMUtils.Monitor_establish_connection_timeout
    | SMUtils.Monitor_connection_failure
    | SMUtils.Monitor_socket_not_ready ->
      raise Exit_status.(Exit_with IDE_out_of_retries)
    | SMUtils.Server_dormant
    | SMUtils.Server_died
    | SMUtils.Server_missing
    | SMUtils.Build_id_mismatched _ ->
      (* IDE mode doesn't handle (re-)starting the server - needs to be done
       * separately with hh start or similar. *)
      raise Exit_status.(Exit_with IDE_no_server)

let read_connection_response fd =
  let res = Marshal_tools.from_fd_with_preamble fd in
  match res with
  | ServerCommandTypes.Connected -> ()

let connect_persistent env ~retries =
  let start_time = Unix.time () in
  try
    let (ic, oc) = connect_persistent env retries start_time in
    HackEventLogger.client_established_connection start_time;
    Cmd.send_connection_type oc ServerCommandTypes.Persistent;
    read_connection_response (Unix.descr_of_out_channel oc);
    (ic, oc)
  with
  | e ->
    HackEventLogger.client_establish_connection_exception e;
    raise e

let malformed_input () =
  raise Exit_status.(Exit_with IDE_malformed_request)

let pending_push_messages = Queue.create ()
let stdin_idle = ref false
let stdin_reader = Buffered_line_reader.create Unix.stdin

let rpc conn command =
  let res, push_messages = Cmd.rpc_persistent conn command in
  List.iter push_messages (fun x -> Queue.push x pending_push_messages);
  res

let read_push_message_from_server fd : ServerCommandTypes.push =
  let open ServerCommandTypes in
  match Marshal_tools.from_fd_with_preamble fd with
  | ServerCommandTypes.Response _ ->
    failwith "unexpected response without a request"
  | Push m -> m
  | ServerCommandTypes.Hello ->
    failwith "unexpected hello after connection already established"

let get_next_push_message fd =
  if Queue.is_empty pending_push_messages
    then read_push_message_from_server fd
    else Queue.take pending_push_messages

let server_disconnected () =
  raise Exit_status.(Exit_with No_error)

let read_request () =
  try Buffered_line_reader.get_next_line stdin_reader
  with Unix.Unix_error _ -> malformed_input ()

let write_response res =
  Printf.printf "%s\n" res;
  flush stdout

let get_ready_message server_in_fd =
  if not @@ Queue.is_empty pending_push_messages then `Server else
  if Buffered_line_reader.has_buffered_content stdin_reader then `Stdin else

  let stdin_fd = Buffered_line_reader.get_fd stdin_reader in

  let change_to_idle = (not @@ !stdin_idle) && begin
    let readable, _, _ = Unix.select [stdin_fd] [] [] 0.0 in
    readable = []
  end in

  if change_to_idle then begin
    stdin_idle := true; `Idle
  end else
  let readable, _, _ = Unix.select [server_in_fd; stdin_fd] [] [] 1.0 in
  if readable = [] then `None
  else if List.mem readable server_in_fd then `Server
  else
    (stdin_idle := false; `Stdin)

let print_message id protocol message =
  Ide_message_printer.to_json
    ~id ~protocol ~version:!init_version ~message |>
  Hh_json.json_to_string |>
  write_response

let print_response id protocol response =
  print_message id protocol (Response response)

let print_push_message protocol notification =
  (* Push notifications are requests without ID field *)
  print_message None protocol (Request (Server_notification notification))

let handle_push_message = function
  | ServerCommandTypes.DIAGNOSTIC (subscription_id, errors) ->
    SMap.iter begin fun diagnostics_notification_filename diagnostics ->
      print_push_message !init_protocol @@
      Diagnostics_notification {
        subscription_id;
        diagnostics_notification_filename;
        diagnostics;
      }
    end errors
  | ServerCommandTypes.FATAL_EXCEPTION _ ->
    Printf.eprintf "Fatal server error. Exiting.\n";
    raise Exit_status.(Exit_with Uncaught_exception)
  | ServerCommandTypes.NEW_CLIENT_CONNECTED ->
    Printf.eprintf "Another persistent client have connected. Exiting.\n";
    raise Exit_status.(Exit_with IDE_new_client_connected)

let handle_error id protocol error =
  match protocol with
  | Nuclide_rpc ->
    (* We never got to implementing "real" error handling for Nuclide-rpc,
     * and there is no point now *)
    Printf.eprintf "%s\n" (Ide_rpc_protocol_parser.error_t_to_string error);
    flush stderr
  | JSON_RPC2 ->
    Json_rpc_message_printer.error_to_json ~id ~error |>
    Hh_json.json_to_string |>
    write_response

let with_id_required id protocol f =
  match id with
  | Some id -> f id
  | None -> handle_error id protocol
      (Internal_error "Id field is required for this request")

let handle_init conn id protocol { client_name=_; client_api_version=_; } =
  if !did_init then
    handle_error id protocol (Server_error "init was already called")
  else begin
    (* Nuclide-rpc allows you to subscribe/unsubscribe from diagnostics at
     * any moment. Not sure if this is useful, so for now just keeping everyone
     * subscribed from the start *)
    rpc conn (Rpc.SUBSCRIBE_DIAGNOSTIC 0);
    did_init := true;
    (* The only version there is at the moment *)
    init_version := V0;
    init_protocol := protocol;
    Init_response {
      server_api_version = Ide_rpc_protocol_parser.version_to_int !init_version;
    } |>
    print_response id protocol
  end

let file_position_to_tuple {filename; position={line; column}} =
  filename, line, column

let handle_request conn id protocol = function
  | Init init_params ->
    handle_init conn id protocol init_params
  | Autocomplete { filename; position; } ->
    let delimit_on_namespaces = false in
    rpc conn (Rpc.IDE_AUTOCOMPLETE (filename, position, delimit_on_namespaces)) |>
    AutocompleteService.autocomplete_result_to_ide_response |>
    print_response id protocol
  | Infer_type args ->
    let filename, line, column = file_position_to_tuple args in
    let filename = ServerUtils.FileName filename in
    rpc conn (Rpc.INFER_TYPE (filename, line, column)) |>
    InferAtPosService.infer_result_to_ide_response |>
    print_response id protocol
  | Identify_symbol args ->
    let filename, line, column = file_position_to_tuple args in
    let filename = ServerUtils.FileName filename in
    rpc conn (Rpc.IDENTIFY_FUNCTION (filename, line, column)) |>
    IdentifySymbolService.result_to_ide_message |>
    print_response id protocol
  | Outline filename ->
    let result = rpc conn (Rpc.OUTLINE filename) in
    Ide_message.Outline_response result |>
    print_response id protocol
  | Find_references args ->
    let filename, line, column = file_position_to_tuple args in
    let filename = ServerUtils.FileName filename in
    let include_defs = false in
    rpc conn (Rpc.IDE_FIND_REFS (filename, line, column, include_defs)) |>
    FindRefsService.result_to_ide_message |>
    print_response id protocol
  | Highlight_references args ->
    let filename, line, column = file_position_to_tuple args in
    let filename = ServerUtils.FileName filename in
    let r = rpc conn (Rpc.IDE_HIGHLIGHT_REFS (filename, line, column)) in
    print_response id protocol (Highlight_references_response r)
  | Format args ->
    begin match rpc conn (Rpc.IDE_FORMAT (ServerFormatTypes.Range args)) with
      | Result.Ok r -> print_response id protocol
                         (Format_response r.ServerFormatTypes.new_text)
      | Result.Error e -> handle_error id protocol (Server_error e)
    end
  | Coverage_levels filename ->
    let filename = ServerUtils.FileName filename in
    rpc conn (Rpc.COVERAGE_LEVELS filename) |>
    Coverage_level.result_to_ide_message |>
    print_response id protocol
  | Did_open_file { did_open_file_filename; did_open_file_text; } ->
    rpc conn (Rpc.OPEN_FILE (did_open_file_filename, did_open_file_text))
  | Did_close_file { did_close_file_filename; } ->
    rpc conn (Rpc.CLOSE_FILE did_close_file_filename)
  | Did_change_file { did_change_file_filename; changes; } ->
    rpc conn (Rpc.EDIT_FILE (did_change_file_filename, changes))
  | Disconnect ->
    rpc conn (Rpc.DISCONNECT);
    server_disconnected ()
  | Subscribe_diagnostics ->
    with_id_required id protocol (fun id ->
      rpc conn (Rpc.SUBSCRIBE_DIAGNOSTIC id)
    )
  | Ide_message.Unsubscribe_call ->
    with_id_required id protocol (fun id ->
      rpc conn (Rpc.UNSUBSCRIBE_DIAGNOSTIC id)
    )
  | Sleep_for_test ->
    Unix.sleep 1

let handle_stdin conn =
  let { id; result; protocol } = Ide_message_parser.parse
    ~message:(read_request ())
    ~version:!init_version
  in
  let protocol = match protocol with
    | Result.Ok protocol -> protocol
    | Result.Error _ -> !init_protocol
  in
  let id = match id with
    | Result.Ok x -> x
    | Result.Error _ -> None
  in
  match result with
  | Result.Ok request -> handle_request conn id protocol request
  | Result.Error e -> handle_error id protocol e

let handle_server fd =
  begin try get_next_push_message fd with
    | Marshal_tools.Reading_Preamble_Exception
    | Unix.Unix_error _ -> server_disconnected ()
  end
  |> handle_push_message

let handle_idle conn =
  rpc conn (Rpc.IDE_IDLE)

let main env =
  Printexc.record_backtrace true;
  let conn = connect_persistent env ~retries:800 in
  let fd = Unix.descr_of_out_channel (snd conn) in
  while true do
    match get_ready_message fd with
    | `None -> ()
    | `Stdin -> handle_stdin conn
    | `Server -> handle_server fd
    | `Idle -> handle_idle conn
  done;
  Exit_status.exit Exit_status.No_error
