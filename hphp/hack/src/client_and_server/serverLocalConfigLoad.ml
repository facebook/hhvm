(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Config_file.Getters
open Hh_prelude
open Option.Monad_infix
open ServerLocalConfig

let output_config_section title output_config =
  Printf.eprintf "** %s:\n%!" title;
  output_config ();
  Printf.eprintf "\n%!";
  ()

let default =
  {
    saved_state = GlobalOptions.default_saved_state;
    min_log_level = Hh_logger.Level.Info;
    attempt_fix_credentials = false;
    log_categories = [];
    log_large_fanouts_threshold = None;
    log_init_proc_stack_also_on_absent_from = false;
    log_inference_constraints = false;
    experiments = [];
    experiments_config_meta = "";
    use_saved_state = false;
    use_saved_state_when_indexing = false;
    require_saved_state = true;
    load_state_natively = false;
    load_state_natively_download_timeout = 60;
    load_state_natively_dirty_files_timeout = 200;
    type_decl_bucket_size = 1000;
    extend_defs_per_file_bucket_size = 2000;
    enable_on_nfs = false;
    enable_fuzzy_search = true;
    max_purgatory_clients = 400;
    search_chunk_size = 0;
    io_priority = 7;
    cpu_priority = 10;
    shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir];
    shm_use_sharded_hashtbl = false;
    shm_cache_size = -1;
    max_workers = None;
    use_dummy_informant = true;
    informant_min_distance_restart = 100;
    use_full_fidelity_parser = true;
    interrupt_on_watchman = false;
    interrupt_on_client = false;
    trace_parsing = false;
    prechecked_files = true;
    enable_global_access_check = false;
    enable_type_check_filter_files = false;
    predeclare_ide = false;
    longlived_workers = false;
    hg_aware = false;
    hg_aware_parsing_restart_threshold = 0;
    hg_aware_redecl_restart_threshold = 0;
    hg_aware_recheck_restart_threshold = 0;
    ide_parser_cache = false;
    store_decls_in_saved_state = false;
    idle_gc_slice = 0;
    populate_member_heaps = true;
    fetch_remote_old_decls = true;
    skip_hierarchy_checks = false;
    skip_tast_checks = false;
    num_local_workers = None;
    defer_class_declaration_threshold = None;
    produce_streaming_errors = true;
    consume_streaming_errors = false;
    rust_elab = false;
    rust_provider_backend = false;
    naming_sqlite_path = None;
    enable_naming_table_fallback = false;
    ide_symbolindex_search_provider = "LocalIndex";
    symbolindex_quiet = false;
    tico_invalidate_files = false;
    tico_invalidate_smart = false;
    per_file_profiling = HackEventLogger.PerFileProfilingConfig.default;
    memtrace_dir = None;
    go_to_implementation = true;
    allow_unstable_features = false;
    watchman = Watchman.default;
    workload_quantile = None;
    rollout_group = None;
    specify_manifold_api_key = false;
    remote_old_decls_no_limit = false;
    cache_remote_decls = false;
    disable_naming_table_fallback_loading = false;
    use_distc = true;
    use_compressed_dep_graph = true;
    hh_distc_fanout_threshold = 250_000;
    hh_distc_exponential_backoff_num_retries = 10;
    ide_load_naming_table_on_disk = true;
    ide_naming_table_update_threshold = 1000;
    dump_tast_hashes = false;
    dump_tasts = [];
    lsp_sticky_quarantine = false;
    lsp_invalidation = false;
    invalidate_all_folded_decls_upon_file_change = false;
    autocomplete_sort_text = false;
    hack_warnings = true;
    warnings_default_all = false;
    warnings_in_sandcastle = true;
  }

let system_config_path =
  let dir =
    try Sys.getenv "HH_LOCALCONF_PATH" with
    | _ -> BuildOptions.system_config_path
  in
  Filename.concat dir "hh.conf"

(** Apply the following overrides in order:
  * JuskKnobs
  * Experiments
  * `overrides`
  *)
