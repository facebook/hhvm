(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

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
      [@printer
        fun fmt e ->
          fprintf fmt "Connect_exception(%s)" (Exception.get_ctor_string e)]
[@@deriving show { with_path = false }]

type connect_failure_phase =
  | Connect_open_socket
  | Connect_send_version
  | Connect_send_newline
  | Connect_receive_connection_ok
  | Connect_send_shutdown
[@@deriving show { with_path = false }]

type connect_to_monitor_failure = {
  server_exists: bool;
      (** This reflects the state of the lock file shortly after the failure happened. *)
  failure_phase: connect_failure_phase;
  failure_reason: connect_failure_reason;
}
[@@deriving show]

type connection_error =
  | Connect_to_monitor_failure of connect_to_monitor_failure
  | Server_died
  | Server_dormant
      (** Server dormant, i.e. waiting for a rebase to settle, and monitor's queue
          of pending connections in the now-full queue of connections waiting for the next server. *)
  | Server_dormant_out_of_retries
  | Build_id_mismatched_monitor_will_terminate of build_mismatch_info option
    (* hh_client binary is a different version from hh_server binary,
       so hh_server will terminate. *)
  | Build_id_mismatched_client_must_terminate of build_mismatch_info
    (* hh_client binary is a different version and must abandon its connection attempt *)
[@@deriving show { with_path = false }]

(** The telemetry we get from this ends up appearing in our telemetry a HUGE number of times.
I guess [HackEventLogger.client_connect_once_failure] is just called a lot.
We therefore take pains to make this as minimal as we can. *)
let connection_error_to_telemetry (e : connection_error) :
    string * Telemetry.t option =
  match e with
  | Server_died
  | Server_dormant
  | Server_dormant_out_of_retries
  | Build_id_mismatched_monitor_will_terminate _
  | Build_id_mismatched_client_must_terminate _ ->
    (* these errors come from MonitorConnection.connect_to_monitor [match cstate] *)
    (show_connection_error e, None)
  | Connect_to_monitor_failure { server_exists; failure_phase; failure_reason }
    ->
    let (reason, stack) =
      match failure_reason with
      | Connect_timeout ->
        (* comes from MonitorConnection.connect_to_monitor [Timeout.open_connection] *)
        ("Connect_timeout", None)
      | Connect_exception e -> begin
        (* comes from MonitorConnection.connect_to_monitor and [phase] says what part *)
        match Exception.to_exn e with
        | Unix.Unix_error (Unix.ECONNREFUSED, "connect", _) ->
          (* Generally comes from [Timeout.open_connection] *)
          ("ECONNREFUSED", None)
        | Unix.Unix_error (Unix.ENOENT, "connect", _) ->
          (* Generally comes from [Timeout.open_connection] *)
          ("ENOENT", None)
        | Unix.Unix_error (Unix.EMFILE, "pipe", _) ->
          (* Usually comes from [Process.exec Exec_command.Pgrep] *)
          ("EMFILE", None)
        | Unix.Unix_error (Unix.ECONNRESET, "read", _) ->
          (* Usually from [let cstate : ... = from_channel_without_buffering ic] *)
          ("ECONNRESET", None)
        | _ ->
          ( Exception.get_ctor_string e,
            Some (Exception.get_backtrace_string e |> Exception.clean_stack) )
      end
    in
    let exists =
      if server_exists then
        "exists"
      else
        "absent"
    in
    let reason =
      Printf.sprintf
        "%s:%s [%s]"
        (show_connect_failure_phase failure_phase)
        reason
        exists
    in
    let telemetry =
      Option.map
        ~f:(fun value ->
          Telemetry.create () |> Telemetry.string_ ~key:"stack" ~value)
        stack
    in
    (reason, telemetry)

(** The first part of the client/monitor handshake is that client sends
a [VersionPayload.serialized] over the socket, followed by a newline byte.
Note that [VersionPayload.serialized] is just an ocaml string, and can
never be anything else, because we have to maintain forwards and backwards
compatibility between different version of hh_client and hh_server. *)
module VersionPayload = struct
  type serialized = string

  type t = {
    client_version: string;
    tracker_id: string;
    terminate_monitor_on_version_mismatch: bool;
  }

  let serialize
      ~(tracker : Connection_tracker.t)
      ~(terminate_monitor_on_version_mismatch : bool) : serialized =
    Hh_json.JSON_Object
      [
        ("client_version", Hh_json.string_ Build_id.build_revision);
        ("tracker_id", Hh_json.string_ (Connection_tracker.log_id tracker));
        ( "terminate_monitor_on_version_mismatch",
          Hh_json.bool_ terminate_monitor_on_version_mismatch );
      ]
    |> Hh_json.json_to_string

  let deserialize (s : serialized) : (t, string) result =
    let open Hh_prelude.Result.Monad_infix in
    (* Newer clients send version in a json object; older clients sent just a client_version string *)
    (if String.is_prefix s ~prefix:"{" then
      try Ok (Hh_json.json_of_string s) with
      | exn -> Error (Exn.to_string exn)
    else
      Ok (Hh_json.JSON_Object [("client_version", Hh_json.string_ s)]))
    >>= fun json ->
    Hh_json_helpers.Jget.string_opt (Some json) "client_version"
    |> Result.of_option ~error:"Missing client_version"
    >>= fun client_version ->
    let tracker_id =
      Hh_json_helpers.Jget.string_opt (Some json) "tracker_id"
      |> Option.value ~default:"t#?"
    in
    let terminate_monitor_on_version_mismatch =
      Hh_json_helpers.Jget.bool_opt
        (Some json)
        "terminate_monitor_on_version_mismatch"
      |> Option.value ~default:true
    in
    Ok { client_version; tracker_id; terminate_monitor_on_version_mismatch }
end

(** The second part of the client/monitor handshake is that monitor sends
[connection_state] over the socket. In case of version mismatch it sends
[Build_id_mismatch_v3], which includes a [MismatchPayload.serialized].
Note that this is just an ocaml string, and can never be anything else,
because we have to maintain forwards and backwards compatibility between
different versions of hh_client and hh_server. (However, every version
in existince understands the binary format of Build_id_mismatch_v3...) *)
module MismatchPayload = struct
  type serialized = string [@@deriving show]

  type t = { monitor_will_terminate: bool }

  let serialize ~(monitor_will_terminate : bool) : serialized =
    Hh_json.JSON_Object
      [("monitor_will_terminate", Hh_json.bool_ monitor_will_terminate)]
    |> Hh_json.json_to_string

  let deserialize (s : serialized) : (t, string) result =
    let open Hh_prelude.Result.Monad_infix in
    (try Ok (Hh_json.json_of_string s) with
    | exn -> Error (Exn.to_string exn))
    >>= fun json ->
    let monitor_will_terminate =
      Hh_json_helpers.Jget.bool_opt (Some json) "monitor_will_terminate"
      |> Option.value ~default:true
    in
    Ok { monitor_will_terminate }
end

type connection_state =
  | Connection_ok
  | Build_id_mismatch
      (** Build_is_mismatch is never used, but it can't be removed, because
          the sequence of constructors here is part of the binary protocol
          we want to support between mismatched versions of client_server. *)
  | Build_id_mismatch_ex of build_mismatch_info
      (** Build_id_mismatch_ex also isn't used. *)
  | Build_id_mismatch_v3 of build_mismatch_info * MismatchPayload.serialized
      (** Build_id_mismatch_v3 is used! *)
  | Connection_ok_v2 of string
      (** Connection_ok_v2 isn't used yet, but might be *)
[@@deriving show]

(* Result of a shutdown monitor RPC. *)
type shutdown_result =
  (* Request sent and channel hung up, indicating the process has exited. *)
  | SHUTDOWN_VERIFIED
  (* Request sent, but channel hasn't hung up. *)
  | SHUTDOWN_UNVERIFIED

(* Message we send to the --waiting-client *)
let ready = "ready"
