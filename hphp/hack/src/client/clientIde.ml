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
open Ide_message
open Ide_rpc_protocol_parser_types

module Cmd = ServerCommand
module Rpc = ServerCommandTypes
module SMUtils = ServerMonitorUtils

type env = {
  root: Path.t;
}

(* Configuration to use before / in absence of init request *)
let default_version = Ide_rpc_protocol_parser_types.V0
let default_protocol = Ide_rpc_protocol_parser_types.Nuclide_rpc

let rec connect_persistent env retries start_time =
  if retries < 0 then raise Exit_status.(Exit_with Out_of_retries);
  let connect_once_start_t = Unix.time () in

  let server_name = HhServerMonitorConfig.Program.hh_server in

  let conn = ServerUtils.connect_to_monitor env.root server_name in
  HackEventLogger.client_connect_once connect_once_start_t;
  match conn with
  | Result.Ok (ic, oc) ->
      (try
        ClientConnect.wait_for_server_hello ic env (Some retries)
          start_time None true;
      with
      | ClientConnect.Server_hung_up ->
        Exit_status.exit Exit_status.No_server_running
      );
      (ic, oc)
  | Result.Error e ->
    match e with
    | SMUtils.Monitor_connection_failure
    | SMUtils.Server_busy
      when retries > 0 -> connect_persistent env (retries-1) start_time
    | SMUtils.Monitor_connection_failure
    | SMUtils.Server_busy ->
      raise Exit_status.(Exit_with IDE_out_of_retries)
    | SMUtils.Server_died
    | SMUtils.Server_missing
    | SMUtils.Build_id_mismatched ->
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
let stdin_reader = Buffered_line_reader.create Unix.stdin

let rpc conn command =
  let res, push_messages = Cmd.rpc_persistent conn command in
  List.iter push_messages (fun x -> Queue.push x pending_push_messages);
  res

let read_push_message_from_server fd : ServerCommandTypes.push =
  let open ServerCommandTypes in
  match Marshal_tools.from_fd_with_preamble fd with
  | Response s -> failwith "unexpected response without a request"
  | Push m -> m

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
  let readable, _, _ = Unix.select
    [server_in_fd; Buffered_line_reader.get_fd stdin_reader] [] [] 1.0 in
  if readable = [] then `None
  else if List.mem readable server_in_fd then `Server
  else `Stdin

let print_response id protocol response =
  Ide_message_printer.to_json
    ~id ~protocol ~response ~version:default_version |>
  Hh_json.json_to_string |>
  write_response

let print_push_message =
  (* Push notifications are responses without ID field *)
  print_response None

let handle_push_message = function
  | ServerCommandTypes.DIAGNOSTIC (subscription_id, errors) ->
    SMap.iter begin fun diagnostics_notification_filename diagnostics ->
      print_push_message default_protocol @@
      Diagnostics_notification {
        subscription_id;
        diagnostics_notification_filename;
        diagnostics;
      }
    end errors
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
    Json_rpc_message_printer.response_to_json ~id ~result:(`Error error) |>
    Hh_json.json_to_string |>
    write_response

let with_id_required id protocol f =
  match id with
  | Some id -> f id
  | None -> handle_error id protocol
      (Internal_error "Id field is required for this request")

let handle_request conn id protocol = function
  | Autocomplete { filename; position; } ->
    rpc conn (Rpc.IDE_AUTOCOMPLETE (filename, position)) |>
    AutocompleteService.autocomplete_result_to_ide_response |>
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
    ~version:default_version
  in
  let protocol = match protocol with
    | Result.Ok protocol -> protocol
    | Result.Error _ -> default_protocol
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

let main env =
  Printexc.record_backtrace true;
  let conn = connect_persistent env ~retries:800 in
  let fd = Unix.descr_of_out_channel (snd conn) in
  while true do
    match get_ready_message fd with
    | `None -> ()
    | `Stdin -> handle_stdin conn
    | `Server -> handle_server fd
  done;
  Exit_status.exit Exit_status.No_error
