(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module CamlGc = Gc
open Hh_prelude
module Gc = CamlGc

(*****************************************************************************
 * The job executed by the worker.
 *
 * The 'serializer' is the job continuation: it is a function that must
 * be called at the end of the request ir order to send back the result
 * to the worker process (this is "internal business", this is not visible outside
 * this module). The clone process will provide the expected function.
 * cf 'send_result' in 'read_and_process_job.
 *
 *****************************************************************************)

type request = Request of (serializer -> unit) * metadata_in

and serializer = { send: 'a. 'a -> unit }

and metadata_in = { log_globals: HackEventLogger.serialized_globals }

type metadata_out = {
  stats: Measure.record_data;
  log_globals: HackEventLogger.serialized_globals;
}

type subprocess_job_status = Subprocess_terminated of Unix.process_status

let on_clone_cancelled parent_outfd =
  (* The cancelling controller will ignore result of cancelled job anyway (see
   * wait_for_cancel function), so we can send back anything. Write twice, since
   * the normal response writes twice too *)
  Marshal_tools.to_fd_with_preamble parent_outfd "anything" |> ignore;
  Marshal_tools.to_fd_with_preamble parent_outfd "anything" |> ignore

(*****************************************************************************
 * Process a single job in a worker (or a clone).
 *****************************************************************************)

type job_outcome =
  [ `Success
  | `Error of Exit_status.t
  | `Worker_cancelled
  | `Controller_has_died
  ]

let read_and_process_job ic oc : job_outcome =
  let start_user_time = ref 0. in
  let start_system_time = ref 0. in
  let start_minor_words = ref 0. in
  let start_promoted_words = ref 0. in
  let start_major_words = ref 0. in
  let start_minor_collections = ref 0 in
  let start_major_collections = ref 0 in
  let start_wall_time = ref 0. in
  let start_proc_fs_status = ref None in
  let infd = Daemon.descr_of_in_channel ic in
  let outfd = Daemon.descr_of_out_channel oc in
  let send_result data =
    Mem_profile.stop ();
    let tm = Unix.times () in
    let end_user_time = tm.Unix.tms_utime +. tm.Unix.tms_cutime in
    let end_system_time = tm.Unix.tms_stime +. tm.Unix.tms_cstime in
    let {
      Gc.minor_words = end_minor_words;
      promoted_words = end_promoted_words;
      major_words = end_major_words;
      minor_collections = end_minor_collections;
      major_collections = end_major_collections;
      _;
    } =
      Gc.quick_stat ()
    in
    let (major_time, minor_time) = Sys_utils.get_gc_time () in
    Measure.sample "worker_gc_major_wall_time" major_time;
    Measure.sample "worker_gc_minor_wall_time" minor_time;

    Measure.sample "worker_user_time" (end_user_time -. !start_user_time);
    Measure.sample "worker_system_time" (end_system_time -. !start_system_time);
    Measure.sample "worker_wall_time" (Unix.gettimeofday () -. !start_wall_time);

    Measure.track_distribution
      "minor_words"
      ~bucket_size:(float (100 * 1024 * 1024));
    Measure.sample "minor_words" (end_minor_words -. !start_minor_words);

    Measure.track_distribution
      "promoted_words"
      ~bucket_size:(float (25 * 1024 * 1024));
    Measure.sample "promoted_words" (end_promoted_words -. !start_promoted_words);

    Measure.track_distribution
      "major_words"
      ~bucket_size:(float (50 * 1024 * 1024));
    Measure.sample "major_words" (end_major_words -. !start_major_words);

    Measure.sample
      "minor_collections"
      (float (end_minor_collections - !start_minor_collections));
    Measure.sample
      "major_collections"
      (float (end_major_collections - !start_major_collections));

    begin
      match (!start_proc_fs_status, ProcFS.status_for_pid (Unix.getpid ())) with
      | ( Some { ProcFS.rss_total = start; _ },
          Ok { ProcFS.rss_total = total; rss_hwm = hwm; _ } ) ->
        Measure.sample "worker_rss_start" (float start);
        Measure.sample "worker_rss_delta" (float (total - start));
        Measure.sample "worker_rss_hwm_delta" (float (hwm - start))
      | _ -> ()
    end;

    (* After this point, it is critical to not throw a Worker_should_exit
       exception; otherwise outfd might end up being corrupted *)
    WorkerCancel.with_no_cancellations (fun () ->
        let len =
          Measure.time "worker_send_response" (fun () ->
              Marshal_tools.to_fd_with_preamble
                ~flags:[Marshal.Closures]
                outfd
                data)
        in
        if len > 30 * 1024 * 1024 (* 30 MiB *) then (
          Hh_logger.log
            ("WARNING(WORKER_LARGE_DATA_SEND): you are sending quite a lot of "
            ^^ "data (%d bytes), which may have an adverse performance impact. "
            ^^ "If you are sending closures, double-check to ensure that "
            ^^ "they have not captured large values in their environment.")
            len;
          HackEventLogger.worker_large_data_send
            ~path:Relative_path.default
            (Telemetry.create () |> Telemetry.int_ ~key:"len" ~value:len)
        );

        Measure.sample "worker_response_len" (float len);

        let metadata_out =
          {
            stats = Measure.serialize (Measure.pop_global ());
            log_globals = HackEventLogger.serialize_globals ();
          }
        in
        let _ = Marshal_tools.to_fd_with_preamble outfd metadata_out in
        ())
  in

  try
    Measure.push_global ();
    let request : request =
      Measure.time "worker_read_request" (fun () ->
          Marshal_tools.from_fd_with_preamble infd)
    in
    let (Request (do_process, { log_globals })) = request in
    let tm = Unix.times () in
    let gc = Gc.quick_stat () in
    Sys_utils.start_gc_profiling ();
    start_user_time := tm.Unix.tms_utime +. tm.Unix.tms_cutime;
    start_system_time := tm.Unix.tms_stime +. tm.Unix.tms_cstime;
    start_minor_words := gc.Gc.minor_words;
    start_promoted_words := gc.Gc.promoted_words;
    start_major_words := gc.Gc.major_words;
    start_minor_collections := gc.Gc.minor_collections;
    start_major_collections := gc.Gc.major_collections;
    start_wall_time := Unix.gettimeofday ();
    start_proc_fs_status :=
      ProcFS.status_for_pid (Unix.getpid ()) |> Core.Result.ok;
    HackEventLogger.deserialize_globals log_globals;
    Mem_profile.start ();
    do_process { send = send_result };
    `Success
  with
  | WorkerCancel.Worker_should_exit -> `Worker_cancelled
  | SharedMem.Out_of_shared_memory -> `Error Exit_status.Out_of_shared_memory
  | SharedMem.Hash_table_full -> `Error Exit_status.Hash_table_full
  | SharedMem.Heap_full -> `Error Exit_status.Heap_full
  | SharedMem.Sql_assertion_failure err_num ->
    `Error
      begin
        match err_num with
        | 11 -> Exit_status.Sql_corrupt
        | 14 -> Exit_status.Sql_cantopen
        | 21 -> Exit_status.Sql_misuse
        | _ -> Exit_status.Sql_assertion_failure
      end
  | End_of_file ->
    (* This happens in the expected graceful shutdown path of our unit tests:
       the controller shuts down its end of the pipe, and therefore when we
       call [from_fd_with_preamble] above to get the next work-item, we get End_of_file.
       We're catching it here, rather than solely around [from_fd_with_preamble],
       because it's easier. This is fine because workers do no reading other than from
       the server. *)
    `Controller_has_died
  | Unix.Unix_error (Unix.EPIPE, _, _) ->
    (* This happens in the expected abrupt shutdown path of hh_server:
       the controller process shuts down, and therefore when we finish our batch
       of work and try to write the answer in [send_result] above, we get EPIPE.
       We're catching it here, rather than solely around [send_result], because
       it's easier. This is fine because workers have no other pipes other than
       to the server. We do log to the server-log, though, which is fair since
       it was an abrupt shutdown. *)
    (* Note: there are other manifestations of server shutdown, e.g.
       Marshal_tools.Reading_Preamble_Exception. I'm not confident I know all
       of them, nor can tell which ones are expected vs unexpected, so I'll
       leave them all to the catch-all handler below. *)
    Hh_logger.log "Worker got EPIPE due to server shutdown";
    `Controller_has_died
  | exn ->
    let e = Exception.wrap exn in
    Hh_logger.log
      "WORKER_EXCEPTION %s"
      (Exception.to_string e |> Exception.clean_stack);
    EventLogger.log_if_initialized (fun () ->
        HackEventLogger.worker_exception e);
    (* What exit code should we emit for an uncaught exception?
       The ocaml runtime emits exit code 2 for uncaught exceptions.
       We should really pick our own different code here, but (history) we don't.
       How can we convey exit code 2? By unfortunate accident, Exit_status.Type_error
       gets turned into "2". So that's how we're going to return exit code 2. Yuck. *)
    `Error Exit_status.Type_error

(*****************************************************************************
 * Entry point for spawned worker.
 *****************************************************************************)

(* The exit code used when the controller died and the clone could not read
 * the input job *)
let controller_has_died_code = 1

let process_job_and_exit ic oc =
  match read_and_process_job ic oc with
  | `Success -> exit 0
  | `Error status -> Exit.exit status
  | `Worker_cancelled ->
    on_clone_cancelled (Daemon.descr_of_out_channel oc);
    exit 0
  | `Controller_has_died -> `Controller_has_died

let win32_worker_main restore (state, _controller_fd) (ic, oc) =
  (* On Windows, there is no clone process, the worker does the job
     directly and exits when it is done. *)
  restore state;
  match process_job_and_exit ic oc with
  | `Controller_has_died -> exit 0

let maybe_send_status_to_controller fd status =
  match fd with
  | None -> ()
  | Some fd ->
    let to_controller fd msg =
      ignore (Marshal_tools.to_fd_with_preamble fd msg : int)
    in
    (match status with
    | Unix.WEXITED 0 -> ()
    | Unix.WEXITED code when code = controller_has_died_code ->
      (* Since the controller died we'd get an error writing to its
       * fd; so we simply do not do anything. *)
      ()
    | _ ->
      Timeout.with_timeout
        ~timeout:10
        ~on_timeout:(fun _ ->
          Hh_logger.log "Timed out sending status to controller")
        ~do_:(fun _ -> to_controller fd (Subprocess_terminated status)))

(* On Unix each job runs in a forked process. The first thing these jobs do is
 * deserialize a marshaled closure which is the job.
 *
 * The marshaled representation of a closure includes a MD5 digest of the code
 * segment and an offset. The digest is lazily computed, but if it has not been
 * computed before the fork, then each forked process will need to compute it.
 *
 * To avoid this, we deserialize a dummy closure before forking, so that we only
 * need to calculate the digest once per worker instead of once per job. *)
let dummy_closure () = ()

(**
 * On Windows, the worker is a process and runs the job directly. See above.
 *
 * On Unix, the worker is split into a main worker process and a clone worker
 * process with the main process reaping the clone process with waitpid.
 * The clone runs the actual job and sends the results over the output channel.
 * If the clone exits normally (exit code 0), the main worker process keeps
 * running and waiting for the next incoming job before forking a new clone.
 *
 * If the clone exits with a non-zero code, the main worker process also exits
 * with the same code. Thus, the contoller process of this worker can just
 * waitpid directly on the main worker process and see correct exit codes.
 *
 * NOTE: `WSIGNALED i` and `WSTOPPED i` are all coalesced into `exit 2`
 * and `exit 3` respectively, so some resolution is lost. If the clone worker
 * is, for example, SIGKILL'd by the OOM killer, the controller process won't
 * be aware of this.
 *
 * To regain this lost resolution, controller_fd can be optionally set. The
 * real exit status (includinng WSIGNALED and WSTOPPED) will be sent over
 * this file descriptor to the controller when the clone worker exits
 * abnormally (with a non-zero exit code).
 *)
let unix_worker_main restore (state, controller_fd) (ic, oc) =
  restore state;
  (* see dummy_closure above *)
  ignore Marshal.(from_bytes (to_bytes dummy_closure [Closures]) 0);
  let in_fd = Daemon.descr_of_in_channel ic in
  while true do
    (* Wait for an incoming job: is there something to read?
       But we don't read it yet. It will be read by the forked clone. *)
    let (readyl, _, _) = Unix.select [in_fd] [] [] (-1.0) in
    if List.is_empty readyl then exit 0;
    (* We fork a clone process for every incoming request
       and we let it exit after one request. This is the quickest GC. *)
    match Fork.fork () with
    | 0 ->
      (match process_job_and_exit ic oc with
      | `Controller_has_died -> exit controller_has_died_code)
    | pid ->
      (* Wait for the clone process termination... *)
      let status = snd (Sys_utils.waitpid_non_intr [] pid) in
      let () = maybe_send_status_to_controller controller_fd status in
      (match status with
      | Unix.WEXITED 0 -> ()
      | Unix.WEXITED code when code = controller_has_died_code ->
        (* The controller has died, we can stop working *)
        exit 0
      | Unix.WEXITED code ->
        Printf.printf "Worker exited (code: %d)\n" code;
        Stdlib.flush stdout;
        Stdlib.exit code
      | Unix.WSIGNALED x ->
        let sig_str = PrintSignal.string_of_signal x in
        Printf.printf "Worker interrupted with signal: %s\n" sig_str;
        Stdlib.flush stdout;
        Stdlib.exit 2
      | Unix.WSTOPPED x ->
        Printf.printf "Worker stopped with signal: %d\n" x;
        Stdlib.flush stdout;
        Stdlib.exit 3)
  done;
  assert false

(* This functions offers the same functionality as unix_worker_main but
 * does not clone a process for each incoming job. *)
let unix_worker_main_no_clone restore (state, controller_fd) (ic, oc) =
  (* T83401330: Long-lived workers are not production ready because
     they will not flush their logs often enough (c.f. EventLogger.flush).
     This can be addressed in this file, or in the user code that needs
     to log. *)
  restore state;
  let exit code =
    let status = Unix.WEXITED (Exit_status.exit_code code) in
    let () = maybe_send_status_to_controller controller_fd status in
    Exit.exit code
  in
  let in_fd = Daemon.descr_of_in_channel ic in
  let out_fd = Daemon.descr_of_out_channel oc in
  while true do
    let (readyl, _, _) = Unix.select [in_fd] [] [] (-1.0) in
    if List.is_empty readyl then exit Exit_status.No_error;
    match read_and_process_job ic oc with
    | `Success -> ()
    | `Error status -> exit status
    | `Worker_cancelled -> on_clone_cancelled out_fd
    | `Controller_has_died ->
      (* The controller has died, we can stop working *)
      exit Exit_status.No_error
  done;
  (* The only way out of the above loop is to exit *)
  assert false
