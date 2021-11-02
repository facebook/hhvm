(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** The deal with logging is that we normally don't want it verbose, but if something went
wrong then we wish it already had been verbose for a while! As a compromise here's what we'll do...
If any connection-tracker encounters an unfortunately slow track, then we'll turn on verbose
logging for EVERY SINGLE track for the next 30 seconds.
There's one subtlety. A tracker object is moved client -> monitor -> server -> client.
It is therefore easy to detect whether a cross-process delay took anomalously long,
e.g. the delay between client sending to monitor and monitor receiving it.
But it's not easy to turn on verbose logging across ALL processes in responses to that.
As a workaround, each tracker object itself will contain its own snapshot of its own
process's value of [verbose_until], and as trackers move from one process to another
then they'll share this as a high-water-mark. *)
let ref_verbose_until = ref 0.

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
  | Client_received_response
[@@deriving eq, show]

type t = {
  id: string;
  rev_tracks: (key * float) list;
      (** All the (trackname, timestamp) that have been tracked, in reverse order *)
  verbose_until: float;
      (** each [t] will carry around a snapshot of its process's ref_verbose_until,
      so all the separate processes can learn about it. *)
}

let create () : t =
  {
    id = Random_id.short_string ();
    rev_tracks = [];
    verbose_until = !ref_verbose_until;
  }

let get_telemetry (t : t) : Telemetry.t =
  List.fold
    t.rev_tracks
    ~init:(Telemetry.create ())
    ~f:(fun telemetry (key, value) ->
      Telemetry.float_ telemetry ~key:(show_key key) ~value)

let log_id (t : t) : string = "t#" ^ t.id

let get_server_unblocked_time (t : t) : float option =
  List.find_map t.rev_tracks ~f:(fun (key, time) ->
      Option.some_if (equal_key key Server_start_handle) time)

let track
    ~(key : key)
    ?(time : float option)
    ?(log : bool = false)
    ?(msg : string option)
    ?(long_delay_okay = false)
    (t : t) : t =
  let tnow = Unix.gettimeofday () in
  let time = Option.value time ~default:tnow in
  ref_verbose_until := Float.max t.verbose_until !ref_verbose_until;
  (* If it's been more than 3s since the last track, let's turn on verbose logging for 30s *)
  begin
    match t.rev_tracks with
    | (_prev_key, prev_time) :: _
      when Float.(time -. prev_time > 3.) && not long_delay_okay ->
      ref_verbose_until := tnow +. 30.;
      Hh_logger.log
        "[%s] Connection_tracker unfortunately slow: %0.1fs. Verbose until %s\n%s"
        (log_id t)
        (time -. prev_time)
        (Utils.timestring !ref_verbose_until)
        (t.rev_tracks
        |> List.rev
        |> List.map ~f:(fun (key, time) ->
               Printf.sprintf "   >%s %s" (Utils.timestring time) (show_key key))
        |> String.concat ~sep:"\n")
    | _ -> ()
  end;
  let t =
    {
      t with
      rev_tracks = (key, time) :: t.rev_tracks;
      verbose_until = !ref_verbose_until;
    }
  in
  if log || Float.(tnow < !ref_verbose_until) then
    Hh_logger.log
      "[%s] %s %s%s"
      (log_id t)
      (show_key key)
      (Option.value msg ~default:"")
      (if String.equal (Utils.timestring tnow) (Utils.timestring time) then
        ""
      else
        ", was at " ^ Utils.timestring time);
  t