let apply_overrides ~silent ~current_version ~config ~from ~overrides =
  (* We'll apply CLI overrides now at the start so that JustKnobs and experiments_config
     can be informed about them, e.g. "--config rollout_group=foo" will be able
     to guide the manner in which JustKnobs picks up values, and "--config use_justknobs=false"
     will be able to disable it. Don't worry though -- we'll apply CLI overrides again at the end,
     so they overwrite any changes brought by JustKnobs and experiments_config. *)
  let config =
    Config_file.apply_overrides ~config ~overrides ~log_reason:None
  in
  (* Now is the time for JustKnobs *)
  let use_justknobs = bool_opt "use_justknobs" config in
  let config =
    match (use_justknobs, Sys_utils.deterministic_behavior_for_tests ()) with
    | (Some false, _)
    (* --config use_justknobs=false (or in hh.conf) will force JK off, regardless of anything else *)
    | (None, true)
    (* if use_justknobs isn't set, HH_TEST_MODE=1 (used in tests) will still turn JK off *)
      ->
      config
    | (Some true, _)
    (* --config use_justknobs=true (or in hh.conf) will force JK on, regardless of anything else *)
    | (None, false)
    (* if use_justknobs isn't set, then HH_TEST_MODE unset or =0 will leave JK on *)
      ->
      ServerLocalConfigKnobs.apply_justknobs_overrides ~silent config ~from
  in
  (* Now is the time for experiments_config overrides *)
  let experiments_enabled =
    bool_if_min_version
      "experiments_config_enabled"
      ~default:false
      ~current_version
      config
  in
  let (experiments_meta, config) =
    if experiments_enabled then begin
      Disk.mkdir_p GlobalConfig.tmp_dir;
      let dir =
        string_ "experiments_config_path" ~default:GlobalConfig.tmp_dir config
      in
      let owner = Sys_utils.get_primary_owner () in
      let file =
        Filename.concat dir (Printf.sprintf "hh.%s.experiments" owner)
      in
      let update =
        bool_if_min_version
          "experiments_config_update"
          ~default:false
          ~current_version
          config
      in
      let ttl =
        float_of_int
          (int_ "experiments_config_ttl_seconds" ~default:86400 config)
      in
      let source = string_opt "experiments_config_source" config in
      let meta =
        if update then
          match Experiments_config_file.update ~silent ~file ~source ~ttl with
          | Ok meta -> meta
          | Error message -> message
        else
          "Updating experimental config not enabled"
      in
      if Disk.file_exists file then
        (* Apply the experiments overrides *)
        let experiment_overrides = Config_file.parse_local_config file in
        let config =
          Config_file.apply_overrides
            ~config
            ~overrides:experiment_overrides
            ~log_reason:(Option.some_if (not silent) "Experiment_overrides")
        in
        (meta, config)
      else
        ("Experimental config not found on disk", config)
    end else
      ("Experimental config not enabled", config)
  in
  (* Finally, reapply the CLI overrides, since they should take
     precedence over the experiments_config and JustKnobs. *)
  let config =
    Config_file.apply_overrides
      ~config
      ~overrides
      ~log_reason:(Option.some_if (not silent) "--config")
  in
  (experiments_meta, config)

let load_
    system_config_path
    ~silent
    ~current_version
    ~current_rolled_out_flag_idx
    ~deactivate_saved_state_rollout
    ~from
    ~overrides : t =
  let config = Config_file.parse_local_config system_config_path in
  let (experiments_config_meta, config) =
    apply_overrides ~silent ~current_version ~config ~from ~overrides
  in
  (if not silent then
    output_config_section "Combined config" @@ fun () ->
    Config_file.print_to_stderr config);

  let experiments =
    string_list "experiments" ~default:default.experiments config
  in

  let log_categories =
    string_list "log_categories" ~default:default.log_categories config
  in
  let log_large_fanouts_threshold =
    int_opt "log_large_fanouts_threshold" config
  in
  let log_init_proc_stack_also_on_absent_from =
    bool_
      "log_init_proc_stack_also_on_absent_from"
      ~default:default.log_init_proc_stack_also_on_absent_from
      config
  in
  let log_inference_constraints =
    bool_
      "log_inference_constraints"
      ~default:default.log_inference_constraints
      config
  in
  let min_log_level =
    match
      Hh_logger.Level.of_enum_string
        (String.lowercase
           (string_
              "min_log_level"
              ~default:(Hh_logger.Level.to_enum_string default.min_log_level)
              config))
    with
    | Some level -> level
    | None -> Hh_logger.Level.Debug
  in

  let use_saved_state =
    bool_if_min_version
      "use_mini_state"
      ~default:default.use_saved_state
      ~current_version
      config
  in
  let use_saved_state_when_indexing =
    bool_if_min_version
      "use_mini_state_when_indexing"
      ~default:default.use_saved_state_when_indexing
      ~current_version
      config
  in
  let require_saved_state =
    bool_if_min_version
      "require_saved_state"
      ~default:default.require_saved_state
      ~current_version
      config
  in
  let saved_state_flags =
    Saved_state_rollouts.make
      ~get_default:(fun name -> bool_ name ~default:false config)
      ~current_rolled_out_flag_idx
      ~deactivate_saved_state_rollout
      ~force_flag_value:(string_opt "ss_force" config)
  in
  (if not silent then
    output_config_section "Saved state rollout flags" @@ fun () ->
    Saved_state_rollouts.output saved_state_flags);
  let project_metadata_w_flags =
    bool_
      "project_metadata_w_flags"
      ~default:default.saved_state.GlobalOptions.project_metadata_w_flags
      config
  in
  let attempt_fix_credentials =
    bool_if_min_version
      "attempt_fix_credentials"
      ~default:default.attempt_fix_credentials
      ~current_version
      config
  in
  let enable_on_nfs =
    bool_if_min_version
      "enable_on_nfs"
      ~default:default.enable_on_nfs
      ~current_version
      config
  in
  let enable_fuzzy_search =
    bool_if_min_version
      "enable_fuzzy_search"
      ~default:default.enable_fuzzy_search
      ~current_version
      config
  in
  let enable_global_access_check =
    bool_
      "enable_global_access_check"
      ~default:default.enable_global_access_check
      config
  in
  let max_purgatory_clients =
    int_ "max_purgatory_clients" ~default:default.max_purgatory_clients config
  in
  let search_chunk_size =
    int_ "search_chunk_size" ~default:default.search_chunk_size config
  in
  let load_state_natively =
    bool_if_min_version
      "load_state_natively_v4"
      ~default:default.load_state_natively
      ~current_version
      config
  in
  let load_state_natively_download_timeout =
    int_
      "load_state_natively_download_timeout"
      ~default:default.load_state_natively_download_timeout
      config
  in
  let load_state_natively_dirty_files_timeout =
    int_
      "load_state_natively_dirty_files_timeout"
      ~default:default.load_state_natively_dirty_files_timeout
      config
  in
  let use_dummy_informant =
    bool_if_min_version
      "use_dummy_informant"
      ~default:default.use_dummy_informant
      ~current_version
      config
  in
  let informant_min_distance_restart =
    int_
      "informant_min_distance_restart"
      ~default:default.informant_min_distance_restart
      config
  in
  let type_decl_bucket_size =
    int_ "type_decl_bucket_size" ~default:default.type_decl_bucket_size config
  in
  let extend_defs_per_file_bucket_size =
    int_
      "extend_defs_per_file_bucket_size"
      ~default:default.extend_defs_per_file_bucket_size
      config
  in
  let io_priority = int_ "io_priority" ~default:default.io_priority config in
  let cpu_priority = int_ "cpu_priority" ~default:default.cpu_priority config in
  let shm_dirs =
    string_list "shm_dirs" ~default:default.shm_dirs config
    |> List.map ~f:(fun dir -> Path.(to_string @@ make dir))
  in
  let shm_use_sharded_hashtbl =
    bool_if_min_version
      "shm_use_sharded_hashtbl"
      ~default:default.shm_use_sharded_hashtbl
      ~current_version
      config
  in
  let shm_cache_size =
    int_ "shm_cache_size" ~default:default.shm_cache_size config
  in
  let max_workers = int_opt "max_workers" config in
  let interrupt_on_watchman =
    bool_if_min_version
      "interrupt_on_watchman"
      ~default:default.interrupt_on_watchman
      ~current_version
      config
  in
  let interrupt_on_client =
    bool_if_min_version
      "interrupt_on_client"
      ~default:default.interrupt_on_client
      ~current_version
      config
  in
  let use_full_fidelity_parser =
    bool_if_min_version
      "use_full_fidelity_parser"
      ~default:default.use_full_fidelity_parser
      ~current_version
      config
  in
  let trace_parsing =
    bool_if_min_version
      "trace_parsing"
      ~default:default.trace_parsing
      ~current_version
      config
  in
  let prechecked_files =
    bool_if_min_version
      "prechecked_files"
      ~default:default.prechecked_files
      ~current_version
      config
  in
  let enable_type_check_filter_files =
    bool_if_min_version
      "enable_type_check_filter_files"
      ~default:default.enable_type_check_filter_files
      ~current_version
      config
  in
  let predeclare_ide =
    bool_if_min_version
      "predeclare_ide"
      ~default:default.predeclare_ide
      ~current_version
      config
  in
  let longlived_workers =
    bool_if_min_version
      "longlived_workers"
      ~default:default.longlived_workers
      ~current_version
      config
  in
  let hg_aware =
    bool_if_min_version
      "hg_aware"
      ~default:default.hg_aware
      ~current_version
      config
  in
  let store_decls_in_saved_state =
    bool_if_min_version
      "store_decls_in_saved_state"
      ~default:default.store_decls_in_saved_state
      ~current_version
      config
  in
  let hg_aware_parsing_restart_threshold =
    int_
      "hg_aware_parsing_restart_threshold"
      ~default:default.hg_aware_parsing_restart_threshold
      config
  in
  let hg_aware_redecl_restart_threshold =
    int_
      "hg_aware_redecl_restart_threshold"
      ~default:default.hg_aware_redecl_restart_threshold
      config
  in
  let hg_aware_recheck_restart_threshold =
    int_
      "hg_aware_recheck_restart_threshold"
      ~default:default.hg_aware_recheck_restart_threshold
      config
  in
  let ide_parser_cache =
    bool_if_min_version
      "ide_parser_cache"
      ~default:default.ide_parser_cache
      ~current_version
      config
  in
  let idle_gc_slice =
    int_ "idle_gc_slice" ~default:default.idle_gc_slice config
  in
  let populate_member_heaps =
    bool_if_min_version
      "populate_member_heaps"
      ~default:default.populate_member_heaps
      ~current_version
      config
  in
  let fetch_remote_old_decls =
    bool_if_min_version
      "fetch_remote_old_decls"
      ~default:default.fetch_remote_old_decls
      ~current_version
      config
  in
  let skip_hierarchy_checks =
    bool_if_min_version
      "skip_hierarchy_checks"
      ~default:default.skip_hierarchy_checks
      ~current_version
      config
  in
  let skip_tast_checks =
    bool_if_min_version
      "skip_tast_checks"
      ~default:default.skip_tast_checks
      ~current_version
      config
  in
  let num_local_workers = int_opt "num_local_workers" config in
  let defer_class_declaration_threshold =
    int_opt "defer_class_declaration_threshold" config
  in
  let produce_streaming_errors =
    bool_
      "produce_streaming_errors"
      ~default:default.produce_streaming_errors
      config
  in
  let consume_streaming_errors =
    bool_
      "consume_streaming_errors"
      ~default:default.consume_streaming_errors
      config
  in
  let watchman =
    Watchman.load ~current_version ~default:default.watchman config
  in
  let enable_naming_table_fallback =
    bool_if_min_version
      "enable_naming_table_fallback"
      ~default:default.enable_naming_table_fallback
      ~current_version
      config
  in
  let naming_sqlite_path =
    if enable_naming_table_fallback then
      string_opt "naming_sqlite_path" config
    else
      None
  in
  let ide_symbolindex_search_provider =
    string_
      "ide_symbolindex_search_provider"
      ~default:default.ide_symbolindex_search_provider
      config
  in
  let symbolindex_quiet =
    bool_if_min_version
      "symbolindex_quiet"
      ~default:default.symbolindex_quiet
      ~current_version
      config
  in
  let tico_invalidate_files =
    bool_if_min_version
      "tico_invalidate_files"
      ~default:default.tico_invalidate_files
      ~current_version
      config
  in
  let tico_invalidate_smart =
    bool_if_min_version
      "tico_invalidate_smart"
      ~default:default.tico_invalidate_smart
      ~current_version
      config
  in
  let profile_log =
    bool_if_min_version
      "profile_log"
      ~default:HackEventLogger.PerFileProfilingConfig.(default.profile_log)
      ~current_version
      config
  in
  let profile_type_check_duration_threshold =
    float_
      "profile_type_check_duration_threshold"
      ~default:
        HackEventLogger.PerFileProfilingConfig.(
          default.profile_type_check_duration_threshold)
      config
  in
  let profile_type_check_memory_threshold_mb =
    int_
      "profile_type_check_memory_threshold_mb"
      ~default:
        HackEventLogger.PerFileProfilingConfig.(
          default.profile_type_check_memory_threshold_mb)
      config
  in
  let profile_type_check_twice =
    bool_if_min_version
      "profile_type_check_twice"
      ~default:
        HackEventLogger.PerFileProfilingConfig.(
          default.profile_type_check_twice)
      ~current_version
      config
  in
  let profile_decling =
    match string_opt "profile_decling" config with
    | None ->
      default.per_file_profiling
        .HackEventLogger.PerFileProfilingConfig.profile_decling
    | Some value_s ->
      (match
         HackEventLogger.PerFileProfilingConfig.ProfileDecling.of_config_value
           value_s
       with
      | Some x -> x
      | None ->
        failwith
        @@ Printf.sprintf
             "Unrecognized value %s for profile_decling. Allowed values ar 'off', 'top_counts', 'all_telemetry', 'all_telemetry_callstacks'"
             value_s)
  in
  let profile_owner = string_opt "profile_owner" config in
  let profile_desc = string_opt "profile_desc" config in
  let profile_slow_threshold =
    float_
      "profile_slow_threshold"
      ~default:
        HackEventLogger.PerFileProfilingConfig.(default.profile_slow_threshold)
      config
  in
  let memtrace_dir = string_opt "memtrace_dir" config in
  let go_to_implementation =
    bool_if_min_version
      "go_to_implementation"
      ~default:default.go_to_implementation
      ~current_version
      config
  in
  let allow_unstable_features =
    bool_if_min_version
      "allow_unstable_features"
      ~default:default.allow_unstable_features
      ~current_version
      config
  in
  let log_saved_state_age_and_distance =
    bool_if_min_version
      "log_saved_state_age_and_distance"
      ~default:
        GlobalOptions.(
          default_saved_state_loading.log_saved_state_age_and_distance)
      ~current_version
      config
  in
  let use_manifold_cython_client =
    bool_if_min_version
      "use_manifold_cython_client"
      ~default:
        GlobalOptions.(default_saved_state_loading.use_manifold_cython_client)
      ~current_version
      config
  in
  let workload_quantile =
    int_list_opt "workload_quantile" config >>= fun l ->
    match l with
    | [m; n] ->
      if 0 <= m && m <= n then
        Some { count = n; index = m }
      else if 0 <= n && n <= m then
        Some { count = m; index = n }
      else
        None
    | _ -> None
  in
  let rollout_group = string_opt "rollout_group" config in
  let specify_manifold_api_key =
    bool_if_min_version
      "specify_manifold_api_key"
      ~default:default.specify_manifold_api_key
      ~current_version
      config
  in
  let remote_old_decls_no_limit =
    bool_if_min_version
      "remote_old_decls_no_limit"
      ~default:default.remote_old_decls_no_limit
      ~current_version
      config
  in
  let saved_state_manifold_api_key =
    (* overriding the local_config value so consumers of saved_state_manifold_api_key
       * don't need to explicitly check for specify_manifold_api_key.
    *)
    if specify_manifold_api_key then
      string_opt "saved_state_manifold_api_key" config
    else
      None
  in
  let rust_elab =
    bool_if_min_version
      "rust_elab"
      ~default:default.rust_elab
      ~current_version
      config
  in
  let rust_provider_backend =
    bool_if_min_version
      "rust_provider_backend"
      ~default:default.rust_provider_backend
      ~current_version
      config
  in
  let rust_provider_backend =
    if rust_provider_backend && not shm_use_sharded_hashtbl then (
      Hh_logger.warn
        "You have rust_provider_backend=true but shm_use_sharded_hashtbl=false. This is incompatible. Turning off rust_provider_backend";
      false
    ) else
      rust_provider_backend
  in
  let rust_provider_backend =
    if rust_provider_backend && populate_member_heaps then (
      Hh_logger.warn
        "You have rust_provider_backend=true but populate_member_heaps=true. This is incompatible. Turning off rust_provider_backend";
      false
    ) else
      rust_provider_backend
  in
  let cache_remote_decls =
    bool_if_min_version
      "cache_remote_decls"
      ~default:default.cache_remote_decls
      ~current_version
      config
  in
  let disable_naming_table_fallback_loading =
    bool_if_min_version
      "disable_naming_table_fallback_loading"
      ~default:default.disable_naming_table_fallback_loading
      ~current_version
      config
  in
  let use_distc =
    bool_if_min_version
      "use_distc"
      ~default:default.use_distc
      ~current_version
      config
  in
  let use_compressed_dep_graph =
    bool_if_min_version
      "use_compressed_dep_graph"
      ~default:default.use_compressed_dep_graph
      ~current_version
      config
  in
  let hh_distc_fanout_threshold =
    int_
      "hh_distc_fanout_threshold"
      ~default:default.hh_distc_fanout_threshold
      config
  in
  let hh_distc_exponential_backoff_num_retries =
    int_
      "hh_distc_exponential_backoff_num_retries"
      ~default:default.hh_distc_exponential_backoff_num_retries
      config
  in
  let ide_load_naming_table_on_disk =
    bool_if_min_version
      "ide_load_naming_table_on_disk"
      ~default:default.ide_load_naming_table_on_disk
      ~current_version
      config
  in
  let ide_naming_table_update_threshold =
    int_
      "ide_naming_table_update_threshold"
      ~default:default.ide_naming_table_update_threshold
      config
  in
  let dump_tast_hashes =
    bool_ "dump_tast_hashes" ~default:default.dump_tast_hashes config
  in
  let dump_tasts =
    let path_opt = string_opt "dump_tasts" config in
    match path_opt with
    | None -> default.dump_tasts
    | Some path -> In_channel.read_lines path
  in
  let lsp_sticky_quarantine =
    bool_ "lsp_sticky_quarantine" ~default:default.lsp_sticky_quarantine config
  in
  let lsp_invalidation =
    bool_ "lsp_invalidation" ~default:default.lsp_invalidation config
  in
  let invalidate_all_folded_decls_upon_file_change =
    bool_
      "invalidate_all_folded_decls_upon_file_change"
      ~default:default.invalidate_all_folded_decls_upon_file_change
      config
  in
  let autocomplete_sort_text =
    bool_
      "autocomplete_sort_text"
      ~default:default.autocomplete_sort_text
      config
  in
  let hack_warnings =
    bool_ "hack_warnings" ~default:default.hack_warnings config
  in
  let zstd_decompress_by_file =
    bool_
      "zstd_decompress_by_file"
      ~default:
        GlobalOptions.(default_saved_state_loading.zstd_decompress_by_file)
      config
  in
  let warnings_default_all =
    bool_ "warnings_default_all" ~default:default.warnings_default_all config
  in
  let warnings_in_sandcastle =
    bool_
      "warnings_in_sandcastle"
      ~default:default.warnings_in_sandcastle
      config
  in
  {
    saved_state =
      {
        GlobalOptions.loading =
          {
            GlobalOptions.saved_state_manifold_api_key;
            log_saved_state_age_and_distance;
            use_manifold_cython_client;
            zstd_decompress_by_file;
            use_compressed_dep_graph;
          };
        rollouts = saved_state_flags;
        project_metadata_w_flags;
      };
    min_log_level;
    attempt_fix_credentials;
    log_categories;
    log_large_fanouts_threshold;
    log_init_proc_stack_also_on_absent_from;
    log_inference_constraints;
    experiments;
    experiments_config_meta;
    use_saved_state;
    use_saved_state_when_indexing;
    require_saved_state;
    load_state_natively;
    load_state_natively_download_timeout;
    load_state_natively_dirty_files_timeout;
    max_purgatory_clients;
    type_decl_bucket_size;
    extend_defs_per_file_bucket_size;
    enable_on_nfs;
    enable_fuzzy_search;
    enable_global_access_check;
    search_chunk_size;
    io_priority;
    cpu_priority;
    shm_dirs;
    shm_use_sharded_hashtbl;
    shm_cache_size;
    max_workers;
    use_dummy_informant;
    informant_min_distance_restart;
    use_full_fidelity_parser;
    interrupt_on_watchman;
    interrupt_on_client;
    trace_parsing;
    prechecked_files;
    enable_type_check_filter_files;
    ide_symbolindex_search_provider;
    predeclare_ide;
    longlived_workers;
    hg_aware;
    hg_aware_parsing_restart_threshold;
    hg_aware_redecl_restart_threshold;
    hg_aware_recheck_restart_threshold;
    ide_parser_cache;
    store_decls_in_saved_state;
    idle_gc_slice;
    populate_member_heaps;
    fetch_remote_old_decls;
    skip_hierarchy_checks;
    skip_tast_checks;
    num_local_workers;
    defer_class_declaration_threshold;
    produce_streaming_errors;
    consume_streaming_errors;
    rust_elab;
    rust_provider_backend;
    naming_sqlite_path;
    enable_naming_table_fallback;
    symbolindex_quiet;
    tico_invalidate_files;
    tico_invalidate_smart;
    per_file_profiling =
      {
        HackEventLogger.PerFileProfilingConfig.profile_log;
        profile_type_check_duration_threshold;
        profile_type_check_memory_threshold_mb;
        profile_type_check_twice;
        profile_decling;
        profile_owner;
        profile_desc;
        profile_slow_threshold;
      };
    memtrace_dir;
    go_to_implementation;
    allow_unstable_features;
    watchman;
    workload_quantile;
    rollout_group;
    specify_manifold_api_key;
    remote_old_decls_no_limit;
    cache_remote_decls;
    disable_naming_table_fallback_loading;
    use_distc;
    use_compressed_dep_graph;
    hh_distc_fanout_threshold;
    hh_distc_exponential_backoff_num_retries;
    ide_load_naming_table_on_disk;
    ide_naming_table_update_threshold;
    dump_tast_hashes;
    dump_tasts;
    lsp_sticky_quarantine;
    lsp_invalidation;
    invalidate_all_folded_decls_upon_file_change;
    autocomplete_sort_text;
    hack_warnings;
    warnings_default_all;
    warnings_in_sandcastle;
  }

let load :
    silent:bool ->
    current_version:Config_file_version.version ->
    current_rolled_out_flag_idx:int ->
    deactivate_saved_state_rollout:bool ->
    from:string ->
    overrides:Config_file_common.t ->
    t =
  load_ system_config_path

let to_rollout_flags (options : t) : HackEventLogger.rollout_flags =
  HackEventLogger.
    {
      log_saved_state_age_and_distance =
        GlobalOptions.(
          options.saved_state.loading.log_saved_state_age_and_distance);
      fetch_remote_old_decls = options.fetch_remote_old_decls;
      specify_manifold_api_key = options.specify_manifold_api_key;
      remote_old_decls_no_limit = options.remote_old_decls_no_limit;
      populate_member_heaps = options.populate_member_heaps;
      shm_use_sharded_hashtbl = options.shm_use_sharded_hashtbl;
      shm_cache_size = options.shm_cache_size;
      use_manifold_cython_client =
        GlobalOptions.(options.saved_state.loading.use_manifold_cython_client);
      disable_naming_table_fallback_loading =
        options.disable_naming_table_fallback_loading;
      load_state_natively_v4 = options.load_state_natively;
      rust_provider_backend = options.rust_provider_backend;
      use_distc = options.use_distc;
      use_compressed_dep_graph = options.use_compressed_dep_graph;
      consume_streaming_errors = options.consume_streaming_errors;
      hh_distc_fanout_threshold = options.hh_distc_fanout_threshold;
      rust_elab = options.rust_elab;
      ide_load_naming_table_on_disk = options.ide_load_naming_table_on_disk;
      ide_naming_table_update_threshold =
        options.ide_naming_table_update_threshold;
      saved_state_rollouts = options.saved_state.GlobalOptions.rollouts;
      zstd_decompress_by_file =
        GlobalOptions.(options.saved_state.loading.zstd_decompress_by_file);
      lsp_sticky_quarantine = options.lsp_sticky_quarantine;
      lsp_invalidation = options.lsp_invalidation;
      invalidate_all_folded_decls_upon_file_change =
        options.invalidate_all_folded_decls_upon_file_change;
      autocomplete_sort_text = options.autocomplete_sort_text;
      warnings_default_all = options.warnings_default_all;
    }
