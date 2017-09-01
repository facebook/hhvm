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
  failwith @@ Printf.sprintf "Event not yet supported: %s" s

let stop_server root =
  let status = ClientStop.main { ClientStop.root = root; } in
  match status with
  | Exit_status.No_error -> ()
  | _ ->
    failwith "Stopping server failed"

let start_server root =
  let check_env = {
    ClientEnv.mode = ClientEnv.MODE_STATUS;
    root = root;
    from = "";
    output_json = false;
    retries = 800;
    timeout = None;
    autostart = true;
    force_dormant_start = false;
    no_load = false;
    profile_log = false;
    ai_mode = None;
  } in
  match ClientCheck.main check_env with
  | Exit_status.No_error ->
    ()
  | Exit_status.Type_error ->
    Printf.eprintf "Warning: server started with typecheck errors"
  | _ ->
    ()

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

(** Plays the event. Returns true if this event indicated the
 * end of recording. *)
let play_event event conn =
  let open Recorder_types in
  match event with
  | Start_recording _ ->
    failwith "unexpected preconnection event Start_recording"
  | Loaded_saved_state _ ->
    failwith "unexpected preconnection event Loaded_saved_state"
  | Fresh_vcs_state _ ->
    not_yet_supported "Fresh_vcs_state"
  | Typecheck ->
    not_yet_supported "Typecheck"
  | HandleServerCommand cmd ->
    playback_server_cmd conn cmd;
    false
  | Disk_files_modified files ->
    write_files_contents files;
    false
  | Stop_recording ->
    true

let rec playback recording conn =
  let event = Marshal.from_channel recording in
  let finished = play_event event conn in
  if finished then
    ()
  else
    playback recording conn

let rec preconnection_playback
skip_hg_update_on_load_state recording root =
  let event = Marshal.from_channel recording in
  let open Recorder_types in
  match event with
  | Start_recording _ ->
    preconnection_playback skip_hg_update_on_load_state recording root
  | Loaded_saved_state ( {
      filename = _filename;
      corresponding_base_revision;
      dirty_files;
      changed_while_parsing;
      build_targets;
    },
    _) ->
      stop_server root;
      if skip_hg_update_on_load_state then
        ()
      else
        Future.get @@
          Hg.update_to_base_rev corresponding_base_revision @@
          Path.to_string root;
      write_files_contents dirty_files;
      write_files_contents changed_while_parsing;
      write_files_contents build_targets;
      start_server root;
      let conn = ClientIde.connect_persistent { ClientIde.root = root }
        ~retries:800 in
      playback recording conn
  | Fresh_vcs_state _
  | Typecheck
  | HandleServerCommand _
  | Disk_files_modified _ ->
    (** Found a real event without loading a saved state. Initiate connection
     * and continue playback. *)
    start_server root;
    let conn = ClientIde.connect_persistent { ClientIde.root = root }
      ~retries:800 in
    ignore @@ play_event event conn;
    playback recording conn
  | Stop_recording ->
    ()

let check_build_id json =
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
    Printf.eprintf "Warning: Build ID mismatch\n%!"

let spin_record skip_hg_update_on_load_state recording_path root =
  Relative_path.set_path_prefix Relative_path.Root root;
  let recording = recording_path |> Path.to_string |> open_in in
  let json = Recorder.Header.parse_header recording in
  let () = check_build_id json in
  try preconnection_playback skip_hg_update_on_load_state recording root with
  | End_of_file ->
    ()
  | Failure s when s = "input_value: truncated object" ->
    ()
