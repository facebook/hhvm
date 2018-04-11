(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The in/out channels don't actually take type unit -- we write directly
 * to the underlying file descriptor -- but we have to declare some type for
 * these phantom types because OCaml doesn't allow polymorphic values that
 * are not functions. *)
val entry:
  (bool * ServerGlobalState.t * ServerArgs.options * int, unit, unit) Daemon.entry

val run_once: ServerArgs.options -> SharedMem.handle -> 'a

val save_state: ServerArgs.options -> SharedMem.handle -> 'a

val initial_check: ServerEnv.genv -> ServerEnv.env -> ServerEnv.env

val serve_one_iteration:
  ServerEnv.genv ->
  ServerEnv.env ->
  ClientProvider.t ->
  ServerEnv.env
