(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type pipe_type =
  | Default
  | Priority
  | Force_dormant_start_only

let pipe_type_to_string = function
  | Default -> "default"
  | Priority -> "priority"
  | Force_dormant_start_only -> "force_dormant_start_only"

type handoff_options = {
  (* If server is dormant because it is waiting for Informant to start one,
   * set this to true to start a server anyway. *)
  force_dormant_start: bool;
  (* There can be multiple named channels between server and monitor in order
   * to prioritize some requests over others. Connecting code needs to specify
   * which channel it wants to use. *)
  pipe_name: string;
}

type command =
  | HANDOFF_TO_SERVER of Connection_tracker.t * handoff_options
  (* Shut down all servers and then the monitor. *)
  | SHUT_DOWN of Connection_tracker.t

(** Sent by monitor in ServerMonitor, received by server in ServerClientProvider *)
type monitor_to_server_handoff_msg = {
  m2s_tracker: Connection_tracker.t;
  m2s_sequence_number: int;
      (** A unique number incremented for each client socket handoff from monitor to server.
            Useful to correlate monitor and server logs. *)
}

let (receipt_serialize, receipt_deserialize) =
  let key = "sequence_number_high_water_mark" in
  let serialize i = Hh_json.JSON_Object [(key, Hh_json.int_ i)] in
  let deserialize json = Hh_json_helpers.Jget.int_exn (Some json) key in
  (serialize, deserialize)

(** This writes to the specified file. Invariants maintained by callers:
(1) the file is deleted shortly after the server launches, if it already existed,
(2) after the server has first received a handoff then it writes to the file,
(3) after each successive handoff the server overwrites the file,
(4) each write is protected by a unix writer lock.
In case of failure, we log but don't raise exceptions, in the hope that a
future write will succeed. *)
let write_server_receipt_to_monitor_file
    ~(server_receipt_to_monitor_file : string)
    ~(sequence_number_high_water_mark : int) : unit =
  let json =
    receipt_serialize sequence_number_high_water_mark
    |> Hh_json.json_to_multiline
  in
  try Sys_utils.protected_write_exn server_receipt_to_monitor_file json with
  | exn ->
    let e = Exception.wrap exn in
    Hh_logger.log
      "SERVER_RECEIPT_TO_MONITOR(write) %s\n%s"
      (Exception.get_ctor_string e)
      (Exception.get_backtrace_string e |> Exception.clean_stack);
    HackEventLogger.server_receipt_to_monitor_write_exn
      ~server_receipt_to_monitor_file
      e;
    ()

(** This reads the specified file, under a unix reader lock.
There are legitimate scenarios where the file might not exist,
e.g. if the server has only recently started up and hasn't yet
written any receipts. In this case we return None.
If there are failures e.g. a malformed file content, then we write
a log and return None. *)
let read_server_receipt_to_monitor_file
    ~(server_receipt_to_monitor_file : string) : int option =
  let content = ref "[not yet read content]" in
  try
    content := Sys_utils.protected_read_exn server_receipt_to_monitor_file;
    let sequence_number_high_water_mark =
      receipt_deserialize (Hh_json.json_of_string !content)
    in
    Some sequence_number_high_water_mark
  with
  | Unix.Unix_error (Unix.ENOENT, _, _) -> None
  | exn ->
    let e = Exception.wrap exn in
    Hh_logger.log
      "SERVER_RECEIPT_TO_MONITOR(read) %s\n%s\n%s"
      (Exception.get_ctor_string e)
      (Exception.get_backtrace_string e |> Exception.clean_stack)
      !content;
    HackEventLogger.server_receipt_to_monitor_read_exn
      ~server_receipt_to_monitor_file
      e
      !content;
    None
