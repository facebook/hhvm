(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  id: string;
  telemetry: Telemetry.t;
      (** A set of timestamps indexed by keys of type [key]. *)
  server_unblocked_time: float;
      (** this field is read by clientLsp: we store it explicitly
      here so we can read it, as well as inside the write-only Telemetry.t *)
}

type key =
  | Client_start_connect
  | Client_opened_socket
  | Client_sent_version
  | Client_got_cstate
  | Client_ready_to_send_handoff
  | Monitor_received_handoff
  | Monitor_ready
  | Monitor_sent_ack_to_client
  | Client_connected_to_monitor
  | Server_sleep_and_check
  | Server_monitor_fd_ready
  | Server_got_tracker
  | Server_got_client_fd
  | Server_start_recheck
  | Server_done_recheck
  | Server_sent_diagnostics
  | Server_start_handle_connection
  | Server_sent_hello
  | Client_received_hello
  | Client_sent_connection_type
  | Server_got_connection_type
  | Server_waiting_for_cmd
  | Client_ready_to_send_cmd
  | Client_sent_cmd
  | Server_got_cmd
  | Server_done_full_recheck
  | Server_start_handle
  | Server_end_handle
  | Server_end_handle2
  | Client_received_response
[@@deriving eq, show]

let create () : t =
  {
    id = Random_id.short_string ();
    server_unblocked_time = 0.;
    telemetry = Telemetry.create ();
  }

let get_telemetry (t : t) : Telemetry.t = t.telemetry

let log_id (t : t) : string = "t#" ^ t.id

let get_server_unblocked_time (t : t) : float = t.server_unblocked_time

let track ~(key : key) ?(time : float option) (t : t) : t =
  let tnow = Unix.gettimeofday () in
  let time = Option.value time ~default:tnow in
  let t =
    if equal_key key Server_start_handle then
      { t with server_unblocked_time = time }
    else
      t
  in
  let key = String_utils.lstrip (show_key key) "Connection_tracker." in
  Hh_logger.log
    "[%s] Connection_tracker.%s%s"
    (log_id t)
    key
    ( if String.equal (Utils.timestring tnow) (Utils.timestring time) then
      ""
    else
      ", was at " ^ Utils.timestring time );
  { t with telemetry = Telemetry.float_ t.telemetry ~key ~value:time }
