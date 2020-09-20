(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* This should stay at toplevel in order to be executed before [Daemon.check_entry_point]. *)
let entry =
  WorkerController.register_entry_point ~restore:Batch_global_state.restore

let catch_and_classify_exceptions : 'x 'b. ('x -> 'b) -> 'x -> 'b =
 fun f x ->
  try f x with
  | Decl_class.Decl_heap_elems_bug -> Exit.exit Exit_status.Decl_heap_elems_bug
  | File_provider.File_provider_stale ->
    Exit.exit Exit_status.File_provider_stale
  | Decl_defs.Decl_not_found x ->
    Hh_logger.log "Decl_not_found %s" x;
    Exit.exit Exit_status.Decl_not_found
  | Caml.Not_found -> Exit.exit Exit_status.Worker_not_found_exception

let make_tmp_dir () =
  let tmpdir = Path.make (Tmp.temp_dir GlobalConfig.tmp_dir "files") in
  Relative_path.set_path_prefix Relative_path.Tmp tmpdir

let make_hhi_dir () =
  let hhi_root = Hhi.get_hhi_root () in
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root

let init_state
    ~(root : Path.t) ~(popt : ParserOptions.t) ~(tcopt : TypecheckerOptions.t) :
    Provider_context.t * Batch_global_state.batch_state =
  Relative_path.(set_path_prefix Root root);
  make_tmp_dir ();
  make_hhi_dir ();
  Typing_global_inference.set_path ();
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:Provider_backend.Shared_memory
  in
  let batch_state = Batch_global_state.save ~trace:true in
  (ctx, batch_state)

let init
    ~(root : Path.t)
    ~(shmem_config : SharedMem.config)
    ~(popt : ParserOptions.t)
    ~(tcopt : TypecheckerOptions.t)
    (t : float) : Provider_context.t * MultiWorker.worker list * float =
  let nbr_procs = Sys_utils.nbr_procs in
  let heap_handle = SharedMem.init ~num_workers:nbr_procs shmem_config in
  let gc_control = Core_kernel.Gc.get () in
  let (ctx, state) = init_state ~root ~popt ~tcopt in
  let workers =
    MultiWorker.make
      ~call_wrapper:{ WorkerController.wrap = catch_and_classify_exceptions }
      ~saved_state:state
      ~entry
      ~nbr_procs
      ~gc_control
      ~heap_handle
  in
  ( ctx,
    workers,
    Hh_logger.Level.log_duration Hh_logger.Level.Debug "make_workers" t )

let init_with_defaults =
  init
    ~root:(Path.make "/")
    ~shmem_config:SharedMem.default_config
    ~popt:ParserOptions.default
    ~tcopt:TypecheckerOptions.default
