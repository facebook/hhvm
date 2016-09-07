(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* The in/out channels don't actually take type unit -- we write directly
 * to the underlying file descriptor -- but we have to declare some type for
 * these phantom types because OCaml doesn't allow polymorphic values that
 * are not functions. *)
val entry: (ServerGlobalState.t * ServerArgs.options, unit, unit) Daemon.entry

val run_once: ServerArgs.options -> SharedMem.handle -> 'a

(* Things exposed for tests *)
type main_loop_stats

val empty_stats: unit -> main_loop_stats
val get_rechecked_count: main_loop_stats -> int

val serve_one_iteration:
  ServerEnv.genv ->
  ServerEnv.env ->
  ClientProvider.t ->
  main_loop_stats ->
  ServerEnv.env
