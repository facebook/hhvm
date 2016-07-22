(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open IdeJson

module Cmd = ServerCommand
module Rpc = ServerRpc
module SMUtils = ServerMonitorUtils

type env = {
  root: Path.t;
}

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

let connect_persistent env ~retries =
  let start_time = Unix.time () in
  try
    let (ic, oc) = connect_persistent env retries start_time in
    HackEventLogger.client_established_connection start_time;
    Cmd.send_connection_type oc Cmd.Persistent;
    (ic, oc)
  with
  | e ->
    HackEventLogger.client_establish_connection_exception e;
    raise e

let malformed_input () =
  raise Exit_status.(Exit_with IDE_malformed_request)

let read_server_message fd : string =
  Marshal_tools.from_fd_with_preamble fd

let read_connection_response fd =
  let res = Marshal_tools.from_fd_with_preamble fd in
  match res with
  | Cmd.Persistent_client_alredy_exists ->
    raise Exit_status.(Exit_with IDE_persistent_client_already_exists)
  | Cmd.Persistent_client_connected -> ()

let server_disconnected () =
  raise Exit_status.(Exit_with No_error)

let read_request () =
  try read_line () with End_of_file ->
    malformed_input ()

let write_response res =
  Printf.printf "%s\n" res;
  flush stdout

let get_ready_channel server_in_fd =
  let stdin_fd = Unix.descr_of_in_channel stdin in
  let readable, _, _ = Unix.select [server_in_fd; stdin_fd] [] [] 1.0 in
  if readable = [] then `None
  else if List.mem server_in_fd readable then `Server
  else `Stdin

let handle oc id call =
match call with
| Auto_complete_call (path, pos) ->
  let raw_result =
    Cmd.rpc_persistent oc (Rpc.IDE_AUTOCOMPLETE (path, pos)) in
  let result =
    List.map AutocompleteService.autocomplete_result_to_json raw_result in
  let result_field = (Hh_json.JSON_Array result) in
  print_endline @@ IdeJsonUtils.json_string_of_response id
    (Auto_complete_response result_field)
| Open_file_call path ->
  Cmd.rpc_persistent oc (Rpc.OPEN_FILE path)
| Close_file_call path ->
  Cmd.rpc_persistent oc (Rpc.CLOSE_FILE path)
| Edit_file_call (path, edits) ->
  Cmd.rpc_persistent oc (Rpc.EDIT_FILE (path, edits))
| Disconnect_call ->
  Cmd.rpc_persistent oc (Rpc.DISCONNECT);
  server_disconnected ()

let main env =
  Printexc.record_backtrace true;
  let ic, oc = connect_persistent env ~retries:800 in
  let in_fd = Timeout.descr_of_in_channel ic in
  read_connection_response in_fd;
  while true do
    match get_ready_channel in_fd with
    | `None -> ()
    | `Stdin ->
      let request = read_request () in
      begin
      match IdeJsonUtils.call_of_string request with
      | Call (id, call) ->
        handle oc id call
      | Invalid_call (id, msg) ->
        print_endline msg
      | Parsing_error msg ->
        print_endline msg
      end
    | `Server ->
      let res = try read_server_message in_fd with
        | Marshal_tools.Reading_Preamble_Exception
        | Unix.Unix_error _ -> server_disconnected ()
      in
      write_response res;
  done;
  Exit_status.exit Exit_status.No_error
