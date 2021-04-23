(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type pipe_from_server = Unix.file_descr

let make_pipe_from_server (fd : Unix.file_descr) : pipe_from_server = fd

let read_from_server (fd : pipe_from_server) :
    MonitorRpc.server_to_monitor_message option =
  try
    let (readable, _, _) = Unix.select [fd] [] [] 0.0 in
    match readable with
    | [] -> None
    | _ -> Some (Marshal_tools.from_fd_with_preamble fd)
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

let send_to_monitor (msg : MonitorRpc.server_to_monitor_message) : unit =
  match !pipe_to_monitor_ref with
  | None -> ()
  (* This function can be invoked in non-server code paths,
   * when there is no monitor. *)
  | Some fd ->
    begin
      match (msg, !previous_message) with
      | (msg, Some previous_message)
        when MonitorRpc.equal_server_to_monitor_message msg previous_message ->
        (* Avoid sending the same message repeatedly. *)
        ()
      | _ ->
        previous_message := Some msg;
        let (_ : int) = Marshal_tools.to_fd_with_preamble fd msg in
        ()
    end

(** latest_progress is the progress message we most recently wrote to server_progress_file *)
let latest_progress : string ref = ref "server about to start up"

(** latest_warning is the warning message we most recently wrote to server_progress_file *)
let latest_warning : string option ref = ref None

let write_progress_file () =
  let pid = Unix.getpid () in
  let server_progress_file = ServerFiles.server_progress_file pid in
  let server_progress =
    ServerCommandTypes.
      {
        server_progress = !latest_progress;
        server_warning = !latest_warning;
        server_timestamp = Unix.gettimeofday ();
      }
  in
  ServerCommandTypesUtils.write_progress_file
    ~server_progress_file
    ~server_progress;
  ()

let send_warning s =
  begin
    match (!latest_warning, s) with
    | (Some latest, Some s) when String.equal latest s -> ()
    | (None, None) -> ()
    | (_, _) ->
      latest_warning := s;
      write_progress_file ()
  end;
  send_to_monitor (MonitorRpc.PROGRESS_WARNING s)

let send_progress ?(include_in_logs = true) fmt =
  let f s =
    if include_in_logs then Hh_logger.log "%s" s;
    if not (String.equal !latest_progress s) then begin
      latest_progress := s;
      write_progress_file ()
    end;
    send_to_monitor (MonitorRpc.PROGRESS s)
  in
  Printf.ksprintf f fmt

(* The message will look roughly like this:
  <operation> <done_count>/<total_count> <unit> <percent done> <extra>*)
let make_percentage_progress_message
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : string =
  let unit =
    if String.equal unit "" then
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

let send_percentage_progress
    ~(operation : string)
    ~(done_count : int)
    ~(total_count : int)
    ~(unit : string)
    ~(extra : string option) : unit =
  send_progress
    ~include_in_logs:false
    "%s"
    (make_percentage_progress_message
       ~operation
       ~done_count
       ~total_count
       ~unit
       ~extra)
