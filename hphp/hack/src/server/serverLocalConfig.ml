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

type t = {
  use_watchman: bool;
  watchman_init_timeout: int;
  (* in seconds *)
  watchman_subscribe: bool;
  watchman_synchronous_timeout: int;
  (* in seconds *)
  use_saved_state: bool;
  (* should we attempt to load saved-state? (subject to further options) *)
  require_saved_state: bool;
  (* if attempting saved-state, should we fail upon failure? *)
  load_state_script_timeout: int;
      (* in seconds *)
      (** Prefer using Ocaml implementation over load script. *)
  load_state_natively: bool;
  type_decl_bucket_size: int;
  extend_fast_bucket_size: int;
  enable_on_nfs: bool;
  enable_fuzzy_search: bool;
  lazy_parse: bool;
  lazy_init: bool;
      (** Limit the number of clients that can sit in purgatory waiting
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
  max_bucket_size: int;  (** See HhMonitorInformant. *)
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
  watchman_debug_logging: bool;
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
  (* If set, defers class declarations after N lazy declarations; if not set,
    always lazily declares classes not already in cache. *)
  defer_class_declaration_threshold: int option;
  (* If set, prevents type checking of files from being deferred more than
    the number of times greater than or equal to the threshold. If not set,
    defers class declarations indefinitely. *)
  max_times_to_defer_type_checking: int option;
  (* The whether to use the hook that prefetches files on an Eden checkout *)
  prefetch_deferred_files: bool;
  (* If set, distributes type checking to remote workers if the number of files to
    type check exceeds the threshold. If not set, then always checks everything locally. *)
  remote_type_check_threshold: int option;
  (* Enables remote type check *)
  remote_type_check: bool;
  (* If set, uses the key to fetch type checking jobs *)
  remote_worker_key: string option;
  (* If set, uses the check ID when logging events in the context of remove init/work *)
  remote_check_id: string option;
  (* Indicates the size of the job below which Eden should be used by the remote worker *)
  remote_worker_eden_checkout_threshold: int;
  (* Dictates the number of remote type checking workers *)
  num_remote_workers: int;
  (* The version of the package the remote worker is to install *)
  remote_version_specifier: string option;
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
  profile_type_check_duration_threshold: float;
  (* Use shared_lru workers *)
  use_lru_workers: bool;
}

let default =
  {
    use_watchman = false;
    (* Buck and hgwatchman use a 10 second timeout too *)
    watchman_init_timeout = 10;
    watchman_subscribe = false;
    watchman_synchronous_timeout = 120;
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
    watchman_debug_logging = false;
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
    defer_class_declaration_threshold = None;
    max_times_to_defer_type_checking = None;
    prefetch_deferred_files = false;
    remote_type_check_threshold = None;
    remote_type_check = true;
    remote_worker_key = None;
    remote_check_id = None;
    num_remote_workers = 4;
    remote_version_specifier = None;
    remote_worker_eden_checkout_threshold = 10000;
    naming_sqlite_path = None;
    enable_naming_table_fallback = false;
    symbolindex_search_provider = "TrieIndex";
    symbolindex_quiet = false;
    symbolindex_file = None;
    tico_invalidate_files = false;
    tico_invalidate_smart = false;
    profile_type_check_duration_threshold = 0.05;
    (* seconds *)
    use_lru_workers = false;
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

let load_ fn ~silent ~current_version overrides =
  let config =
    Config_file.apply_overrides
      ~silent
      ~config:(Config_file.parse_local_config ~silent fn)
      ~overrides
  in
  let use_watchman =
    bool_if_version "use_watchman" ~default:default.use_watchman config
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
  let watchman_init_timeout =
    int_ "watchman_init_timeout" ~default:default.watchman_init_timeout config
  in
  let watchman_subscribe =
    bool_if_version
      "watchman_subscribe_v2"
      ~default:default.watchman_subscribe
      config
  in
  let watchman_synchronous_timeout =
    int_
      "watchman_synchronous_timeout"
      ~default:default.watchman_synchronous_timeout
      config
  in
  let io_priority = int_ "io_priority" ~default:default.io_priority config in
  let cpu_priority =
    int_ "cpu_priority" ~default:default.cpu_priority config
  in
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
  let watchman_debug_logging =
    bool_if_version
      "watchman_debug_logging"
      ~default:default.watchman_debug_logging
      config
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
  let remote_type_check_threshold =
    int_opt "remote_type_check_threshold" config
  in
  let remote_type_check =
    bool_if_min_version
      "remote_type_check"
      ~default:true
      ~current_version
      config
  in
  let remote_worker_key = string_opt "remote_worker_key" config in
  let remote_check_id = string_opt "remote_check_id" config in
  let num_remote_workers =
    int_ "num_remote_workers" ~default:default.num_remote_workers config
  in
  let remote_version_specifier =
    string_opt "remote_version_specifier" config
  in
  let remote_worker_eden_checkout_threshold =
    int_
      "remote_worker_eden_checkout_threshold"
      ~default:default.remote_worker_eden_checkout_threshold
      config
  in
  let naming_sqlite_path = string_opt "naming_sqlite_path" config in
  let enable_naming_table_fallback =
    match naming_sqlite_path with
    | Some _ -> true
    | None ->
      bool_if_min_version
        "enable_naming_table_fallback"
        ~default:default.enable_naming_table_fallback
        ~current_version
        config
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
  let use_lru_workers =
    bool_if_version "use_lru_workers" ~default:default.use_lru_workers config
  in
  {
    use_watchman;
    watchman_init_timeout;
    watchman_subscribe;
    watchman_synchronous_timeout;
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
    watchman_debug_logging;
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
    defer_class_declaration_threshold;
    max_times_to_defer_type_checking;
    prefetch_deferred_files;
    remote_worker_eden_checkout_threshold;
    remote_type_check_threshold;
    remote_type_check;
    remote_worker_key;
    remote_check_id;
    num_remote_workers;
    remote_version_specifier;
    naming_sqlite_path;
    enable_naming_table_fallback;
    symbolindex_search_provider;
    symbolindex_quiet;
    symbolindex_file;
    tico_invalidate_files;
    tico_invalidate_smart;
    profile_type_check_duration_threshold;
    use_lru_workers;
  }

let load ~silent ~current_version config_overrides =
  load_ path ~silent ~current_version config_overrides
