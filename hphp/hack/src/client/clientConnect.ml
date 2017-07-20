(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SMUtils = ServerMonitorUtils

exception Server_hung_up

type env = {
  root : Path.t;
  autostart : bool;
  force_dormant_start : bool;
  retries : int option;
  retry_if_init : bool;
  expiry : float option;
  no_load : bool;
  profile_log : bool;
  ai_mode : string option;
  progress_callback: string option -> unit;
  do_post_handoff_handshake: bool;
}

let tty_progress_reporter (status: string option) : unit =
  if Tty.spinner_used () then Tty.print_clear_line stderr;
  match status with
  | None -> ()
  | Some s -> Tty.eprintf "hh_server is busy: %s %s%!" s (Tty.spinner())

let null_progress_reporter (_status: string option) : unit =
  ()

let running_load_script_re = Str.regexp_string "Running load script"

let load_state_found_re = Str.regexp_string "Load state found"

let load_state_not_found_re = Str.regexp_string "Load state not found"

let parsing_re = Str.regexp_string "Parsing"

let naming_re = Str.regexp_string "Naming"

let determining_changes_re = Str.regexp_string "Determining changes"

let type_decl_re = Str.regexp_string "Type-decl"

let type_check_re = Str.regexp_string "Type-check"

let server_ready_re = Str.regexp_string "Server is READY"

let matches_re re s =
  let pos = try Str.search_forward re s 0 with Not_found -> -1 in
  pos > -1

let re_list =
  [running_load_script_re;
   load_state_found_re;
   load_state_not_found_re;
   parsing_re;
   naming_re;
   determining_changes_re;
   type_decl_re;
   type_check_re;
   server_ready_re;
  ]

let is_valid_line s =
  List.exists (fun re -> matches_re re s) re_list


let rec is_load_state_not_found l =
  match l with
  | [] -> false
  | s::ss ->
     if matches_re load_state_found_re s then
       false
     else if matches_re load_state_not_found_re s then
       true
     else if matches_re server_ready_re s then
       false
     else
       is_load_state_not_found ss

let msg_of_tail tail_env =
  let line = Tail.last_line tail_env in
  if matches_re running_load_script_re line then
    "[running load script]"
  else if matches_re load_state_found_re line then
    "[load state found]"
  else if matches_re load_state_not_found_re line then
    "[load state not found]"
  else if matches_re parsing_re line then
    "[parsing]"
  else if matches_re naming_re line then
    "[naming]"
  else if matches_re determining_changes_re line then
    "[determining changes]"
  else if matches_re type_decl_re line then
    "[type decl]"
  else if matches_re type_check_re line then
    "[type check]"
  else if matches_re server_ready_re line then
    "[server is ready]"
  else
    "[processing]"

let delta_t : float = 3.0

let open_and_get_tail_msg start_time tail_env =
  let curr_time = Unix.time () in
  if not (Tail.is_open_env tail_env) &&
       curr_time -. start_time > delta_t then begin
      Tail.open_env tail_env;
      Tail.update_env is_valid_line tail_env;
    end else if Tail.is_open_env tail_env then
    Tail.update_env is_valid_line tail_env;
  let load_state_not_found =
    is_load_state_not_found (Tail.get_lines tail_env) in
  Tail.set_lines tail_env [];
  let tail_msg = msg_of_tail tail_env in
  load_state_not_found, tail_msg

let print_wait_msg progress_callback start_time tail_env =
  let load_state_not_found, tail_msg =
    open_and_get_tail_msg start_time tail_env in
  if load_state_not_found then
    Printf.eprintf "%s\n%!" ClientMessages.load_state_not_found_msg;
  progress_callback (Some tail_msg)

(** Sleeps until the server says hello. While waiting, prints out spinner and
 * useful messages by tailing the server logs. *)
let rec wait_for_server_hello
  ~ic
  ~retries
  ~progress_callback
  ~start_time
  ~tail_env =
  match retries with
  | Some n when n < 0 ->
      (if Option.is_some tail_env then
        Printf.eprintf "\nError: Ran out of retries, giving up!\n");
      raise Exit_status.(Exit_with Out_of_retries)
  | Some _
  | None -> ();
  let readable, _, _  = Unix.select
    [Timeout.descr_of_in_channel ic] [] [Timeout.descr_of_in_channel ic] 1.0 in
  if readable = [] then (
    Option.iter tail_env
      (fun t -> print_wait_msg progress_callback start_time t);
    wait_for_server_hello
      ~ic
      ~retries:(Option.map retries (fun x -> x - 1))
      ~progress_callback
      ~start_time
      ~tail_env
  ) else
    try
      let fd = Timeout.descr_of_in_channel ic in
      let msg = Marshal_tools.from_fd_with_preamble fd in
      (match msg with
      | ServerCommandTypes.Hello ->
        ()
      | _ ->
        Option.iter tail_env
          (fun t -> print_wait_msg progress_callback start_time t);
        wait_for_server_hello ic (Option.map retries (fun x -> x - 1))
          progress_callback start_time tail_env
      )
    with
    | End_of_file
    | Sys_error _ ->
      raise Server_hung_up

let rec connect ?(first_attempt=false) env retries start_time tail_env =
  match retries with
  | Some n when n < 0 ->
      Printf.eprintf "\nError: Ran out of retries, giving up!\n";
      raise Exit_status.(Exit_with Out_of_retries)
  | Some _
  | None -> ();
  let has_timed_out = match env.expiry with
    | None -> false
    | Some t -> Unix.time() > t
  in
  if has_timed_out then begin
    Printf.eprintf "\nError: hh_client hit timeout, giving up!\n%!";
    raise Exit_status.(Exit_with Out_of_time)
  end;
  let connect_once_start_t = Unix.time () in

  let server_name = HhServerMonitorConfig.Program.hh_server in
  let handoff_options = {
    MonitorRpc.server_name = server_name;
    force_dormant_start = env.force_dormant_start;
  } in
  let retries, conn =
    let start_t = Unix.gettimeofday () in
    let timeout = (Option.value retries ~default:30) + 1 in
    let conn = ServerUtils.connect_to_monitor
      ~timeout
      env.root
      handoff_options in
    let elapsed_t = int_of_float (Unix.gettimeofday () -. start_t) in
    let retries = Option.map retries
      ~f:(fun retries -> max 0 (retries - elapsed_t)) in
    retries, conn
  in
  HackEventLogger.client_connect_once connect_once_start_t;
  match conn with
  | Result.Ok (ic, oc) ->
      if env.do_post_handoff_handshake then begin
        try
          wait_for_server_hello ic retries env.progress_callback start_time
            (Some tail_env);
          env.progress_callback None
        with
        | Server_hung_up ->
          (Printf.eprintf "hh_server died unexpectedly. Maybe you recently \
          launched a different version of hh_server. Now exiting hh_client.";
          raise Exit_status.(Exit_with No_server_running))
      end;
      (ic, oc)
  | Result.Error e ->
    if first_attempt then
      Printf.eprintf
        "For more detailed logs, try `tail -f $(hh_client --monitor-logname) \
        $(hh_client --logname)`\n";
    match e with
    | SMUtils.Server_died
    | SMUtils.Monitor_connection_failure ->
      connect env (Option.map retries (fun x -> x - 1)) start_time tail_env
    | SMUtils.Server_missing ->
      if env.autostart then begin
        ClientStart.start_server { ClientStart.
          root = env.root;
          no_load = env.no_load;
          profile_log = env.profile_log;
          silent = false;
          exit_on_failure = true;
          ai_mode = env.ai_mode;
          debug_port = None;
        };
        connect env retries start_time tail_env
      end else begin
        Printf.eprintf begin
          "Error: no hh_server running. Either start hh_server"^^
          " yourself or run hh_client without --autostart-server false\n%!"
        end;
        raise Exit_status.(Exit_with No_server_running)
      end
    | SMUtils.Server_dormant ->
      Printf.eprintf begin
        "Error: No server running and connection limit reached for waiting"^^
        " on next server to be started. Please wait patiently. If you really"^^
        " know what you're doing, maybe try --force-dormant-start\n%!"
      end;
      raise Exit_status.(Exit_with No_server_running)
    | SMUtils.Monitor_socket_not_ready ->
      let _, tail_msg = open_and_get_tail_msg start_time tail_env in
      env.progress_callback (Some tail_msg);
      HackEventLogger.client_connect_once_busy start_time;
      connect env (Option.map retries pred) start_time tail_env
    | SMUtils.Monitor_establish_connection_timeout ->
      (** This should only happen if the Monitor is being DDOSed or has
       * wedged itself. To ameliorate inadvertent self DDOSing by hh_clients,
       * we don't auto-retry a connection when the Monitor is busy .*)
      HackEventLogger.client_connect_once_busy start_time;
      Printf.eprintf "\nError: Monitor is busy. Giving up!\n%!";
      raise Exit_status.(Exit_with Monitor_connection_failure)
    | SMUtils.Build_id_mismatched mismatch_info_opt ->
      let open ServerMonitorUtils in
      Printf.eprintf
        "hh_server's version doesn't match the client's, so it will exit.\n";
      begin match mismatch_info_opt with
        | None -> ()
        | Some mismatch_info ->
          let secs = int_of_float
            ((Unix.gettimeofday ()) -. mismatch_info.existing_launch_time) in
          let time =
            if secs > 86400 then Printf.sprintf "%n days" (secs / 86400)
            else if secs > 3600 then Printf.sprintf "%n hours" (secs / 3600)
            else if secs > 60 then Printf.sprintf "%n minutes" (secs / 60)
            else Printf.sprintf "%n seconds" (secs) in
          Printf.eprintf
            "  hh_server '%s' was launched %s ago.\n%!"
            (String.concat " " mismatch_info.existing_argv)
            time;
          ()
      end;
      if env.autostart
      then
        let start_time = Unix.time () in
        begin
          Printf.eprintf "Going to launch a new one.\n%!";
          (* Don't decrement retries -- the server is definitely not running,
           * so the next time round will hit Server_missing above, *but*
           * before that will actually start the server -- we need to make
           * sure that happens.
           *)
          Tail.close_env tail_env;
          connect env retries start_time tail_env
        end else raise Exit_status.(Exit_with Exit_status.Build_id_mismatch)

let connect env =
  let link_file = ServerFiles.log_link env.root in
  let log_file = Sys_utils.readlink_no_fail link_file in
  let start_time = Unix.time () in
  let tail_env = Tail.create_env log_file in
  try
    let (ic, oc) =
      connect ~first_attempt:true env env.retries start_time tail_env in
    Tail.close_env tail_env;
    HackEventLogger.client_established_connection start_time;
    if env.do_post_handoff_handshake then begin
      ServerCommand.send_connection_type oc ServerCommandTypes.Non_persistent;
    end;
    (ic, oc)
  with
  | e ->
    HackEventLogger.client_establish_connection_exception e;
    raise e
