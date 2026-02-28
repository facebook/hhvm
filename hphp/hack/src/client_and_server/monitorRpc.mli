(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pipe_type =
  | Default
  | Priority
  | Force_dormant_start_only

module PipeTypeMap : Stdlib.Map.S with type key = pipe_type

val pipe_type_to_string : pipe_type -> string

type handoff_options = {
  force_dormant_start: bool;
      (** If server is dormant because it is waiting for Informant to start one,
          set this to true to start a server anyway. *)
  pipe_type: pipe_type;
      (** There can be multiple named channels between server and monitor in order
          to prioritize some requests over others. Connecting code needs to specify
          which channel it wants to use. *)
}

type command =
  | HANDOFF_TO_SERVER of Connection_tracker.t * handoff_options
  | SHUT_DOWN of Connection_tracker.t
      (** Shut down all servers and then the monitor. *)

(** Sent by monitor in ServerMonitor, received by server in ServerClientProvider *)
type monitor_to_server_handoff_msg = {
  m2s_tracker: Connection_tracker.t;
  m2s_sequence_number: int;
      (** A unique number incremented for each client socket handoff from monitor to server.
            Useful to correlate monitor and server logs. *)
}

(** This writes to the specified file. Invariants maintained by callers:
(1) the file is deleted shortly after the server launches, if it already existed,
(2) after the server has first received a handoff then it writes to the file,
(3) after each successive handoff the server overwrites the file,
(4) each write is protected by a unix writer lock.
In case of failure, we log but don't raise exceptions, in the hope that a
future write will succeed. *)
val write_server_receipt_to_monitor_file :
  server_receipt_to_monitor_file:string ->
  sequence_number_high_water_mark:int ->
  unit

(** This reads the specified file, under a unix reader lock.
There are legitimate scenarios where the file might not exist,
e.g. if the server has only recently started up and hasn't yet
written any receipts. In this case we return None.
If there are failures e.g. a malformed file content, then we write
a log and return None. *)
val read_server_receipt_to_monitor_file :
  server_receipt_to_monitor_file:string -> int option
