(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Recorder_types

module Header = struct
  (** This should never be changed because we read exactly these many bytes
   * when opening a file.. *)
  let header_open_tag size =
    Printf.sprintf "<hack-recording-header size=%05d>" size
  let header_close_tag = "</hack-recording-header>"

  let open_tag_width =
    let example = header_open_tag 1 in
    String.length example

  let make_header ?for_revision build_id =
    let for_revision = match for_revision with
    | None -> ""
    | Some for_revision ->
      Printf.sprintf "\"loaded_state_for_base_revision\" : \"%s\"," for_revision
    in
    let build_id = Printf.sprintf "\"build_id\" : \"%s\"" build_id in
    let blob = Printf.sprintf "{ %s %s }" for_revision build_id in
    Printf.sprintf "%s%s%s\n" (header_open_tag (String.length blob))
      blob header_close_tag

  exception Invalid_header

  let blob_size open_tag =
    let re = "<hack-recording-header size=\\([0-9]+\\)>" in
    let re = Str.regexp re in
    if Str.string_match re open_tag 0
    then
      try int_of_string (Str.matched_group 1 open_tag) with
      | Not_found -> raise Invalid_header
    else
      raise Invalid_header

  let parse_header ic =
    let open_tag = really_input_string ic open_tag_width in
    let size = blob_size open_tag in
    let blob = really_input_string ic size in
    let close_tag = really_input_string ic (String.length header_close_tag) in
    if close_tag = header_close_tag
    then
      let result = try Hh_json.json_of_string ~strict:true blob with
      | Hh_json.Syntax_error _ -> raise Invalid_header
      in
      let newline = really_input_string ic 1 in
      if newline = "\n" then
        result
      else
        raise Invalid_header
    else
      raise Invalid_header

end

module DE = Debug_event

(** Recorder handles the stream of debug events from hack to
 * create a valid recording. It does things like ignoring events
 * until a valid recording can begin, catting the actual contents
 * of files from disk, attempting to detect when a recording goes bad,
 * and identifying and summarizing version control checkout changes
 * (to avoid catting file contents between checkouts). *)

let file_extension = "hrec"

type env = {
  start_time : float;
  (** Reversed list of events. i.e., the most-recent event is first in the
   * list. *)
  rev_buffered_recording: event list;
}

type instance =
  | Switched_off
  | Active of env
  (** Hack server loaded a saved state. *)
  | Active_and_loaded_saved_state of string * env
  | Finished of event list
  | Finished_and_loaded_saved_state of string * (event list)

let is_finished instance = match instance with
  | Finished _ -> true
  | Finished_and_loaded_saved_state _ -> true
  | _ -> false

let get_events instance = match instance with
  | Finished events -> events
  | Finished_and_loaded_saved_state (_, events) -> events
  | Switched_off -> []
  | Active_and_loaded_saved_state (_, env)
  | Active env -> List.rev env.rev_buffered_recording

let get_header instance = match instance with
  | Active_and_loaded_saved_state (for_revision, _)
  | Finished_and_loaded_saved_state (for_revision, _) ->
    Header.make_header ~for_revision Build_id.build_id_ohai
  | Switched_off | Active _ | Finished _ ->
    Header.make_header Build_id.build_id_ohai

let start init_env =
  let start = Unix.gettimeofday () in
  let buffer = [Start_recording init_env] in
  Active ({ start_time = start; rev_buffered_recording = buffer; })

let default_instance = Switched_off

(** Use fixed buffer to minimize heap allocations. *)
let chunk_size = 65536
let read_buffer = String.create chunk_size

exception File_read_error

let rec fetch_file_contents_batched acc fd =
  let bytes_read = Unix.read fd read_buffer 0 chunk_size in
  if bytes_read = 0 then
    acc
  else if bytes_read < 0 then
    raise File_read_error
  else
    let b = String.sub read_buffer 0 bytes_read in
    fetch_file_contents_batched (b :: acc) fd

let with_opened_file path f =
  let fd = Unix.openfile path [Unix.O_RDONLY] 0o440 in
  let f () = f fd in
  let finally () = Unix.close fd in
  Utils.try_finally ~f ~finally

let fetch_file_contents path =
  let path = Relative_path.to_absolute path in
  let buffers_rev = with_opened_file path @@ fetch_file_contents_batched [] in
  String.concat "" @@ List.rev buffers_rev

let fetch_file_contents_opt path =
  try Some (fetch_file_contents path) with
  | Unix.Unix_error(Unix.ENOENT, _, _) -> None

let files_with_contents files =
  let files = Relative_path.Set.elements files in
  Relative_path.Map.from_keys files ~f:fetch_file_contents

(** Like files_with_contents but skips over non-existent files. *)
let files_with_contents_opt files =
  let files = Relative_path.Set.elements files in
  let contents = Relative_path.Map.from_keys files ~f:fetch_file_contents_opt in
  Relative_path.Map.fold contents ~init:Relative_path.Map.empty
    ~f:(fun key data acc ->
      match data with
      | None -> acc
      | Some data -> Relative_path.Map.add acc ~key ~data
    )

let convert_event debug_event = match debug_event with
  | DE.Loaded_saved_state (
    { DE.filename;
      corresponding_base_revision;
      dirty_files;
      changed_while_parsing;
      build_targets; },
    global_state) ->
    let dirty_files = files_with_contents_opt dirty_files in
    let changed_while_parsing = files_with_contents changed_while_parsing in
    (** TODO: Some build target files might not exist. Skip them.
     * Improve this. *)
    let build_targets = files_with_contents_opt build_targets in
    Loaded_saved_state (
      { filename;
      corresponding_base_revision;
      dirty_files;
      changed_while_parsing;
      build_targets; },
      global_state)
  | DE.Fresh_vcs_state s -> Fresh_vcs_state s
  | DE.Typecheck -> Typecheck
  | DE.HandleServerCommand cmd -> HandleServerCommand cmd
  | DE.Disk_files_modified files ->
    let contents = Relative_path.Map.from_keys files ~f:fetch_file_contents in
    Disk_files_modified contents
  | DE.Stop_recording ->
    Stop_recording

let with_event event env =
  { env with rev_buffered_recording =
    (convert_event event) :: env.rev_buffered_recording; }

let add_event event instance = match instance, event with
  | Finished_and_loaded_saved_state _, _
  | Finished _, _ ->
    instance
  | Switched_off, _ ->
    instance
  | Active env, DE.Stop_recording ->
    Finished (List.rev env.rev_buffered_recording)
  | Active_and_loaded_saved_state (for_revision, env), DE.Stop_recording ->
    Finished_and_loaded_saved_state (for_revision,
      (List.rev env.rev_buffered_recording))
  | Active env, (DE.Loaded_saved_state
    ({ DE.corresponding_base_revision; _ }, _)) ->
    let env = with_event event env in
    Active_and_loaded_saved_state (corresponding_base_revision, env)
  | Active env, _ ->
    let env = with_event event env in
    Active env
  | (Active_and_loaded_saved_state (for_revision, env)), _ ->
    let env = with_event event env in
    Active_and_loaded_saved_state (for_revision, env)
