(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let not_yet_supported s =
  Printf.eprintf "Type not yet supported: %s" s

let playback_server_cmd : type a. Timeout.in_channel * out_channel ->
  a ServerCommandTypes.command -> unit = fun conn cmd ->
  match cmd with
  | ServerCommandTypes.Rpc rpc ->
    let _ = ClientIde.rpc conn rpc in
    ()
  | ServerCommandTypes.Stream _ ->
    not_yet_supported "ServerCommandTypes.Stream"
  | ServerCommandTypes.Debug ->
    not_yet_supported "ServerCommandTypes.Debug"

let write_file_contents filename contents =
  let filename = Relative_path.to_absolute filename in
  let oc = Pervasives.open_out filename in
  let f () = output_string oc contents in
  let finally () = close_out oc in
  Utils.try_finally ~f ~finally

let write_files_contents files =
  Relative_path.Map.iter files ~f:(fun filename contents ->
    write_file_contents filename contents
  )

let rec playback recording conn =
  let x = Marshal.from_channel recording in
  let open Recorder_types in
  let () = match x with
  | Start_recording init_env ->
    let open Relative_path in
    set_path_prefix Root init_env.root_path;
    set_path_prefix Hhi init_env.hhi_path
  | Loaded_saved_state _ ->
    (** TODO *)
    playback recording conn
  | Fresh_vcs_state _ ->
    not_yet_supported "Fresh_vcs_state"
  | Typecheck ->
    not_yet_supported "Typecheck"
  | HandleServerCommand cmd ->
    playback_server_cmd conn cmd
  | Disk_files_modified files ->
    write_files_contents files
  | Stop_recording ->
    not_yet_supported "Stop_recording"
  in
  playback recording conn

let check_build_id json =
  let open Hh_json.Access in
  (return json)
    >>= get_string "build_id"
    |> project
      (fun build_id ->
        if build_id = Build_id.build_id_ohai
        then
          ()
        else
          Printf.eprintf "Warning: Build ID mismatch\n%!";
      )
      (fun e ->
        Printf.eprintf "%s\n%!" (access_failure_to_string e);
        ())

let spin_record recording_path root =
  let conn = ClientIde.connect_persistent { ClientIde.root = root }
    ~retries:800 in
  let recording = recording_path |> Path.to_string |> open_in in
  let json = Recorder.Header.parse_header recording in
  let () = check_build_id json in
  try playback recording conn with
  | End_of_file ->
    ()
  | Failure s when s = "input_value: truncated object" ->
    ()
