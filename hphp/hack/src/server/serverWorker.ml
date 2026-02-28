(*
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* As for [Daemon.register_entry_point], this should stay
   at toplevel, in order to be executed before
   [Daemon.check_entry_point]. *)
let entry =
  WorkerControllerEntryPoint.register ~restore:ServerGlobalState.restore

(** We use the call_wrapper to classify some exceptions in all calls in the
 * same way. *)
let catch_and_classify_exceptions : 'x 'b. ('x -> 'b) -> 'x -> 'b =
 fun f x ->
  try f x with
  | Decl_class.Decl_heap_elems_bug _ ->
    Exit.exit Exit_status.Decl_heap_elems_bug
  | File_provider.File_provider_stale ->
    Exit.exit Exit_status.File_provider_stale
  | Decl_defs.Decl_not_found x ->
    Hh_logger.log "Decl_not_found %s" x;
    Exit.exit Exit_status.Decl_not_found
  | Not_found_s _
  | Stdlib.Not_found ->
    Exit.exit Exit_status.Worker_not_found_exception

let make ~longlived_workers ~nbr_procs gc_control heap_handle ~logging_init =
  MultiWorker.make
    ~call_wrapper:{ WorkerController.wrap = catch_and_classify_exceptions }
    ~saved_state:(ServerGlobalState.save ~logging_init)
    ~entry
    ~longlived_workers
    nbr_procs
    ~gc_control
    ~heap_handle
