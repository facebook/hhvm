(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let num_build_retries = 120

type env = ServerMsg.build_opts

let rec connect env retries =
  try
    ClientUtils.connect env.ServerMsg.root
  with
  | ClientExceptions.Server_cant_connect ->
    Printf.printf "Can't connect to server, retrying.\n%!";
    if retries > 0
    then begin
      Unix.sleep 1;
      connect env (retries - 1)
    end
    else exit 2
  | ClientExceptions.Server_initializing ->
    Printf.printf "Server still initializing.\n%!";
    if retries > 0
    then begin
      Unix.sleep 1;
      connect env (retries - 1)
    end
    else exit 2

let main env =
  (* Check if a server is up *)
  if not (ClientUtils.server_exists env.ServerMsg.root)
  then ClientStart.start_server env.ServerMsg.root;
  let ic, oc = connect env num_build_retries in
  ServerMsg.cmd_to_channel oc (ServerMsg.BUILD env);
  try
    while true do
      print_endline (input_line ic)
    done
  with End_of_file ->
    ()
