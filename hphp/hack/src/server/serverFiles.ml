(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Utils
open String_utils

(**
 * Slash-escaped path in the system temp directory corresponding
 * with this root directory for this extension.
 *)
let path_of_root root extension =
  (* TODO: move this to places that write this file *)
  Sys_utils.mkdir_no_fail GlobalConfig.tmp_dir;
  let root_part = Path.slash_escaped_string_of_path root in
  Filename.concat GlobalConfig.tmp_dir (spf "%s.%s" root_part extension)

let is_of_root root fn =
  let root_part = Path.slash_escaped_string_of_path root in
  string_starts_with fn (Filename.concat GlobalConfig.tmp_dir root_part)

(**
 * Lock on this file will be held after the server has finished initializing.
 * *)
let lock_file root = path_of_root root "lock"
let log_link root = path_of_root root "log"
let pids_file root = path_of_root root "pids"
let socket_file root = path_of_root root "sock"
let dfind_log root = path_of_root root "dfind"

let monitor_log_link root = path_of_root root "monitor_log"
