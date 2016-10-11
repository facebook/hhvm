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
  use_mini_state: bool;
  load_mini_script_timeout: int; (* in seconds *)
  type_decl_bucket_size: int;
  enable_on_nfs: bool;
  enable_fuzzy_search: bool;
  lazy_decl: bool;
  lazy_parse: bool;
  io_priority: int;
  cpu_priority: int;
  shm_dirs: string list;
  load_script_config: LoadScriptConfig.t;
}

let default = {
  use_watchman = false;
  watchman_init_timeout = 10;
  watchman_subscribe = false;
  use_mini_state = false;
  load_mini_script_timeout = 20;
  type_decl_bucket_size = 1000;
  enable_on_nfs = false;
  enable_fuzzy_search = true;
  lazy_decl = false;
  lazy_parse = false;
  io_priority = 7;
  cpu_priority = 10;
  shm_dirs = [GlobalConfig.shm_dir; GlobalConfig.tmp_dir;];
  load_script_config = LoadScriptConfig.default;
}

let path =
  let dir = try Sys.getenv "HH_LOCALCONF_PATH" with _ -> "/etc" in
  Filename.concat dir "hh.conf"

let load_ fn =
  (* Print out the contents in our logs so we know what settings this server
   * was started with *)
  let contents = Sys_utils.cat fn in
  Printf.eprintf "%s:\n%s\n" fn contents;
  let config = Config_file.parse_contents contents in
  let use_watchman = bool_ "use_watchman" ~default:false config in
  let use_mini_state = bool_ "use_mini_state" ~default:false config in
  let enable_on_nfs = bool_ "enable_on_nfs" ~default:false config in
  let enable_fuzzy_search = bool_ "enable_fuzzy_search" ~default:true config in
  let lazy_decl = bool_ "lazy_decl" ~default:false config in
  let lazy_parse = bool_ "lazy_parse" ~default:false config in
  let load_mini_script_timeout =
    int_ "load_mini_script_timeout" ~default:20 config in
  let type_decl_bucket_size =
    int_ "type_decl_bucket_size" ~default:1000 config in
  (* Buck and hgwatchman use a 10 second timeout too *)
  let watchman_init_timeout =
    int_ "watchman_init_timeout" ~default:10 config in
  let watchman_subscribe = bool_ "watchman_subscribe" ~default:false config in
  let io_priority = int_ "io_priority" ~default:7 config in
  let cpu_priority = int_ "cpu_priority" ~default:10 config in
  let shm_dirs = string_list
    ~delim:(Str.regexp ",")
    "shm_dirs"
    ~default:default.shm_dirs
    config
  |> List.map ~f:(fun(dir) -> Path.(to_string @@ make dir)) in
  let saved_state_load_type =
    LoadScriptConfig.saved_state_load_type_ config in
  let load_script_config =
    LoadScriptConfig.createLoadScriptConfig saved_state_load_type in
  {
    use_watchman;
    watchman_init_timeout;
    watchman_subscribe;
    use_mini_state;
    load_mini_script_timeout;
    type_decl_bucket_size;
    enable_on_nfs;
    enable_fuzzy_search;
    lazy_decl;
    lazy_parse;
    io_priority;
    cpu_priority;
    shm_dirs;
    load_script_config;
  }

let load () =
  try load_ path
  with _ ->
    Hh_logger.log "Could not load config at %s, using defaults" path;
    default
