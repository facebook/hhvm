(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type server_start_options = ServerArgs.options

(** Start the server. Optionally takes in the exit code of the previously
      running server that exited. *)
val start_server :
  informant_managed:bool ->
  prior_exit_status:int option ->
  server_start_options ->
  ServerProcess.process_data

val kill_server : violently:bool -> ServerProcess.process_data -> unit

val wait_for_server_exit : timeout_t:float -> ServerProcess.process_data -> bool

val wait_pid : ServerProcess.process_data -> int * Unix.process_status

val is_saved_state_precomputed : server_start_options -> bool
