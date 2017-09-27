(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Config_file.Getters
open Core

type t = {
  use_watchman: bool;
  watchman_init_timeout: int; (* in seconds *)
  watchman_subscribe: bool;
  (** The directory (relative path) for touching temporary files
   * meant for synchronized watchman queries. *)
  watchman_sync_directory: string;
  use_mini_state: bool;
  use_hackfmt: bool;
  load_mini_script_timeout: int; (* in seconds *)
  (** Prefer using Ocaml implementation over load script. *)
  load_state_natively: bool;
  type_decl_bucket_size: int;
  enable_on_nfs: bool;
  enable_fuzzy_search: bool;
  lazy_parse: bool;
  lazy_init: bool;
  incremental_init : bool;
  (** Limit the number of clients that can sit in purgatory waiting
   * for a server to be started because we don't want this to grow
   * unbounded. *)
  max_purgatory_clients: int;
  search_chunk_size: int;
  io_priority: int;
  cpu_priority: int;
  shm_dirs: string list;
  start_with_recorder_on : bool;
  (** See HhMonitorInformant. *)
  use_dummy_informant : bool;
  informant_min_distance_restart: int;
  load_script_config: LoadScriptConfig.t;
}

let default = {
  use_watchman = false;
  (* Buck and hgwatchman use a 10 second timeout too *)
  watchman_init_timeout = 10;
  watchman_subscribe = false;
  watchman_sync_directory = "";
  use_mini_state = false;
  use_hackfmt = false;
  load_mini_script_timeout = 20;
  load_state_natively = false;
  type_decl_bucket_size = 1000;
  enable_on_nfs = false;
  enable_fuzzy_search = true;
  lazy_parse = false;
  lazy_init = false;
  incremental_init = false;
  max_purgatory_clients = 400;
  search_chunk_size = 0;
  io_priority = 7;
  cpu_priority = 10;
  shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir;];
  start_with_recorder_on = false;
  use_dummy_informant = true;
  informant_min_distance_restart = 100;
  load_script_config = LoadScriptConfig.default;
}

let path =
  let dir = try Sys.getenv "HH_LOCALCONF_PATH" with _ -> "/etc" in
  Filename.concat dir "hh.conf"

let warn_dir_not_exist dir = match dir with
  | None ->
    Hh_logger.log "%s" ("Watchman sync directory not specified. We will be " ^
      "using the repo root.");
    default.watchman_sync_directory
  | Some dir ->
    dir

let load_ fn ~silent =
  (* Print out the contents in our logs so we know what settings this server
   * was started with *)
  let contents = Sys_utils.cat fn in
  if not silent then Printf.eprintf "%s:\n%s\n" fn contents;
  let config = Config_file.parse_contents contents in
  let use_watchman = bool_ "use_watchman"
    ~default:default.use_watchman config in
  let use_mini_state = bool_ "use_mini_state"
    ~default:default.use_mini_state config in
  let enable_on_nfs = bool_ "enable_on_nfs"
    ~default:default.enable_on_nfs config in
  let enable_fuzzy_search = bool_ "enable_fuzzy_search"
    ~default:default.enable_fuzzy_search config in
  let lazy_parse = bool_ "lazy_parse"
    ~default:default.lazy_parse config in
  let lazy_init = bool_ "lazy_init2"
    ~default:default.lazy_init config in
  let incremental_init = bool_ "incremental_init"
    ~default:default.incremental_init config in
  let max_purgatory_clients = int_ "max_purgatory_clients"
    ~default:default.max_purgatory_clients config in
  let search_chunk_size = int_ "search_chunk_size"
    ~default:default.search_chunk_size config in
  let load_mini_script_timeout = int_ "load_mini_script_timeout"
    ~default:default.load_mini_script_timeout config in
  let load_state_natively = bool_ "load_state_natively"
    ~default:default.load_state_natively config in
  let use_hackfmt = bool_ "use_hackfmt"
    ~default:default.use_hackfmt config in
  let start_with_recorder_on = bool_ "start_with_recorder_on"
    ~default:default.start_with_recorder_on config in
  let use_dummy_informant = bool_ "use_dummy_informant"
    ~default:default.use_dummy_informant config in
  let informant_min_distance_restart = int_ "informant_min_distance_restart"
    ~default:default.informant_min_distance_restart config in
  let type_decl_bucket_size = int_ "type_decl_bucket_size"
    ~default:default.type_decl_bucket_size config in
  let watchman_init_timeout = int_ "watchman_init_timeout"
    ~default:default.watchman_init_timeout config in
  let watchman_subscribe = bool_ "watchman_subscribe_v2"
    ~default:default.watchman_subscribe config in
  let watchman_sync_directory_opt =
    string_opt "watchman_sync_directory" config in
  let watchman_sync_directory =
    if use_watchman
    then warn_dir_not_exist watchman_sync_directory_opt
    else
      ""
  in
  let io_priority = int_ "io_priority"
    ~default:default.io_priority config in
  let cpu_priority = int_ "cpu_priority"
    ~default:default.cpu_priority config in
  let shm_dirs = string_list
    ~delim:(Str.regexp ",")
    "shm_dirs"
    ~default:default.shm_dirs
    config
  |> List.map ~f:(fun(dir) -> Path.(to_string @@ make dir)) in
  let load_script_config = LoadScriptConfig.default in
  {
    use_watchman;
    watchman_init_timeout;
    watchman_subscribe;
    watchman_sync_directory;
    use_mini_state;
    use_hackfmt;
    load_mini_script_timeout;
    load_state_natively;
    max_purgatory_clients;
    type_decl_bucket_size;
    enable_on_nfs;
    enable_fuzzy_search;
    lazy_parse;
    lazy_init;
    incremental_init;
    search_chunk_size;
    io_priority;
    cpu_priority;
    shm_dirs;
    start_with_recorder_on;
    use_dummy_informant;
    informant_min_distance_restart;
    load_script_config;
  }

let load ~silent =
  try load_ path ~silent
  with
  | e ->
    Hh_logger.log "Loading config exception: %s" (Printexc.to_string e);
    Hh_logger.log "Could not load config at %s, using defaults" path;
    default
