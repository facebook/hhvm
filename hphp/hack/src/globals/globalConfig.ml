(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(*****************************************************************************)
(* Configuration file *)
(*****************************************************************************)

let program_name = "hh_server"

(* Configures only the workers. Workers can have more relaxed GC configs as
 * they are short-lived processes *)
let gc_control = Gc.get ()

let scuba_table_name = "hh_server_events"

(* Where to write temp files *)
let tmp_dir =
  try Sys.getenv "HH_TMPDIR"
  with _ ->
    Path.to_string
    @@ Path.concat (Path.make Sys_utils.temp_dir_name) "hh_server"

let shm_dir = (try Sys.getenv "HH_SHMDIR" with _ -> "/dev/shm")
