(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module MC = MonitorConnection

exception FailedToKill

type env = {
  root: Path.t;
  from: string;
}

let wait_for_death root secs =
  let i = ref 0 in
  try
    while MC.server_exists (ServerFiles.lock_file root) do
      incr i;
      if !i < secs then
        ignore @@ Unix.sleep 1
      else
        raise Exit
    done;
    true
  with
  | Exit -> false

let nice_kill env =
  let root_s = Path.to_string env.root in
  Printf.eprintf "Attempting to nicely kill server for %s\n%!" root_s;
  let tracker = Connection_tracker.create () in
  Hh_logger.log "[%s] ClientStop.nice_kill" (Connection_tracker.log_id tracker);
  try
    match MonitorConnection.connect_and_shut_down ~tracker env.root with
    | Ok shutdown_result -> begin
      match shutdown_result with
      | MonitorUtils.SHUTDOWN_VERIFIED ->
        Printf.eprintf "Successfully killed server for %s\n%!" root_s
      | MonitorUtils.SHUTDOWN_UNVERIFIED ->
        Printf.eprintf
          "Failed to kill server nicely for %s (Shutdown not verified)\n%!"
          root_s;
        raise FailedToKill
    end
    | Error (MonitorUtils.Build_id_mismatched_monitor_will_terminate _) ->
      Printf.eprintf "Successfully killed server for %s\n%!" root_s
    | Error
        MonitorUtils.(Connect_to_monitor_failure { server_exists = false; _ })
      ->
      Printf.eprintf "No server to kill for %s\n%!" root_s
    | Error _ ->
      Printf.eprintf "Failed to kill server nicely for %s\n%!" root_s;
      raise FailedToKill
  with
  | _ ->
    Printf.eprintf "Failed to kill server nicely for %s\n%!" root_s;
    raise FailedToKill

let mean_kill env =
  let root_s = Path.to_string env.root in
  Printf.eprintf "Attempting to meanly kill server for %s\n%!" root_s;
  let pids =
    try PidLog.get_pids (ServerFiles.pids_file env.root) with
    | PidLog.FailedToGetPids ->
      Printf.eprintf
        "Unable to figure out pids of running Hack server. Try manually killing it with `pkill hh_server`\n%!";
      raise FailedToKill
  in
  let success =
    try
      List.iter pids ~f:(fun (pid, _reason) ->
          try Sys_utils.terminate_process pid with
          | Unix.Unix_error (Unix.ESRCH, "kill", _) ->
            (* no such process *)
            ());
      wait_for_death env.root 3
    with
    | e ->
      print_endline (Exn.to_string e);
      false
  in
  if not success then (
    Printf.eprintf
      "Failed to kill server meanly for %s. Try manually killing it with `pkill hh_server`\n%!"
      root_s;
    raise FailedToKill
  ) else
    Printf.eprintf "Successfully killed server for %s\n%!" root_s

let do_kill env =
  try nice_kill env with
  | FailedToKill ->
    (try mean_kill env with
    | FailedToKill -> raise Exit_status.(Exit_with Kill_error))

let main (env : env) : Exit_status.t Lwt.t =
  HackEventLogger.client_stop ();
  do_kill env;
  Lwt.return Exit_status.No_error

let kill_server root from = do_kill { root; from }
