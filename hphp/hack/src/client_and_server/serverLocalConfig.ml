(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Config_file.Getters

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

(** These are the flags loaded from the following sources:
  - /etc/hh.conf
  - overridden by Knobs
  - overridden by ExperimentsConfig
  - overridden by CLI --config flags *)
type t = {
  saved_state: GlobalOptions.saved_state;
  min_log_level: Hh_logger.Level.t;
  attempt_fix_credentials: bool;
      (** Indicates whether we attempt to fix the credentials if they're broken *)
  log_categories: string list;
  log_large_fanouts_threshold: int option;
      (** If a fanout is greater than this value, log stats about that fanout. *)
  log_init_proc_stack_also_on_absent_from: bool;
      (** A few select events like to log the init_proc_stack, but it's voluminous!
      The default behavior is to log init_proc_stack only when "--from" is absent.
      This flag lets us log it also when "--from" is present. *)
  log_inference_constraints: bool;
      (** log type inference constraints into HackEventLogger *)
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
  use_dummy_informant: bool;  (** See Informant. *)
  informant_min_distance_restart: int;
  use_full_fidelity_parser: bool;
  interrupt_on_watchman: bool;
  interrupt_on_client: bool;
  trace_parsing: bool;
  prechecked_files: bool;
      (** Whether we use the prechecked algorithm, which only includes the fanout of
          changes since the mergebase when initializing *)
  enable_global_access_check: bool;
      (** run global access checker to check global writes and reads *)
  enable_type_check_filter_files: bool;
      (** Let the user configure which files to type check and
      which files to ignore. This flag is not expected to be
      rolled out broadly, rather it is meant to be used by
      power users only. *)
  ide_symbolindex_search_provider: string;
      (** Selects a search provider for autocomplete and symbol search *)
  predeclare_ide: bool;
  longlived_workers: bool;
  hg_aware: bool;
  hg_aware_parsing_restart_threshold: int;
  hg_aware_redecl_restart_threshold: int;
  hg_aware_recheck_restart_threshold: int;
  ide_parser_cache: bool;
  store_decls_in_saved_state: bool;
      (** When enabled, save hot class declarations (for now, specified in a special
      file in the repository) when generating a saved state. *)
  idle_gc_slice: int;
      (** Size of Gc.major_slice to be performed when server is idle. 0 to disable *)
  populate_member_heaps: bool;
      (** Populate the member signature heaps.

      If disabled, instead load lazily from shallow classes. *)
  fetch_remote_old_decls: bool;
      (** Option to fetch old decls from remote decl store *)
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
  defer_class_declaration_threshold: int option;
      (** If set, defers class declarations after N lazy declarations; if not set,
      always lazily declares classes not already in cache. *)
  produce_streaming_errors: bool;
      (** whether hh_server should write errors to errors.bin file *)
  consume_streaming_errors: bool;
      (** whether hh_client should read errors from errors.bin file *)
  rust_provider_backend: bool;
      (** Use Provider_backend.Rust_provider_backend as the global provider
       * backend, servicing File_provider, Naming_provider, and Decl_provider
       * using the hackrs implementation. *)
  rust_elab: bool;
      (** Use the Rust implementation of naming elaboration and NAST checks. *)
  naming_sqlite_path: string option;
      (** Enables the reverse naming table to fall back to SQLite for queries. *)
  enable_naming_table_fallback: bool;
  symbolindex_quiet: bool;
  tico_invalidate_files: bool;
      (** Allows hh_server to invalidate units in hhvm based on local changes *)
  tico_invalidate_smart: bool;  (** Use finer grain hh_server dependencies *)
  per_file_profiling: HackEventLogger.PerFileProfilingConfig.t;
      (** turns on memtrace .ctf writes to this directory *)
  memtrace_dir: string option;
  go_to_implementation: bool;
      (** Allows the IDE to show the 'find all implementations' button *)
  allow_unstable_features: bool;
      (** Allows unstable features to be enabled within a file via the '__EnableUnstableFeatures' attribute *)
  watchman: Watchman.t;
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
  cache_remote_decls: bool;
      (** Configure whether fetch and cache remote decls *)
  disable_naming_table_fallback_loading: bool;
      (** Stop loading from OCaml marshalled naming table if sqlite table is missing. *)
  use_distc: bool;
      (** use remote type-checking (hh_distc) rather than only local type-checking*)
  hh_distc_fanout_threshold: int;
      (** POC: @bobren - fanout threshold where we trigger hh_distc *)
  hh_distc_exponential_backoff_num_retries: int;
  ide_load_naming_table_on_disk: bool;
      (** POC: @nzthomas - allow ClientIdeDaemon to grab any naming table from disk before trying Watchman / Manifold *)
  ide_naming_table_update_threshold: int;
      (** POC: @nzthomas, if clientIDEDaemon is loading a naming table from disk instead of Manifold, set a globalrev distance threshold *)
  dump_tast_hashes: bool;
      (** Dump tast hashes into /tmp/hh_server/tast_hashes *)
  dump_tasts: string list;
      (** List of files whose TASTs to be dumped in /tmp/hh_server/tasts. *)
  use_compressed_dep_graph: bool;
      (** POC: @bobren, use new fancy compressed dep graph that is 25% the size of the old one *)
  lsp_sticky_quarantine: bool;
      (** POC: @ljw - if true, only exit quarantine when entering a new one *)
  lsp_invalidation: bool;
      (** POC: @ljw - controls how quarantine invalidates folded decls *)
  autocomplete_sort_text: bool;
      (** POC: @mckenzie - if true, autocomplete sorts using sort text attribute *)
  hack_warnings: bool;  (** POC: @catg - turn on hack warnings. *)
  warnings_default_all: bool;
      (** If true, `hh` is equivalent to `hh -Wall`, i.e. warnings are shown.
        Otherwise, `hh` is equivalent to `hh -Wnone`, i.e. warnings are not shown. *)
}
