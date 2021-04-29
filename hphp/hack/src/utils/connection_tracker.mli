(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Allow to record the timestamps on multiple connection events. *)
type t

(** The connection events whose timestamps we want to track. *)
type key =
  | Client_start_connect  (** Client starts connection to monitor *)
  | Client_opened_socket  (** Has opened socket to monitor *)
  | Client_sent_version  (** Has sent version to the monitor *)
  | Client_got_cstate  (** Has received cstate from monitor *)
  | Client_ready_to_send_handoff  (** Will now send the tracker to monitor *)
  | Monitor_received_handoff  (** The monitor received handoff from client *)
  | Monitor_ready  (** Monitor is now ready to do its work *)
  | Monitor_sent_ack_to_client  (** Monitor has sent ack back to client *)
  | Client_connected_to_monitor  (** Received ack from monitor *)
  | Server_sleep_and_check  (** Server loops doing slices of major GC... *)
  | Server_monitor_fd_ready  (** ...until it detects data on monitor's FD *)
  | Server_got_tracker  (** Synchronously reads tracker from monitor *)
  | Server_got_client_fd  (** Synchronously reads the client FD from monitor *)
  | Server_start_recheck  (** serve_one_iteration is ready *)
  | Server_done_recheck  (** Finished processing all outstanding changes 1st *)
  | Server_sent_diagnostics  (** Sent diagnostics to persistent connection *)
  | Server_start_handle_connection  (** Now turns its attention to the client *)
  | Server_sent_hello  (** Has sent a hello message to the client *)
  | Client_received_hello  (** Received "hello" from server *)
  | Client_sent_connection_type  (** Sent connection_type to server *)
  | Server_got_connection_type  (** Received connection_type from client *)
  | Server_waiting_for_cmd  (** Ready to receive cmd from the client *)
  | Client_ready_to_send_cmd  (** Someone invoked ClientConnect.rpc *)
  | Client_sent_cmd  (** Client has sent the rpc command to server *)
  | Server_got_cmd  (** Received cmd from the client *)
  | Server_done_full_recheck  (** Does another recheck if needed *)
  | Server_start_handle  (** Sent a ping; next up ServerRpc.handle *)
  | Server_end_handle  (** Has finished ServerRpc.handle *)
  | Server_end_handle2  (** A bit more work; next up respond to client *)
  | Client_received_response  (** Received rpc response from server *)

(** Create a connection tracker, which allows to record the timestamps
    on multiple connection events. *)
val create : unit -> t

(** Get the unique ID of the tracker. *)
val log_id : t -> string

(** Add a timestampt to the tracker for event specified by [key]. *)
val track : key:key -> ?time:float -> t -> t

(** Get the timestamp corresponding to last [Server_start_handle] event. *)
val get_server_unblocked_time : t -> float

(** Retrieve all the timestamps recorded so far. *)
val get_telemetry : t -> Telemetry.t
