(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open File_content
open Option.Monad_infix
open ServerEnv
open File_heap

let try_relativize_path x =
  Option.try_with (fun () -> Relative_path.(create Root x))

let get_file_content_from_disk path =
  let f () = Sys_utils.cat (Relative_path.to_absolute path) in
  Option.try_with f

let get_file_content = function
  | ServerCommandTypes.FileContent s -> s
  | ServerCommandTypes.FileName path ->
    begin try_relativize_path path >>= fun path ->
      match File_heap.FileHeap.get path with
        | Some (Ide f) -> Some f
        | Some (Disk c) -> Some c
        | None -> get_file_content_from_disk path
    end
      (* In case of errors, proceed with empty file contents *)
      |> Option.value ~default:""

(* Warning: this takes O(global error list) time. Should be OK while
 * it's only used in editor, where opening a file is a rare (compared to other
 * kind of queries) operation, but if this ever ends up being called by
 * other automation there is room for improvement (i.e finally changing global
 * error list to be a error map)
 *)
let update_diagnostics diag_subscribe editor_open_files errorl  =
  Option.map diag_subscribe ~f:begin fun diag_subscribe ->
    Diagnostic_subscription.update diag_subscribe
      ~priority_files:editor_open_files
      ~reparsed:Relative_path.Set.empty
      ~rechecked:Relative_path.Map.empty
      ~global_errors:errorl
      ~full_check_done:true
  end

let open_file ~predeclare env path content =
  let prev_content = get_file_content (ServerCommandTypes.FileName path) in
  let new_env = try_relativize_path path >>= fun path ->
    (* Before making any changes, pre-load (into Decl_heap) currently existing
     * declarations so there is always a previous version to compare against,
     * which makes incremental mode perform better. *)
    if predeclare && (not (Relative_path.Set.mem env.editor_open_files path)) then
      Decl.make_env env.tcopt path;
    let editor_open_files = Relative_path.Set.add env.editor_open_files path in
    FileHeap.remove_batch (Relative_path.Set.singleton path);
    FileHeap.add path (Ide content);
    let ide_needs_parsing, diag_subscribe =
      if content = prev_content && (env.full_check = Full_check_done) then begin
        (* Try to avoid telling the user that a check is needed when the file
         * was unchanged. But even in this case, we might need to push
         * errors that were previously throttled. They are available only
         * when full recheck was completed and there were no further changes. In
         * all other cases, we will need to (lazily) recheck the file. *)
        env.ide_needs_parsing,
        update_diagnostics env.diag_subscribe editor_open_files env.errorl
      end else
        Relative_path.Set.add env.ide_needs_parsing path, env.diag_subscribe
      in
    (* Need to re-parse this file during next full check to update
     * global error list positions that refer to it *)
    let disk_needs_parsing =
      Relative_path.Set.add env.disk_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      editor_open_files; ide_needs_parsing; last_command_time; diag_subscribe;
      disk_needs_parsing;
    } in
  Option.value new_env ~default:env

let close_relative_path env path =
  let editor_open_files = Relative_path.Set.remove env.editor_open_files path in
  let contents = (match (FileHeap.find_unsafe path) with
  | Ide f -> f
  | _ -> assert false) in
  FileHeap.remove_batch (Relative_path.Set.singleton path);
  let new_contents = File_heap.get_contents path in
  let ide_needs_parsing =
    if new_contents = Some contents then env.ide_needs_parsing
    else Relative_path.Set.add env.ide_needs_parsing path in
  let disk_needs_parsing =
    Relative_path.Set.add env.disk_needs_parsing path in
  let last_command_time = Unix.gettimeofday () in
  { env with
    editor_open_files; ide_needs_parsing; last_command_time; disk_needs_parsing
  }

let close_file env path =
  let new_env = try_relativize_path path >>| close_relative_path env in
  Option.value new_env ~default:env

let edit_file ~predeclare env path (edits: File_content.text_edit list) =
  let new_env = try_relativize_path path >>= fun path ->
    (* See similar predeclare in open_file function *)
    if predeclare && (not (Relative_path.Set.mem env.editor_open_files path)) then
      Decl.make_env env.tcopt path;
    ServerBusyStatus.send env ServerCommandTypes.Needs_local_typecheck;
    let fc = match FileHeap.get path with
    | Some Ide f -> f
    | Some Disk content -> content
    | None ->
        try Sys_utils.cat (Relative_path.to_absolute path) with _ -> "" in
    let edited_fc = match edit_file fc edits with
      | Ok r -> r
      | Error (reason, _stack) ->
        Hh_logger.log "%s" reason;
        (* TODO: do not crash, but surface this to the client somehow *)
        assert false
    in
    let editor_open_files =
      Relative_path.Set.add env.editor_open_files path in
    FileHeap.remove_batch (Relative_path.Set.singleton path);
    FileHeap.add path (Ide edited_fc);
    let ide_needs_parsing =
      Relative_path.Set.add env.ide_needs_parsing path in
    let disk_needs_parsing =
      Relative_path.Set.add env.disk_needs_parsing path in
    let last_command_time = Unix.gettimeofday () in
    Some { env with
      editor_open_files; ide_needs_parsing; last_command_time;
      disk_needs_parsing
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

let get_unsaved_changes env =
  Relative_path.Set.fold env.editor_open_files
    ~init:Relative_path.Map.empty
    ~f:begin fun path acc ->
      match FileHeap.get path with
      | Some Ide ide_contents ->
        begin match get_file_content_from_disk path with
        | Some disk_contents when ide_contents <> disk_contents ->
          Relative_path.Map.add acc path (ide_contents, disk_contents)
        | Some _ -> acc
        | None ->
          (* If one creates a new file, then there will not be corresponding
          * disk contents, and we should consider there to be unsaved changes in
          * the editor. *)
          Relative_path.Map.add acc path (ide_contents, "")
        end
      | _ -> acc
    end

let has_unsaved_changes env =
  not @@ Relative_path.Map.is_empty (get_unsaved_changes env)

let toggle_dynamic_view (env : env) (t : bool) : env =
 ServerDynamicView.toggle := t;
 {
   env with
   ide_needs_parsing = env.editor_open_files
 }
