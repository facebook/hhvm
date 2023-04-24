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
    ~(deps_mode : Typing_deps_mode.t) : (module LocalServerApi) =
  (module struct
    let send_progress (message : string) : unit =
      ServerProgress.write "%s" message

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

    let upload_naming_table ~(nonce : string) : unit =
      Hh_logger.log "Uploading the naming table...";
      let cmd = "manifold mkdirs hulk/tree/naming/" ^ nonce in
      Hh_logger.log "Running %s" cmd;
      ignore (Sys.command cmd);

      let blob_dir =
        Tempfile.mkdtemp_with_dir (Path.make GlobalConfig.tmp_dir)
      in
      let blob_path = Path.(to_string (concat blob_dir "naming.bin")) in
      let chan = Stdlib.open_out_bin blob_path in
      Marshal.to_channel chan naming_table [];
      Stdlib.close_out chan;

      let cmd =
        Printf.sprintf
          "manifold putr --overwrite %s hulk/tree/naming/%s"
          (Path.to_string blob_dir)
          nonce
      in
      Hh_logger.log "Executing %s" cmd;
      ignore (Sys.command cmd);
      Hh_logger.log "Uploaded the naming table";
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
            (changed_file_path, File_provider.get_contents changed_file_path))
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
      | Some naming_table -> begin
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
             let t = Hh_logger.log_duration "Prefetched naming dirty files" t in
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

    let build_naming_table () =
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
      let defs_per_file =
        Direct_decl_service.go
          ctx
          workers
          ~ide_files:Relative_path.Set.empty
          ~get_next
          ~trace:false
          ~cache_decls:true
      in
      Hh_logger.log "Building naming table - Naming";
      let naming_table = Naming_table.create defs_per_file in
      Naming_table.iter naming_table ~f:(fun k v ->
          let _ = Naming_global.ndecl_file_error_if_already_bound ctx k v in
          ());
      Hh_logger.log "Building naming table - Done!";
      naming_table

    let load_naming_and_dep_table
        (saved_state_main_artifacts :
          Saved_state_loader.Naming_and_dep_table_info.main_artifacts) :
        (Naming_table.t * Path.t, string) result =
      let {
        Saved_state_loader.Naming_and_dep_table_info.naming_table_path = _;
        dep_table_path;
        naming_sqlite_table_path;
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

    let download_naming_and_dep_table_from_saved_state
        (ssopt : GlobalOptions.saved_state_loading) (manifold_path : string) :
        (Naming_table.t * Path.t, string) result =
      let target_path = "/tmp/hh_server/" ^ Random_id.short_string () in
      Disk.mkdir_p target_path;

      let naming_table_future =
        State_loader_futures.download_and_unpack_saved_state_from_manifold
          ~ssopt
          ~progress_callback:(fun _ -> ())
          ~saved_state_type:Saved_state_loader.Naming_and_dep_table
          ~manifold_path
          ~target_path:(Path.make target_path)
      in
      match Future.get ~timeout:60 naming_table_future with
      | Error err ->
        let err = Future.error_to_string err in
        Hh_logger.error "Downloading dep table failed: %s" err;
        Error err
      | Ok download_result -> begin
        match download_result with
        | Error (err, _telemetry) ->
          Error (Saved_state_loader.LoadError.debug_details_of_error err)
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
          load_naming_and_dep_table main_artifacts
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

    let update_naming_table
        (naming_table_opt : naming_table)
        (changed_files : Relative_path.t list option) : (unit, string) result =
      match (naming_table_opt, changed_files) with
      | (_, None) ->
        Error "No changed files uploaded for remote worker's payload"
      | (None, _) -> Error "No naming table present for the remote worker"
      | (Some naming_table, Some changed_files) ->
        Hh_logger.log "Cleaning naming table of changed files";
        ignore
          (clean_changed_files_state
             ctx
             naming_table
             changed_files
             ~t:(Unix.gettimeofday ()));
        Ok (naming_table, changed_files)
        >>= fun (naming_table, changed_files) ->
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
              ~cache_decls:true )
        >>= fun (naming_table, fast_parsed) ->
        Hh_logger.log "Built updated decls for naming table";
        Hh_logger.log "Clearing old decls from naming table";
        remove_decls naming_table fast_parsed;
        Hh_logger.log "Updating naming table";
        ignore (Naming_table.update_many naming_table fast_parsed);
        Ok fast_parsed >>= fun fast_parsed ->
        Hh_logger.log "Updating naming global";
        Naming_table.create fast_parsed
        |> Naming_table.iter ~f:(fun k v ->
               let _ =
                 Naming_global.ndecl_file_error_if_already_bound ctx k v
               in
               ());
        Ok ()

    let rec download_naming_table_for_full_init_helper
        ~(naming_table_base : Path.t) ~(nonce : Int64.t) : naming_table =
      let naming_table_manifold_path =
        Printf.sprintf "hulk/tree/naming/%s/naming.bin" (Int64.to_string nonce)
      in
      let check_manifold_path_exists_cmd =
        Printf.sprintf "manifold exists %s" naming_table_manifold_path
      in
      match Sys_utils.exec_try_read check_manifold_path_exists_cmd with
      | Some output when Str.(string_match (regexp ".*EXISTS") output 0) ->
        Hh_logger.log "Downloading the naming table...";

        let cmd =
          Printf.sprintf
            "manifold get %s %s"
            naming_table_manifold_path
            (Path.to_string naming_table_base)
        in
        Hh_logger.log "Executing %s" cmd;
        ignore (Sys.command cmd);

        let naming_table_path = Path.to_string naming_table_base in
        let chan = In_channel.create ~binary:true naming_table_path in
        let naming_table = Marshal.from_channel chan in
        let sqlite_naming_table_base =
          Path.(concat (dirname naming_table_base) "naming.sql")
        in
        ignore
        @@ Naming_table.save
             naming_table
             (Path.to_string sqlite_naming_table_base);
        (match
           load_naming_table_base
             ~naming_table_base:(Some sqlite_naming_table_base)
         with
        | Ok naming_table -> naming_table
        | _ -> None)
      | _ ->
        (* If the naming table hasn't been uploaded yet,
           sleep for 5 seconds and try again *)
        Unix.sleep 5;
        download_naming_table_for_full_init_helper ~naming_table_base ~nonce

    let download_naming_table_for_full_init ~(nonce : Int64.t) : naming_table =
      let naming_dir =
        Tempfile.mkdtemp_with_dir (Path.make GlobalConfig.tmp_dir)
      in
      let naming_table_base = Path.concat naming_dir "naming.bin" in
      download_naming_table_for_full_init_helper ~naming_table_base ~nonce

    let download_naming_and_dep_table
        (ssopt : GlobalOptions.saved_state_loading)
        ~(nonce : Int64.t)
        (saved_state_manifold_path : string option) :
        naming_table * string option =
      match saved_state_manifold_path with
      | Some path ->
        let download_result =
          download_naming_and_dep_table_from_saved_state ssopt path
        in
        (match download_result with
        | Ok (naming_table, dep_table) ->
          (Some naming_table, Some (Path.to_string dep_table))
        | _ ->
          let naming_table = build_naming_table () in
          (Some naming_table, None))
      | None ->
        let naming_table = download_naming_table_for_full_init ~nonce in
        (naming_table, None)

    let load_shallow_decls_saved_state
        (saved_state_main_artifacts :
          Saved_state_loader.Shallow_decls_info.main_artifacts) :
        (string I64Map.t, string) result =
      let { Saved_state_loader.Shallow_decls_info.shallow_decls_path } =
        saved_state_main_artifacts
      in
      if not (Sys.file_exists (Path.to_string shallow_decls_path)) then
        Error
          (Printf.sprintf
             "Expected shallow_decls_saved_state at %s"
             (Path.to_string shallow_decls_path))
      else
        let chan = Stdlib.open_in_bin (Path.to_string shallow_decls_path) in
        let contents = Marshal.from_channel chan in
        Stdlib.close_in chan;
        Ok contents

    let download_shallow_decls_saved_state
        (ssopt : GlobalOptions.saved_state_loading) (manifold_path : string) :
        (string I64Map.t, string) result =
      let target_path = "/tmp/hh_server/" ^ Random_id.short_string () in
      Disk.mkdir_p target_path;

      let shallow_decls_future =
        State_loader_futures.download_and_unpack_saved_state_from_manifold
          ~ssopt
          ~progress_callback:(fun _ -> ())
          ~saved_state_type:Saved_state_loader.Shallow_decls
          ~manifold_path
          ~target_path:(Path.make target_path)
      in
      match Future.get ~timeout:600 shallow_decls_future with
      | Error err ->
        let err = Future.error_to_string err in
        Hh_logger.error "Downloading shallow_decls saved state failed: %s" err;
        Error err
      | Ok download_result -> begin
        match download_result with
        | Error (err, _telemetry) ->
          Error (Saved_state_loader.LoadError.debug_details_of_error err)
        | Ok (main_artifacts, _telemetry) ->
          let (_ : float) =
            Hh_logger.log_duration
              "Finished downloading shallow_decls saved state."
              (Future.start_t shallow_decls_future)
          in
          let shallow_decls_path =
            main_artifacts
              .Saved_state_loader.Shallow_decls_info.shallow_decls_path
          in
          Hh_logger.log
            "Downloaded shallow_decls to %s"
            (Path.to_string shallow_decls_path);
          load_shallow_decls_saved_state main_artifacts
      end

    let unmarshal_decls_from_download_result
        ~(ctx : Provider_context.t)
        (shallow_decls : string I64Map.t)
        (classnames : SSet.elt list) :
        Shallow_decl_defs.shallow_class option SMap.t =
      (* match shallow decls with classnames according to hash code *)
      let db_path_opt = Remote_old_decl_client.Utils.db_path_of_ctx ~ctx in
      match db_path_opt with
      | None -> SMap.empty
      | Some db_path ->
        let decl_name_and_hashes =
          List.filter_map
            ~f:(fun name ->
              match
                Remote_old_decl_client.Utils.name_to_decl_hash_opt
                  ~name
                  ~db_path
              with
              | None -> None
              | Some hash -> Some (name, Int64.of_string hash))
            classnames
        in
        Hh_logger.log "constructed smap";
        List.fold_left
          ~init:SMap.empty
          ~f:(fun acc (name, hash) ->
            match I64Map.find_opt hash shallow_decls with
            | None -> acc
            | Some marshalled_decl ->
              let decl = Marshal.from_string marshalled_decl 0 in
              SMap.add name (Some decl) acc)
          decl_name_and_hashes

    let fetch_remote_decls_from_saved_state ssopt manifold_path classnames =
      match download_shallow_decls_saved_state ssopt manifold_path with
      | Ok shallow_decls_download_result ->
        let _ =
          Hh_logger.log
            "loaded %d shallow decls from saved state"
            (I64Map.cardinal shallow_decls_download_result)
        in
        let state_decls =
          unmarshal_decls_from_download_result
            ~ctx
            shallow_decls_download_result
            classnames
        in
        let _ =
          Hh_logger.log
            "extracted %d shallow decls from saved state"
            (SMap.cardinal state_decls)
        in
        state_decls
      | Error err -> failwith err

    let fetch_remote_decls_from_remote_old_decl_service classnames =
      let job (acc : 'a SMap.t) (classnames : string list) : 'a SMap.t =
        Hh_logger.log
          "Fecthing %d decls from the remote decl store"
          (List.length classnames);
        let remotely_fetched_decls =
          Remote_old_decl_client.fetch_old_decls ~ctx classnames
        in
        Hh_logger.log
          "Fetched %d decls from the remote decl store"
          (SMap.cardinal remotely_fetched_decls);
        SMap.merge
          (fun _key a b ->
            if Option.is_some a then
              a
            else
              b)
          acc
          remotely_fetched_decls
      in
      MultiWorker.call
        workers
        ~job
        ~neutral:SMap.empty
        ~merge:
          (SMap.merge (fun _key a b ->
               if Option.is_some a then
                 a
               else
                 b))
        ~next:(MultiWorker.next ~max_size:50000 workers classnames)

    let fetch_and_cache_remote_decls
        ~ctx
        ~(ssopt : GlobalOptions.saved_state_loading)
        (naming_table_opt : naming_table)
        ~(from_saved_state : bool)
        ~(manifold_path : string) =
      match naming_table_opt with
      | None -> ()
      | Some naming_table ->
        let start_t = Unix.gettimeofday () in
        let fileinfos_from_naming_table =
          naming_table
          |> Naming_table.to_defs_per_file ~warn_on_naming_costly_iter:false
          |> Relative_path.Map.elements
        in
        let classnames =
          List.fold
            fileinfos_from_naming_table
            ~init:[]
            ~f:(fun acc (_filename, fileinfo) ->
              SSet.fold
                (fun class_name acc -> class_name :: acc)
                fileinfo.FileInfo.n_classes
                acc)
        in
        let remotely_fetched_decls =
          if from_saved_state then
            fetch_remote_decls_from_saved_state ssopt manifold_path classnames
          else
            fetch_remote_decls_from_remote_old_decl_service classnames
        in
        List.iter fileinfos_from_naming_table ~f:(fun (filename, fileinfo) ->
            let class_names = fileinfo.FileInfo.n_classes in
            let pfh_decls : (string * Shallow_decl_defs.decl * Int64.t) list =
              SSet.fold
                (fun name acc ->
                  let remotely_fetched_decl =
                    Option.join (SMap.find_opt name remotely_fetched_decls)
                  in
                  match
                    Option.Monad_infix.(
                      remotely_fetched_decl >>= fun decl ->
                      Remote_old_decl_client.Utils.db_path_of_ctx ~ctx
                      >>= fun db_path ->
                      Remote_old_decl_client.Utils.name_to_decl_hash_opt
                        ~name
                        ~db_path
                      >>= fun hash ->
                      Some
                        ( name,
                          Shallow_decl_defs.Class decl,
                          Int64.of_string hash ))
                  with
                  | Some pfh_decl -> pfh_decl :: acc
                  | None -> acc)
                class_names
                []
            in
            Direct_decl_utils.cache_decls ctx filename pfh_decls);
        let (_ : float) =
          Hh_logger.log_duration
            "Fetched and cached decls from remote decl store"
            start_t
        in
        ()

    let type_check
        ctx ~init_id ~check_id files_to_check ~state_filename ~telemetry =
      let t = Unix.gettimeofday () in
      Hh_logger.log "Type checking a batch...";
      let check_info =
        {
          init_id;
          check_reason = "remote_server_api";
          recheck_id = Some check_id;
          log_errors = true;
          use_max_typechecker_worker_memory_for_decl_deferral = false;
          per_file_profiling = HackEventLogger.PerFileProfilingConfig.default;
          memtrace_dir = None;
        }
      in
      (* It doesn't make sense for a remote worker to produce errors.bin itself. *)
      ServerProgress.enable_error_production false;
      (* TODO: use the telemetry *)
      let { Typing_check_service.errors; telemetry; _ } =
        Typing_check_service.go
          ctx
          workers
          Typing_service_delegate.default
          telemetry
          files_to_check
          ~root:None
          ~memory_cap:(Some 200000)
          ~longlived_workers:false
          ~use_hh_distc_instead_of_hulk:false
          ~hh_distc_fanout_threshold:None
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
