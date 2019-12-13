(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_service_types

let make_local_server_api
    (naming_table : Naming_table.t) ~(root : string) ~(ignore_hh_version : bool)
    : (module LocalServerApi) =
  ( module struct
    let send_progress (message : string) : unit =
      ServerProgress.send_progress_to_monitor "%s" message

    let send_percentage_progress =
      ServerProgress.send_percentage_progress_to_monitor

    let update_state ~(state_filename : string) : unit =
      let (t : int) =
        SharedMem.load_dep_table_blob state_filename ignore_hh_version
      in
      Hh_logger.log "Updated dependency graph: %d seconds" t

    let snapshot_naming_table_base ~(destination_path : string) : unit =
      send_progress "Snapshotting the naming table for delegated type checking";
      let t = Unix.gettimeofday () in
      let () =
        match Naming_table.get_forward_naming_fallback_path naming_table with
        | Some source_path ->
          Hh_logger.log
            "Updating the existing table - moving %s to %s"
            source_path
            source_path;
          FileUtil.cp [source_path] destination_path;
          let (_ : Naming_table.save_result) =
            Naming_table.save naming_table destination_path
          in
          ()
        | None ->
          Hh_logger.log "Creating a new table %s" destination_path;
          let _symbols_added =
            Naming_table.save naming_table destination_path
          in
          ()
      in
      HackEventLogger.remote_scheduler_save_naming_end t;
      let (t : float) =
        Hh_logger.log_duration
          (Printf.sprintf "Saved SQLite naming table to %s" destination_path)
          t
      in
      send_progress (Printf.sprintf "Snapshotted the naming table base: %f" t)

    let snapshot_naming_table_diff ~(destination_path : string) : unit =
      Hh_logger.log "snapshot_naming_table_diff: %s" destination_path;
      Naming_table.save_changes_since_baseline naming_table ~destination_path

    let begin_get_dirty_files ~(mergebase : string option) :
        string list Future.t =
      (* TODO: capture the current timestamp so it can be used for logging
        when the promise is fulfilled and retrieved. *)
      match mergebase with
      | Some mergebase -> Hg.files_changed_since_rev (Hg.Hg_rev mergebase) root
      | None -> Future.of_error "Expected a non-empty mergebase"

    let write_dirty_files
        (dirty_files : string list) ~(destination_path : string) : unit =
      let dirty_files =
        List.map dirty_files ~f:(fun dirty_file ->
            let dirty_file = FilePath.make_absolute root dirty_file in
            let dirty_file_path =
              Relative_path.create Relative_path.Root dirty_file
            in
            (dirty_file_path, File_provider.get_contents dirty_file_path))
      in
      let chan = Pervasives.open_out_bin destination_path in
      Marshal.to_channel chan dirty_files [];
      Pervasives.close_out chan
  end : LocalServerApi )
