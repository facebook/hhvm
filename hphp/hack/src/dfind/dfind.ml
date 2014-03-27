(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)
open DfindEnv

(*****************************************************************************)
(* Entry point. *)
(*****************************************************************************)

let usage = "dfind [-f] <directory> <handle>\n"

let start_server_if_not_running () =
  if not (DfindServer.is_running ())
  then begin
    let ready_in = DfindServer.fork () in
    (* The server will let us know when it is ready *)
    DfindServer.wait_for_server ready_in
  end
  else ()

let parse_options() =
  let check = ref false in
  let kill = ref false in
  let follow = ref false in
  let arg ref () = ref := true in
  let other = ref [] in
  let ping = ref false in
  let options =
    ["--check", Arg.Unit (arg check), "check the sanity of the server";
     "--kill", Arg.Unit (arg kill), "kills the server";
     "-kill", Arg.Unit (arg kill), "kills the server";
     "-f", Arg.Unit (arg follow), "follow (incremental mode)";
     "--ping", Arg.Unit (arg ping), "";
   ] in
  Arg.parse options (fun s -> other := s :: !other) usage;
  !check, !kill, !ping, !follow, (List.rev !other)

let rec follow_dir dir =
  try
    let next_dir = Unix.readlink dir in
    if String.length next_dir > 1 && next_dir.[0] = '/'
    then follow_dir next_dir
    else follow_dir (Filename.dirname dir ^ "/" ^ next_dir)
  with Unix.Unix_error (x, y, z) ->
    dir

let follow_dir dir =
  (* Getting rid of the / at the end *)
  let dir = follow_dir dir in
  if dir.[String.length dir - 1] = '/'
  then String.sub dir 0 (String.length dir - 1)
  else dir

let get_message () =
  let check, kill, ping, follow, args = parse_options() in
  if check
  then DfindServer.Check
  else if kill
  then DfindServer.Kill
  else if ping
  then DfindServer.Ping
  else
    match args with
    | [dir; handle] when follow ->
        let dir = follow_dir dir in
        DfindServer.Find_handle_follow (dir, handle)
    | [dir; handle] ->
        let dir = follow_dir dir in
        DfindServer.Find_handle (dir, handle)
    | _ ->
        Printf.fprintf stderr "%s\n" usage;
        exit 2

let main() =
  let msg = get_message() in
  start_server_if_not_running();
  let ic, oc = DfindServer.client_socket() in
  Marshal.to_channel oc msg [];
  flush oc;
  try
    while true do
      output_string stdout (input_line ic);
      output_char stdout '\n';
      flush stdout
    done;
  with End_of_file -> ()

let () = main()

