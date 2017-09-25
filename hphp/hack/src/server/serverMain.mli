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
val entry:
  (bool * ServerGlobalState.t * ServerArgs.options, unit, unit) Daemon.entry

val run_once: ServerArgs.options -> SharedMem.handle -> 'a

val save_state: ServerArgs.options -> SharedMem.handle -> 'a

val serve_one_iteration:
  iteration_flag:ServerEnv.recheck_iteration_flag option ->
  ServerEnv.genv ->
  ServerEnv.env ->
  ClientProvider.t ->
  ServerEnv.env * ServerEnv.recheck_iteration_flag option
