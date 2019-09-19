(*
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* As for [Daemon.register_entry_point], this should stay
   at toplevel, in order to be executed before
   [Daemon.check_entry_point]. *)
let entry =
  WorkerController.register_entry_point ~restore:ServerGlobalState.restore

(** We use the call_wrapper to classify some exceptions in all calls in the
 * same way. *)
let catch_and_classify_exceptions : 'x 'b. ('x -> 'b) -> 'x -> 'b =
 fun f x ->
  try f x with
  | Decl_class.Decl_heap_elems_bug -> Exit_status.(exit Decl_heap_elems_bug)
  | File_provider.File_provider_stale -> Exit_status.(exit File_provider_stale)
  | Decl_defs.Decl_not_found x ->
    Hh_logger.log "Decl_not_found %s" x;
    Exit_status.(exit Decl_not_found)
  | Not_found -> Exit_status.(exit Worker_not_found_exception)

let make ~nbr_procs gc_control heap_handle ~logging_init =
  MultiWorker.make
    ~call_wrapper:{ WorkerController.wrap = catch_and_classify_exceptions }
    ~saved_state:(ServerGlobalState.save ~logging_init)
    ~entry
    ~nbr_procs
    ~gc_control
    ~heap_handle
