(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Config_file.Getters
module Hack_bucket = Bucket
open Hh_prelude
open Option.Monad_infix
module Bucket = Hack_bucket

let output_config_section title output_config =
  Printf.eprintf "** %s:\n%!" title;
  output_config ();
  Printf.eprintf "\n%!";
  ()

module Watchman = struct
  type t = {
    (* use_watchman *)
    enabled: bool;
    (* in seconds *)
    debug_logging: bool;
    init_timeout: int;
    sockname: string option;
    subscribe: bool;
    (* in seconds *)
    synchronous_timeout: int;
  }

  let default =
    {
      debug_logging = false;
      enabled = false;
      (* buck and hgwatchman use a 10 second timeout, too *)
      init_timeout = 10;
      sockname = None;
      subscribe = false;
      synchronous_timeout = 120;
    }

  let load ~current_version ~default config =
    let use_watchman =
      bool_if_min_version
        "use_watchman"
        ~default:default.enabled
        ~current_version
        config
    in
    let enabled =
      bool_if_min_version
        "watchman_enabled"
        ~default:use_watchman
        ~current_version
        config
    in
    let init_timeout =
      int_ "watchman_init_timeout" ~default:default.init_timeout config
    in
    let sockname = string_opt "watchman_sockname" config in
    let subscribe =
      bool_if_min_version
        "watchman_subscribe_v2"
        ~default:default.subscribe
        ~current_version
        config
    in
    let synchronous_timeout =
      int_
        "watchman_synchronous_timeout"
        ~default:default.synchronous_timeout
        config
    in
    let debug_logging =
      bool_if_min_version
        "watchman_debug_logging"
        ~default:default.debug_logging
        ~current_version
        config
    in
    {
      debug_logging;
      enabled;
      init_timeout;
      sockname;
      subscribe;
      synchronous_timeout;
    }
end

module RemoteTypeCheck = struct
  type t = {
    (* Controls the `defer_class_declaration_threshold` setting on the remote worker *)
    declaration_threshold: int;
    (* A list of error phases; if, before type checking, errors in these phases
        are present, then remote type checking will be disabled *)
    disabled_on_errors: Errors.phase list;
    (* Enables remote type check *)
    enabled: bool;
    (* A non-interactive host is, e.g., a dev host not currently associated with a user,
        or a host used for non-interactive jobs (e.g., CI) *)
    enabled_for_noninteractive_hosts: bool;
    (* Dictates the number of remote type checking workers *)
    num_workers: int;
    (* The sandcastle tenant used to spawn remote workers *)
    remote_worker_sandcastle_tenant: string;
    prefetch_deferred_files: bool;
    worker_min_log_level: Hh_logger.Level.t;
    remote_type_check_recheck_threshold: int;
    (* Whether the local typechecker steals workloads from remote jobs *)
    work_stealing_enabled: bool;
  }

  let default =
    {
      enabled = false;
      enabled_for_noninteractive_hosts = false;
      declaration_threshold = 50;
      disabled_on_errors = [];
      num_workers = 4;
      remote_worker_sandcastle_tenant = "interactive";
      prefetch_deferred_files = false;
      worker_min_log_level = Hh_logger.Level.Info;
      remote_type_check_recheck_threshold = 1_000_000;
      work_stealing_enabled = false;
    }

  let load ~current_version ~default config =
    let declaration_threshold =
      int_
        "remote_type_check_declaration_threshold"
        ~default:default.declaration_threshold
        config
    in
    let enabled_on_errors =
      string_list
        "remote_type_check_enabled_on_errors"
        ~default:["typing"]
        config
      |> List.fold ~init:[] ~f:(fun acc phase ->
             match Errors.phase_of_string phase with
             | Some phase -> phase :: acc
             | None -> acc)
    in
    let disabled_on_errors =
      List.filter
        [Errors.Typing; Errors.Decl; Errors.Parsing; Errors.Init; Errors.Naming]
        ~f:(fun phase ->
          not
            (List.exists enabled_on_errors ~f:(fun enabled_phase ->
                 Errors.equal_phase enabled_phase phase)))
    in
    let num_workers =
      int_ "remote_type_check_num_workers" ~default:default.num_workers config
    in
    let remote_worker_sandcastle_tenant =
      string_
        "remote_worker_sandcastle_tenant"
        ~default:default.remote_worker_sandcastle_tenant
        config
    in
    let prefetch_deferred_files =
      bool_if_min_version
        "remote_type_check_prefetch_deferred_files"
        ~default:default.prefetch_deferred_files
        ~current_version
        config
    in
    let remote_type_check_recheck_threshold =
      int_
        "remote_type_check_recheck_threshold"
        ~default:default.remote_type_check_recheck_threshold
        config
    in
    let enabled =
      bool_if_min_version
        "remote_type_check_enabled"
        ~default:default.enabled
        ~current_version
        config
    in
    let enabled_for_noninteractive_hosts =
      bool_if_min_version
        "remote_type_check_enabled_for_noninteractive_hosts"
        ~default:default.enabled_for_noninteractive_hosts
        ~current_version
        config
    in
    let worker_min_log_level =
      match
        Hh_logger.Level.of_enum_string
          (String.lowercase
             (string_
                "remote_type_check_worker_min_log_level"
                ~default:
                  (Hh_logger.Level.to_enum_string default.worker_min_log_level)
                config))
      with
      | Some level -> level
      | None -> Hh_logger.Level.Debug
    in
    let work_stealing_enabled =
      bool_if_min_version
        "remote_type_check_work_stealing_enabled"
        ~default:default.work_stealing_enabled
        ~current_version
        config
    in
    {
      declaration_threshold;
      disabled_on_errors;
      enabled;
      enabled_for_noninteractive_hosts;
      num_workers;
      prefetch_deferred_files;
      remote_type_check_recheck_threshold;
      worker_min_log_level;
      remote_worker_sandcastle_tenant;
      work_stealing_enabled;
    }
end

module RecheckCapture = struct
  type t = {
    (* Enables recheck environment capture *)
    enabled: bool;
    (* If the error theshold is not met, then the recheck environment that
        doesn't meet the fanout-related thresholds will not be captured. *)
    error_threshold: int;
    (* We will automatically capture the recheck environment if
        the number of files to recheck exceeds this threshold.
        The number of rechecked files is always less than or equal to
        the fanout *)
    fanout_threshold: int;
    (* We will automatically capture the recheck environment if
        the number of rechecked files exceeds this threshold *)
    rechecked_files_threshold: int;
    (* If set, determines the rate of sampling of rechecks regardless of
        their fanout size. The error threshold would still apply.
        NOTE: valid values fall between 0.0 (0%) and 1.0 (100%).
        Values less than 0.0 will be interpreted as 0.0; values greater than
        1.0 will be interpreted as 1.0 *)
    sample_threshold: float;
  }

  let default =
    {
      enabled = false;
      (* We wouldn't capture small rechecks unless they have at least
              this many errors. *)
      error_threshold = 1;
      (* If capturing is enabled and the recheck fanout (pre-type-check)
              meets this threshold, then we would snapshot the changed files. *)
      fanout_threshold = 40_000;
      (* If the number of files actually rechecked meets this threshold
              and we already snapshotted the changed files based on fanout
              size or sampling, we would capture the recheck environment. *)
      rechecked_files_threshold = 5_000;
      (* We wouldn't take changed files snapshots of small fanouts
              unless they are randomly selected with the probability controlled
              by the sample_threshold setting. By default, we don't snapshot
              any small fanouts. *)
      sample_threshold = 0.0;
    }

  let load ~current_version ~default config =
    let enabled =
      bool_if_min_version
        "recheck_capture_enabled"
        ~default:default.enabled
        ~current_version
        config
    in
    let error_threshold =
      int_
        "recheck_capture_error_threshold"
        ~default:default.error_threshold
        config
    in
    let fanout_threshold =
      int_
        "recheck_capture_fanout_threshold"
        ~default:default.fanout_threshold
        config
    in
    let rechecked_files_threshold =
      int_
        "recheck_capture_rechecked_files_threshold"
        ~default:default.rechecked_files_threshold
        config
    in
    let sample_threshold =
      let sample_threshold =
        float_
          "recheck_capture_sample_threshold"
          ~default:default.sample_threshold
          config
      in
      if Float.(sample_threshold > 1.0) then
        1.0
      else if Float.(sample_threshold < 0.0) then
        0.0
      else
        sample_threshold
    in
    {
      enabled;
      error_threshold;
      fanout_threshold;
      rechecked_files_threshold;
      sample_threshold;
    }
end

(** Allows to typecheck only a certain quantile of the workload. *)
type quantile = {
  count: int;
      (** The number of quantiles we want.
          If this is n, we'll divide the workload in n groups. *)
  index: int;
      (** The index of the subgroup we'll process.
          If this is i, we'll typecheck group number i out of the n groups.
          this should be in interval [0; n] *)
}

type t = {
  saved_state: GlobalOptions.saved_state;
  min_log_level: Hh_logger.Level.t;
  attempt_fix_credentials: bool;
      (** Indicates whether we attempt to fix the credentials if they're broken *)
  log_categories: string list;
  log_large_fanouts_threshold: int option;
      (** If a fanout is greater than this value, log stats about that fanout. *)
  experiments: string list;
      (** the list of experiments from the experiments config *)
  experiments_config_meta: string;  (** a free-form diagnostic string *)
  use_saved_state: bool;
      (** should we attempt to load saved-state? (subject to further options) *)
  use_saved_state_when_indexing: bool;
      (** should we attempt to load saved-state when running glean indexing? *)
  require_saved_state: bool;
      (** if attempting saved-state, should we fail upon failure? *)
  load_state_natively: bool;
      (** make hh_server query and download saved state. *)
  load_state_natively_download_timeout: int;  (** in seconds *)
  load_state_natively_dirty_files_timeout: int;  (** in seconds *)
  type_decl_bucket_size: int;
  extend_defs_per_file_bucket_size: int;
  enable_on_nfs: bool;
  enable_fuzzy_search: bool;
  lazy_parse: bool;
  lazy_init: bool;
  max_purgatory_clients: int;
      (** Monitor: Limit the number of clients that can sit in purgatory waiting
      for a server to be started because we don't want this to grow unbounded. *)
  search_chunk_size: int;
  io_priority: int;
  cpu_priority: int;
  shm_dirs: string list;
  shm_use_sharded_hashtbl: bool;
  shm_cache_size: int;
      (** Maximum shared memory cache size for evictable data.

      If this is set to a negative value, eviction is disabled. *)
  max_workers: int option;
  max_bucket_size: int;
      (** max_bucket_size is the default bucket size for ALL users of MultiWorker unless they provide a specific override max_size *)
  use_dummy_informant: bool;  (** See HhMonitorInformant. *)
  informant_min_distance_restart: int;
  use_full_fidelity_parser: bool;
  interrupt_on_watchman: bool;
  interrupt_on_client: bool;
  trace_parsing: bool;
  prechecked_files: bool;
  enable_type_check_filter_files: bool;
      (** Let the user configure which files to type check and
      which files to ignore. This flag is not expected to be
      rolled out broadly, rather it is meant to be used by
      power users only. *)
  ide_serverless: bool;
      (** whether clientLsp should use serverless-ide, but with RPC hh_server for squiggles *)
  ide_standalone: bool;
      (** whether clientLsp should use serverless-ide, solely shelling-out to hh for some things. This flag overrides ide_serverless. *)
  ide_max_num_decls: int;  (** tuning of clientIdeDaemon local cache *)
  ide_max_num_shallow_decls: int;  (** tuning of clientIdeDaemon local cache *)
  ide_symbolindex_search_provider: string;
      (** like [symbolindex_search_provider] but for IDE *)
  predeclare_ide: bool;
  max_typechecker_worker_memory_mb: int option;
      (** if set, the worker will stop early at the end of a file if its heap exceeds this number *)
  use_max_typechecker_worker_memory_for_decl_deferral: bool;
      (** if set, the worker will perform the same check as for [max_typechecker_worker_memory_mb] after each decl
      and, if over the limit, will defer *)
  longlived_workers: bool;
  hg_aware: bool;
  hg_aware_parsing_restart_threshold: int;
  hg_aware_redecl_restart_threshold: int;
  hg_aware_recheck_restart_threshold: int;
  force_remote_type_check: bool;  (** forces Hulk *)
  ide_parser_cache: bool;
  store_decls_in_saved_state: bool;
      (** When enabled, save hot class declarations (for now, specified in a special
      file in the repository) when generating a saved state. *)
  load_decls_from_saved_state: bool;
      (** When enabled, load class declarations stored in the saved state, if any, on
      server init. *)
  idle_gc_slice: int;
      (** Size of Gc.major_slice to be performed when server is idle. 0 to disable *)
  populate_member_heaps: bool;
      (** Populate the member signature heaps.

      If disabled, instead load lazily from shallow classes. *)
  fetch_remote_old_decls: bool;
      (** Option to fetch old decls from remote decl store *)
  use_hack_64_naming_table: bool;
      (** Load naming table from hack/64 saved state. *)
  skip_hierarchy_checks: bool;
      (** Skip checks on hierarchy e.g. overrides, require extend, etc.
      Set to true only for debugging purposes! *)
  skip_tast_checks: bool;
      (** Skip checks implemented using TAST visitors.
      Set to true only for debugging purposes! *)
  num_local_workers: int option;
      (** If None, only the type check delegate's logic will be used.
      If the delegate fails to type check, the typing check service as a whole
      will fail. *)
  parallel_type_checking_threshold: int;
      (** If the number of files to type check is fewer than this value, the files
      will be type checked sequentially (in the master process). Otherwise,
      the files will be type checked in parallel (in MultiWorker workers). *)
  defer_class_declaration_threshold: int option;
      (** If set, defers class declarations after N lazy declarations; if not set,
      always lazily declares classes not already in cache. *)
  prefetch_deferred_files: bool;
      (** The whether to use the hook that prefetches files on an Eden checkout *)
  recheck_capture: RecheckCapture.t;
      (** Settings controlling how and whether we capture the recheck environment *)
  remote_nonce: Int64.t;
      (** The unique identifier of a particular remote typechecking run *)
  remote_type_check: RemoteTypeCheck.t;
      (** Remote type check settings that can be changed, e.g., by GK *)
  remote_worker_key: string option;
      (** If set, uses the key to fetch type checking jobs *)
  remote_check_id: string option;
      (** If set, uses the check ID when logging events in the context of remove init/work *)
  remote_version_specifier_required: bool;
      (** Indicates whether the remote version specifier is required for remote type check from non-prod server *)
  remote_version_specifier: string option;
      (** The version of the package the remote worker is to install *)
  remote_worker_saved_state_manifold_path: string option;
      (** A manifold path to a naming table to be used for Hulk Lite when typechecking. *)
  rust_provider_backend: bool;
      (** Use Provider_backend.Rust_provider_backend as the global provider
       * backend, servicing File_provider, Naming_provider, and Decl_provider
       * using the hackrs implementation. *)
  rust_elab: bool;
      (** Use the Rust implementation of naming elaboration and NAST checks. *)
  naming_sqlite_path: string option;
      (** Enables the reverse naming table to fall back to SQLite for queries. *)
  enable_naming_table_fallback: bool;
  symbolindex_search_provider: string;
      (** Selects a search provider for autocomplete and symbol search; see also [ide_symbolindex_search_provider] *)
  symbolindex_quiet: bool;
  symbolindex_file: string option;
  tico_invalidate_files: bool;
      (** Allows hh_server to invalidate units in hhvm based on local changes *)
  tico_invalidate_smart: bool;  (** Use finer grain hh_server dependencies *)
  per_file_profiling: HackEventLogger.PerFileProfilingConfig.t;
      (** turns on memtrace .ctf writes to this directory *)
  memtrace_dir: string option;
  go_to_implementation: bool;
      (** Allows the IDE to show the 'find all implementations' button *)
  allow_unstable_features: bool;
      (** Allows unstabled features to be enabled within a file via the '__EnableUnstableFeatures' attribute *)
  watchman: Watchman.t;
  naming_sqlite_in_hack_64: bool;
      (** Add sqlite naming table to hack/64 ss job *)
  workload_quantile: quantile option;
      (** Allows to typecheck only a certain quantile of the workload. *)
  rollout_group: string option;
      (** A string from hh.conf, written to HackEventLogger telemetry. Before it got
       into here, [t], it was first used as a lookup in ServerLocalConfigKnobs.
       Intended meaning: what class of user is running hh_server, hence what experiments
       should they be subject to. *)
  specify_manifold_api_key: bool;
  remote_old_decls_no_limit: bool;
      (**  Remove remote old decl fetching limit *)
  no_marshalled_naming_table_in_saved_state: bool;
      (** Remove marshalled naming table from saved state *)
  cache_remote_decls: bool;
      (** Configure whether fetch and cache remote decls *)
  use_shallow_decls_saved_state: bool;
      (** (only when cache_remote_decls == true) Configure where to fetch and cache remote decls
          true --> from saved_state hach/shallow_decls
          false --> from remote old shallow decl service *)
  shallow_decls_manifold_path: string option;
      (** A manifold path to a shallow_decls to be used for Hulk Lite when typechecking. *)
  disable_naming_table_fallback_loading: bool;
      (** Stop loading from OCaml marshalled naming table if sqlite table is missing. *)
  use_type_alias_heap: bool;  (** optimize type alias expansions *)
  override_load_state_natively: bool;
      (** Overrides load_state_natively on Sandcastle when true *)
  use_server_revision_tracker_v2: bool;
      (** control serverRevisionTracker.ml watchman subscription event tracking *)
  load_hack_64_distc_saved_state: bool;
      (** use saved state generated by hh_distc *)
  ide_should_use_hack_64_distc: bool;
      (** clientIdeDaemon should load a naming table from hack/64_distc *)
}

let default =
  {
    saved_state = GlobalOptions.default_saved_state;
    min_log_level = Hh_logger.Level.Info;
    attempt_fix_credentials = false;
    log_categories = [];
    log_large_fanouts_threshold = None;
    experiments = [];
    experiments_config_meta = "";
    force_remote_type_check = false;
    use_saved_state = false;
    use_saved_state_when_indexing = false;
    require_saved_state = false;
    load_state_natively = false;
    load_state_natively_download_timeout = 60;
    load_state_natively_dirty_files_timeout = 200;
    type_decl_bucket_size = 1000;
    extend_defs_per_file_bucket_size = 2000;
    enable_on_nfs = false;
    enable_fuzzy_search = true;
    lazy_parse = false;
    lazy_init = false;
    max_purgatory_clients = 400;
    search_chunk_size = 0;
    io_priority = 7;
    cpu_priority = 10;
    shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir];
    shm_use_sharded_hashtbl = false;
    shm_cache_size = -1;
    max_workers = None;
    max_bucket_size = Bucket.max_size ();
    use_dummy_informant = true;
    informant_min_distance_restart = 100;
    use_full_fidelity_parser = true;
    interrupt_on_watchman = false;
    interrupt_on_client = false;
    trace_parsing = false;
    prechecked_files = false;
    enable_type_check_filter_files = false;
    ide_serverless = false;
    ide_standalone = false;
    ide_max_num_decls = 5000;
    ide_max_num_shallow_decls = 10000;
    predeclare_ide = false;
    max_typechecker_worker_memory_mb = None;
    use_max_typechecker_worker_memory_for_decl_deferral = false;
    longlived_workers = false;
    hg_aware = false;
    hg_aware_parsing_restart_threshold = 0;
    hg_aware_redecl_restart_threshold = 0;
    hg_aware_recheck_restart_threshold = 0;
    ide_parser_cache = false;
    store_decls_in_saved_state = false;
    load_decls_from_saved_state = false;
    idle_gc_slice = 0;
    populate_member_heaps = true;
    fetch_remote_old_decls = false;
    use_hack_64_naming_table = true;
    skip_hierarchy_checks = false;
    skip_tast_checks = false;
    num_local_workers = None;
    parallel_type_checking_threshold = 10;
    defer_class_declaration_threshold = None;
    prefetch_deferred_files = false;
    recheck_capture = RecheckCapture.default;
    remote_nonce = Int64.zero;
    remote_type_check = RemoteTypeCheck.default;
    remote_worker_key = None;
    remote_check_id = None;
    remote_version_specifier_required = true;
    remote_version_specifier = None;
    remote_worker_saved_state_manifold_path = None;
    rust_elab = false;
    rust_provider_backend = false;
    naming_sqlite_path = None;
    enable_naming_table_fallback = false;
    symbolindex_search_provider = "SqliteIndex";
    (* the code actually doesn't use this default for ide_symbolindex_search_provider;
       it defaults to whatever was computed for symbolindex_search_provider. *)
    ide_symbolindex_search_provider = "SqliteIndex";
    symbolindex_quiet = false;
    symbolindex_file = None;
    tico_invalidate_files = false;
    tico_invalidate_smart = false;
    per_file_profiling = HackEventLogger.PerFileProfilingConfig.default;
    memtrace_dir = None;
    go_to_implementation = true;
    allow_unstable_features = false;
    watchman = Watchman.default;
    naming_sqlite_in_hack_64 = false;
    workload_quantile = None;
    rollout_group = None;
    specify_manifold_api_key = false;
    remote_old_decls_no_limit = false;
    no_marshalled_naming_table_in_saved_state = false;
    cache_remote_decls = false;
    use_shallow_decls_saved_state = false;
    shallow_decls_manifold_path = None;
    disable_naming_table_fallback_loading = false;
    use_type_alias_heap = false;
    override_load_state_natively = false;
    use_server_revision_tracker_v2 = false;
    load_hack_64_distc_saved_state = false;
    ide_should_use_hack_64_distc = false;
  }

let system_config_path =
  let dir =
    try Sys.getenv "HH_LOCALCONF_PATH" with
    | _ -> BuildOptions.system_config_path
  in
  Filename.concat dir "hh.conf"

let apply_overrides ~silent ~current_version ~config ~overrides =
  (* We'll apply CLI overrides now at the start so that JustKnobs and experiments_config
     can be informed about them, e.g. "--config rollout_group=foo" will be able
     to guide the manner in which JustKnobs picks up values, and "--config use_justknobs=false"
     will be able to disable it. Don't worry though -- we'll apply CLI overrides again at the end,
     so they overwrite any changes brought by JustKnobs and experiments_config. *)
  let config = Config_file.apply_overrides ~from:None ~config ~overrides in
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
      ServerLocalConfigKnobs.apply_justknobs_overrides ~silent config
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
            ~from:(Option.some_if (not silent) "Experiment_overrides")
            ~config
            ~overrides:experiment_overrides
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
      ~from:(Option.some_if (not silent) "--config")
      ~config
      ~overrides
  in
  (experiments_meta, config)

let load_
    system_config_path
    ~silent
    ~current_version
    ~current_rolled_out_flag_idx
    overrides : t =
  let config = Config_file.parse_local_config system_config_path in
  let (experiments_config_meta, config) =
    apply_overrides ~silent ~current_version ~config ~overrides
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
  let force_remote_type_check =
    bool_if_min_version
      "force_remote_type_check"
      ~default:default.force_remote_type_check
      ~current_version
      config
  in
  let lazy_parse =
    bool_if_min_version
      "lazy_parse"
      ~default:default.lazy_parse
      ~current_version
      config
  in
  let lazy_init =
    bool_if_min_version
      "lazy_init2"
      ~default:default.lazy_init
      ~current_version
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
  let max_bucket_size =
    int_ "max_bucket_size" ~default:default.max_bucket_size config
  in
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
  (* ide_serverless CANNOT use bool_if_min_version, since it's needed before we yet know root/version *)
  let ide_serverless =
    bool_ "ide_serverless" ~default:default.ide_serverless config
  in
  (* ide_standalone CANNOT use bool_if_min_version, since it's needed before we yet know root/version *)
  let ide_standalone =
    bool_ "ide_standalone" ~default:default.ide_serverless config
  in
  let ide_max_num_decls =
    int_ "ide_max_num_decls" ~default:default.ide_max_num_decls config
  in
  let ide_max_num_shallow_decls =
    int_
      "ide_max_num_shallow_decls"
      ~default:default.ide_max_num_shallow_decls
      config
  in
  let predeclare_ide =
    bool_if_min_version
      "predeclare_ide"
      ~default:default.predeclare_ide
      ~current_version
      config
  in
  let max_typechecker_worker_memory_mb =
    int_opt "max_typechecker_worker_memory_mb" config
  in
  let use_max_typechecker_worker_memory_for_decl_deferral =
    bool_
      "use_max_typechecker_worker_memory_for_decl_deferral"
      ~default:default.use_max_typechecker_worker_memory_for_decl_deferral
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
  let load_decls_from_saved_state =
    bool_if_min_version
      "load_decls_from_saved_state"
      ~default:default.load_decls_from_saved_state
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
  let use_hack_64_naming_table =
    bool_if_min_version
      "use_hack_64_naming_table"
      ~default:default.use_hack_64_naming_table
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
  let parallel_type_checking_threshold =
    int_
      "parallel_type_checking_threshold"
      ~default:default.parallel_type_checking_threshold
      config
  in
  let num_local_workers = int_opt "num_local_workers" config in
  let defer_class_declaration_threshold =
    int_opt "defer_class_declaration_threshold" config
  in
  let prefetch_deferred_files =
    bool_if_min_version
      "prefetch_deferred_files"
      ~default:false
      ~current_version
      config
  in
  let recheck_capture =
    RecheckCapture.load ~current_version ~default:default.recheck_capture config
  in
  let remote_nonce =
    match string_opt "remote_nonce" config with
    | Some n -> Int64.of_string n
    | None -> Int64.zero
  in
  let remote_type_check =
    RemoteTypeCheck.load
      ~current_version
      ~default:default.remote_type_check
      config
  in
  let watchman =
    Watchman.load ~current_version ~default:default.watchman config
  in
  let remote_worker_key = string_opt "remote_worker_key" config in
  let remote_check_id = string_opt "remote_check_id" config in
  let remote_version_specifier_required =
    bool_if_min_version
      "remote_version_specifier_required"
      ~default:default.remote_version_specifier_required
      ~current_version
      config
  in
  let remote_version_specifier = string_opt "remote_version_specifier" config in
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
  let symbolindex_search_provider =
    string_
      "symbolindex_search_provider"
      ~default:default.symbolindex_search_provider
      config
  in
  let ide_symbolindex_search_provider =
    string_
      "ide_symbolindex_search_provider"
      ~default:symbolindex_search_provider
      config
  in
  let symbolindex_quiet =
    bool_if_min_version
      "symbolindex_quiet"
      ~default:default.symbolindex_quiet
      ~current_version
      config
  in
  let symbolindex_file = string_opt "symbolindex_file" config in
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
    match string_ "profile_decling" ~default:"off" config with
    | "off" -> HackEventLogger.PerFileProfilingConfig.DeclingOff
    | "top_counts" -> HackEventLogger.PerFileProfilingConfig.DeclingTopCounts
    | "all_telemetry" ->
      HackEventLogger.PerFileProfilingConfig.DeclingAllTelemetry
        { callstacks = false }
    | "all_telemetry_callstacks" ->
      HackEventLogger.PerFileProfilingConfig.DeclingAllTelemetry
        { callstacks = true }
    | _ ->
      failwith
        "profile_decling: off | top_counts | all_telemetry | all_telemetry_callstacks"
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
  let naming_sqlite_in_hack_64 =
    bool_if_min_version
      "naming_sqlite_in_hack_64"
      ~default:default.naming_sqlite_in_hack_64
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
  let no_marshalled_naming_table_in_saved_state =
    bool_if_min_version
      "no_marshalled_naming_table_in_saved_state"
      ~default:default.no_marshalled_naming_table_in_saved_state
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
  let remote_worker_saved_state_manifold_path =
    string_opt "remote_worker_saved_state_manifold_path" config
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
  let use_shallow_decls_saved_state =
    bool_if_min_version
      "use_shallow_decls_saved_state"
      ~default:default.use_shallow_decls_saved_state
      ~current_version
      config
  in
  let shallow_decls_manifold_path =
    string_opt "shallow_decls_manifold_path" config
  in
  let disable_naming_table_fallback_loading =
    bool_if_min_version
      "disable_naming_table_fallback_loading"
      ~default:default.disable_naming_table_fallback_loading
      ~current_version
      config
  in
  let use_type_alias_heap =
    bool_if_min_version
      "use_type_alias_heap"
      ~default:default.use_type_alias_heap
      ~current_version
      config
  in
  let override_load_state_natively =
    bool_if_min_version
      "override_load_state_natively"
      ~default:default.override_load_state_natively
      ~current_version
      config
  in
  let use_server_revision_tracker_v2 =
    bool_if_min_version
      "use_server_revision_tracker_v2"
      ~default:default.use_server_revision_tracker_v2
      ~current_version
      config
  in
  let load_hack_64_distc_saved_state =
    bool_if_min_version
      "load_hack_64_distc_saved_state"
      ~default:default.load_hack_64_distc_saved_state
      ~current_version
      config
  in
  let ide_should_use_hack_64_distc =
    bool_if_min_version
      "ide_should_use_hack_64_distc"
      ~default:default.ide_should_use_hack_64_distc
      ~current_version
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
          };
        rollouts = saved_state_flags;
        project_metadata_w_flags;
      };
    min_log_level;
    attempt_fix_credentials;
    log_categories;
    log_large_fanouts_threshold;
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
    lazy_parse;
    lazy_init;
    search_chunk_size;
    io_priority;
    cpu_priority;
    shm_dirs;
    shm_use_sharded_hashtbl;
    shm_cache_size;
    max_workers;
    max_bucket_size;
    use_dummy_informant;
    informant_min_distance_restart;
    use_full_fidelity_parser;
    interrupt_on_watchman;
    interrupt_on_client;
    trace_parsing;
    prechecked_files;
    enable_type_check_filter_files;
    ide_serverless;
    ide_standalone;
    ide_max_num_decls;
    ide_max_num_shallow_decls;
    ide_symbolindex_search_provider;
    predeclare_ide;
    max_typechecker_worker_memory_mb;
    use_max_typechecker_worker_memory_for_decl_deferral;
    longlived_workers;
    hg_aware;
    hg_aware_parsing_restart_threshold;
    hg_aware_redecl_restart_threshold;
    hg_aware_recheck_restart_threshold;
    ide_parser_cache;
    store_decls_in_saved_state;
    load_decls_from_saved_state;
    idle_gc_slice;
    populate_member_heaps;
    fetch_remote_old_decls;
    use_hack_64_naming_table;
    skip_hierarchy_checks;
    skip_tast_checks;
    num_local_workers;
    parallel_type_checking_threshold;
    defer_class_declaration_threshold;
    prefetch_deferred_files;
    recheck_capture;
    remote_nonce;
    remote_type_check;
    remote_worker_key;
    remote_check_id;
    remote_version_specifier_required;
    remote_version_specifier;
    remote_worker_saved_state_manifold_path;
    rust_elab;
    rust_provider_backend;
    naming_sqlite_path;
    enable_naming_table_fallback;
    symbolindex_search_provider;
    symbolindex_quiet;
    symbolindex_file;
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
    force_remote_type_check;
    naming_sqlite_in_hack_64;
    workload_quantile;
    rollout_group;
    specify_manifold_api_key;
    remote_old_decls_no_limit;
    no_marshalled_naming_table_in_saved_state;
    cache_remote_decls;
    use_shallow_decls_saved_state;
    shallow_decls_manifold_path;
    disable_naming_table_fallback_loading;
    use_type_alias_heap;
    override_load_state_natively;
    use_server_revision_tracker_v2;
    load_hack_64_distc_saved_state;
    ide_should_use_hack_64_distc;
  }

(** Loads the config from [path]. Uses JustKnobs and ExperimentsConfig to override.
On top of that, applies [config_overrides]. If [silent] then prints what it's doing
to stderr. *)
let load :
    silent:bool ->
    current_version:Config_file_version.version ->
    current_rolled_out_flag_idx:int ->
    Config_file_common.t ->
    t =
  load_ system_config_path

let to_rollout_flags (options : t) : HackEventLogger.rollout_flags =
  HackEventLogger.
    {
      log_saved_state_age_and_distance =
        GlobalOptions.(
          options.saved_state.loading.log_saved_state_age_and_distance);
      naming_sqlite_in_hack_64 = options.naming_sqlite_in_hack_64;
      use_hack_64_naming_table = options.use_hack_64_naming_table;
      fetch_remote_old_decls = options.fetch_remote_old_decls;
      ide_max_num_decls = options.ide_max_num_decls;
      ide_max_num_shallow_decls = options.ide_max_num_shallow_decls;
      max_bucket_size = options.max_bucket_size;
      max_workers = Option.value options.max_workers ~default:(-1);
      max_typechecker_worker_memory_mb =
        Option.value options.max_typechecker_worker_memory_mb ~default:(-1);
      use_max_typechecker_worker_memory_for_decl_deferral =
        options.use_max_typechecker_worker_memory_for_decl_deferral;
      specify_manifold_api_key = options.specify_manifold_api_key;
      remote_old_decls_no_limit = options.remote_old_decls_no_limit;
      no_marshalled_naming_table_in_saved_state =
        options.no_marshalled_naming_table_in_saved_state;
      populate_member_heaps = options.populate_member_heaps;
      shm_use_sharded_hashtbl = options.shm_use_sharded_hashtbl;
      shm_cache_size = options.shm_cache_size;
      use_manifold_cython_client =
        GlobalOptions.(options.saved_state.loading.use_manifold_cython_client);
      disable_naming_table_fallback_loading =
        options.disable_naming_table_fallback_loading;
      use_type_alias_heap = options.use_type_alias_heap;
      override_load_state_natively = options.override_load_state_natively;
      use_server_revision_tracker_v2 = options.use_server_revision_tracker_v2;
      rust_provider_backend = options.rust_provider_backend;
      load_hack_64_distc_saved_state = options.load_hack_64_distc_saved_state;
      ide_should_use_hack_64_distc = options.ide_should_use_hack_64_distc;
    }
