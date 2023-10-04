(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let purpose = "Profile Decl Folding on www repo"

let usage =
  Printf.sprintf "Usage: %s <options> www-root\n%s" Sys.argv.(0) purpose

let make_workers
    (root : Path.t)
    (server_config : ServerConfig.t)
    (server_local_config : ServerLocalConfig.t)
    ~(longlived_workers : bool)
    ~(log_sharedmem_stats : bool) : MultiWorker.worker list =
  let num_workers = Sys_utils.nbr_procs in
  let gc_control = Gc.get () in
  let hhconfig_version =
    server_config |> ServerConfig.version |> Config_file.version_to_string_opt
  in
  let shmem_config = ServerConfig.sharedmem_config server_config in
  let log_level =
    if log_sharedmem_stats then
      1
    else
      0
  in
  let heap_handle =
    SharedMem.(init ~num_workers { shmem_config with log_level })
  in
  ServerWorker.make
    ~longlived_workers
    ~nbr_procs:num_workers
    gc_control
    heap_handle
    ~logging_init:(fun () ->
      HackEventLogger.init_worker
        ~root
        ~custom_columns:[]
        ~always_add_sandcastle_info:
          server_local_config.ServerLocalConfig.log_events_with_sandcastle_info
        ~rollout_flags:(ServerLocalConfig.to_rollout_flags server_local_config)
        ~rollout_group:server_local_config.ServerLocalConfig.rollout_group
        ~hhconfig_version
        ~init_id:(Random_id.short_string ())
        ~time:(Unix.gettimeofday ())
        ~per_file_profiling:
          server_local_config.ServerLocalConfig.per_file_profiling)

let measure_time (action : string) (f : unit -> 'a) : 'a =
  Hh_logger.log "Start %s..." action;
  let start_time = Unix.gettimeofday () in
  let result = f () in
  let _ =
    Hh_logger.log_duration (Printf.sprintf "Finished %s" action) start_time
  in
  result

let init
    (root : Path.t)
    (naming_table_path : string option)
    ~(longlived_workers : bool)
    ~(rust_provider_backend : bool)
    ~(log_sharedmem_stats : bool) :
    Provider_context.t * Naming_table.t option * MultiWorker.worker list option
    =
  Relative_path.set_path_prefix Relative_path.Root root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmpdir_NOT_USED");
  Relative_path.set_path_prefix Relative_path.Hhi (Hhi.get_hhi_root ());

  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  let server_args =
    ServerArgs.(
      set_config
        server_args
        ([
           ("rust_provider_backend", string_of_bool rust_provider_backend);
           ("shm_use_sharded_hashtbl", "true");
           ("shm_cache_size", string_of_int (60 * (1024 * 1024 * 1024)));
           ("populate_member_heaps", "false");
         ]
        @ config server_args))
  in
  let (server_config, server_local_config) =
    ServerConfig.load ~silent:true server_args
  in
  let popt = ServerConfig.parser_options server_config in
  let tcopt = { popt with GlobalOptions.tco_higher_kinded_types = true } in
  if rust_provider_backend then Provider_backend.set_rust_backend popt;
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.get ())
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let workers =
    make_workers
      root
      server_config
      server_local_config
      ~longlived_workers
      ~log_sharedmem_stats
  in
  Hh_logger.log
    "About to decl %s with %d workers %s"
    (Path.to_string root)
    (List.length workers)
    (match naming_table_path with
    | Some path -> Printf.sprintf "with a naming table at %s" path
    | None -> "without a naming table");
  let naming_table =
    Option.map naming_table_path ~f:(fun path ->
        if Sys.file_exists path then
          Naming_table.load_from_sqlite ctx path
        else
          failwith (Printf.sprintf "The file '%s' does not exist" path))
  in
  (ctx, naming_table, Some workers)

let parse_repo
    (ctx : Provider_context.t)
    (root : Path.t)
    (workers : MultiWorker.worker list option) : FileInfo.t Relative_path.Map.t
    =
  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:
        (Find.make_next_files ~name:"root" ~filter:FindUtils.is_hack root)
      ~extra_roots:(ServerConfig.extra_paths ServerConfig.default_config)
  in
  measure_time "parsing repo" @@ fun () ->
  Direct_decl_service.go ctx workers ~get_next ~trace:false ~cache_decls:true

let populate_naming_table
    (ctx : Provider_context.t)
    (file_summaries : FileInfo.t Relative_path.Map.t)
    (workers : MultiWorker.worker list option) : unit =
  measure_time "populating naming table" @@ fun () ->
  let summaries = Relative_path.Map.elements file_summaries in
  MultiWorker.call
    workers
    ~job:(fun _ batch ->
      List.iter
        ~f:(fun (path, fileinfo) ->
          Naming_global.ndecl_file_skip_if_already_bound ctx path fileinfo)
        batch)
    ~merge:(fun _ _ -> ())
    ~neutral:()
    ~next:(MultiWorker.next workers summaries)

let fold_single_file
    (ctx : Provider_context.t)
    ((_file, summary) : Relative_path.t * FileInfo.names) : unit =
  let class_names = summary.FileInfo.n_classes in
  SSet.iter
    (fun class_name ->
      match Decl_provider.get_class ctx class_name with
      | Some _ -> ()
      | None -> failwith ("missing class: " ^ class_name))
    class_names

let fold_repo
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (summaries : (Relative_path.t * FileInfo.names) list) : unit =
  measure_time "folding repo" @@ fun () ->
  MultiWorker.call
    workers
    ~job:(fun _ batch -> List.iter ~f:(fold_single_file ctx) batch)
    ~merge:(fun _ _ -> ())
    ~neutral:()
    ~next:(MultiWorker.next workers summaries)

let () =
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();
  let repo = ref None in
  let naming_table_path = ref None in
  let longlived_workers = ref false in
  let rust_provider_backend = ref false in
  let log_sharedmem_stats = ref false in
  let num_files = ref None in
  let args =
    [
      ( "--naming-table",
        Arg.String (fun s -> naming_table_path := Some s),
        " Path to a SQLite naming table (allowing parsing to be done lazily instead of up-front)"
      );
      ( "--longlived-workers",
        Arg.Set longlived_workers,
        " Enable longlived worker processes" );
      ( "--rust-provider-backend",
        Arg.Set rust_provider_backend,
        " Use the Rust implementation of Provider_backend (including decl-folding)"
      );
      ( "--num-files",
        Arg.Int (fun n -> num_files := Some n),
        " Fold only this number of files instead of the entire repository" );
      ( "--log-sharedmem-stats",
        Arg.Set log_sharedmem_stats,
        " Record sharedmem telemetry and print before exit" );
    ]
  in
  Arg.parse args (fun filepath -> repo := Some filepath) usage;

  let root =
    match !repo with
    | Some root -> Path.make root
    | _ ->
      Arg.usage args usage;
      Exit.exit Exit_status.Input_error
  in
  let (ctx, naming_table_opt, workers) =
    init
      root
      !naming_table_path
      ~longlived_workers:!longlived_workers
      ~rust_provider_backend:!rust_provider_backend
      ~log_sharedmem_stats:!log_sharedmem_stats
  in
  let file_summaries =
    match naming_table_opt with
    | Some naming_table ->
      (try
         Naming_table.to_defs_per_file
           ~warn_on_naming_costly_iter:false
           naming_table
       with
      | _ ->
        Hh_logger.log
          "Failed to query classes from the naming table. Falling back to parsing the whole repo.";
        parse_repo ctx root workers
        |> Relative_path.Map.map ~f:FileInfo.simplify)
    | None ->
      let file_summaries = parse_repo ctx root workers in
      populate_naming_table ctx file_summaries workers;
      Relative_path.Map.map file_summaries ~f:FileInfo.simplify
  in
  let files_in_repo = Relative_path.Map.cardinal file_summaries in
  Hh_logger.log "Collected %d file summaries" files_in_repo;

  let file_summaries =
    match !num_files with
    | None -> file_summaries
    | Some n ->
      let files = Relative_path.Map.elements file_summaries in
      let files = List.drop files ((files_in_repo - n) / 2) in
      let files = List.take files n in
      Relative_path.Map.of_list files
  in

  let with_shmem_logging f =
    if SharedMem.SMTelemetry.hh_log_level () > 0 then Measure.push_global ();
    f ();
    if SharedMem.SMTelemetry.hh_log_level () > 0 then
      Measure.print_stats () ~record:(Measure.pop_global ())
  in

  Hh_logger.log "Phase: writing folded decls";
  with_shmem_logging (fun () ->
      fold_repo ctx workers (Relative_path.Map.elements file_summaries));

  Hh_logger.log "Phase: reading back folded decls";
  with_shmem_logging (fun () ->
      fold_repo ctx workers (Relative_path.Map.elements file_summaries));

  ()
