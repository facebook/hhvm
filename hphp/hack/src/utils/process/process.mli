(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Utilities to deal with subprocesses. *)

open Process_types

(** exec program ?env args
 *
 * Shells out the program with the given args. *)
val exec : string -> ?input:string -> ?env:string list -> string list ->
  Process_types.t

(** Wraps a Daemon entry point inside a Process, so we get Process's
 * goodness for free (read_and_wait_pid and is_ready). The daemon_entry
 * will be spawned into a separate process. *)
val run_daemon : ('a, 'b, 'c) Daemon.entry -> 'a -> Process_types.t

(**
 * Read data from stdout and stderr until EOF is reached. Waits for
 * process to terminate returns the stderr and stdout
 * and stderr.
 *
 * Idempotent.
 *
 * If process exits with something other than (Unix.WEXITED 0), will return a
 * Result.Error
 *)
val read_and_wait_pid :
  timeout: int -> Process_types.t ->
  ((string * string), failure) Result.t

val failure_msg : failure -> string

(** Returns true if read_and_close_pid would be nonblocking. *)
val is_ready : Process_types.t -> bool
