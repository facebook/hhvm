(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Configuration file *)
(*****************************************************************************)

let program_name = "hh_server"

(* Configures only the workers. Workers can have more relaxed GC configs as
 * they are short-lived processes *)
let gc_control = Gc.get ()

let scuba_table_name =
  try Sys.getenv "HH_SCUBA_TABLE" with
  | _ -> "hh_server_events"

(* Where to write temp files *)
let tmp_dir = Tmp.hh_server_tmp_dir

let shm_dir =
  try Sys.getenv "HH_SHMDIR" with
  | _ -> "/dev/shm"
