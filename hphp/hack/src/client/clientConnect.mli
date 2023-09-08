(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Used solely as an argument to [connect] *)
type env = {
  root: Path.t;
  from: string;
  local_config: ServerLocalConfig.t;
  autostart: bool;
  force_dormant_start: bool;
  deadline: float option;
  no_load: bool;
  watchman_debug_logging: bool;
  log_inference_constraints: bool;
  progress_callback: string option -> unit;
  do_post_handoff_handshake: bool;
  ignore_hh_version: bool;
  save_64bit: string option;
  save_human_readable_64bit_dep_map: string option;
  saved_state_ignore_hhconfig: bool;
  mini_state: string option;
  use_priority_pipe: bool;
  prechecked: bool option;
  config: (string * string) list;
  custom_hhi_path: string option;
  custom_telemetry_data: (string * string) list;
  allow_non_opt_build: bool;
}

(* [connect] returns this record, which contains everything needed for subsequent rpc calls *)
type conn = {
  connection_log_id: string;
  t_connected_to_monitor: float;
  t_received_hello: float;
  t_sent_connection_type: float;
  channels: Timeout.in_channel * out_channel;
  server_specific_files: ServerCommandTypes.server_specific_files;
  conn_progress_callback: string option -> unit;
  conn_root: Path.t;
  conn_deadline: float option;
  from: string;
}

(** Establishes a connection to the server: (1) connects to the monitor and exchanges
messages, (2) has the monitor handoff the FD to the server, (3) if env.do_post_handoff_handshake
is true then also waits for the server to send back ServerCommandTypes.Hello. *)
val connect : env -> conn Lwt.t

(** Sends a request to the server, and waits for the response *)
val rpc :
  conn -> desc:string -> 'a ServerCommandTypes.t -> ('a * Telemetry.t) Lwt.t

(** A handful of rpc commands (find-refs, go-to-impl, refactor), for grotty implementation
details, don't return an answer but instead return the message "Done_or_retry.Retry"
indicating that ClientConnect should make the exact same request a second time.
Which, through this API, it does. *)
val rpc_with_retry :
  (unit -> conn Lwt.t) ->
  desc:string ->
  'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t ->
  'a Lwt.t

(**For batch commands, retries each result in turn **)
val rpc_with_retry_list :
  (unit -> conn Lwt.t) ->
  desc:string ->
  'a ServerCommandTypes.Done_or_retry.t list ServerCommandTypes.t ->
  'a list Lwt.t
