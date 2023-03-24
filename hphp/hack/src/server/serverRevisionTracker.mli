(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

val initialize : Hg.global_rev -> unit

(* state_name -> use_tracker_v2 *)
val on_state_enter : string -> bool -> unit

val on_state_leave :
  Path.t ->
  (* project root *)
  string ->
  (* state name *)
  Hh_json.json option ->
  (* state metadata *)
  bool ->
  (* use_tracker_v2 *)
  unit

val is_hg_updating : bool (* use_tracker_v2 *) -> bool

val check_blocking : unit -> unit

val check_non_blocking : is_full_check_done:bool -> unit

(* This module tracks changes to mergebase, and is also informed (by functions below)
 * about the sizes of jobs that are being processed. If we are in "mergebase changed"
 * state AND a big job have occurred, it can infer that this job was DUE to rebase,
 * and that restarting will be faster than going forward (due to possibility of better
 * saved state being available, or possibility to treat some files as prechecked
 * (see ServerPrecheckedFiles.ml).
 *
 * If there is no better saved state available and the only reason we restart is due
 * to handling of prechecked files, we could theoretically avoid the restart.
 * But initialization and incremental mode are so divergent that this would require
 * implementing entire thing twice (and was not done - we can only treat files
 * as prechecked during init).
 *
 * Moreover:
 *
 * - attributing incremental file changes to rebases is based on timing and
 *   there is a risk that we would precheck a real local change
 * - even if we just want to treat files as prechecked, without fully processing
 *   them, it can require quiet a lot of work in incrmental mode to invalidate all
 *   things that need to be invalidated (which are absent during init)
 * *)
val files_changed : ServerLocalConfig.t -> int -> unit

val decl_changed : ServerLocalConfig.t -> int -> unit

val typing_changed : ServerLocalConfig.t -> int -> unit
