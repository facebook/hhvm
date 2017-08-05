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

let get_hhserver () =
  let exe_name =
    if Sys.win32 then "hh_server.exe" else "hh_server" in
  let server_next_to_client =
    Path.(to_string @@ concat (dirname executable_name) exe_name) in
  if Sys.file_exists server_next_to_client
  then server_next_to_client
  else exe_name

type env = {
  root: Path.t;
  no_load : bool;
  profile_log : bool;
  silent : bool;
  exit_on_failure : bool;
  ai_mode : string option;
  debug_port: Unix.file_descr option;
}

let start_server env =
  (* Create a pipe for synchronization with the server: we will wait
     until the server finishes its initialisation phase. *)
  let in_fd, out_fd = Unix.pipe () in
  Unix.set_close_on_exec in_fd;
  let ic = Unix.in_channel_of_descr in_fd in

  let ai_options =
    match env.ai_mode with
    | Some ai -> [| "--ai"; ai |]
    | None -> [||] in
  let hh_server = get_hhserver () in
  let hh_server_args =
    Array.concat [
      [|hh_server; "-d"; Path.to_string env.root|];
      if env.no_load then [| "--no-load" |] else [||];
      if env.profile_log then [| "--profile-log" |] else [||];
      ai_options;
      (** If the client starts up a server monitor process, the output of that
       * bootup is passed to this FD - so this FD needs to be threaded
       * through the server monitor process then to the typechecker process.
       *
       * Note: Yes, the FD is available in the monitor process as well, but
       * it doesn't, and shouldn't, use it. *)
      [| "--waiting-client"; string_of_int (Handle.get_handle out_fd) |];
      match env.debug_port with
        | None -> [| |]
        | Some fd ->
          [| "--debug-client"; string_of_int @@ Handle.get_handle fd |]
    ] in
  if not env.silent then
    Printf.eprintf "Server launched with the following command:\n\t%s\n%!"
      (String.concat " "
         (Array.to_list (Array.map Filename.quote hh_server_args)));

  let stdin, stdout, stderr = if env.silent then
    let nfd = Unix.openfile Sys_utils.null_path [Unix.O_RDWR] 0 in nfd, nfd, nfd
  else
    Unix.(stdin, stdout, stderr) in

  try
    let server_pid =
      Unix.create_process hh_server hh_server_args stdin stdout stderr in
    Unix.close out_fd;

    match Unix.waitpid [] server_pid with
    | _, Unix.WEXITED 0 ->
      assert (input_line ic = ServerMonitorUtils.ready);
      close_in ic
    | _, Unix.WEXITED i ->
      if not env.silent then Printf.eprintf
          "Starting hh_server failed. Exited with status code: %d!\n" i;
      if env.exit_on_failure then exit 77
    | _ ->
      if not env.silent then Printf.eprintf "Could not start hh_server!\n";
      if env.exit_on_failure then exit 77
  with _ ->
    if not env.silent then Printf.eprintf "Could not start hh_server!\n";
    if env.exit_on_failure then exit 77


let should_start env =
  let root_s = Path.to_string env.root in
  let handoff_options = {
    MonitorRpc.server_name = HhServerMonitorConfig.Program.hh_server;
    force_dormant_start = false;
  } in
  match ServerUtils.connect_to_monitor
    ~timeout:3
    env.root handoff_options with
  | Result.Ok _conn -> false
  | Result.Error
      ( SMUtils.Server_missing
      | SMUtils.Build_id_mismatched _
      | SMUtils.Server_died
      ) -> true
  | Result.Error SMUtils.Server_dormant ->
    Printf.eprintf
      "Server already exists but is dormant";
    false
  | Result.Error SMUtils.Monitor_socket_not_ready
  | Result.Error SMUtils.Monitor_establish_connection_timeout
  | Result.Error SMUtils.Monitor_connection_failure ->
    Printf.eprintf "Replacing unresponsive server for %s\n%!" root_s;
    ClientStop.kill_server env.root;
    true

let main env =
  HackEventLogger.client_start ();
  (* TODO(ljw): There are some race conditions here. First scenario: two      *)
  (* processes simultaneously do 'hh start' while the server isn't running.   *)
  (* Both their calls to should_start will see the lockfile absent and        *)
  (* immediately get back 'Server_missing'. So both of them launch hh_server. *)
  (* One hh_server process will be slightly faster and will create a lockfile *)
  (* and shortly start listening on the socket. The other will see that the   *)
  (* lockfile already exists and so terminate with error code 0 immediately   *)
  (* without sending the "ready" message. And the second start_server call    *)
  (* will fail its assert that a "ready" message should come.                 *)
  (*                                                                          *)
  (* Second scenario: one process does 'hh start', and creates the lockfile,  *)
  (* and after a short delay will start listening on its socket. Another      *)
  (* process does 'hh start', sees the lockfile is present, tries to connect, *)
  (* but we're still in above short delay and so it gets ECONNREFUSED         *)
  (* immediately, which (given the presence of a lockfile) is returned as     *)
  (* Monitor_not_ready. The should_start routine deems this an unresponsive   *)
  (* server and so kills it and launches a new server again. The first        *)
  (* process might or might not report failure, depending on how far it got   *)
  (* with its 'hh start' -- i.e. whether or not it yet observed the "ready".  *)
  (*                                                                          *)
  (* Third and most typical scenario: an LSP client like Nuclide is running,  *)
  (* and the user does 'hh restart' which kills the server and then calls     *)
  (* start_server. As soon as LSP sees the server killed, it too immediately  *)
  (* calls ClientStart.main. Maybe it will see the lockfile absent, and so    *)
  (* proceed according to the first race scenario above. Maybe it will see    *)
  (* the lockfile present and proceed according to the second race scenario   *)
  (* above. In both cases the LSP client will see problem reports.            *)
  (*                                                                          *)
  (* The root problem is that should_start assumes an invariant that "if      *)
  (* hh_server monitor is busy then it has failed and should be shut down."   *)
  (* This invariant isn't true. Note that the reason we call should_start,    *)
  (* rather than just using the default ClientConnect.autorestart which calls *)
  (* into ClientStart.start_server, is because we do like its ability to kill *)
  (* an unresponsive server. So the should_start function should simply give  *)
  (* a grace period in case of a temporarily busy server.                     *)
  (*                                                                          *)
  (* I believe there's a second similar problem inside ClientConnect.connect  *)
  (* when its env.autorestart is true. During the short delay window it might *)
  (* immediately get Server_missing similar to the first scenario, and so     *)
  (* ClientStart.start_server, which will fail in the same way. Or it might   *)
  (* immediately get ECONNREFUSED during the delay window, so it will retry   *)
  (* connection attempt immediately, and it might burn through all of its     *)
  (* available retries instantly. That's because ECONNREFUSED is immediate.   *)
  if should_start env
  then begin
    start_server env;
    Exit_status.No_error
  end else begin
    if not env.silent then Printf.eprintf
      "Error: Server already exists for %s\n\
      Use hh_client restart if you want to kill it and start a new one\n%!"
      (Path.to_string env.root);
    Exit_status.Server_already_exists
  end
