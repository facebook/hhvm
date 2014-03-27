(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

exception FailedToKill

type env = {
  root: Path.path;
}

let p2s = Path.string_of_path

let nice_kill env =
  Printf.fprintf stderr "Attempting to nicely kill server for %s\n%!"
    (p2s env.root);
  let response = try 
    Sys.set_signal 
      Sys.sigalrm 
      (Sys.Signal_handle (fun _ -> raise ClientExceptions.Server_busy));
    ignore(Unix.alarm 6);

    let ic, oc = ClientUtils.connect env.root in
    let command = ServerMsg.KILL in
    ServerMsg.cmd_to_channel oc command;
    let response = ServerMsg.response_from_channel ic in
    ignore (Unix.alarm 0);
    response
  with e -> begin
    Printf.fprintf stderr "%s\n%!" (Printexc.to_string e);
    raise FailedToKill
  end in
  match response with 
  | ServerMsg.SERVER_OUT_OF_DATE
  | ServerMsg.SERVER_DYING ->
      ignore(Unix.sleep 1);
      if ClientUtils.server_exists env.root
      then raise FailedToKill
      else Printf.fprintf stderr "Successfully killed server for %s\n%!" (p2s env.root)
  | e ->
      Printf.fprintf stderr "Unexpected response from the server: %s\n"
        (ServerMsg.response_to_string response);
      raise FailedToKill

let mean_kill env =
  Printf.fprintf stderr "Attempting to meanly kill server for %s\n%!"
    (p2s env.root);
  let pids = 
    try PidLog.get_pids env.root 
    with PidLog.FailedToGetPids ->
      Printf.fprintf stderr "Unable to figure out pids of running Hack server. \
        Try manually killing it with 'pkill hh_server' (be careful on shared \
        devservers)\n%!";
      raise FailedToKill
  in
  List.iter (fun (pid, reason) -> Unix.kill pid 9) pids;
  ignore(Unix.sleep 1);
  if ClientUtils.server_exists env.root
  then raise FailedToKill
  else Printf.fprintf stderr "Successfully killed server for %s\n%!" (p2s env.root)

let kill_server env = 
  Printf.fprintf stderr "Killing server for %s\n%!" 
    (p2s env.root);
  try nice_kill env
  with FailedToKill ->
    Printf.fprintf stderr "Failed to kill server nicely for %s\n%!"
      (p2s env.root);
    try mean_kill env
    with FailedToKill ->
      Printf.fprintf stderr "Failed to kill server meanly for %s\n%!"
        (p2s env.root);
      exit 1

let main env =
  if ClientUtils.server_exists env.root
  then kill_server env
  else Printf.fprintf stderr "Error: no server to kill for %s\n%!"
    (p2s env.root)

let kill_server root = kill_server {root = root;}
