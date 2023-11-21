(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* This should stay at toplevel in order to be executed before [Daemon.check_entry_point]. *)
let entry =
  WorkerControllerEntryPoint.register ~restore:Batch_global_state.restore

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

let make_tmp_dir () =
  let tmpdir = Path.make (Tmp.temp_dir GlobalConfig.tmp_dir "files") in
  Relative_path.set_path_prefix Relative_path.Tmp tmpdir

let make_hhi_dir () =
  let hhi_root = Hhi.get_hhi_root () in
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root

let init_state
    ~(root : Path.t)
    ~(popt : ParserOptions.t)
    ~(tcopt : TypecheckerOptions.t)
    ~(deps_mode : Typing_deps_mode.t) :
    Provider_context.t * Batch_global_state.batch_state =
  Relative_path.(set_path_prefix Root root);
  make_tmp_dir ();
  make_hhi_dir ();
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:Provider_backend.Shared_memory
      ~deps_mode
  in
  let batch_state = Batch_global_state.save ~trace:true in
  (ctx, batch_state)

let init
    ~(root : Path.t)
    ~(shmem_config : SharedMem.config)
    ~(popt : ParserOptions.t)
    ~(tcopt : TypecheckerOptions.t)
    ~(deps_mode : Typing_deps_mode.t)
    ?(gc_control : Gc.control option)
    (t : float) : Provider_context.t * MultiWorker.worker list * float =
  let nbr_procs = Sys_utils.nbr_procs in
  let heap_handle = SharedMem.init ~num_workers:nbr_procs shmem_config in
  let gc_control =
    match gc_control with
    | Some c -> c
    | None -> Core.Gc.get ()
  in
  let (ctx, state) = init_state ~root ~popt ~tcopt ~deps_mode in
  let workers =
    MultiWorker.make
      ~call_wrapper:{ WorkerController.wrap = catch_and_classify_exceptions }
      ~longlived_workers:false
      ~saved_state:state
      ~entry
      nbr_procs
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
    ~deps_mode:(Typing_deps_mode.InMemoryMode None)
    ?gc_control:None
