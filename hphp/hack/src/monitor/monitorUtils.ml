(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type monitor_config = {
  socket_file: string;
      (** The socket file on which the monitor is listening for connections. *)
  lock_file: string;  (** This lock is held when a monitor is alive. *)
  server_log_file: string;  (** The path to the server log file *)
  monitor_log_file: string;  (** The path to the monitor log file *)
}

(* In an Informant-directed restart, Watchman provided a new
 * mergebase, a new clock, and a list of files changed w.r.t.
 * that mergebase.
 *
 * A new server instance can "resume" from that new mergebase
 * given that it handles the list of files changed w.r.t. that
 * new mergebase, and just starts a watchman subscription
 * beginning with that clock.
 *)
type watchman_mergebase = {
  (* Watchman says current repo mergebase is this. *)
  mergebase_global_rev: int;
  (* ... plus these files changed to represent its current state *)
  files_changed: SSet.t; [@printer SSet.pp_large]
  (* ...as of this clock *)
  watchman_clock: string;
}
[@@deriving show]

let watchman_mergebase_to_string
    { mergebase_global_rev; files_changed; watchman_clock } =
  Printf.sprintf
    "watchman_mergebase (mergebase_global_rev: %d; files_changed count: %d; watchman_clock: %s)"
    mergebase_global_rev
    (SSet.cardinal files_changed)
    watchman_clock

type build_mismatch_info = {
  existing_version: string;
  existing_build_commit_time: string;
  existing_argv: string list;
  existing_launch_time: float;
}
[@@deriving show]

let current_build_info =
  {
    existing_version = Build_id.build_revision;
    existing_build_commit_time = Build_id.build_commit_time_string;
    existing_argv = Array.to_list Sys.argv;
    existing_launch_time = Unix.gettimeofday ();
  }

type connect_failure_reason =
  | Connect_timeout
  | Connect_exception of Exception.t

type connect_failure_phase =
  | Connect_open_socket
  | Connect_send_version
  | Connect_send_newline
  | Connect_receive_connection_ok
  | Connect_send_shutdown
[@@deriving show]

type connect_to_monitor_failure = {
  server_exists: bool;
      (** This reflects the state of the lock file shortly after the failure happened. *)
  failure_phase: connect_failure_phase;
  failure_reason: connect_failure_reason;
}

type connection_error =
  | Connect_to_monitor_failure of connect_to_monitor_failure
  | Server_died
  (* Server dormant and can't join the (now full) queue of connections
   * waiting for the next server. *)
  | Server_dormant
  | Server_dormant_out_of_retries
  (* Build ID mismatch indicates that hh_client binary is a different
   * version from hh_server binary, and hence hh_server will shutdown.
   *
   * It may happen due to several reasons:
   * - It is the expected mechanism by which hh_server shuts down upon a
   *   version bump (i.e. it doesn't shutdown until a newer version of the client
   *   pings it).
   * - It can arise also if you've rebuilt Hack yourself and this versionless
   *   hh_client connects to an already-running hh_server.
   * - More rarely, it may happen if chef/fbpkg didn't update binaries on disk
   *   correctly.
   *)
  | Build_id_mismatched of build_mismatch_info option

let connection_error_to_telemetry (e : connection_error) : Telemetry.t =
  let telemetry = Telemetry.create () in
  match e with
  | Server_died ->
    telemetry |> Telemetry.string_ ~key:"kind" ~value:"Server_died"
  | Server_dormant ->
    telemetry |> Telemetry.string_ ~key:"kind" ~value:"Server_dormant"
  | Server_dormant_out_of_retries ->
    telemetry
    |> Telemetry.string_ ~key:"kind" ~value:"Server_dormant_out_of_retries"
  | Build_id_mismatched _ ->
    telemetry |> Telemetry.string_ ~key:"kind" ~value:"Build_id_mismatched"
  | Connect_to_monitor_failure { server_exists; failure_phase; failure_reason }
    ->
    let (reason, exn, stack) =
      match failure_reason with
      | Connect_timeout -> ("timeout", None, None)
      | Connect_exception e ->
        ( "exception",
          Some (Exception.get_ctor_string e),
          Some (Exception.get_backtrace_string e |> Exception.clean_stack) )
    in
    telemetry
    |> Telemetry.string_ ~key:"kind" ~value:"Connection_to_monitor_Failure"
    |> Telemetry.bool_ ~key:"server_exists" ~value:server_exists
    |> Telemetry.string_
         ~key:"phase"
         ~value:(show_connect_failure_phase failure_phase)
    |> Telemetry.string_ ~key:"reason" ~value:reason
    |> Telemetry.string_opt ~key:"exn" ~value:exn
    |> Telemetry.string_opt ~key:"exn_stack" ~value:stack

type connection_state =
  | Connection_ok
  | Build_id_mismatch
      (** Build_is_mismatch is never used, but it can't be removed, because
          the sequence of constructors here is part of the binary protocol
          we want to support between mismatched versions of client_server. *)
  | Build_id_mismatch_ex of build_mismatch_info
      (** Build_id_mismatch_ex *is* used. Ex stands for 'extended' *)
  | Build_id_mismatch_v3 of build_mismatch_info * string
      (** Build_id_mismatch_v3 isn't used yet, but might be *)
  | Connection_ok_v2 of string
      (** Connection_ok_v2 isn't used yet, but might be *)

(* Result of a shutdown monitor RPC. *)
type shutdown_result =
  (* Request sent and channel hung up, indicating the process has exited. *)
  | SHUTDOWN_VERIFIED
  (* Request sent, but channel hasn't hung up. *)
  | SHUTDOWN_UNVERIFIED

(* Message we send to the --waiting-client *)
let ready = "ready"
