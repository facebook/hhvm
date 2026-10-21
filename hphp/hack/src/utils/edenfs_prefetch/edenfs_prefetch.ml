(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** [relpath:] is relative to Sapling's [--cwd], which we set to the Hack root,
    so we can pass the file's root-relative suffix directly. *)
let file_pattern file = "relpath:" ^ Relative_path.suffix file

type prefetch = {
  process: Process_types.t;
  pattern_file: string;
  start_time: float;
  count: int;
}

type process_status =
  | Running
  | Finished of {
      outcome: string;
      duration: float option;
    }

let active_prefetches = ref []

let log_failure ~operation message =
  try Hh_logger.log "EdenFS prefetch %s failed: %s" operation message with
  | _ -> ()

let attempt_cleanup ~cleanup_failed_ref f =
  try Some (f ()) with
  | exn ->
    cleanup_failed_ref := true;
    log_failure ~operation:"cleanup" (Exn.to_string exn);
    None

let duration_from_stderr stderr =
  List.find_map (String.split_lines stderr) ~f:(fun line ->
      match String.split line ~on:' ' with
      | "time:" :: "real" :: seconds :: "secs" :: _ ->
        Float.of_string_opt seconds
        |> Option.filter ~f:(fun seconds ->
               Float.is_finite seconds && Float.(seconds >= 0.0))
      | _ -> None)

let read_finished_status process =
  let (outcome, stderr) =
    match Process.read_and_wait_pid ~timeout:0 process with
    | Ok result -> ("completed", Some result.Process_types.stderr)
    | Error failure ->
      log_failure ~operation:"command" (Process.failure_msg failure);
      let stderr =
        match failure with
        | Process_types.Abnormal_exit { stderr; _ } -> Some stderr
        | _ -> None
      in
      ("command_failed", stderr)
  in
  Finished { outcome; duration = Option.bind stderr ~f:duration_from_stderr }

let make_telemetry ~count ~outcome ~cleanup_failed ~timing =
  let telemetry =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"files" ~value:count
    |> Telemetry.string_ ~key:"outcome" ~value:outcome
    |> Telemetry.bool_ ~key:"cleanup_failed" ~value:cleanup_failed
  in
  Option.value_map timing ~default:telemetry ~f:(fun (seconds, source) ->
      telemetry
      |> Telemetry.float_ ~key:"duration_sec" ~value:seconds
      |> Telemetry.string_ ~key:"duration_source" ~value:source)

let cleanup ~cleanup_failed_ref prefetch =
  let { process; pattern_file; start_time; count } = prefetch in
  let status =
    match
      attempt_cleanup ~cleanup_failed_ref (fun () -> Process.is_ready process)
    with
    | Some true -> Some (read_finished_status process)
    | Some false -> Some Running
    | None -> None
  in
  let outcome =
    match status with
    | Some (Finished { outcome; duration = _ }) -> outcome
    | Some Running
    | None ->
      "stopped"
  in
  let stopped_at =
    Option.map
      (attempt_cleanup ~cleanup_failed_ref (fun () ->
           Utils.try_all
             [
               (fun () ->
                 Process.terminate
                   ~graceful_signal:(Some Sys.sigterm)
                   ~grace_period:1.0
                   process);
               (fun () -> Sys.remove pattern_file);
               (fun () ->
                 active_prefetches :=
                   List.filter !active_prefetches ~f:(fun active ->
                       not (phys_equal active prefetch)));
             ]))
      ~f:Unix.gettimeofday
  in
  let timing =
    match status with
    | Some Running ->
      Option.map stopped_at ~f:(fun end_time ->
          (end_time -. start_time, "controller"))
    | Some (Finished { outcome = _; duration }) ->
      Option.map duration ~f:(fun seconds -> (seconds, "sl"))
    | None -> None
  in
  let duration_message =
    Option.value_map timing ~default:"" ~f:(fun (seconds, _) ->
        Printf.sprintf "duration=%.3fs " seconds)
  in
  (try
     Hh_logger.log
       "EdenFS prefetch: files=%d %soutcome=%s cleanup_failed=%b"
       count
       duration_message
       outcome
       !cleanup_failed_ref
   with
  | exn -> log_failure ~operation:"logging" (Exn.to_string exn));
  make_telemetry ~count ~outcome ~cleanup_failed:!cleanup_failed_ref ~timing

(* Exit.exit and Stdlib.exit do not unwind try_finally, e.g. during SIGUSR2
   shutdown. Exit hooks clean up prefetch processes on those paths. *)
let cleanup_on_exit () =
  List.iter !active_prefetches ~f:(fun prefetch ->
      ignore (cleanup ~cleanup_failed_ref:(ref false) prefetch))

let exit_hooks_registered = ref false

let register_exit_hooks () =
  if not !exit_hooks_registered then begin
    Exit.add_hook_upon_clean_exit (fun _ -> cleanup_on_exit ());
    Stdlib.at_exit cleanup_on_exit;
    exit_hooks_registered := true
  end

let write_pattern_file ~cleanup_failed_ref files =
  let (pattern_file, channel) =
    Stdlib.Filename.open_temp_file "hh_edenfs_prefetch_" ".patterns"
  in
  try
    Relative_path.Set.iter files ~f:(fun file ->
        Out_channel.output_string channel (file_pattern file);
        Out_channel.output_char channel '\n');
    Out_channel.close channel;
    pattern_file
  with
  | exn ->
    ignore
      (attempt_cleanup ~cleanup_failed_ref (fun () ->
           Utils.try_all
             [
               (fun () -> Out_channel.close channel);
               (fun () -> Sys.remove pattern_file);
             ])
        : unit option);
    raise exn

let start ~root ~files ~workers ~cleanup_failed_ref =
  if Relative_path.Set.is_empty files then
    None
  else
    let count = Relative_path.Set.cardinal files in
    let pattern_file = write_pattern_file ~cleanup_failed_ref files in
    try
      register_exit_hooks ();
      Hh_logger.log "EdenFS prefetch: starting files=%d" count;
      (* [sl prefetch -r .] caches file contents at the current commit. In theory, we could filter
         out locally modified files (no point to fetch their state as of the commit), but that would
         just add too much complexity here. *)
      let start_time = Unix.gettimeofday () in
      let process =
        Process.exec
          Exec_command.Sl
          [
            "--cwd";
            Path.to_string root;
            "--quiet";
            "--time";
            "--config";
            Printf.sprintf "filewalk.concurrent-fetches=%d" workers;
            "prefetch";
            "-r";
            ".";
            "--include";
            "listfile:" ^ pattern_file;
          ]
      in
      let prefetch = { process; pattern_file; start_time; count } in
      active_prefetches := prefetch :: !active_prefetches;
      Some prefetch
    with
    | exn ->
      ignore
        (attempt_cleanup ~cleanup_failed_ref (fun () -> Sys.remove pattern_file)
          : unit option);
      raise exn

let with_prefetch ~root ~files ~workers ~f =
  let telemetry = ref (Telemetry.create ()) in
  let cleanup_failed_ref = ref false in
  let prefetch =
    try start ~root ~files ~workers ~cleanup_failed_ref with
    | exn ->
      log_failure ~operation:"launch" (Exn.to_string exn);
      telemetry :=
        make_telemetry
          ~count:(Relative_path.Set.cardinal files)
          ~outcome:"launch_failed"
          ~cleanup_failed:!cleanup_failed_ref
          ~timing:None;
      None
  in
  let result =
    Utils.try_finally ~f ~finally:(fun () ->
        Option.iter prefetch ~f:(fun prefetch ->
            telemetry := cleanup ~cleanup_failed_ref prefetch))
  in
  (result, !telemetry)
