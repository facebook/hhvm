(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open File_content
open Option.Monad_infix
open ServerEnv
open File_heap

let try_relativize_path x =
  Option.try_with (fun () -> Relative_path.(create Root x))

let open_file env path content =
  let new_env = try_relativize_path path >>= fun path ->
    let editor_open_files = Relative_path.Set.add env.editor_open_files path in
    FileHeap.remove_batch (Relative_path.Set.singleton path);
    FileHeap.add path (Ide content);
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      editor_open_files; ide_needs_parsing; last_command_time;
    } in
  Option.value new_env ~default:env

let close_relative_path env path =
  let editor_open_files = Relative_path.Set.remove env.editor_open_files path in
  let contents = (match (FileHeap.find_unsafe path) with
  | Ide f -> f
  | _ -> assert false) in
  FileHeap.remove_batch (Relative_path.Set.singleton path);
  FileHeap.add path (Disk contents);
  let ide_needs_parsing =
    Relative_path.Set.add env.ide_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  { env with
    editor_open_files; ide_needs_parsing; last_command_time
  }

let close_file env path =
  let new_env = try_relativize_path path >>| close_relative_path env in
  Option.value new_env ~default:env

let edit_file env path edits =
  let new_env = try_relativize_path path >>= fun path ->
    let fc = match FileHeap.get path with
    | Some Ide f -> f
    | Some Disk content -> content
    | None ->
        try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
    let edited_fc = match edit_file fc edits with
      | Result.Ok r -> r
      | Result.Error e ->
        Hh_logger.log "%s" e;
        (* TODO: do not crash, but surface this to the client somehow *)
        assert false
    in
    let editor_open_files =
      Relative_path.Set.add env.editor_open_files path in
    FileHeap.remove_batch (Relative_path.Set.singleton path);
    FileHeap.add path (Ide edited_fc);
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      editor_open_files; ide_needs_parsing; last_command_time
    } in
  Option.value new_env ~default:env

let clear_sync_data env =
  let env = Relative_path.Set.fold env.editor_open_files
    ~init:env
    ~f:(fun x env -> close_relative_path env x)
  in
{ env with
  persistent_client = None;
  diag_subscribe = None;
}

let get_file_content_from_disk path =
  let f () = Sys_utils.cat (Relative_path.to_absolute path) in
  Option.try_with f

let get_file_content = function
  | ServerUtils.FileContent s -> s
  | ServerUtils.FileName path ->
    begin try_relativize_path path >>= fun path ->
      match File_heap.FileHeap.get path with
        | Some (Ide f) -> Some f
        | Some (Disk c) -> Some c
        | None -> get_file_content_from_disk path
    end
      (* In case of errors, proceed with empty file contents *)
      |> Option.value ~default:""

let has_unsaved_changes env =
  Relative_path.Set.exists env.editor_open_files ~f:begin fun path ->
    match FileHeap.get path with
    | Some Ide contents ->
      begin match get_file_content_from_disk path with
      | Some disk_contents -> contents <> disk_contents
      | None ->
        (* If one creates a new file, then there will not be corresponding
        * disk contents, and we should consider there to be unsaved changes in
        * the editor. *)
        true
      end
    | _ -> false
  end
