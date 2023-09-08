(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open ServerEnv

let try_relativize_path x =
  Option.try_with (fun () -> Relative_path.(create Root x))

let get_file_content_from_disk_rel (path : Relative_path.t) : string option =
  let f () = Sys_utils.cat (Relative_path.to_absolute path) in
  Option.try_with f

let get_file_content_from_disk (path : string) : string =
  match try_relativize_path path with
  | None -> ""
  | Some path -> get_file_content_from_disk_rel path |> Option.value ~default:""

let get_file_content = function
  | ServerCommandTypes.FileContent s -> s
  | ServerCommandTypes.FileName path ->
    try_relativize_path path
    >>= (fun path ->
          match File_provider.get path with
          | Some (File_provider.Ide f) -> Some f
          | Some (File_provider.Disk c) -> Some c
          | None -> get_file_content_from_disk_rel path)
    (* In case of errors, proceed with empty file contents *)
    |> Option.value ~default:""

let open_file
    ~(predeclare : bool)
    (env : ServerEnv.env)
    (path : string)
    (content : string) : ServerEnv.env =
  let prev_content = get_file_content (ServerCommandTypes.FileName path) in
  match try_relativize_path path with
  | None -> env
  | Some path ->
    (* Before making any changes, pre-load (into Decl_heap) currently existing
     * declarations so there is always a previous version to compare against,
     * which makes incremental mode perform better. *)
    (if predeclare && not (Relative_path.Set.mem env.editor_open_files path)
    then
      let ctx = Provider_utils.ctx_from_server_env env in
      Decl.make_env ~sh:SharedMem.Uses ctx path);
    let editor_open_files = Relative_path.Set.add env.editor_open_files path in
    File_provider.remove_batch (Relative_path.Set.singleton path);
    File_provider.provide_file_for_ide path content;
    let ide_needs_parsing =
      if
        String.equal content prev_content
        && is_full_check_done env.full_check_status
      then
        (* Try to avoid telling the user that a check is needed when the file
         * was unchanged. But even in this case, we might need to push
         * errors that were previously throttled. They are available only
         * when full recheck was completed and there were no further changes. In
         * all other cases, we will need to (lazily) recheck the file. *)
        env.ide_needs_parsing
      else
        Relative_path.Set.add env.ide_needs_parsing path
    in
    (* Need to re-parse this file during next full check to update
     * global error list positions that refer to it *)
    let disk_needs_parsing =
      Relative_path.Set.add env.disk_needs_parsing path
    in
    let last_command_time = Unix.gettimeofday () in
    {
      env with
      editor_open_files;
      ide_needs_parsing;
      last_command_time;
      disk_needs_parsing;
    }

let close_relative_path (env : ServerEnv.env) (path : Relative_path.t) :
    ServerEnv.env =
  let editor_open_files = Relative_path.Set.remove env.editor_open_files path in
  let contents =
    match File_provider.get_unsafe path with
    | File_provider.Ide f -> f
    | _ -> assert false
  in
  File_provider.remove_batch (Relative_path.Set.singleton path);
  let new_contents = File_provider.get_contents path in
  let ide_needs_parsing =
    match new_contents with
    | Some c when String.equal c contents -> env.ide_needs_parsing
    | _ -> Relative_path.Set.add env.ide_needs_parsing path
  in
  let disk_needs_parsing = Relative_path.Set.add env.disk_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  {
    env with
    editor_open_files;
    ide_needs_parsing;
    last_command_time;
    disk_needs_parsing;
  }

let close_file (env : ServerEnv.env) (path : string) : ServerEnv.env =
  let new_env = try_relativize_path path >>| close_relative_path env in
  Option.value new_env ~default:env

let edit_file ~predeclare env path (edits : File_content.text_edit list) =
  let new_env =
    try_relativize_path path >>= fun path ->
    (* See similar predeclare in open_file function *)
    (if predeclare && not (Relative_path.Set.mem env.editor_open_files path)
    then
      let ctx = Provider_utils.ctx_from_server_env env in
      Decl.make_env ~sh:SharedMem.Uses ctx path);
    let file_content =
      match File_provider.get path with
      | Some (File_provider.Ide content)
      | Some (File_provider.Disk content) ->
        content
      | None ->
        (try Sys_utils.cat (Relative_path.to_absolute path) with
        | _ -> "")
    in
    let edited_file_content =
      match File_content.edit_file file_content edits with
      | Ok new_content -> new_content
      | Error (reason, e) ->
        Hh_logger.log "SERVER_FILE_EDITED_ERROR - %s" reason;
        HackEventLogger.server_file_edited_error e ~reason;
        Exception.reraise e
    in
    let editor_open_files = Relative_path.Set.add env.editor_open_files path in
    File_provider.remove_batch (Relative_path.Set.singleton path);
    File_provider.provide_file_for_ide path edited_file_content;
    let ide_needs_parsing = Relative_path.Set.add env.ide_needs_parsing path in
    let disk_needs_parsing =
      Relative_path.Set.add env.disk_needs_parsing path
    in
    let last_command_time = Unix.gettimeofday () in
    Some
      {
        env with
        editor_open_files;
        ide_needs_parsing;
        last_command_time;
        disk_needs_parsing;
      }
  in
  Option.value new_env ~default:env

let clear_sync_data env =
  let env =
    Relative_path.Set.fold env.editor_open_files ~init:env ~f:(fun x env ->
        close_relative_path env x)
  in
  env

(** Determine which files are different in the IDE and on disk.
    This is achieved using File_provider for the IDE content and reading file from disk.
    Returns a map from filename to a tuple of ide contents and disk contents. *)
let get_unsaved_changes env =
  Relative_path.Set.fold
    env.editor_open_files
    ~init:Relative_path.Map.empty
    ~f:(fun path acc ->
      match File_provider.get path with
      | Some (File_provider.Ide ide_contents) -> begin
        match get_file_content_from_disk_rel path with
        | Some disk_contents when not (String.equal ide_contents disk_contents)
          ->
          Relative_path.Map.add acc ~key:path ~data:(ide_contents, disk_contents)
        | Some _ -> acc
        | None ->
          (* If one creates a new file, then there will not be corresponding
             * disk contents, and we should consider there to be unsaved changes in
             * the editor. *)
          Relative_path.Map.add acc ~key:path ~data:(ide_contents, "")
      end
      | _ -> acc)

let has_unsaved_changes env =
  not @@ Relative_path.Map.is_empty (get_unsaved_changes env)
