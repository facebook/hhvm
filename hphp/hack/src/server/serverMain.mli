(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type params = {
  informant_managed: bool;
  state: ServerGlobalState.t;
  options: ServerArgs.options;
  monitor_pid: int;
  priority_in_fd: Unix.file_descr;
  force_dormant_start_only_in_fd: Unix.file_descr;
}

(* The in/out channels don't actually take type unit -- we write directly
 * to the underlying file descriptor -- but we have to declare some type for
 * these phantom types because OCaml doesn't allow polymorphic values that
 * are not functions. *)
val entry : (params, unit, unit) Daemon.entry

val run_once : ServerArgs.options -> ServerConfig.t -> ServerLocalConfig.t -> 'a

val serve_one_iteration :
  ServerEnv.genv -> ServerEnv.env -> ClientProvider.t -> ServerEnv.env

(* Main loop can choose to batch several rechecks together. Setting this will
 * disable this behavior, forcing only one recheck per serve_one_iteration
 * call. This is useful in tests to observe intermediate state. *)
val force_break_recheck_loop_for_test : bool -> unit

val program_init : ServerEnv.genv -> ServerEnv.env -> ServerEnv.env

val setup_server :
  informant_managed:bool ->
  monitor_pid:int option ->
  ServerArgs.options ->
  ServerConfig.t ->
  ServerLocalConfig.t ->
  MultiWorker.worker list * ServerEnv.env
