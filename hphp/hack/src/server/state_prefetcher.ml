(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


type t = Path.t option
type svn_rev = int
let dummy = None

let of_script_opt v = v

let run_and_log (script, svn_rev) =
  HackEventLogger.init_informant_prefetcher_runner (Unix.time ());
  let start_t = Unix.time () in
  let process = Process.exec
    (Path.to_string script) [(string_of_int svn_rev)] in
  (** We can block since this is already daemonized in a separate process. *)
  match Process.read_and_wait_pid ~timeout:30 process with
  | Result.Ok _ ->
    Hh_logger.log "Prefetcher finished successfully.";
    HackEventLogger.informant_prefetcher_success start_t;
    exit 0
  | Result.Error (Process_types.Process_exited_abnormally (es, _, stderr)) ->
    let exit_kind, exit_code = Exit_status.unpack es in
    let msg = Printf.sprintf "%s %d. stderr: %s" exit_kind exit_code stderr in
    Hh_logger.log "Prefetcher failed. %s" msg;
    HackEventLogger.informant_prefetcher_failed start_t msg;
    exit exit_code
  | Result.Error (Process_types.Timed_out (_, stderr)) ->
    let msg = Printf.sprintf "Timed out. stderr: %s" stderr in
    Hh_logger.log "Prefetcher timed out. %s" msg;
    HackEventLogger.informant_prefetcher_failed start_t msg;
    exit 1
  | Result.Error (Process_types.Process_aborted_input_too_large) ->
    (** This is impossible. Consider making Process a GADT to avoid this. *)
    Hh_logger.log ("Prefetcher failed. Process_aborted_input_too_large, " ^^
      "but this should never happen since no input was given.");
    exit 1


(**
 * We have to annotate the type here because the type variables
 * cannot be generalized - they're just part of
 * Daemon's phantom types, which are immediately discarded below.
 *
 * Just set them to unit. (Ouch.)
 *)
let run_and_log_entry : (Path.t * int) Process.Entry.t =
  Process.register_entry_point
    "Prefetcher_run_and_log" run_and_log

let run svn_rev t  = match t with
  | None -> Process_types.dummy
  | Some script ->
    Process.run_entry run_and_log_entry
      (script, svn_rev)
