(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module MC = MonitorConnection

let shutdown_client (_ic, oc) =
  let cli = Unix.descr_of_out_channel oc in
  try
    Unix.shutdown cli Unix.SHUTDOWN_ALL;
    close_out oc
  with _ -> ()

let hh_monitor_config root = ServerMonitorUtils.({
  lock_file = ServerFiles.lock_file root;
  socket_file = ServerFiles.socket_file root;
  server_log_file = ServerFiles.log_link root;
  monitor_log_file = ServerFiles.monitor_log_link root;
})

let shut_down_server root =
  MC.connect_and_shut_down (hh_monitor_config root)

let connect_to_monitor ~timeout root =
  MC.connect_once ~timeout (hh_monitor_config root)

let die_nicely () =
  HackEventLogger.killed ();
  (** Monitor will exit on its next check loop when it sees that
   * the typechecker process has exited. *)
  Hh_logger.log "Sent KILL command by client. Dying.";
  (* XXX when we exit, the dfind process will attempt to read from the broken
   * pipe and then exit with SIGPIPE, so it is unnecessary to kill it
   * explicitly *)
  exit 0

let print_hash_stats () =
  Core_result.try_with SharedMem.dep_stats
  |> Core_result.map_error ~f:Hh_logger.exc
  |> Core_result.iter ~f:begin fun { SharedMem.
    used_slots;
    slots;
    nonempty_slots = _ } ->
    let load_factor = float_of_int used_slots /. float_of_int slots in
    Hh_logger.log "Dependency table load factor: %d / %d (%.02f)"
      used_slots slots load_factor
  end;
  Core_result.try_with SharedMem.hash_stats
  |> Core_result.map_error ~f:Hh_logger.exc
  |> Core_result.iter ~f:begin fun { SharedMem.
    used_slots;
    slots;
    nonempty_slots } ->
    let load_factor = float_of_int used_slots /. float_of_int slots in
    Hh_logger.log
      "Hashtable load factor: %d / %d (%.02f) with %d nonempty slots"
      used_slots slots load_factor nonempty_slots
  end

let with_exit_on_exception f =
  try f () with
  | SharedMem.Out_of_shared_memory ->
    print_hash_stats ();
    Printf.eprintf "Error: failed to allocate in the shared heap.\n%!";
    Exit_status.(exit Out_of_shared_memory)
  | SharedMem.Hash_table_full ->
    print_hash_stats ();
    Printf.eprintf "Error: failed to allocate in the shared hashtable.\n%!";
    Exit_status.(exit Hash_table_full)
  | Watchman.Watchman_error s as e ->
    Hh_logger.exc e;
    Hh_logger.log "Exiting. Failed due to watchman error: %s" s;
    Exit_status.(exit Watchman_failed)
  | MultiThreadedCall.Coalesced_failures failures as e -> begin
    Hh_logger.exc e;
    let failure_strings = List.fold_left
      (fun acc f -> (WorkerController.failure_to_string f) :: acc)
      []
      failures
    in
    let failure_msg = Printf.sprintf "Coalesced_failures[%s]"
      (String.concat ", " failure_strings) in
    Hh_logger.log "%s" failure_msg;
    let is_oom_failure f = match f with
      | WorkerController.Worker_oomed -> true
      | _ -> false in
    let has_oom_failure = List.exists is_oom_failure failures in
    if has_oom_failure then
      let () = Hh_logger.log "Worker oomed. Exiting" in
      Exit_status.(exit Worker_oomed)
    else
      (** We attempt to exit with the same code as a worker by folding over
       * all the failures and looking for a WEXITED. *)
      let worker_exit f = match f with
        | WorkerController.Worker_quit (Unix.WEXITED i) ->
          Some i
        | _ ->
          None
      in
      let exit_code = List.fold_left (fun acc f ->
        if Option.is_some acc then acc else worker_exit f
        ) None failures
      in
      match exit_code with
      | Some i ->
        (** Exit with same code. *)
        exit i
      | None ->
        failwith failure_msg
  end
  (** In single-threaded mode, WorkerController exceptions are raised directly
   * instead of being grouped into MultiThreaadedCall.Coalesced_failures *)
  | WorkerController.(Worker_failed (_, Worker_oomed)) as e->
    Hh_logger.exc e;
    Exit_status.(exit Worker_oomed)
  | WorkerController.Worker_busy as e ->
    Hh_logger.exc e;
    Exit_status.(exit Worker_busy)
  | (WorkerController.(Worker_failed (_, Worker_quit(Unix.WEXITED i)))) as e ->
    Hh_logger.exc e;
    (** Exit with the same exit code that that worker used. *)
    exit i
  | WorkerController.Worker_failed_to_send_job _ as e->
    Hh_logger.exc e;
    Exit_status.(exit Worker_failed_to_send_job)
  | File_heap.File_heap_stale ->
    Exit_status.(exit File_heap_stale)
  | Decl_class.Decl_heap_elems_bug ->
    Exit_status.(exit Decl_heap_elems_bug)
  | SharedMem.C_assertion_failure _ as e ->
    Hh_logger.exc e;
    Exit_status.(exit Shared_mem_assertion_failure)
  | SharedMem.Sql_assertion_failure err_num as e ->
    Hh_logger.exc e;
    let exit_code = match err_num with
      | 11 -> Exit_status.Sql_corrupt
      | 14 -> Exit_status.Sql_cantopen
      | 21 -> Exit_status.Sql_misuse
      | _ -> Exit_status.Sql_assertion_failure
    in
    Exit_status.exit exit_code
  | Exit_status.Exit_with ec ->
    Exit_status.(exit ec)
  | e ->
    Hh_logger.exc e;
    Exit_status.(exit Uncaught_exception)
