(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  id: string;
  (* client *)
  mutable t_start_connect_to_monitor: float;
  mutable t_opened_socket: float;
  mutable t_sent_version: float;
  mutable t_got_cstate: float;
  mutable t_ready_to_send_handoff: float;
  mutable t_connected_to_monitor: float;
  mutable t_received_hello: float;
  mutable t_sent_connection_type: float;
  mutable t_ready_to_send_cmd: float;
  mutable t_sent_cmd: float;
  mutable t_received_response: float;
  (* monitor *)
  mutable t_received_handoff: float;
  mutable t_monitor_ready: float;
  mutable t_sent_ack_to_client: float;
  mutable t_sent_fd: float;
  (* server *)
  mutable t_sleep_and_check: float;
  mutable t_monitor_fd_ready: float;
  mutable t_got_client_fd: float;
  mutable t_got_tracker: float;
  mutable t_start_server: float;
  mutable t_done_recheck: float;
  mutable t_sent_diagnostics: float;
  mutable t_start_handle_connection: float;
  mutable t_sent_hello: float;
  mutable t_got_connection_type: float;
  mutable t_waiting_for_cmd: float;
  mutable t_got_cmd: float;
  mutable t_done_full_recheck: float;
  mutable t_start_server_handle: float;
  mutable t_end_server_handle: float;
  mutable t_end_server_handle2: float;
}

let create () : t =
  {
    id = Random_id.short_string ();
    (* client *)
    t_start_connect_to_monitor = 0.;
    t_opened_socket = 0.;
    t_sent_version = 0.;
    t_got_cstate = 0.;
    t_ready_to_send_handoff = 0.;
    t_connected_to_monitor = 0.;
    t_received_hello = 0.;
    t_sent_connection_type = 0.;
    t_ready_to_send_cmd = 0.;
    t_sent_cmd = 0.;
    t_received_response = 0.;
    (* monitor *)
    t_received_handoff = 0.;
    t_monitor_ready = 0.;
    t_sent_ack_to_client = 0.;
    t_sent_fd = 0.;
    (* server *)
    t_sleep_and_check = 0.;
    t_monitor_fd_ready = 0.;
    t_got_client_fd = 0.;
    t_got_tracker = 0.;
    t_start_server = 0.;
    t_done_recheck = 0.;
    t_sent_diagnostics = 0.;
    t_start_handle_connection = 0.;
    t_sent_hello = 0.;
    t_got_connection_type = 0.;
    t_waiting_for_cmd = 0.;
    t_got_cmd = 0.;
    t_done_full_recheck = 0.;
    t_start_server_handle = 0.;
    t_end_server_handle = 0.;
    t_end_server_handle2 = 0.;
  }

let log_id (t : t) : string = "mc#" ^ t.id
