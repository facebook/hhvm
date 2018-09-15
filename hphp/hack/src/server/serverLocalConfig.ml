(**
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
  watchman_init_timeout: int; (* in seconds *)
  watchman_subscribe: bool;
  watchman_synchronous_timeout : int; (* in seconds *)
  use_mini_state: bool;
  load_mini_script_timeout: int; (* in seconds *)
  (** Prefer using Ocaml implementation over load script. *)
  load_state_natively: bool;
  type_decl_bucket_size: int;
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
  shm_dirs: string list;
  state_loader_timeouts : State_loader_config.timeouts;
  max_workers : int;
  max_bucket_size : int;
  (** See HhMonitorInformant. *)
  use_dummy_informant : bool;
  informant_min_distance_restart: int;
  informant_use_xdb: bool;
  use_full_fidelity_parser : bool;
  interrupt_on_watchman : bool;
  interrupt_on_client : bool;
  trace_parsing : bool;
  prechecked_files : bool;
  predeclare_ide : bool;
  watchman_debug_logging : bool;
}

let default = {
  use_watchman = false;
  (* Buck and hgwatchman use a 10 second timeout too *)
  watchman_init_timeout = 10;
  watchman_subscribe = false;
  watchman_synchronous_timeout = 120;
  use_mini_state = false;
  load_mini_script_timeout = 20;
  load_state_natively = false;
  type_decl_bucket_size = 1000;
  enable_on_nfs = false;
  enable_fuzzy_search = true;
  lazy_parse = false;
  lazy_init = false;
  max_purgatory_clients = 400;
  search_chunk_size = 0;
  io_priority = 7;
  cpu_priority = 10;
  saved_state_cache_limit = 20;
  shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir;];
  max_workers = GlobalConfig.nbr_procs;
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
  watchman_debug_logging = false;
}

let path =
  let dir = try Sys.getenv "HH_LOCALCONF_PATH" with
    _ ->
      BuildOptions.system_config_path in
      Filename.concat dir "hh.conf"

let state_loader_timeouts_ ~default config =
  let open State_loader_config in
  let package_fetch_timeout = int_ "state_loader_timeout_package_fetch"
    ~default:default.package_fetch_timeout config in
  let find_exact_state_timeout = int_ "state_loader_timeout_find_exact_state"
    ~default:default.find_exact_state_timeout config in
  let find_nearest_state_timeout = int_ "state_loader_timeout_find_nearest_state"
    ~default:default.find_nearest_state_timeout config in
  let current_hg_rev_timeout =
    int_ "state_loader_timeout_current_hg_rev"
    ~default:default.current_hg_rev_timeout config in
  let current_base_rev_timeout =
    int_ "state_loader_timeout_current_base_rev_timeout"
    ~default:default.current_base_rev_timeout config in
  {
    State_loader_config.package_fetch_timeout;
    find_exact_state_timeout;
    find_nearest_state_timeout;
    current_hg_rev_timeout;
    current_base_rev_timeout;
  }

