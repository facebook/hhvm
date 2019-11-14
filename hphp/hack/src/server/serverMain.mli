(*
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
val entry :
  ( bool
    * ServerGlobalState.t
    * ServerArgs.options
    * int
    * Unix.file_descr
    * Unix.file_descr,
    unit,
    unit )
  Daemon.entry

val run_once : ServerArgs.options -> ServerConfig.t -> ServerLocalConfig.t -> 'a

val serve_one_iteration :
  ServerEnv.genv -> ServerEnv.env -> ClientProvider.t -> ServerEnv.env

(* Main loop can choose to batch several rechecks together. Setting this will
 * disable this behavior, forcing only one recheck per serve_one_iteration
 * call. This is useful in tests to observe intermediate state. *)
val force_break_recheck_loop_for_test : bool -> unit
