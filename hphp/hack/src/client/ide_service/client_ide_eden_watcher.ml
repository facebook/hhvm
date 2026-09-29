(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  instance: Edenfs_watcher.instance;
  polling_task: unit Lwt.t;
}

let log s = Hh_logger.log ("[ide-eden-watcher] " ^^ s)

let destroy instance =
  match Edenfs_watcher.destroy instance with
  | Ok () -> ()
  | Error error ->
    log
      "Failed to destroy watcher: %s"
      (Edenfs_watcher.show_edenfs_watcher_error error)

let stop { instance; polling_task } =
  Lwt.cancel polling_task;
  (* Destruction must finish before the process exits, even if Lwt
     defers the polling task's finalizer. Native destruction is idempotent. *)
  destroy instance

let process_change
    ~max_changed_files ~on_changes (change : Edenfs_watcher_types.changes) :
    bool =
  match change with
  | Edenfs_watcher_types.FileChanges _
  | Edenfs_watcher_types.StateEnter _
  | Edenfs_watcher_types.StateLeave _ ->
    true
  | Edenfs_watcher_types.CommitTransition { file_changes; _ } ->
    let changes =
      file_changes
      |> List.filter ~f:Find_utils.file_filter
      |> List.map ~f:Relative_path.create_detect_prefix
      |> Relative_path.Set.of_list
    in
    let count = Relative_path.Set.cardinal changes in
    if count = 0 then
      true
    else if count > max_changed_files then (
      (* Edenfs_watcher has already expanded the commit diff; this limit only
         bounds naming-table and cache-invalidation work in the daemon. *)
      log "Skipping commit transition with %d relevant files" count;
      true
    ) else (
      log "Enqueuing commit transition with %d relevant files" count;
      on_changes changes
    )

let poll instance ~max_changed_files ~on_changes : unit Lwt.t =
  let rec loop () =
    let%lwt () = Lwt_unix.sleep 0.1 in
    match Edenfs_watcher.get_changes_async instance with
    | Error error ->
      (* Edenfs_watcher already logs these errors to Scuba. *)
      log
        "Stopped after watcher failure: %s"
        (Edenfs_watcher.show_edenfs_watcher_error error);
      Lwt.return_unit
    | Ok (changes, _clock, _telemetry) ->
      if List.for_all changes ~f:(process_change ~max_changed_files ~on_changes)
      then
        (loop [@tailcall]) ()
      else
        Lwt.return_unit
  in
  try%lwt loop () with
  | Lwt.Canceled -> Lwt.return_unit
  | exn ->
    let e = Exception.wrap exn in
    Client_ide_utils.log_bug "ide_eden_watcher_poll" ~e ~telemetry:true;
    Lwt.return_unit

let start ~root ~max_changed_files ~on_changes : unit =
  let settings =
    {
      Edenfs_watcher_types.root;
      watch_spec =
        { Files_to_ignore.server_watch_spec with include_file_names = [] };
      debug_logging = false;
      timeout_secs = 60;
      throttle_time_ms = 0;
      report_telemetry = false;
      state_tracking = true;
      sync_queries_obey_deferral = false;
      tracked_states = ["hg.update"; "hg.transaction"];
    }
  in
  match Edenfs_watcher.init ~destroy_on_exit:false settings with
  | Error error ->
    log "Not started: %s" (Edenfs_watcher.show_edenfs_watcher_error error)
  | Ok (instance, _clock) ->
    let polling_task =
      Lwt.finalize
        (fun () -> poll instance ~max_changed_files ~on_changes)
        (fun () ->
          destroy instance;
          Lwt.return_unit)
    in
    let watcher = { instance; polling_task } in
    Stdlib.at_exit (fun () ->
        try stop watcher with
        | _ -> ());
    log "Started commit-transition watcher"
