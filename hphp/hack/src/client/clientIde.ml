(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module SMUtils = ServerMonitorUtils

type env = {
  root: Path.t;
}

let rec connect env ~retries =
  let conn = try ServerUtils.connect_to_monitor
        env.root HhServerMonitorConfig.Program.ide_server with
  | SMUtils.Last_server_died ->
    raise Exit_status.(Exit_with IDE_no_server) in
  match conn with
  | Result.Ok (ic, oc) ->
    SMUtils.send_ide_client_type oc SMUtils.Persistent;
    (ic, oc)
  | Result.Error e -> begin match e with
    | SMUtils.Monitor_connection_failure
    | SMUtils.Server_busy when retries > 0 -> connect env ~retries:(retries-1)
    | SMUtils.Monitor_connection_failure
    | SMUtils.Server_busy ->
      raise Exit_status.(Exit_with IDE_out_of_retries)
    | SMUtils.Server_died
    | SMUtils.Server_missing
    | SMUtils.Build_id_mismatched ->
      (* IDE mode doesn't handle (re-)starting the server - needs to be done
       * separately with hh start or similar. *)
      raise Exit_status.(Exit_with IDE_no_server)
  end

let malformed_input () =
  raise Exit_status.(Exit_with IDE_malformed_request)

let read_server_message fd : string =
  Marshal_tools.from_fd_with_preamble fd

let server_disconnected () =
  raise Exit_status.(Exit_with Ok)

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

let main env =
  Printexc.record_backtrace true;
  let (ic, oc) = connect env ~retries:3 in
  let in_fd = Timeout.descr_of_in_channel ic in
  while true do
    match get_ready_channel in_fd with
    | `None -> ()
    | `Stdin ->
      let request = read_request () in
      Marshal.to_channel oc request [];
      flush oc;
    | `Server ->
      let res = try read_server_message in_fd with
        | Marshal_tools.Reading_Preamble_Exception
        | Unix.Unix_error _ -> server_disconnected ()
      in
      write_response res;
  done;
  Exit_status.exit Exit_status.Ok
