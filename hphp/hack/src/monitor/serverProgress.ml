(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

type pipe_from_server = Unix.file_descr

type seconds = int

let make_pipe_from_server (fd : Unix.file_descr) : pipe_from_server = fd

let read_from_server (fd : pipe_from_server) :
    MonitorRpc.server_to_monitor_message option =
  try
    let (readable, _, _) = Unix.select [fd] [] [] 0.0 in
    if readable = [] then
      None
    else
      Some (Marshal_tools.from_fd_with_preamble fd)
  with e ->
    (* If something went wrong here, the system is likely in broken state
     * (the server died). We'll keep going so that monitor
     * can resolve this (by restarting the server / exiting itself *)
    let stack = Printexc.get_backtrace () in
    Hh_logger.exc stack e;
    None

let pipe_to_monitor_ref : Unix.file_descr option ref = ref None

let previous_message : MonitorRpc.server_to_monitor_message option ref =
  ref None

let make_pipe_to_monitor (fd : Unix.file_descr) : unit =
  pipe_to_monitor_ref := Some fd

type timeout_outcome =
  | Did_time_out
  | Did_not_time_out

let send_with_timeout ~(timeout : seconds) fd msg : timeout_outcome =
  Timeout.with_timeout
    ~timeout
    ~on_timeout:(fun _ -> Did_time_out)
    ~do_:(fun timeout ->
      let (_ : int) = Marshal_tools.to_fd_with_preamble ~timeout fd msg in
      Did_not_time_out)

let send_without_timeout fd msg : unit =
  let (_ : int) = Marshal_tools.to_fd_with_preamble fd msg in
  ()

let send_to_monitor
    (type res)
    (send_to_fd : _ -> _ -> res)
    ~(default : res)
    (msg : MonitorRpc.server_to_monitor_message) : res =
  match !pipe_to_monitor_ref with
  | None ->
    (* This function can be invoked in non-server code paths,
     * when there is no monitor. *)
    default
  | Some fd ->
    begin
      match (msg, !previous_message) with
      | (msg, Some previous_message) when msg = previous_message ->
        (* Avoid sending the same message repeatedly. *)
        default
      | _ ->
        previous_message := Some msg;
        send_to_fd fd msg
    end

let send_to_monitor_with_timeout ~(timeout : seconds) msg : timeout_outcome =
  send_to_monitor (send_with_timeout ~timeout) ~default:Did_not_time_out msg

let send_to_monitor_without_timout msg : unit =
  send_to_monitor send_without_timeout ~default:() msg

let send_progress_to_monitor_w_timeout : ?include_in_logs:bool -> _ =
 fun ?(include_in_logs = true) fmt ->
  let f s =
    if include_in_logs then Hh_logger.log "%s" s;
    let (_ : timeout_outcome) =
      send_to_monitor_with_timeout ~timeout:1 (MonitorRpc.PROGRESS s)
    in
    ()
  in
  Printf.ksprintf f fmt

let send_progress_warning_to_monitor msg =
  send_to_monitor_without_timout (MonitorRpc.PROGRESS_WARNING msg)

(* The message will look roughly like this:
  <operation> <done_count>/<total_count> <unit> <percent done> <extra>*)
let make_percentage_progress_message
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : string =
  let unit =
    if unit = "" then
      unit
    else
      unit ^ " "
  in
  let percent = 100.0 *. float_of_int done_count /. float_of_int total_count in
  let main_message =
    Printf.sprintf
      "%s %d/%d %s(%.1f%%)"
      operation
      done_count
      total_count
      unit
      percent
  in
  match extra with
  | Some extra -> main_message ^ " " ^ extra
  | None -> main_message

let send_percentage_progress_to_monitor_w_timeout
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : unit =
  send_progress_to_monitor_w_timeout
    ~include_in_logs:false
    "%s"
    (make_percentage_progress_message
       ~operation
       ~done_count
       ~total_count
       ~unit
       ~extra)
