(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

type env = {
  (** Number of times to retry establishing a connection to the server. *)
  retries : int;
  root : Path.t;
  wait : bool;
  (** Force the monitor to start a server if one isn't running. *)
  force_dormant_start : bool;
  build_opts : ServerBuild.build_opts;
}

let handle_response env ic =
  let finished = ref false in
  let exit_code = ref Exit_status.No_error in
  HackEventLogger.client_build_begin_work
    (ServerBuild.build_type_of env.build_opts)
    env.build_opts.ServerBuild.id;
  try
    while true do
      let line:ServerBuild.build_progress = Timeout.input_value ic in
      match line with
      | ServerBuild.BUILD_PROGRESS s -> print_endline s
      | ServerBuild.BUILD_ERROR s ->
          exit_code := Exit_status.Build_error; print_endline s
      | ServerBuild.BUILD_FINISHED -> finished := true
    done;
    Exit_status.No_error
  with
  | End_of_file ->
    if not !finished then begin
      Printf.fprintf stderr ("Build unexpectedly terminated! "^^
        "You may need to do `hh_client restart`.\n");
      Exit_status.Build_terminated
    end else !exit_code
  | Failure _ as e ->
    (* We are seeing Failure "input value: bad object" which can
     * realistically only happen from Marshal.from_channel ic.
     * This admittedly won't help us root cause this, but at least
     * this will help us identify where it is occurring
     *)
    let backtrace = Printexc.get_backtrace () in
    let e_str = Printexc.to_string e in
    Printf.fprintf stderr "Unexpected error: %s\n%s%!" e_str backtrace;
    raise e

let main_exn env =
  let build_type = ServerBuild.build_type_of env.build_opts in
  let request_id = env.build_opts.ServerBuild.id in
  HackEventLogger.client_build build_type request_id;
  let ic, oc = ClientConnect.connect { ClientConnect.
    root = env.root;
    autostart = true;
    (** When running Hack Build, we want to force the monitor to start
     * a Hack server if one isn't running. This is for the case where
     * Hack was not running, a 3-way merge occurs triggering Mercurial's
     * merge driver, the merge driver calls Hack build. During Hack startup,
     * it won't start a server because it is waiting for repo settling (for
     * the update/rebase to complete); but since need Hack to finish the
     * update/rebase, we need to force it to be started. *)
    force_dormant_start = env.force_dormant_start;
    retries = if env.wait then None else Some env.retries;
    retry_if_init = true;
    expiry = None;
    no_load = false;
    profile_log = false;
    ai_mode = None;
    progress_callback = ClientConnect.tty_progress_reporter;
    do_post_handoff_handshake = true;
  } in
  let old_svnrev = Option.try_with begin fun () ->
    Sys_utils.read_file ServerBuild.svnrev_path
  end in
  let exit_status = with_context
    ~enter:(fun () -> ())
    ~exit:(fun () ->
      Printf.eprintf "\nHack build id: %s\n%!" request_id)
    ~do_:(fun () ->
      ServerCommand.stream_request oc (ServerCommandTypes.BUILD env.build_opts);
      handle_response env ic) in
  let svnrev = Option.try_with begin fun () ->
    Sys_utils.read_file ServerBuild.svnrev_path
  end in
  HackEventLogger.client_build_finish
    ~rev_changed:(svnrev <> old_svnrev) ~build_type ~request_id ~exit_status;
  exit_status

let main env =
  try main_exn env with
  | Exit_status.Exit_with Exit_status.No_server_running ->
    Printf.eprintf "Retrying build...\n";
    main_exn env
