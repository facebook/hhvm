(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Config_file.Getters
open Hh_core

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
    let prefix = Some "watchman" in
    let use_watchman =
      bool_if_min_version
        "use_watchman"
        ~default:default.enabled
        ~current_version
        config
    in
    let enabled =
      bool_if_min_version
        "enabled"
        ~prefix
        ~default:use_watchman
        ~current_version
        config
    in
    let init_timeout =
      int_ "init_timeout" ~prefix ~default:default.init_timeout config
    in
    let sockname = string_opt "sockname" ~prefix config in
    let subscribe =
      bool_if_min_version
        "subscribe_v2"
        ~prefix
        ~default:default.subscribe
        ~current_version
        config
    in
    let synchronous_timeout =
      int_
        "synchronous_timeout"
        ~prefix
        ~default:default.synchronous_timeout
        config
    in
    let debug_logging =
      bool_if_min_version
        "debug_logging"
        ~prefix
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
    (* Indicates how long to wait between heartbeats (in seconds) *)
    heartbeat_period: int;
    load_naming_table_on_full_init: bool;
    max_batch_size: int;
    min_batch_size: int;
    (* Dictates the number of remote type checking workers *)
    num_workers: int;
    (* Indicates whether files-to-declare should be fetched by VFS
        (see `declaration_threshold`) *)
    prefetch_deferred_files: bool;
    (* If set, distributes type checking to remote workers if the number of files to
    type check exceeds the threshold. If not set, then always checks everything locally. *)
    recheck_threshold: int option;
    worker_min_log_level: Hh_logger.Level.t;
    (* Indicates the size of the job below which a virtual file system should
    be used by the remote worker *)
    worker_vfs_checkout_threshold: int;
    (* File system mode used by ArtifactStore *)
    file_system_mode: ArtifactStore.file_system_mode;
  }

  let default =
    {
      enabled = false;
      enabled_for_noninteractive_hosts = false;
      declaration_threshold = 50;
      disabled_on_errors = [];
      (* Indicates how long to wait between heartbeats (in seconds) *)
      heartbeat_period = 15;
      load_naming_table_on_full_init = false;
      max_batch_size = 25_000;
      min_batch_size = 5_000;
      num_workers = 4;
      prefetch_deferred_files = false;
      recheck_threshold = None;
      worker_min_log_level = Hh_logger.Level.Info;
      worker_vfs_checkout_threshold = 10_000;
      file_system_mode = ArtifactStore.Distributed;
    }

  let load ~current_version ~default config =
    let prefix = Some "remote_type_check" in
    let declaration_threshold =
      int_
        "declaration_threshold"
        ~prefix
        ~default:default.declaration_threshold
        config
    in

    let file_system_mode =
      let open ArtifactStore in
      let file_system_mode =
        string_
          "file_system_mode"
          ~prefix
          ~default:(string_of_file_system_mode Distributed)
          config
      in
      match file_system_mode_of_string file_system_mode with
      | Some mode -> mode
      | None -> Distributed
    in
    let enabled_on_errors =
      string_list
        "enabled_on_errors"
        ~delim:(Str.regexp ",")
        ~prefix
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
                 enabled_phase = phase)))
    in
    let heartbeat_period =
      int_ "heartbeat_period" ~prefix ~default:default.heartbeat_period config
    in
    let num_workers =
      int_ "num_workers" ~prefix ~default:default.num_workers config
    in
    let max_batch_size =
      int_ "max_batch_size" ~prefix ~default:default.max_batch_size config
    in
    let min_batch_size =
      int_ "min_batch_size" ~prefix ~default:default.min_batch_size config
    in
    let prefetch_deferred_files =
      bool_if_min_version
        "prefetch_deferred_files"
        ~prefix
        ~default:default.prefetch_deferred_files
        ~current_version
        config
    in
    let recheck_threshold = int_opt "recheck_threshold" ~prefix config in
    let load_naming_table_on_full_init =
      bool_if_min_version
        "load_naming_table_on_full_init"
        ~prefix
        ~default:default.load_naming_table_on_full_init
        ~current_version
        config
    in
    let enabled =
      bool_if_min_version
        "enabled"
        ~prefix
        ~default:default.enabled
        ~current_version
        config
    in
    let enabled_for_noninteractive_hosts =
      bool_if_min_version
        "enabled_for_noninteractive_hosts"
        ~prefix
        ~default:default.enabled_for_noninteractive_hosts
        ~current_version
        config
    in
    let worker_min_log_level =
      match
        Hh_logger.Level.of_enum_string
          (String.lowercase_ascii
             (string_
                ~prefix
                "worker_min_log_level"
                ~default:
                  (Hh_logger.Level.to_enum_string default.worker_min_log_level)
                config))
      with
      | Some level -> level
      | None -> Hh_logger.Level.Debug
    in
    let worker_vfs_checkout_threshold =
      int_
        "worker_vfs_checkout_threshold"
        ~prefix
        ~default:default.worker_vfs_checkout_threshold
        config
    in
    {
      declaration_threshold;
      disabled_on_errors;
      enabled;
      enabled_for_noninteractive_hosts;
      heartbeat_period;
      load_naming_table_on_full_init;
      max_batch_size;
      min_batch_size;
      num_workers;
      prefetch_deferred_files;
      recheck_threshold;
      worker_min_log_level;
      worker_vfs_checkout_threshold;
      file_system_mode;
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
    let prefix = Some "recheck_capture" in
    let enabled =
      bool_if_min_version
        "enabled"
        ~prefix
        ~default:default.enabled
        ~current_version
        config
    in
    let error_threshold =
      int_ "error_threshold" ~prefix ~default:default.error_threshold config
    in
    let fanout_threshold =
      int_ "fanout_threshold" ~prefix ~default:default.fanout_threshold config
    in
    let rechecked_files_threshold =
      int_
        "rechecked_files_threshold"
        ~prefix
        ~default:default.rechecked_files_threshold
        config
    in
    let sample_threshold =
      let sample_threshold =
        float_
          "sample_threshold"
          ~prefix
          ~default:default.sample_threshold
          config
      in
      if sample_threshold > 1.0 then
        1.0
      else if sample_threshold < 0.0 then
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

type t = {
  min_log_level: Hh_logger.Level.t;
  (* Indicates whether we attempt to fix the credentials if they're broken *)
  attempt_fix_credentials: bool;
  log_categories: string list;
  (* the list of experiments from the experiments config *)
  experiments: string list;
  (* a free-form diagnostic string *)
  experiments_config_meta: string;
  use_saved_state: bool;
  (* should we attempt to load saved-state? (subject to further options) *)
  require_saved_state: bool;
  (* if attempting saved-state, should we fail upon failure? *)
  load_state_script_timeout: int;
      (** Prefer using Ocaml implementation over load script. *)
  (* in seconds *)
  load_state_natively: bool;
  type_decl_bucket_size: int;
  extend_fast_bucket_size: int;
  enable_on_nfs: bool;
  enable_fuzzy_search: bool;
  lazy_parse: bool;
  lazy_init: bool;
  (* Limit the number of clients that can sit in purgatory waiting
   * for a server to be started because we don't want this to grow
   * unbounded. *)
  max_purgatory_clients: int;
  search_chunk_size: int;
  io_priority: int;
  cpu_priority: int;
  saved_state_cache_limit: int;
  can_skip_deptable: bool;
  shm_dirs: string list;
  state_loader_timeouts: State_loader_config.timeouts;
  max_workers: int option;
  max_bucket_size: int;
  (* See HhMonitorInformant. *)
  use_dummy_informant: bool;
  informant_min_distance_restart: int;
  informant_use_xdb: bool;
  use_full_fidelity_parser: bool;
  interrupt_on_watchman: bool;
  interrupt_on_client: bool;
  trace_parsing: bool;
  prechecked_files: bool;
  predeclare_ide: bool;
  predeclare_ide_deps: bool;
  max_typechecker_worker_memory_mb: int option;
  hg_aware: bool;
  hg_aware_parsing_restart_threshold: int;
  hg_aware_redecl_restart_threshold: int;
  hg_aware_recheck_restart_threshold: int;
  (* Flag to disable conservative behavior in incremental-mode typechecks.
   *
   * By default, when a class has changed and we do not have access to the old
   * version of its declaration (and thus cannot determine HOW it has changed),
   * we conservatively redeclare the entire set of files where the class or any
   * of its members were referenced. Likewise for definitions of functions or
   * global constants.
   *
   * This flag disables that behavior--instead, when a class has changed, we
   * only redeclare files with an Extends dependency on the class, and we do not
   * redeclare any files when a function or global constant changes.
   *)
  disable_conservative_redecl: bool;
  ide_parser_cache: bool;
  ide_tast_cache: bool;
  (* When enabled, save hot class declarations (for now, specified in a special
     file in the repository) when generating a saved state. *)
  store_decls_in_saved_state: bool;
  (* When enabled, load class declarations stored in the saved state, if any, on
     server init. *)
  load_decls_from_saved_state: bool;
  (* Size of Gc.major_slice to be performed when server is idle. 0 to disable *)
  idle_gc_slice: int;
  (* Look up class members lazily from shallow declarations instead of eagerly
     computing folded declarations representing the entire class type. *)
  shallow_class_decl: bool;
  (* If false, only the type check delegate's logic will be used.
    If the delegate fails to type check, the typing check service as a whole
    will fail. *)
  num_local_workers: int option;
  (* If the number of files to type check is fewer than this value, the files
    will be type checked sequentially (in the master process). Otherwise,
    the files will be type checked in parallel (in MultiWorker workers). *)
  parallel_type_checking_threshold: int;
  (* If set, defers class declarations after N lazy declarations; if not set,
    always lazily declares classes not already in cache. *)
  defer_class_declaration_threshold: int option;
  (* If set, prevents type checking of files from being deferred more than
    the number of times greater than or equal to the threshold. If not set,
    defers class declarations indefinitely. *)
  max_times_to_defer_type_checking: int option;
  (* The whether to use the hook that prefetches files on an Eden checkout *)
  prefetch_deferred_files: bool;
  (* Settings controlling how and whether we capture the recheck environment *)
  recheck_capture: RecheckCapture.t;
  (* The version of the Remote Execution CLI tool to use *)
  recli_version: string;
  (* Remote type check settings that can be changed, e.g., by GK *)
  remote_type_check: RemoteTypeCheck.t;
  (* If set, uses the key to fetch type checking jobs *)
  remote_worker_key: string option;
  (* If set, uses the check ID when logging events in the context of remove init/work *)
  remote_check_id: string option;
  (* Indicates whether the remote version specifier is required for remote type check from non-prod server *)
  remote_version_specifier_required: bool;
  (* The version of the package the remote worker is to install *)
  remote_version_specifier: string option;
  (* Name of the transport channel used by remote type checking. TODO: move into remote_type_check. *)
  remote_transport_channel: string option;
  (* Enables the reverse naming table to fall back to SQLite for queries. *)
  naming_sqlite_path: string option;
  enable_naming_table_fallback: bool;
  (* Selects a search provider for autocomplete and symbol search *)
  symbolindex_search_provider: string;
  symbolindex_quiet: bool;
  symbolindex_file: string option;
  (* Allows hh_server to invalidate units in hhvm based on local changes *)
  tico_invalidate_files: bool;
  (* Use finer grain hh_server dependencies *)
  tico_invalidate_smart: bool;
  (* If --profile-log, we'll record telemetry on typechecks that took longer than the threshold. In case of profile_type_check_twice we judge by the second type check. *)
  profile_type_check_duration_threshold: float;
  (* The flag "--config profile_type_check_twice=true" causes each file to be typechecked twice in succession. If --profile-log then both times are logged. *)
  profile_type_check_twice: bool;
  (* If --profile-log, we can use "--config profile_owner=<str>" to send an arbitrary "owner" along with the telemetry *)
  profile_owner: string option;
  (* If --profile-log, we can use "--config profile_desc=<str>" to send an arbitrary "desc" along with telemetry *)
  profile_desc: string;
  (* Allows the IDE to show the 'find all implementations' button *)
  go_to_implementation: bool;
  (* Allows unstabled features to be enabled within a file via the '__EnableUnstableFeatures' attribute *)
  allow_unstable_features: bool;
  watchman: Watchman.t;
}

let default =
  {
    min_log_level = Hh_logger.Level.Info;
    attempt_fix_credentials = false;
    log_categories = [];
    experiments = [];
    experiments_config_meta = "";
    use_saved_state = false;
    require_saved_state = false;
    load_state_script_timeout = 20;
    load_state_natively = false;
    type_decl_bucket_size = 1000;
    extend_fast_bucket_size = 2000;
    enable_on_nfs = false;
    enable_fuzzy_search = true;
    lazy_parse = false;
    lazy_init = false;
    max_purgatory_clients = 400;
    search_chunk_size = 0;
    io_priority = 7;
    cpu_priority = 10;
    saved_state_cache_limit = 20;
    can_skip_deptable = true;
    shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir];
    max_workers = None;
    max_bucket_size = Bucket.max_size ();
    state_loader_timeouts = State_loader_config.default_timeouts;
    use_dummy_informant = true;
    informant_min_distance_restart = 100;
    informant_use_xdb = false;
    use_full_fidelity_parser = true;
    interrupt_on_watchman = false;
    interrupt_on_client = false;
    trace_parsing = false;
    prechecked_files = false;
    predeclare_ide = false;
    predeclare_ide_deps = false;
    max_typechecker_worker_memory_mb = None;
    hg_aware = false;
    hg_aware_parsing_restart_threshold = 0;
    hg_aware_redecl_restart_threshold = 0;
    hg_aware_recheck_restart_threshold = 0;
    disable_conservative_redecl = false;
    ide_parser_cache = false;
    ide_tast_cache = false;
    store_decls_in_saved_state = false;
    load_decls_from_saved_state = false;
    idle_gc_slice = 0;
    shallow_class_decl = false;
    num_local_workers = None;
    parallel_type_checking_threshold = 10;
    defer_class_declaration_threshold = None;
    max_times_to_defer_type_checking = None;
    prefetch_deferred_files = false;
    recheck_capture = RecheckCapture.default;
    recli_version = "STABLE";
    remote_type_check = RemoteTypeCheck.default;
    remote_worker_key = None;
    remote_check_id = None;
    remote_version_specifier_required = true;
    remote_version_specifier = None;
    remote_transport_channel = None;
    naming_sqlite_path = None;
    enable_naming_table_fallback = false;
    symbolindex_search_provider = "SqliteIndex";
    symbolindex_quiet = false;
    symbolindex_file = None;
    tico_invalidate_files = false;
    tico_invalidate_smart = false;
    profile_type_check_duration_threshold = 0.05;
    profile_type_check_twice = false;
    profile_owner = None;
    profile_desc = "";
    (* seconds *)
    go_to_implementation = true;
    allow_unstable_features = false;
    watchman = Watchman.default;
  }

let path =
  let dir =
    try Sys.getenv "HH_LOCALCONF_PATH"
    with _ -> BuildOptions.system_config_path
  in
  Filename.concat dir "hh.conf"

let state_loader_timeouts_ ~default config =
  State_loader_config.(
    let package_fetch_timeout =
      int_
        "state_loader_timeout_package_fetch"
        ~default:default.package_fetch_timeout
        config
    in
    let find_exact_state_timeout =
      int_
        "state_loader_timeout_find_exact_state"
        ~default:default.find_exact_state_timeout
        config
    in
    let find_nearest_state_timeout =
      int_
        "state_loader_timeout_find_nearest_state"
        ~default:default.find_nearest_state_timeout
        config
    in
    let current_hg_rev_timeout =
      int_
        "state_loader_timeout_current_hg_rev"
        ~default:default.current_hg_rev_timeout
        config
    in
    let current_base_rev_timeout =
      int_
        "state_loader_timeout_current_base_rev_timeout"
        ~default:default.current_base_rev_timeout
        config
    in
    {
      State_loader_config.package_fetch_timeout;
      find_exact_state_timeout;
      find_nearest_state_timeout;
      current_hg_rev_timeout;
      current_base_rev_timeout;
    })

let apply_overrides ~silent ~current_version ~config ~overrides =
  (* First of all, apply the CLI overrides so the settings below could be specified
    altered via the CLI, even though the CLI overrides take precedence
    over the experiments overrides *)
  let config = Config_file.apply_overrides ~silent ~config ~overrides in
  let prefix = Some "experiments_config" in
  let enabled =
    bool_if_min_version "enabled" ~prefix ~default:false ~current_version config
  in
  if enabled then (
    Disk.mkdir_p GlobalConfig.tmp_dir;
    let dir = string_ "path" ~prefix ~default:GlobalConfig.tmp_dir config in
    let owner = Sys_utils.get_primary_owner () in
    let file = Filename.concat dir (Printf.sprintf "hh.%s.experiments" owner) in
    let update =
      bool_if_min_version
        "update"
        ~prefix
        ~default:false
        ~current_version
        config
    in
    let ttl = float_of_int (int_ "ttl_seconds" ~prefix ~default:86400 config) in
    let source = string_opt "source" ~prefix config in
    let meta =
      if update then
        match Experiments_config_file.update ~file ~source ~ttl with
        | Ok meta -> meta
        | Error message -> message
      else
        "Updating experimental config not enabled"
    in
    if Disk.file_exists file then
      (* Apply the experiments overrides *)
      let experiment_overrides = Config_file.parse_local_config ~silent file in
      let config =
        Config_file.apply_overrides
          ~silent
          ~config
          ~overrides:experiment_overrides
      in
      (* Finally, reapply the CLI overrides, since they should take
          precedence over the experiments overrides *)
      (meta, Config_file.apply_overrides ~silent ~config ~overrides)
    else
      ("Experimental config not found on disk", config)
  ) else
    ("Experimental config not enabled", config)

let load_ fn ~silent ~current_version overrides =
  let config = Config_file.parse_local_config ~silent fn in
  let (experiments_config_meta, config) =
    apply_overrides ~silent ~current_version ~config ~overrides
  in
  let experiments =
    string_list
      "experiments"
      ~delim:(Str.regexp ",")
      ~default:default.experiments
      config
  in

  let log_categories =
    string_list
      "log_categories"
      ~delim:(Str.regexp ",")
      ~default:default.log_categories
      config
  in
  let min_log_level =
    match
      Hh_logger.Level.of_enum_string
        (String.lowercase_ascii
           (string_
              "min_log_level"
              ~default:(Hh_logger.Level.to_enum_string default.min_log_level)
              config))
    with
    | Some level -> level
    | None -> Hh_logger.Level.Debug
  in

  let use_saved_state =
    bool_if_version "use_mini_state" ~default:default.use_saved_state config
  in
  let require_saved_state =
    bool_if_version
      "require_saved_state"
      ~default:default.require_saved_state
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
    bool_if_version "enable_on_nfs" ~default:default.enable_on_nfs config
  in
  let enable_fuzzy_search =
    bool_if_version
      "enable_fuzzy_search"
      ~default:default.enable_fuzzy_search
      config
  in
  let lazy_parse =
    bool_if_version "lazy_parse" ~default:default.lazy_parse config
  in
  let lazy_init =
    bool_if_version "lazy_init2" ~default:default.lazy_init config
  in
  let max_purgatory_clients =
    int_ "max_purgatory_clients" ~default:default.max_purgatory_clients config
  in
  let search_chunk_size =
    int_ "search_chunk_size" ~default:default.search_chunk_size config
  in
  let load_state_script_timeout =
    int_
      "load_mini_script_timeout"
      ~default:default.load_state_script_timeout
      config
  in
  let load_state_natively =
    bool_if_version
      "load_state_natively_v4"
      ~default:default.load_state_natively
      config
  in
  let state_loader_timeouts =
    state_loader_timeouts_ ~default:State_loader_config.default_timeouts config
  in
  let use_dummy_informant =
    bool_if_version
      "use_dummy_informant"
      ~default:default.use_dummy_informant
      config
  in
  let informant_min_distance_restart =
    int_
      "informant_min_distance_restart"
      ~default:default.informant_min_distance_restart
      config
  in
  let informant_use_xdb =
    bool_if_version
      "informant_use_xdb_v5"
      ~default:default.informant_use_xdb
      config
  in
  let type_decl_bucket_size =
    int_ "type_decl_bucket_size" ~default:default.type_decl_bucket_size config
  in
  let extend_fast_bucket_size =
    int_
      "extend_fast_bucket_size"
      ~default:default.extend_fast_bucket_size
      config
  in
  let io_priority = int_ "io_priority" ~default:default.io_priority config in
  let cpu_priority = int_ "cpu_priority" ~default:default.cpu_priority config in
  let saved_state_cache_limit =
    int_
      "saved_state_cache_limit"
      ~default:default.saved_state_cache_limit
      config
  in
  let can_skip_deptable =
    bool_if_version
      "can_skip_deptable"
      ~default:default.can_skip_deptable
      config
  in
  let shm_dirs =
    string_list
      ~delim:(Str.regexp ",")
      "shm_dirs"
      ~default:default.shm_dirs
      config
    |> List.map ~f:(fun dir -> Path.(to_string @@ make dir))
  in
  let max_workers = int_opt "max_workers" config in
  let max_bucket_size =
    int_ "max_bucket_size" ~default:default.max_bucket_size config
  in
  let interrupt_on_watchman =
    bool_if_version
      "interrupt_on_watchman"
      ~default:default.interrupt_on_watchman
      config
  in
  let interrupt_on_client =
    bool_if_version
      "interrupt_on_client"
      ~default:default.interrupt_on_client
      config
  in
  let use_full_fidelity_parser =
    bool_if_version
      "use_full_fidelity_parser"
      ~default:default.use_full_fidelity_parser
      config
  in
  let trace_parsing =
    bool_if_version "trace_parsing" ~default:default.trace_parsing config
  in
  let prechecked_files =
    bool_if_version "prechecked_files" ~default:default.prechecked_files config
  in
  let predeclare_ide =
    bool_if_version "predeclare_ide" ~default:default.predeclare_ide config
  in
  let predeclare_ide_deps =
    bool_if_version
      "predeclare_ide_deps"
      ~default:default.predeclare_ide_deps
      config
  in
  let max_typechecker_worker_memory_mb =
    int_opt "max_typechecker_worker_memory_mb" config
  in
  let hg_aware = bool_if_version "hg_aware" ~default:default.hg_aware config in
  let disable_conservative_redecl =
    bool_if_version
      "disable_conservative_redecl"
      ~default:default.disable_conservative_redecl
      config
  in
  let store_decls_in_saved_state =
    bool_if_version
      "store_decls_in_saved_state"
      ~default:default.store_decls_in_saved_state
      config
  in
  let load_decls_from_saved_state =
    bool_if_version
      "load_decls_from_saved_state"
      ~default:default.load_decls_from_saved_state
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
    bool_if_version "ide_parser_cache" ~default:default.ide_parser_cache config
  in
  let ide_tast_cache =
    bool_if_version "ide_tast_cache" ~default:default.ide_tast_cache config
  in
  let idle_gc_slice =
    int_ "idle_gc_slice" ~default:default.idle_gc_slice config
  in
  let shallow_class_decl =
    bool_if_version
      "shallow_class_decl"
      ~default:default.shallow_class_decl
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
  let max_times_to_defer_type_checking =
    int_opt "max_times_to_defer_type_checking" config
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
  let remote_type_check =
    RemoteTypeCheck.load
      ~current_version
      ~default:default.remote_type_check
      config
  in
  let watchman =
    Watchman.load ~current_version ~default:default.watchman config
  in
  let recli_version =
    string_ "recli_version" ~default:default.recli_version config
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
  let remote_transport_channel = string_opt "remote_transport_channel" config in
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
  let symbolindex_quiet =
    bool_if_version
      "symbolindex_quiet"
      ~default:default.symbolindex_quiet
      config
  in
  let symbolindex_file = string_opt "symbolindex_file" config in
  let tico_invalidate_files =
    bool_if_version
      "tico_invalidate_files"
      ~default:default.tico_invalidate_files
      config
  in
  let tico_invalidate_smart =
    bool_if_version
      "tico_invalidate_smart"
      ~default:default.tico_invalidate_smart
      config
  in
  let profile_type_check_duration_threshold =
    float_
      "profile_type_check_duration_threshold"
      ~default:default.profile_type_check_duration_threshold
      config
  in
  let profile_type_check_twice =
    bool_if_version
      "profile_type_check_twice"
      ~default:default.profile_type_check_twice
      config
  in
  let profile_owner = string_opt "profile_owner" config in
  let profile_desc =
    string_ "profile_desc" ~default:default.profile_desc config
  in
  let go_to_implementation =
    bool_if_version
      "go_to_implementation"
      ~default:default.go_to_implementation
      config
  in
  let allow_unstable_features =
    bool_if_version
      "allow_unstable_features"
      ~default:default.allow_unstable_features
      config
  in
  {
    min_log_level;
    attempt_fix_credentials;
    log_categories;
    experiments;
    experiments_config_meta;
    use_saved_state;
    require_saved_state;
    load_state_script_timeout;
    load_state_natively;
    max_purgatory_clients;
    type_decl_bucket_size;
    extend_fast_bucket_size;
    enable_on_nfs;
    enable_fuzzy_search;
    lazy_parse;
    lazy_init;
    search_chunk_size;
    io_priority;
    cpu_priority;
    saved_state_cache_limit;
    can_skip_deptable;
    shm_dirs;
    max_workers;
    max_bucket_size;
    state_loader_timeouts;
    use_dummy_informant;
    informant_min_distance_restart;
    informant_use_xdb;
    use_full_fidelity_parser;
    interrupt_on_watchman;
    interrupt_on_client;
    trace_parsing;
    prechecked_files;
    predeclare_ide;
    max_typechecker_worker_memory_mb;
    hg_aware;
    hg_aware_parsing_restart_threshold;
    hg_aware_redecl_restart_threshold;
    hg_aware_recheck_restart_threshold;
    disable_conservative_redecl;
    predeclare_ide_deps;
    ide_parser_cache;
    ide_tast_cache;
    store_decls_in_saved_state;
    load_decls_from_saved_state;
    idle_gc_slice;
    shallow_class_decl;
    num_local_workers;
    parallel_type_checking_threshold;
    defer_class_declaration_threshold;
    max_times_to_defer_type_checking;
    prefetch_deferred_files;
    recheck_capture;
    recli_version;
    remote_type_check;
    remote_worker_key;
    remote_check_id;
    remote_version_specifier_required;
    remote_version_specifier;
    remote_transport_channel;
    naming_sqlite_path;
    enable_naming_table_fallback;
    symbolindex_search_provider;
    symbolindex_quiet;
    symbolindex_file;
    tico_invalidate_files;
    tico_invalidate_smart;
    profile_type_check_duration_threshold;
    profile_type_check_twice;
    profile_owner;
    profile_desc;
    go_to_implementation;
    allow_unstable_features;
    watchman;
  }

let load ~silent ~current_version config_overrides =
  load_ path ~silent ~current_version config_overrides
