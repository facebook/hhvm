(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type env = {
  client : (in_channel * out_channel) option;
  (* When this is true, we are between points when typechecker process
   * sent us RunIdeCommands and StopIdeCommands, and it's safe to use
   * data from shared heap. *)
  run_ide_commands : bool;
}

let empty_env = {
  client = None;
  run_ide_commands = false;
}

let get_ready_channel monitor_ic client typechecker =
  let monitor_in_fd = Daemon.descr_of_in_channel monitor_ic in
  let typechecker_in_fd = typechecker.IdeProcessPipe.in_fd in
  match client with
  | None ->
    let readable, _, _ =
      Unix.select [monitor_in_fd; typechecker_in_fd] [] [] 1.0 in
    if readable = [] then `None
    else if List.mem typechecker_in_fd readable then `Typechecker
    else  `Monitor
  | Some ((client_ic, _) as client) ->
    let client_in_fd = Unix.descr_of_in_channel client_ic in
    let readable, _, _ =
      Unix.select [monitor_in_fd; client_in_fd; typechecker_in_fd] [] [] 1.0 in
    if readable = [] then `None
    else if List.mem typechecker_in_fd readable then `Typechecker
    else if List.mem client_in_fd readable then `Client client
    else `Monitor

let handle_already_has_client oc =
  let response = Hh_json.(json_to_string (
    JSON_Object [
      ("type", JSON_String "response");
      ("success", JSON_Bool false);
      ("message", JSON_String
         ("There already is a client connected in IDE mode.")
      );
    ]
  )) in
  Marshal.to_channel oc response [];
  close_out oc

let handle_new_client env parent_ic =
  let parent_in_fd = Daemon.descr_of_in_channel parent_ic in
  let socket = Libancillary.ancil_recv_fd parent_in_fd in
  let ic, oc =
    (Unix.in_channel_of_descr socket), (Unix.out_channel_of_descr socket) in
  match env.client with
  | None ->
    Hh_logger.log "Connected new client";
    { env with client = Some (ic, oc) }
  | Some _ ->
    Hh_logger.log "Rejected a client";
    handle_already_has_client oc; env

let handle_client_request env (ic, oc) =
  Hh_logger.log "Handling client request";
  let request = Marshal.from_channel ic in
  let response = Hh_json.(json_to_string (
    JSON_Object [
      ("type", JSON_String "response");
      ("success", JSON_Bool false);
      ("message", JSON_String (
         "Server busy: " ^ (if env.run_ide_commands then "false" else "true") ^
         ", request: " ^ request
      ));
    ]
  )) in
  Marshal.to_channel oc response [];
  flush oc;
  env

let handle_typechecker_message env typechecker_process =
  match IdeProcessPipe.recv typechecker_process with
  | IdeProcessMessage.RunIdeCommands ->
    { env with run_ide_commands = true }
  | IdeProcessMessage.StopIdeCommands ->
    IdeProcessPipe.send typechecker_process IdeProcessMessage.IdeCommandsDone;
    { env with run_ide_commands = false }

let handle_gone_client env =
  Hh_logger.log "Client went away";
  { env with client = None }

let daemon_main _ (parent_ic, _parent_oc) =
  Printexc.record_backtrace true;
  let parent_in_fd = Daemon.descr_of_in_channel parent_ic in
  let typechecker_process = IdeProcessPipeInit.ide_recv parent_in_fd in
  let env = ref empty_env in
  while true do
    ServerMonitorUtils.exit_if_parent_dead ();
    let new_env = try
        match get_ready_channel parent_ic !env.client typechecker_process with
        | `None -> !env
        | `Typechecker -> handle_typechecker_message !env typechecker_process
        | `Monitor -> handle_new_client !env parent_ic
        | `Client c -> handle_client_request !env c
      with
      | End_of_file
      | Sys_error _ ->
        (* client went away in the meantime *)
        handle_gone_client !env
    in
    env := new_env
  done

let entry =
  Daemon.register_entry_point "IdeMain.daemon_main" daemon_main
