(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ClientExceptions

let get_hhserver () =
  try
    let p = Unix.getenv "HH_HOME" in
    p ^ "/hh_server"
  with Not_found ->
    "hh_server"

type env = {
  root: Path.path;
  wait: bool;
}

let rec wait env =
  begin try 
    Unix.sleep(1);
    ignore(ClientUtils.connect env.root);
    Printf.fprintf stderr "Done waiting!\n%!"
  with 
  | Server_initializing ->
    Printf.fprintf stderr "Waiting for server to initialize\n%!";
    wait env
  | e -> 
    Printf.fprintf stderr
      "Error: something went wrong while waiting for the server to start up\n%s\n%!"
      (Printexc.to_string e);
    exit 77
  end

let start_server env = 
  Printf.fprintf stderr "Server launched for %s\n%!" 
    (Path.string_of_path env.root);
  let hh_server =  Printf.sprintf "%s -d %s"
    (get_hhserver())
    (Path.string_of_path env.root) in
  ignore(Unix.system hh_server);
  if env.wait then wait env;
  ()

let should_start env = 
  if ClientUtils.server_exists env.root
  then begin
    try
      (* Let's ping the server to make sure it's up and not out of date *)
      Sys.set_signal Sys.sigalrm (Sys.Signal_handle (fun _ -> raise Server_busy));
      ignore(Unix.alarm 6);
      let ic, oc = ClientUtils.connect env.root in
      ServerMsg.cmd_to_channel oc ServerMsg.PING;
      let response = ServerMsg.response_from_channel ic in
      ignore (Unix.alarm 0);
      match response with 
      | ServerMsg.PONG -> false
      | ServerMsg.SERVER_OUT_OF_DATE -> 
          Printf.fprintf 
            stderr
            "Replacing out of date server for %s\n%!"
            (Path.string_of_path env.root);
          ignore(Unix.sleep 1);
          true 
      | ServerMsg.SERVER_DYING 
      | ServerMsg.NO_ERRORS
      | ServerMsg.ERRORS _
      | ServerMsg.DIRECTORY_MISMATCH _ ->
        let r = (ServerMsg.response_to_string response) in
        failwith ("Unexpected response from the server: "^r)
    with 
      | Server_busy -> 
          Printf.fprintf 
            stderr 
            "Replacing busy server for %s\n%!"
            (Path.string_of_path env.root);
          ClientStop.kill_server env.root;
          true
      | Server_initializing -> 
          Printf.fprintf 
            stderr 
            "Found initializing server for %s\n%!"
            (Path.string_of_path env.root);
          false
      | Server_cant_connect -> 
          Printf.fprintf 
            stderr 
            "Replacing unresponsive server for %s\n%!"
            (Path.string_of_path env.root);
          ClientStop.kill_server env.root;
          true
  end else true

let main env = 
  if should_start env
  then start_server env
  else begin
    Printf.fprintf
      stderr
      "Error: Server already exists for %s\n\
      Use hh restart if you want to kill it and start a new one\n%!"
      (Path.string_of_path env.root);
    exit 77
  end

let start_server ?wait:(wait=false) root = 
  start_server {root = root; wait = wait;}
