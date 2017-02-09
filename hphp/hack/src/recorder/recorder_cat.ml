(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** The recording file contains marshaled Ocaml objects, which aren't
 * readable by non-Ocaml systems. This binary cats them to a human-readable
 * string. *)

module Args = struct

  type t = {
    recording : Path.t;
  }

  let usage = Printf.sprintf "Usage: %s [Recording path]\n" Sys.argv.(0)

  let parse () =
    let recording = ref None in
    let () = Arg.parse [] (fun s -> recording := (Some s)) usage in
    match !recording with
    | None ->
      Printf.eprintf "%s" usage;
      exit 1
    | Some recording ->
      { recording = Path.make recording; }

  let recording args = args.recording

end;;

let rec print_events channel =
  let event = try Marshal.from_channel channel with
  | End_of_file ->
    flush stdout;
    exit 0
  | Failure s when s = "input_value: truncated object" ->
    flush stdout;
    exit 0
  in
  Printf.printf "%s\n" (Recorder_types.to_string event);
  print_events channel


let cat_recording channel =
  let json = Recorder.Header.parse_header channel in
  let open Hh_json.Access in
  let build_id = (return json)
    >>= get_string "build_id"
    |> counit_with @@ (fun e ->
      Printf.eprintf "%s\n%!" (access_failure_to_string e);
      exit 1)
  in
  if build_id = Build_id.build_id_ohai
  then
    ()
  else
    Printf.eprintf "Warning: Build ID mismatch\n%!";
  print_events channel

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let args = Args.parse () in
  let recording = Unix.openfile (Path.to_string (Args.recording args))
  [Unix.O_RDONLY]
  0o640
  in
  let recording = Unix.in_channel_of_descr recording in
  cat_recording recording
