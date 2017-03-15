(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module MC = MonitorConnection

let shutdown_client (_ic, oc) =
  let cli = Unix.descr_of_out_channel oc in
  try
    Unix.shutdown cli Unix.SHUTDOWN_ALL;
    close_out oc
  with _ -> ()

type file_input =
  | FileName of string
  | FileContent of string

let hh_monitor_config root = ServerMonitorUtils.({
  lock_file = ServerFiles.lock_file root;
  socket_file = ServerFiles.socket_file root;
  server_log_file = ServerFiles.log_link root;
  monitor_log_file = ServerFiles.monitor_log_link root;
  load_script_log_file = ServerFiles.load_log root;
})

let shut_down_server root =
  MC.connect_and_shut_down (hh_monitor_config root)

let connect_to_monitor root =
  MC.connect_once (hh_monitor_config root)

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
  let { SharedMem.
    used_slots;
    slots;
    nonempty_slots = _ } = SharedMem.dep_stats () in
  let load_factor = float_of_int used_slots /. float_of_int slots in
  Hh_logger.log "Dependency table load factor: %d / %d (%.02f)"
    used_slots slots load_factor;
  let { SharedMem.
    used_slots;
    slots;
    nonempty_slots } = SharedMem.hash_stats () in
  let load_factor = float_of_int used_slots /. float_of_int slots in
  Hh_logger.log
    "Hashtable load factor: %d / %d (%.02f) with %d nonempty slots"
    used_slots slots load_factor nonempty_slots;
  ()

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
  | Worker.Worker_oomed as e->
    Hh_logger.exc e;
    Exit_status.(exit Worker_oomed)
  | Worker.Worker_busy as e ->
    Hh_logger.exc e;
    Exit_status.(exit Worker_busy)
  | (Worker.Worker_exited_abnormally i) as e ->
    Hh_logger.exc e;
    (** Exit with the same exit code that that worker used. *)
    exit i
  | Worker.Worker_failed_to_send_job _ as e->
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
  | e ->
    Hh_logger.exc e;
    Exit_status.(exit Uncaught_exception)