let load_ fn ~silent =
  (* Print out the contents in our logs so we know what settings this server
   * was started with *)
  let contents = Sys_utils.cat fn in
  if not silent then Printf.eprintf "%s:\n%s\n" fn contents;
  let config = Config_file.parse_contents contents in
  let use_watchman = bool_if_version "use_watchman"
    ~default:default.use_watchman config in
  let use_mini_state = bool_if_version "use_mini_state"
    ~default:default.use_mini_state config in
  let enable_on_nfs = bool_if_version "enable_on_nfs"
    ~default:default.enable_on_nfs config in
  let enable_fuzzy_search = bool_if_version "enable_fuzzy_search"
    ~default:default.enable_fuzzy_search config in
  let lazy_parse = bool_if_version "lazy_parse"
    ~default:default.lazy_parse config in
  let lazy_init = bool_if_version "lazy_init2"
    ~default:default.lazy_init config in
  let max_purgatory_clients = int_ "max_purgatory_clients"
    ~default:default.max_purgatory_clients config in
  let search_chunk_size = int_ "search_chunk_size"
    ~default:default.search_chunk_size config in
  let load_mini_script_timeout = int_ "load_mini_script_timeout"
    ~default:default.load_mini_script_timeout config in
  let load_state_natively = bool_if_version "load_state_natively_v4"
    ~default:default.load_state_natively config in
  let state_loader_timeouts = state_loader_timeouts_
    ~default:State_loader_config.default_timeouts config in
  let use_dummy_informant = bool_if_version "use_dummy_informant"
    ~default:default.use_dummy_informant config in
  let informant_min_distance_restart = int_ "informant_min_distance_restart"
    ~default:default.informant_min_distance_restart config in
  let informant_use_xdb = bool_if_version "informant_use_xdb_v5"
    ~default:default.informant_use_xdb config in
  let type_decl_bucket_size = int_ "type_decl_bucket_size"
    ~default:default.type_decl_bucket_size config in
  let watchman_init_timeout = int_ "watchman_init_timeout"
    ~default:default.watchman_init_timeout config in
  let watchman_subscribe = bool_if_version "watchman_subscribe_v2"
    ~default:default.watchman_subscribe config in
  let watchman_synchronous_timeout = int_ "watchman_synchronous_timeout"
    ~default:default.watchman_synchronous_timeout config in
  let io_priority = int_ "io_priority"
    ~default:default.io_priority config in
  let cpu_priority = int_ "cpu_priority"
    ~default:default.cpu_priority config in
  let saved_state_cache_limit = int_ "saved_state_cache_limit"
    ~default:default.saved_state_cache_limit config in
  let shm_dirs = string_list
    ~delim:(Str.regexp ",")
    "shm_dirs"
    ~default:default.shm_dirs
    config
  |> List.map ~f:(fun(dir) -> Path.(to_string @@ make dir)) in
  let max_workers = int_ "max_workers"
    ~default:default.max_workers config in
  (* Do not allow max workers to exceed the number of processors *)
  if max_workers > GlobalConfig.nbr_procs then
    Hh_logger.log "Warning: max_workers is higher than the number of processors. Ignoring.";
  let max_workers = min GlobalConfig.nbr_procs max_workers in
  let max_bucket_size = int_ "max_bucket_size"
    ~default:default.max_bucket_size config in
  let interrupt_on_watchman = bool_if_version "interrupt_on_watchman"
    ~default:default.interrupt_on_watchman config in
  let interrupt_on_client = bool_if_version "interrupt_on_client"
    ~default:default.interrupt_on_client config in
  let use_full_fidelity_parser = bool_if_version "use_full_fidelity_parser"
    ~default:default.use_full_fidelity_parser config in
  let trace_parsing = bool_if_version "trace_parsing"
    ~default:default.trace_parsing config in
  let prechecked_files = bool_if_version "prechecked_files"
    ~default:default.prechecked_files config in
  let predeclare_ide = bool_if_version "predeclare_ide"
    ~default:default.predeclare_ide config in
  let watchman_debug_logging = bool_if_version "watchman_debug_logging"
    ~default:default.watchman_debug_logging config in
  {
    use_watchman;
    watchman_init_timeout;
    watchman_subscribe;
    watchman_synchronous_timeout;
    use_mini_state;
    load_mini_script_timeout;
    load_state_natively;
    max_purgatory_clients;
    type_decl_bucket_size;
    enable_on_nfs;
    enable_fuzzy_search;
    lazy_parse;
    lazy_init;
    search_chunk_size;
    io_priority;
    cpu_priority;
    saved_state_cache_limit;
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
    watchman_debug_logging;
  }

let load ~silent =
  try load_ path ~silent
  with
  | e ->
    Hh_logger.log "Loading config exception: %s" (Printexc.to_string e);
    Hh_logger.log "Could not load config at %s, using defaults" path;
    default
