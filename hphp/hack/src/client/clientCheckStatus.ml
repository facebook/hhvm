(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes

let print_error_raw e = Printf.printf "%s" (Raw_error_formatter.to_string e)

let print_error_plain e = Printf.printf "%s" (Errors.to_string e)

let print_error_contextual e =
  Printf.printf "%s" (Contextual_error_formatter.to_string e)

let print_error_highlighted e =
  Printf.printf "%s" (Highlighted_error_formatter.to_string e)

let print_error ~(error_format : Errors.format) (e : Errors.finalized_error) :
    unit =
  match error_format with
  | Errors.Raw -> print_error_raw e
  | Errors.Plain -> print_error_plain e
  | Errors.Context -> print_error_contextual e
  | Errors.Highlighted -> print_error_highlighted e

let is_stale_msg liveness =
  match liveness with
  | Stale_status ->
    Some
      ("(but this may be stale, probably due to"
      ^ " watchman being unresponsive)\n")
  | Live_status -> None

let go status output_json from error_format max_errors =
  let { Server_status.liveness; error_list; dropped_count; last_recheck_stats }
      =
    status
  in
  let stale_msg = is_stale_msg liveness in
  if
    output_json
    || (not (String.equal from "" || String.equal from "[sh]"))
    || List.is_empty error_list
  then
    ServerError.print_error_list
      stdout
      ~stale_msg
      ~output_json
      ~error_list
      ~save_state_result:None
      ~recheck_stats:last_recheck_stats
  else begin
    List.iter error_list ~f:(print_error ~error_format);
    Option.iter
      (Errors.format_summary
         error_format
         ~displayed_count:(List.length error_list)
         ~dropped_count:(Some dropped_count)
         ~max_errors)
      ~f:(fun msg -> Printf.printf "%s" msg);
    (* [stale_msg] ultimately comes from [ServerMain.query_notifier], and says whether the check
       reflects data from a watchman sync, or just whatever has arrived so far over the watchman
       subscription. *)
    Option.iter stale_msg ~f:(fun msg -> Printf.printf "%s" msg)
  end;
  if List.is_empty error_list then
    Exit_status.No_error
  else
    Exit_status.Type_error

(** This function produces streaming errors: it reads the errors.bin opened
as [fd], displays errors as they come using the [args.error_format] over stdout, and
displays a progress-spinner over stderr if [args.show_spinner]. It keeps "tailing"
[fd] until eventually the producing process writes an end-sentinel in it (signalling
that the typecheck has been completed or restarted or aborted), or until the producing
process terminates. *)
let go_streaming_on_fd
    ~(pid : int)
    (fd : Unix.file_descr)
    (args : ClientEnv.client_check_env)
    ~(partial_telemetry_ref : Telemetry.t option ref)
    ~(progress_callback : string option -> unit) :
    (Exit_status.t * Telemetry.t) Lwt.t =
  let ClientEnv.{ max_errors; error_format; _ } = args in
  let errors_stream = Server_progress_lwt.watch_errors_file ~pid fd in
  let start_time = Unix.gettimeofday () in
  let first_error_time = ref None in
  let latest_progress = ref None in
  let errors_file_telemetry = ref (Telemetry.create ()) in

  (* This constructs a telemetry that we'll use for both partial-progress
     and completed events. *)
  let telemetry_so_far displayed_count =
    !errors_file_telemetry
    |> Telemetry.bool_ ~key:"streaming" ~value:true
    |> Telemetry.int_opt ~key:"max_errors" ~value:max_errors
    |> Telemetry.float_opt
         ~key:"first_error_time"
         ~value:(Option.map !first_error_time ~f:(fun t -> t -. start_time))
    |> Telemetry.int_ ~key:"displayed_count" ~value:displayed_count
  in

  (* If the user does Ctrl+C, it generates SIGINT and we can't run any code in response.
     But we'd still like to report telemetry in that case. So we pre-emptively store
     telemetry in [partial_telemetry_ref] which the SIGINT handler will be able to read:
     if it finds [Some], then the SIGINT handler will deem the operation a success
     and will log CLIENT_CHECK_PARTIAL instead of CLIENT_CHECK_BAD_EXIT. *)
  let update_partial_telemetry displayed_count =
    if displayed_count > 0 then
      partial_telemetry_ref := Some (telemetry_so_far displayed_count)
  in

  (* this lwt process reads the progress.json file once a second and displays a spinner;
     it never terminates *)
  let rec show_progress () : 'a Lwt.t =
    let message =
      try (Server_progress.read ()).Server_progress.message with
      | _ -> "unknown"
    in
    latest_progress := Some message;
    progress_callback !latest_progress;
    let%lwt () = Lwt_unix.sleep 1.0 in
    show_progress ()
  in

  (* this lwt process consumes errors from the errors.bin file by polling
     every 0.2s, and displays them. It terminates once the errors.bin file has an "end"
     sentinel written to it, or the process that was writing errors.bin terminates. *)
  let rec consume (displayed_count : int) :
      (int * Server_progress.errors_file_error * string) Lwt.t =
    let%lwt errors = Lwt_stream.get errors_stream in
    match errors with
    | None ->
      (* The contract of [errors_stream] (from [Server_progress_lwt.watch_errors_file]) is
         that it will always have a single [Some error] item as its last item, before the
         stream is closed and hence subsequent [Lwt_stream.get] return [None].
         If we're here, it means that contract has been violated. *)
      failwith "Expected end_sentinel before end of stream"
    | Some (Error (end_sentinel, log_message)) ->
      Lwt.return (displayed_count, end_sentinel, log_message)
    | Some (Ok (Server_progress.Telemetry telemetry_item)) ->
      errors_file_telemetry :=
        Telemetry.merge !errors_file_telemetry telemetry_item;
      update_partial_telemetry displayed_count;
      consume displayed_count
    | Some (Ok (Server_progress.Errors { errors; timestamp = _ })) ->
      first_error_time :=
        Option.first_some !first_error_time (Some (Unix.gettimeofday ()));
      (* We'll clear the spinner, print errs to stdout, flush stdout, and restore the spinner *)
      progress_callback None;
      let found_new = ref 0 in
      begin
        try
          Relative_path.Map.iter errors ~f:(fun _path errors_in_file ->
              List.iter errors_in_file ~f:(fun error ->
                  incr found_new;
                  print_error ~error_format error));
          Printf.printf "%!"
        with
        | Sys_error msg when String.equal msg "Broken pipe" ->
          (* We catch this error very locally, as small as we can around [printf],
             since if we caught it more globally then we'd not know which pipe raised it --
             the pipe to stdout, or stderr, or the pipe used to connect to monitor/server. *)
          let backtrace = Stdlib.Printexc.get_raw_backtrace () in
          Stdlib.Printexc.raise_with_backtrace
            Exit_status.(Exit_with Client_broken_pipe)
            backtrace
      end;
      progress_callback !latest_progress;
      let displayed_count = displayed_count + !found_new in
      update_partial_telemetry displayed_count;
      if displayed_count >= Option.value max_errors ~default:Int.max_value then
        Lwt.return
          ( displayed_count,
            Server_progress.Complete (Telemetry.create ()),
            "max-errors" )
      else
        consume displayed_count
  in

  (* We will show progress indefinitely until "consume" finishes,
     at which Lwt.pick will cancel show_progress. *)
  let%lwt (displayed_count, end_sentinel, _log_message) =
    Lwt.pick [consume 0; show_progress ()]
  in
  (* Clear the spinner *)
  progress_callback None;

  let (exit_status, telemetry) =
    match end_sentinel with
    | Server_progress.Complete telemetry ->
      (* complete either because the server completed, or we truncated early *)
      Option.iter
        (Errors.format_summary
           error_format
           ~displayed_count
           ~dropped_count:None
           ~max_errors)
        ~f:(fun msg -> Printf.printf "%s" msg);
      let telemetry =
        Telemetry.merge (telemetry_so_far displayed_count) telemetry
      in
      if displayed_count = 0 then
        (Exit_status.No_error, telemetry)
      else
        (Exit_status.Type_error, telemetry)
    | Server_progress.Restarted { user_message; log_message } ->
      Hh_logger.log
        "Errors-file: on %s, read Restarted(%s,%s)"
        (Sys_utils.show_inode fd)
        user_message
        log_message;
      HackEventLogger.client_check_errors_file_restarted
        (user_message ^ "\n" ^ log_message);
      (* All errors, warnings, informationals go to stdout. *)
      let msg =
        Tty.apply_color (Tty.Bold Tty.Red) (user_message ^ "\nPlease re-run hh.")
      in
      Printf.printf "\n%s\n%!" msg;
      raise Exit_status.(Exit_with Exit_status.Typecheck_restarted)
    | _ ->
      Hh_logger.log
        "Errors-file: on %s, read %s"
        (Sys_utils.show_inode fd)
        (Server_progress.show_errors_file_error end_sentinel);
      Printf.printf
        "Hh_server has terminated. [%s]\n%!"
        (Server_progress.show_errors_file_error end_sentinel);
      raise Exit_status.(Exit_with Exit_status.Typecheck_abandoned)
  in

  Lwt.return (exit_status, telemetry)

(** Gets all files-changed since [clock] which match the standard hack
predicate [FilesToIgnore.watchman_server_expression_terms].
The returns are relative to the watchman root.
CARE! This is not the same as a Relative_path.t. For instance if root
contains a symlink root/a.php ~> /tmp/b.php and the symlink itself
changed, then watchman would report a change on "a.php" but Relative_path.t
by definition only refers to paths after symlinks have been resolved
which in this case would be (Relative_path.Tmp,"b.php").

The caller must explicitly pass in [~fail_on_new_instance:false]
and [~fail_during_state:false] so that the callsite is self-documenting
about what it will do during these two scenarios. (no other options
are currently supported.) *)
let watchman_get_raw_updates_since
    ~(root : Path.t)
    ~(clock : Watchman.clock)
    ~(fail_on_new_instance : bool)
    ~(fail_during_state : bool) : (string list, string) result Lwt.t =
  if fail_on_new_instance then
    failwith "Not yet implemented: fail_on_new_instance";
  if fail_during_state then failwith "Not yet implemented: fail_during_state";
  let query =
    Hh_json.(
      JSON_Array
        [
          JSON_String "query";
          JSON_String (Path.to_string root);
          JSON_Object
            [
              ("since", JSON_String clock);
              ("fields", JSON_Array [JSON_String "name"]);
              ( "expression",
                Hh_json_helpers.AdhocJsonHelpers.pred
                  "allof"
                  FilesToIgnore.watchman_server_expression_terms );
            ];
        ])
    |> Hh_json.json_to_string
  in
  Hh_logger.log "watchman query: %s" query;
  let%lwt result =
    Lwt_utils.exec_checked Exec_command.Watchman ~input:query [| "-j" |]
  in
  match result with
  | Error e ->
    let msg = Lwt_utils.Process_failure.to_string e in
    Hh_logger.log "watchman response: %s" msg;
    Lwt.return_error msg
  | Ok { Lwt_utils.Process_success.stdout; _ } -> begin
    let files =
      try
        let json = Hh_json.json_of_string stdout in
        Ok (json, Hh_json_helpers.Jget.string_array_exn (Some json) "files")
      with
      | exn -> Error (Exception.wrap exn)
    in
    match files with
    | Error e ->
      let msg = Exception.to_string e in
      Hh_logger.log "watchman parse failure: %s\nRESPONSE:%s\n" msg stdout;
      Lwt.return_error msg
    | Ok (json, files) ->
      let has_changed = ref false in
      let json_str =
        Hh_json.json_truncate ~max_array_elt_count:10 ~has_changed json
        |> Hh_json.json_to_string
      in
      let has_changed =
        if !has_changed then
          " [truncated]"
        else
          ""
      in
      Hh_logger.log "watchman response:%s %s" has_changed json_str;
      Lwt.return_ok files
  end

(** This helper will trying to open the errors.bin file.
There is a whole load of ceremony to do with what happens when you want to open errors.bin,
e.g. start the server if necessary, check for version mismatch, report failures to the user.
Concrete examples:
* There is no server running. "hh check" must start up a server, before it can stream errors from it.
* There is no server running, but our attempt to start the server failed, either
   synchronously (e.g. we had `--autostart-server false`)
   or asynchronously (e.g. saved-state failed to load).
   "hh check" must detect that no streaming errors are forthcoming.
* There is a server running, and it has already written errors.bin, but it is the wrong binary version.
   "hh check" must cause it to shut down, and a new server start up, before it can stream errors.
* There is an errors.bin from an existing server, and its typecheck started 20s ago, but moments
   before "hh check" a file was changed on disk. "hh check" must detect this,
   learn that it cannot use the existing errors.bin, and instead use the new errors.bin that will be forthcoming.

How this function fulfills those cases:
* It of course has to check whether files on disk have changed since this errors.bin was started.
  It does this with a synchronous watchman query,
  and displays "hh_server is busy [watchman sync]".
* If there is a fault like "errors.bin was started too long ago and files have changed
  in the meantime" which can be rectified by waiting and trying again on the assumption
  that a server is running and will pick up the changes, then this will wait and try
  again indefinitely. It will display "hh_server is busy [hh_server sync]".
  The parameter [already_checked_clock] indicates that we have already done a
  (costly) watchman query for this clock, and will not do another watchman query until
  we see a new errors.bin which started at a different (newer) clock.
* If there is a fault like "missing errors.bin" or "binary mismatch" which can
  be rectified by connecting to the monitor+server, this will do so at most once
  with the [connect_then_close] callback parameter, then resume looking for a
  suitable errors.bin again with "hh_server is busy [hh_server sync]".
  If it already tried once with no effect, then it will raise Exit_status.Exit_with.

Here's a really subtle example involving precise details of the errors.bin clock:
1. server starts a check at clock0, hence starts errors.bin as of clock0
2. modify file A
3. modify file B
4. user does "hh", sees that files have changed since clock0, so repeats [keep_trying_to_open]
5. server picks up change to file A and restarts errors.bin as of clockA
6. repeat of [keep_trying_to_open] must NOT use errors.bin since that would miss "B"

There are two implementations we could imagine for the repeat of [keep_trying_to_open]:
(A) on subsequent repeats, it could either keep waiting until an errors.bin comes about
which has a clock that's more recent than the start time of "hh"; or (B) it could keep
waiting until an errors.bin comes about where there are no changed files between
errors.bin's start time and "now". Both are correct; the former is more principled,
would result in more "files changed under your feet", but alas there doesn't exist
the ability in watchman to compare two clocks so we can't use it. Therefore we use (B).
How do we know that (B) will eventually terminate? Well if the user keeps modifying
files then it never will! But if the user stops, then hh_server is guaranteed to
do a final recheck, hence guaranteed to write an errors.bin with no files changed
after it, and hence [keep_trying_to_open] is guaranteed to eventually terminate.

The semantics of the [deadline] parameter are that this is how long we'll wait
until we open an errors.bin file (i.e. until the typecheck has started). We don't
expose a parameter to control how long the typecheck must last.

SPINNER! This is surprisingly delicate. If we leave our spinner up and then
the process exits, the user sees a dead spinner in the line above their bash prompt.
If we leave our spinner up and then print an error message to stderr, the user
sees a dead spinner above the error message (and perhaps even another spinner down
below if we continued work). In summary it's crucial to clear out the spinner at the right time:
* [connect_then_close] is assumed to clear the spinner in case it writes to stderr
* This function clears the spinner in case of exceptions
* But it leaves the spinner at "watchman sync" in case of success, in the expectation
that subsequent code will update the spinner. *)
let rec keep_trying_to_open
    ~(has_already_attempted_connect : bool)
    ~(connect_then_close : unit -> unit Lwt.t)
    ~(already_checked_clock : Watchman.clock option)
    ~(progress_callback : string option -> unit)
    ~(deadline : float option)
    ~(root : Path.t) : (int * Unix.file_descr) Lwt.t =
  Option.iter deadline ~f:(fun deadline ->
      if Float.(Unix.gettimeofday () > deadline) then begin
        progress_callback None;
        raise (Exit_status.Exit_with Exit_status.Out_of_time)
      end);
  progress_callback (Some "hh_server sync");
  let fd_opt =
    try
      Some
        (Unix.openfile
           (ServerFiles.errors_file_path root)
           [Unix.O_RDONLY; Unix.O_NONBLOCK]
           0)
    with
    | _ -> None
  in
  match fd_opt with
  | None ->
    (* If no existing file, then there must not yet be a server up and running.
       So, connect to the monitor and wait for server "hello" as proof that the server
       started okay. Our call might fail, e.g. in case of --autostart false or failure to
       load saved-state. In these cases, it will raise an ExitStatus exception which
       bubbles up to our caller. *)
    let%lwt () =
      if not has_already_attempted_connect then begin
        Hh_logger.log "Errors-file: absent, so connecting then trying again";
        let%lwt () = connect_then_close () in
        Lwt.return_unit
      end else begin
        (* Retry opening errors file every 0.1 second from now on. *)
        let%lwt () = Lwt_unix.sleep 0.1 in
        Lwt.return_unit
      end
    in
    keep_trying_to_open
      ~has_already_attempted_connect:true
      ~connect_then_close
      ~already_checked_clock
      ~progress_callback
      ~deadline
      ~root
  | Some fd -> begin
    match Server_progress.ErrorsRead.openfile fd with
    | Error (end_sentinel, log_message) when not has_already_attempted_connect
      ->
      Hh_logger.log
        "Errors-file: on %s, read sentinel %s [%s], so connecting then trying again"
        (Sys_utils.show_inode fd)
        (Server_progress.show_errors_file_error end_sentinel)
        log_message;
      (* If there was an existing file but it was bad -- e.g. came from a dead server,
         or binary mismatch, then we will connect as before. In the case of binary mismatch,
         we rely on the standard codepaths in clientConnect and monitorConnect: namely, it will
         either succeed in starting and connecting to a new server of the correct binary, or
         (e.g. without autostart) it will raise Exit_status.(Exit_with Exit_status.Build_id_mismatch).
         The exception will bubble up to our caller; here we need only handle success. *)
      Unix.close fd;
      let%lwt () = connect_then_close () in
      keep_trying_to_open
        ~has_already_attempted_connect:true
        ~connect_then_close
        ~already_checked_clock
        ~progress_callback
        ~deadline
        ~root
    | Error (end_sentinel, log_message) -> begin
      (* But if there was an existing file that's still bad even after our connection
         attempt, there's not much we can do *)
      Hh_logger.log
        "Errors-file: on %s, read sentinel %s [%s], and already connected once, so giving up."
        (Sys_utils.show_inode fd)
        (Server_progress.show_errors_file_error end_sentinel)
        log_message;
      progress_callback None;
      match end_sentinel with
      | Server_progress.Build_id_mismatch ->
        raise (Exit_status.Exit_with Exit_status.Build_id_mismatch)
      | Server_progress.Killed finale_data ->
        raise
          (Exit_status.Exit_with
             (Exit_status.Server_hung_up_should_abort finale_data))
      | _ ->
        failwith
          (Printf.sprintf
             "Unexpected error from openfile: %s [%s]"
             (Server_progress.show_errors_file_error end_sentinel)
             log_message)
    end
    | Ok { Server_progress.ErrorsRead.pid; clock = None; _ } ->
      (* If there's an existing file, but it's not using watchman, then we cannot offer
         consistency guarantees. We'll just go with it. This happens for instance
         if the server was started using dfind instead of watchman. *)
      Hh_logger.log
        "Errors-file: %s is present, without watchman, so just going with it."
        (Sys_utils.show_inode fd);
      Lwt.return (pid, fd)
    | Ok { Server_progress.ErrorsRead.clock = Some clock; _ }
      when Option.equal String.equal (Some clock) already_checked_clock ->
      (* we've already checked this clock! so just wait a short time, then re-open the file
         so we soon discover when it has a new clock. *)
      let%lwt () = Lwt_unix.sleep 0.1 in
      keep_trying_to_open
        ~has_already_attempted_connect
        ~connect_then_close
        ~already_checked_clock:(Some clock)
        ~progress_callback
        ~deadline
        ~root
    | Ok { Server_progress.ErrorsRead.pid; clock = Some clock; _ } -> begin
      Hh_logger.log
        "Errors-file: %s is present, was started at clock %s, so querying watchman..."
        (Sys_utils.show_inode fd)
        clock;
      (* Watchman doesn't support "what files have changed from error.bin's clock until
         hh-invocation clock?". We'll instead use the (less permissive, still correct) query
         "what files have changed from error.bin's clock until now?". *)
      progress_callback (Some "watchman sync");
      let%lwt since_result =
        watchman_get_raw_updates_since
          ~root
          ~clock
          ~fail_on_new_instance:false
          ~fail_during_state:false
      in
      progress_callback (Some "hh_server sync");
      match since_result with
      | Error e ->
        Hh_logger.log "Errors-file: watchman failure:\n%s\n" e;
        progress_callback None;
        Printf.eprintf "Watchman failure.\n%s\n%!" e;
        raise Exit_status.(Exit_with Exit_status.Watchman_failed)
      | Ok relative_raw_updates ->
        let raw_updates =
          relative_raw_updates
          |> List.map ~f:(fun file ->
                 Filename.concat (Path.to_string root) file)
          |> SSet.of_list
        in
        let updates =
          FindUtils.post_watchman_filter_from_fully_qualified_raw_updates
            ~root
            ~raw_updates
        in
        if Relative_path.Set.is_empty updates then begin
          (* If there was an existing errors.bin, and no files have changed since then,
             then use it! *)
          Hh_logger.log
            "Errors-file: %s is present, was started at clock %s and watchman reports no updates since then, so using it!"
            (Sys_utils.show_inode fd)
            clock;
          Lwt.return (pid, fd)
        end else begin
          (* But if files have changed since the errors.bin, then we will keep spinning
             under trust that hh_server will eventually recognize those changes and create
             a new errors.bin file, which we'll then pick up on another iteration.
             CARE! This only works if hh_server's test for "are there new files" is at least
             as strict as our own.
             They're identical, in fact, because they both use the same watchman filter
             [FilesToIgnore.watchman_server_expression_terms] and the same [FindUtils.post_watchman_filter]. *)
          Hh_logger.log
            "Errors-file: %s is present, was started at clock %s, but watchman reports updates since then, so trying again. %d updates, for example %s"
            (Sys_utils.show_inode fd)
            clock
            (Relative_path.Set.cardinal updates)
            (Relative_path.Set.choose updates |> Relative_path.suffix);
          let%lwt () = Lwt_unix.sleep 0.1 in
          keep_trying_to_open
            ~has_already_attempted_connect
            ~connect_then_close
            ~already_checked_clock:(Some clock)
            ~progress_callback
            ~deadline
            ~root
        end
    end
  end

(** Provides typechecker errors by displaying them to stdout.
    Errors are printed soon after they are known instead of all at once at the end. *)
let go_streaming
    (args : ClientEnv.client_check_env)
    ~(partial_telemetry_ref : Telemetry.t option ref)
    ~(connect_then_close : unit -> unit Lwt.t) :
    (Exit_status.t * Telemetry.t) Lwt.t =
  let ClientEnv.{ root; show_spinner; deadline; _ } = args in
  let progress_callback : string option -> unit =
    ClientSpinner.report ~to_stderr:show_spinner ~angery_reaccs_only:false
  in
  let connect_then_close () =
    (* must clear spinner in case [connect_then_close] writes to stderr *)
    progress_callback None;
    connect_then_close ()
  in
  let%lwt (pid, fd) =
    keep_trying_to_open
      ~has_already_attempted_connect:false
      ~already_checked_clock:None
      ~connect_then_close
      ~progress_callback
      ~deadline
      ~root
  in
  let%lwt (exit_status, telemetry) =
    go_streaming_on_fd ~pid fd args ~partial_telemetry_ref ~progress_callback
  in
  Lwt.return (exit_status, telemetry)
