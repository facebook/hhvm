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

let default_sharedmem_config =
  let gig = 1024 * 1024 * 1024 in
  {
    SharedMem.global_size = gig;
    heap_size = 20 * gig;
    dep_table_pow = 17;
    (* 1 << 17 *)
    hash_table_pow = 18;
    (* 1 << 18 *)
    shm_dirs = [shm_dir; tmp_dir];
    shm_min_avail = gig / 2;
    (* Half a gig by default *)
    log_level = 0;
    sample_rate = 0.0;
  }

(* There are places where we don't expect to write to shared memory, and doing
 * so would be a memory leak. But since shared memory is global, it's very easy
 * to accidentally call a function that will attempt such write. Setting all the
 * sizes to 0 will make it fail immediately. *)
let empty_sharedmem_config =
  {
    SharedMem.global_size = 0;
    heap_size = 0;
    dep_table_pow = 0;
    hash_table_pow = 0;
    shm_dirs = [];
    shm_min_avail = 0;
    log_level = 0;
    sample_rate = 0.0;
  }
