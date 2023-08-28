(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Utils

let tmp : Path.t option ref = ref None

(** Normally, the ServerFiles.* functions use GlobalConfig.tmp_dir
which is initialized at startup to be either HH_TMPDIR if defined
or /tmp/hh_server otherwise.

But some tests like to use these ServerFiles.* functions with a
different tmpdir chosen at runtime. These tests can call
ServerFiles.set_tmp_FOR_TESTING_ONLY, and all subsequent ServerFiles.*
will refer to that new tmp directory.
CARE! This is only to be used in limited circumstances. All the
rest of the codebase will continue to use HH_TMPDIR-or-/tmp/hh_server.
This only affects the ServerFiles.* functions; nothing else.
You have been warned. *)
let set_tmp_FOR_TESTING_ONLY (t : Path.t) : unit = tmp := Some t

let get_tmp () : string =
  Option.value_map !tmp ~f:Path.to_string ~default:GlobalConfig.tmp_dir

(** Slash-escaped path in the system temp directory corresponding
    with this root directory for this extension. *)
let path_of_root root extension =
  (* TODO: move this to places that write this file *)
  Sys_utils.mkdir_no_fail (get_tmp ());
  let root_part = Path.slash_escaped_string_of_path root in
  Filename.concat (get_tmp ()) (spf "%s%s" root_part extension)

let is_of_root root fn =
  let root_part = Path.slash_escaped_string_of_path root in
  String.is_prefix fn ~prefix:(Filename.concat (get_tmp ()) root_part)

(**
 * Lock on this file will be held after the server has finished initializing.
 * *)
let lock_file root = path_of_root root ".lock"

let log_link root = path_of_root root ".log"

let pids_file root = path_of_root root ".pids"

let socket_file root = path_of_root root ".sock"

let dfind_log root = path_of_root root ".dfind"

let client_log root = path_of_root root ".client_log"

let client_lsp_log root = path_of_root root ".client_lsp_log"

let client_ide_log root = path_of_root root ".client_ide_log"

let client_ide_naming_table root = path_of_root root ".client_ide_naming_table"

let monitor_log_link root = path_of_root root ".monitor_log"

let errors_file_path (root : Path.t) : string = path_of_root root ".errors.bin"

let server_finale_file (pid : int) : string =
  Filename.concat (get_tmp ()) (spf "%d.fin" pid)

let server_progress_file (root : Path.t) : string =
  path_of_root root ".progress.json"

let server_receipt_to_monitor_file (pid : int) : string =
  Filename.concat (get_tmp ()) (spf "server_receipt_to_monitor.%d.json" pid)
