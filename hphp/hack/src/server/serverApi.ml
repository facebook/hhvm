(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_prelude.Result.Monad_infix
open RemoteWorker
open Typing_service_types

let make_local_server_api
    (naming_table : Naming_table.t)
    ~(root : string)
    ~(init_id : string)
    ~(enable_disk_heap : bool)
    ~(deps_mode : Typing_deps_mode.t) : (module LocalServerApi) =
  (module struct
    let send_progress (message : string) : unit =
      ServerProgress.send_progress "%s" message

    let update_state ~(state_filename : string) ~(check_id : string option) :
        unit =
      let check_id =
        Option.value check_id ~default:(Random_id.short_string ())
      in
      HackEventLogger.with_id ~stage:`Recheck check_id @@ fun () ->
      let start_t = Unix.gettimeofday () in
      let edges = Typing_deps.load_discovered_edges deps_mode state_filename in
      HackEventLogger.remote_scheduler_update_dependency_graph_end
        ~edges
        start_t;
      let (_t : float) =
        Hh_logger.log_duration
          (Printf.sprintf "Updated dependency graph: added %d edges" edges)
          start_t
      in
      ()

    let snapshot_naming_table_base ~destination_path : unit Future.t =
      send_progress "Snapshotting the naming table for delegated type checking";
      let start_t = Unix.gettimeofday () in
      let future =
        match Naming_table.get_forward_naming_fallback_path naming_table with
        | Some source_path ->
          Hh_logger.log
            "Updating the existing table - moving %s to %s"
            source_path
            destination_path;
          FileUtil.cp [source_path] destination_path;
          let (_ : Naming_sqlite.save_result) =
            Naming_table.save naming_table destination_path
          in
          Future.of_value ()
        | None ->
          Naming_table.save_async naming_table ~init_id ~root ~destination_path
      in
      Future.continue_with future @@ fun () ->
      HackEventLogger.remote_scheduler_save_naming_end start_t;
      let (start_t : float) =
        Hh_logger.log_duration
          (Printf.sprintf "Saved SQLite naming table to %s" destination_path)
          start_t
      in
      send_progress
        (Printf.sprintf "Snapshotted the naming table base: %f" start_t)

    let snapshot_naming_table_diff ~(destination_path : string) : unit =
      Hh_logger.log "snapshot_naming_table_diff: %s" destination_path;
      Naming_table.save_changes_since_baseline naming_table ~destination_path

    let begin_get_changed_files ~(mergebase : string option) :
        string list Future.t =
      let t = Unix.gettimeofday () in
      match mergebase with
      | Some mergebase ->
        let hg_future = Hg.files_changed_since_rev (Hg.Hg_rev mergebase) root in
        Future.continue_with hg_future @@ fun changed_files ->
        let telemetry =
          Telemetry.create ()
          |> Telemetry.int_
               ~key:"changed_files"
               ~value:(List.length changed_files)
        in
        HackEventLogger.remote_scheduler_get_dirty_files_end telemetry t;
        changed_files
      | None -> Future.of_value []

    let load_changed_files (changed_files : string list) :
        (Relative_path.t * string option) list =
      let changed_files_and_content =
        List.map changed_files ~f:(fun changed_file ->
            let changed_file = FilePath.make_absolute root changed_file in
            let changed_file_path =
              Relative_path.create Relative_path.Root changed_file
            in
            ( changed_file_path,
              File_provider.get_contents
                ~writeback_disk_contents_in_shmem_provider:enable_disk_heap
                changed_file_path ))
      in
      changed_files_and_content

    let write_changed_files
        (changed_files : string list) ~(destination_path : string) : unit =
      let changed_filepaths_and_content = load_changed_files changed_files in
      let chan = Stdlib.open_out_bin destination_path in
      Marshal.to_channel chan changed_filepaths_and_content [];
      Stdlib.close_out chan
  end : LocalServerApi)

let make_remote_server_api
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (root : Path.t) :
    (module RemoteServerApi with type naming_table = Naming_table.t option) =
  (module struct
    type naming_table = Naming_table.t option

    let load_naming_table_base ~(naming_table_base : Path.t option) :
        (naming_table, string) result =
      Hh_logger.log "Loading naming table base...";

      match naming_table_base with
      | None ->
        Error
          "Expected naming table base path to be set when loading naming table, but it was not"
      | Some naming_table_base ->
        Ok
          (Some
             (Naming_table.load_from_sqlite
                ctx
                (Path.to_string naming_table_base)))

    (**
      There is a variety of state that the server accumulates after type
      checking files. We want to make sure we remove such state before a
      recheck. In order to do this cleaning, we need a list of files that
      changed.
      *)
    let clean_changed_files_state ctx naming_table changed_files ~t =
      let (changed_names : FileInfo.names) =
        List.fold changed_files ~init:FileInfo.empty_names ~f:(fun names file ->
            match Naming_table.get_file_info naming_table file with
            | Some (file_info : FileInfo.t) ->
              FileInfo.merge_names names (FileInfo.simplify file_info)
            | None -> names)
      in
      let t =
        Hh_logger.log_duration "Got names changed since naming table baseline" t
      in
      let changed_files = Relative_path.set_of_list changed_files in
      File_provider.remove_batch changed_files;
      Ast_provider.remove_batch changed_files;
      Fixme_provider.remove_batch changed_files;
      Decl_redecl_service.remove_old_defs
        ctx
        ~bucket_size:1000
        workers
        changed_names;
      Hh_logger.log_duration "Cleaned state associated with changed files" t

    let load_naming_table_changes_since_baseline
        (ctx : Provider_context.t)
        ~(naming_table : Naming_table.t option)
        ~(naming_table_diff : Naming_table.changes_since_baseline) :
        (Naming_table.t option, string) result =
      Hh_logger.log "Loading naming table changes since baseline...";
      match naming_table with
      | None -> Error "Expected naming table base"
      | Some naming_table ->
        begin
          match Naming_table.get_forward_naming_fallback_path naming_table with
          | None ->
            Error "Expected naming table base path to be set, but it was not"
          | Some naming_table_base ->
            (try
               let t = Unix.gettimeofday () in
               let changed_files =
                 Naming_table.get_files_changed_since_baseline naming_table_diff
               in
               let t =
                 Hh_logger.log_duration
                   "Got files changed since naming table baseline"
                   t
               in
               let t =
                 clean_changed_files_state ctx naming_table changed_files ~t
               in
               Hh_logger.log "Prefetching naming dirty files...";
               Vfs.prefetch changed_files;
               let t =
                 Hh_logger.log_duration "Prefetched naming dirty files" t
               in
               let (naming_table : Naming_table.t) =
                 Naming_table.load_from_sqlite_with_changes_since_baseline
                   ctx
                   naming_table_diff
                   naming_table_base
               in
               HackEventLogger.remote_worker_load_naming_end t;
               let _t : float =
                 Hh_logger.log_duration "Loaded naming table from SQLite" t
               in
               Ok (Some naming_table)
             with
            | e -> Error (Exn.to_string e))
        end

    let build_naming_table _ =
      Hh_logger.log "Building naming table";
      let indexer =
        Find.make_next_files ~name:"root" ~filter:FindUtils.is_hack root
      in
      let get_next =
        ServerUtils.make_next
          ~hhi_filter:(fun _ -> true)
          ~indexer
          ~extra_roots:(ServerConfig.extra_paths ServerConfig.default_config)
      in
      Hh_logger.log "Building naming table - Parsing";
      let (defs_per_file, _errorl, _failed_parsing) =
        ( Direct_decl_service.go
            ctx
            workers
            ~ide_files:Relative_path.Set.empty
            ~get_next
            ~trace:false
            ~cache_decls:true,
          Errors.empty,
          Relative_path.Set.empty )
      in
      Hh_logger.log "Building naming table - Naming";
      let naming_table = Naming_table.create defs_per_file in
      Naming_table.iter naming_table ~f:(fun k v ->
          let _ = Naming_global.ndecl_file_error_if_already_bound ctx k v in
          ());
      Hh_logger.log "Building naming table - Done!";
      ()

    let download_dep_table
        (manifold_api_key : string option)
        (use_manifold_cython_client : bool)
        (manifold_path : string) :
        ( Saved_state_loader.Naming_and_dep_table_info.main_artifacts,
          string )
        result =
      let target_path = "/tmp/hh_server/" ^ Random_id.short_string () in
      Disk.mkdir_p target_path;

      let naming_table_future =
        State_loader_futures.download_and_unpack_saved_state_from_manifold
          ~env:
            {
              Saved_state_loader.log_saved_state_age_and_distance = false;
              Saved_state_loader.saved_state_manifold_api_key = manifold_api_key;
              Saved_state_loader.use_manifold_cython_client;
            }
          ~progress_callback:(fun _ -> ())
          ~saved_state_type:
            (Saved_state_loader.Naming_and_dep_table { naming_sqlite = true })
          ~manifold_path
          ~target_path:(Path.make target_path)
      in
      match Future.get ~timeout:60 naming_table_future with
      | Error err ->
        let err = Future.error_to_string err in
        Hh_logger.error "Downloading dep table failed: %s" err;
        Error err
      | Ok download_result ->
        begin
          match download_result with
          | Error (err, _telemetry) ->
            Error (Saved_state_loader.debug_details_of_error err)
          | Ok (main_artifacts, _telemetry) ->
            let (_ : float) =
              Hh_logger.log_duration
                "Finished downloading dep table."
                (Future.start_t naming_table_future)
            in
            let naming_table_path =
              main_artifacts
                .Saved_state_loader.Naming_and_dep_table_info
                 .naming_sqlite_table_path
            in
            Hh_logger.log
              "Downloaded naming table to %s"
              (Path.to_string naming_table_path);
            Ok main_artifacts
        end

    let remove_decls naming_table fast_parsed =
      Relative_path.Map.iter fast_parsed ~f:(fun fn _ ->
          match Naming_table.get_file_info naming_table fn with
          | None -> ()
          | Some
              {
                FileInfo.funs;
                classes;
                typedefs;
                consts;
                modules;
                file_mode = _;
                comments = _;
                hash = _;
              } ->
            (* we use [snd] to strip away positions *)
            let snd (_, x, _) = x in
            Naming_global.remove_decls
              ~backend:(Provider_backend.get ())
              ~funs:(List.map funs ~f:snd)
              ~classes:(List.map classes ~f:snd)
              ~typedefs:(List.map typedefs ~f:snd)
              ~consts:(List.map consts ~f:snd)
              ~modules:(List.map modules ~f:snd))

    let load_dep_table
        (saved_state_main_artifacts :
          Saved_state_loader.Naming_and_dep_table_info.main_artifacts) :
        (Naming_table.t * Path.t, string) result =
      let {
        Saved_state_loader.Naming_and_dep_table_info.naming_table_path = _;
        dep_table_path;
        naming_sqlite_table_path;
        legacy_hot_decls_path = _;
        shallow_hot_decls_path = _;
        errors_path = _;
      } =
        saved_state_main_artifacts
      in
      if not (Sys.file_exists (Path.to_string naming_sqlite_table_path)) then
        Error
          (Printf.sprintf
             "Expected naming sqlite table at %s"
             (Path.to_string naming_sqlite_table_path))
      else
        let naming_table =
          Naming_table.load_from_sqlite
            ctx
            (Path.to_string naming_sqlite_table_path)
        in
        Ok (naming_table, dep_table_path)

    let download_and_update_naming_table_helper
        (manifold_api_key : string option)
        (use_manifold_cython_client : bool)
        (path : string)
        (changed_files : Relative_path.t list option) : (string, string) result
        =
      download_dep_table manifold_api_key use_manifold_cython_client path
      >>= fun saved_state_main_artifacts ->
      Hh_logger.log "Loading dep graph artifacts...";
      load_dep_table saved_state_main_artifacts
      >>= fun (naming_table, dep_table_path) ->
      match changed_files with
      | None -> Error "No changed files uploaded for remote worker's payload"
      | Some lst ->
        Ok (naming_table, lst) >>= fun (naming_table, changed_files) ->
        Hh_logger.log "Cleaning naming table of changed files";
        ignore
          (clean_changed_files_state
             ctx
             naming_table
             changed_files
             ~t:(Unix.gettimeofday ()));
        Ok (naming_table, changed_files, dep_table_path)
        >>= fun (naming_table, changed_files, dep_table_path) ->
        let changed_hack_files =
          List.filter_map changed_files ~f:(fun file ->
              if FindUtils.is_hack (Relative_path.suffix file) then
                Some (Path.to_string root ^ "/" ^ Relative_path.suffix file)
              else
                None)
        in
        let indexer =
          let state = ref changed_hack_files in
          let max_files_per_batch = 1000 in
          fun () ->
            let (next, rest) = List.split_n !state max_files_per_batch in
            state := rest;
            next
        in
        let get_next =
          ServerUtils.make_next
            ~hhi_filter:(fun _ -> true)
            ~indexer
            ~extra_roots:(ServerConfig.extra_paths ServerConfig.default_config)
        in
        Ok
          ( naming_table,
            Direct_decl_service.go
              ctx
              workers
              ~ide_files:Relative_path.Set.empty
              ~get_next
              ~trace:false
              ~cache_decls:true,
            dep_table_path )
        >>= fun (naming_table, fast_parsed, dep_table_path) ->
        Hh_logger.log "Built updated decls for naming table";
        Hh_logger.log "Clearing old decls from naming table";
        remove_decls naming_table fast_parsed;
        Hh_logger.log "Updating naming table";
        ignore (Naming_table.update_many naming_table fast_parsed);
        Ok (fast_parsed, dep_table_path)
        >>= fun (fast_parsed, dep_table_path) ->
        Hh_logger.log "Updating naming global";
        Naming_table.create fast_parsed
        |> Naming_table.iter ~f:(fun k v ->
               let _ =
                 Naming_global.ndecl_file_error_if_already_bound ctx k v
               in
               ());
        Ok (Path.to_string dep_table_path)

    let download_and_update_naming_table
        ~(manifold_api_key : string option)
        ~(use_manifold_cython_client : bool)
        (saved_state_manifold_path : string option)
        (changed_files : Relative_path.t list option) : string option =
      match saved_state_manifold_path with
      | None ->
        Hh_logger.log
          "[hulk lite] No saved_state_manifold_path, will fall back to building naming table locally";
        build_naming_table ();
        None
      | Some path ->
        (match
           download_and_update_naming_table_helper
             manifold_api_key
             use_manifold_cython_client
             path
             changed_files
         with
        | Ok dep_table_path -> Some dep_table_path
        | Error err ->
          Hh_logger.log "Could not build naming table from saved state: %s" err;
          Hh_logger.log "Falling back to generating naming table";
          build_naming_table ();
          None)

    let type_check
        ctx ~init_id ~check_id files_to_check ~state_filename ~telemetry =
      let t = Unix.gettimeofday () in
      Hh_logger.log "Type checking a batch...";
      let check_info =
        {
          init_id;
          check_reason = "remote_server_api";
          recheck_id = Some check_id;
          use_max_typechecker_worker_memory_for_decl_deferral = false;
          per_file_profiling = HackEventLogger.PerFileProfilingConfig.default;
          memtrace_dir = None;
        }
      in
      (* TODO: use the telemetry *)
      let { Typing_check_service.errors; telemetry; _ } =
        Typing_check_service.go
          ctx
          workers
          Typing_service_delegate.default
          telemetry
          files_to_check
          ~memory_cap:(Some 200000)
          ~longlived_workers:false
          ~mode:HulkStrategy.Legacy
          ~check_info
      in
      HackEventLogger.remote_worker_type_check_end telemetry ~start_t:t;
      let t = Hh_logger.log_duration "Type checked files in remote worker" t in
      let dep_table_edges_added =
        Typing_deps.save_discovered_edges
          (Provider_context.get_deps_mode ctx)
          ~dest:state_filename
          ~reset_state_after_saving:true
      in
      let _t : float =
        Hh_logger.log_duration
          (Printf.sprintf
             "Saved partial dependency graph (%d edges)"
             dep_table_edges_added)
          t
      in
      errors
  end : RemoteServerApi
    with type naming_table = Naming_table.t option)
