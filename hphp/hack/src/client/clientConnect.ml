(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
module SMUtils = ServerMonitorUtils

exception Server_hung_up of ServerCommandTypes.finale_data option

type env = {
  root : Path.t;
  from : string;
  autostart : bool;
  force_dormant_start : bool;
  retries : int option;
  expiry : float option;
  no_load : bool;
  watchman_debug_logging : bool;
  log_inference_constraints : bool;
  profile_log : bool;
  ai_mode : string option;
  progress_callback: string option -> unit;
  do_post_handoff_handshake: bool;
  ignore_hh_version : bool;
  saved_state_ignore_hhconfig : bool;
  use_priority_pipe : bool;
  prechecked : bool option;
  config : (string * string) list;
}

type conn = {
  channels : Timeout.in_channel * Out_channel.t;
  server_finale_file : string;
  conn_retries : int option;
  conn_progress_callback: string option -> unit;
  conn_start_time: float;
  conn_root: Path.t;
}

let get_finale_data (server_finale_file: string) : ServerCommandTypes.finale_data option =
  try
    let ic = Pervasives.open_in_bin server_finale_file in
    let contents : ServerCommandTypes.finale_data = Marshal.from_channel ic in
    Pervasives.close_in ic;
    Some contents
  with _ ->
    None


let tty_progress_reporter () =
  let angery_reaccs_only =
    Tty.supports_emoji () && ClientMessages.angery_reaccs_only () in
  fun (status: string option) : unit ->
    if Tty.spinner_used () then Tty.print_clear_line stderr;
    match status with
    | None -> ()
    | Some s -> Tty.eprintf "hh_server is busy: %s %s%!" s
                  (Tty.spinner ~angery_reaccs_only ())

let null_progress_reporter (_status: string option) : unit =
  ()

(* what is the server doing? or None if nothing *)
let progress : string option ref = ref None

(* if the server has something not going right, what? *)
let progress_warning : string option ref = ref None

let default_progress_message = "processing"

let check_progress (root: Path.t) : unit =
  match ServerUtils.server_progress ~timeout:3 root with
  | Ok (msg, warning) ->
    progress := msg;
    progress_warning := warning;
  | _ -> ()

let delta_t : float = 3.0

let print_wait_msg (progress_callback: string option -> unit) (root: Path.t) : unit =
  let had_warning = Option.is_some !progress_warning in
  check_progress root;
  if not had_warning then
    Option.iter !progress_warning ~f:(Printf.eprintf "%s\n%!");
  let progress = Option.value !progress ~default:default_progress_message in
  let final_suffix = if Option.is_some !progress_warning then
      " - this can take a long time, see warning above]" else "]" in
  progress_callback (Some ("[" ^ progress ^ final_suffix))

(** Sleeps until the server says hello. While waiting, prints out spinner and
 * useful messages by tailing the server logs. *)
let rec wait_for_server_message
    ~(expected_message : 'a ServerCommandTypes.message_type option)
    ~(ic : Timeout.in_channel)
    ~(server_finale_file : string)
    ~(retries : int option)
    ~(progress_callback : string option -> unit)
    ~(start_time : float)
    ~(root: Path.t)
  : 'a ServerCommandTypes.message_type Lwt.t =
  let elapsed_t = int_of_float (Unix.time () -. start_time) in
  match retries with
  | Some n when elapsed_t > n ->
    Printf.eprintf "\nError: Ran out of retries, giving up!\n";
    raise Exit_status.(Exit_with Out_of_retries)
  | Some _
  | None -> ();
    let%lwt readable, _, _ = Lwt_utils.select
        [Timeout.descr_of_in_channel ic] [] [Timeout.descr_of_in_channel ic] 1.0 in
    if readable = [] then (
      print_wait_msg progress_callback root;
      wait_for_server_message
        ~expected_message
        ~ic
        ~server_finale_file
        ~retries
        ~progress_callback
        ~start_time
        ~root
    ) else
      try%lwt
        let fd = Timeout.descr_of_in_channel ic in
        let msg : 'a ServerCommandTypes.message_type =
          Marshal_tools.from_fd_with_preamble fd in
        let is_ping = (msg = ServerCommandTypes.Ping) in
        if (not is_ping) &&
           (Option.is_none expected_message || Some msg = expected_message) then
          begin
            progress_callback None;
            Lwt.return msg
          end else begin
          if not is_ping then print_wait_msg progress_callback root;
          wait_for_server_message ~expected_message ~ic ~server_finale_file ~retries
            ~progress_callback ~start_time ~root
        end
      with
      | End_of_file
      | Sys_error _ ->
        let finale_data = get_finale_data server_finale_file in
        progress_callback None;
        raise (Server_hung_up finale_data)

let wait_for_server_hello
    (ic : Timeout.in_channel)
    (server_finale_file : string)
    (retries : int option)
    (progress_callback : string option -> unit)
    (start_time : float)
    (root: Path.t)
  : unit Lwt.t =
  let%lwt _ : 'a ServerCommandTypes.message_type =
    wait_for_server_message
      ~expected_message:(Some ServerCommandTypes.Hello)
      ~ic
      ~server_finale_file
      ~retries
      ~progress_callback
      ~start_time
      ~root
  in
  Lwt.return_unit

let with_server_hung_up (f : unit -> 'a Lwt.t) : 'a Lwt.t =
  try%lwt f () with
  | Server_hung_up (Some finale_data) ->
    let open ServerCommandTypes in
    Printf.eprintf "Hack server disconnected suddenly [%s]\n   %s\n"
      (Exit_status.to_string finale_data.exit_status)
      finale_data.msg;
    if finale_data.exit_status = Exit_status.Failed_to_load_should_abort then
      raise Exit_status.(Exit_with Server_hung_up_should_abort)
    else
      raise Exit_status.(Exit_with Server_hung_up_should_retry)
  | Server_hung_up None ->
    Printf.eprintf
      ("Hack server disconnected suddenly. Most likely a new one" ^^
       " is being initialized with a better saved state after a large rebase/update.\n");
    raise Exit_status.(Exit_with Server_hung_up_should_retry)

let rec connect
    ?(first_attempt=false)
    (env : env)
    (retries : int option)
    (start_time : float)
  : conn Lwt.t =
  let elapsed_t = int_of_float (Unix.time () -. start_time) in
  match retries with
  | Some n when elapsed_t > n ->
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

    let handoff_options = {
      MonitorRpc.force_dormant_start = env.force_dormant_start;
      pipe_name = HhServerMonitorConfig.pipe_type_to_string
          (if env.force_dormant_start then HhServerMonitorConfig.Force_dormant_start_only
           else if env.use_priority_pipe then HhServerMonitorConfig.Priority
           else HhServerMonitorConfig.Default)
    } in
    let retries, conn =
      let start_t = Unix.gettimeofday () in
      let timeout = (Option.value retries ~default:30) + 1 in
      let conn = ServerUtils.connect_to_monitor
          ~timeout
          env.root
          handoff_options
      in
      let elapsed_t = int_of_float (Unix.gettimeofday () -. start_t) in
      let retries = Option.map retries
          ~f:(fun retries -> max 0 (retries - elapsed_t)) in
      retries, conn
    in
    HackEventLogger.client_connect_once connect_once_start_t;
    match conn with
    | Ok (ic, oc, server_finale_file) ->
      let%lwt () =
        if env.do_post_handoff_handshake then
          with_server_hung_up (fun () -> wait_for_server_hello
            ic server_finale_file retries env.progress_callback start_time env.root)
        else
          Lwt.return_unit
      in
      Lwt.return {
        channels = (ic, oc);
        server_finale_file;
        conn_retries = retries;
        conn_progress_callback = env.progress_callback;
        conn_start_time = start_time;
        conn_root = env.root;
      }
    | Error e ->
      if first_attempt then
        Printf.eprintf
          "For more detailed logs, try `tail -f $(hh_client --monitor-logname) \
           $(hh_client --logname)`\n";
      match e with
      | SMUtils.Server_died
      | SMUtils.Monitor_connection_failure ->
        Unix.sleepf 0.1;
        connect env retries start_time
      | SMUtils.Server_missing ->
        if env.autostart then begin
          ClientStart.start_server { ClientStart.
                                     root = env.root;
                                     from = env.from;
                                     no_load = env.no_load;
                                     watchman_debug_logging = env.watchman_debug_logging;
                                     log_inference_constraints = env.log_inference_constraints;
                                     profile_log = env.profile_log;
                                     silent = false;
                                     exit_on_failure = false;
                                     ai_mode = env.ai_mode;
                                     debug_port = None;
                                     ignore_hh_version = env.ignore_hh_version;
                                     saved_state_ignore_hhconfig = env.saved_state_ignore_hhconfig;
                                     dynamic_view = false;
                                     prechecked = env.prechecked;
                                     config = env.config;
                                   };
          connect env retries start_time
        end else begin
          Printf.eprintf begin
            "Error: no hh_server running. Either start hh_server"^^
            " yourself or run hh_client without --autostart-server false\n%!"
          end;
          raise Exit_status.(Exit_with No_server_running_should_retry)
        end
      | SMUtils.Server_dormant_out_of_retries ->
        Printf.eprintf begin
          "Ran out of retries while waiting for Mercurial to finish rebase. Starting " ^^
          "the server in the middle of rebase is strongly not recommended and you should " ^^
          "first finish the rebase before retrying. If you really " ^^
          "know what you're doing, maybe try --force-dormant-start\n%!"
        end;
        raise Exit_status.(Exit_with Out_of_retries)
      | SMUtils.Server_dormant ->
        Printf.eprintf begin
          "Error: No server running and connection limit reached for waiting"^^
          " on next server to be started. Please wait patiently. If you really"^^
          " know what you're doing, maybe try --force-dormant-start\n%!"
        end;
        raise Exit_status.(Exit_with No_server_running_should_retry)
      | SMUtils.Monitor_socket_not_ready ->
        HackEventLogger.client_connect_once_busy start_time;
        Unix.sleepf 0.1;
        connect env retries start_time
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
              "  hh_server '%s' was launched %s ago;\n  hh_client '%s' launched now.\n%!"
              (String.concat ~sep:" " mismatch_info.existing_argv)
              time
              (String.concat ~sep:" " (Array.to_list Sys.argv));
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
            connect env retries start_time
          end else raise Exit_status.(Exit_with Exit_status.Build_id_mismatch)

let connect (env : env) : conn Lwt.t =
  let start_time = Unix.time () in
  try%lwt
    let%lwt {channels = (_, oc); _} as conn =
      connect ~first_attempt:true env env.retries start_time in
    HackEventLogger.client_established_connection start_time;
    if env.do_post_handoff_handshake then begin
      ServerCommandLwt.send_connection_type oc ServerCommandTypes.Non_persistent;
    end;
    Lwt.return conn
  with
  | e ->
    (* we'll log this exception, then re-raise the exception, but using the *)
    (* original backtrace of "e" rather than generating a new backtrace.    *)
    let backtrace = Caml.Printexc.get_raw_backtrace () in
    HackEventLogger.client_establish_connection_exception e;
    Caml.Printexc.raise_with_backtrace e backtrace

let rpc : type a. conn -> a ServerCommandTypes.t -> a Lwt.t
  = fun {
    channels = (ic, oc);
    server_finale_file;
    conn_retries = retries;
    conn_progress_callback = progress_callback;
    conn_start_time = start_time;
    conn_root;
  } cmd ->
    Marshal.to_channel oc (ServerCommandTypes.Rpc cmd) [];
    Out_channel.flush oc;
    with_server_hung_up @@ fun () ->
    let%lwt res = wait_for_server_message
        ~expected_message:None
        ~ic
        ~server_finale_file
        ~retries
        ~progress_callback
        ~start_time
        ~root:conn_root
    in
    match res with
    | ServerCommandTypes.Response (response, _) -> Lwt.return response
    | ServerCommandTypes.Push _ -> failwith "unexpected 'push' RPC response"
    | ServerCommandTypes.Hello -> failwith "unexpected 'hello' RPC response"
    | ServerCommandTypes.Ping -> failwith "unexpected 'ping' RPC response"

let rpc_with_retry
    (conn_f : unit -> conn Lwt.t)
    (cmd : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t)
  : 'a Lwt.t =
  ServerCommandTypes.Done_or_retry.call ~f:(fun () ->
      let%lwt conn = conn_f () in
      let%lwt result = rpc conn cmd in
      Lwt.return result
    )
