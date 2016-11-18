(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* As for [Daemon.register_entry_point], this should stay
   at toplevel, in order to be executed before
   [Daemon.check_entry_point]. *)
let entry = Worker.register_entry_point ~restore:(ServerGlobalState.restore)

(** We use the call_wrapper to classify some exceptions in all calls in the
 * same way. *)
let catch_and_classify_exceptions: 'x 'b. ('x -> 'b) -> 'x -> 'b = fun f x ->
  try f x with
  | Decl_class.Decl_heap_elems_bug ->
    Exit_status.(exit Decl_heap_elems_bug)
  | File_heap.File_heap_stale ->
    Exit_status.(exit File_heap_stale)
  | Not_found ->
    Exit_status.(exit Worker_not_found_exception)

let make ?nbr_procs gc_control heap_handle =
  let nbr_procs = match nbr_procs with
    | None -> GlobalConfig.nbr_procs
    | Some x -> x in
  Worker.make
    ~call_wrapper:{ Worker.wrap = catch_and_classify_exceptions; }
    ~saved_state:(ServerGlobalState.save ())
    ~entry
    ~nbr_procs
    ~gc_control
    ~heap_handle
