(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Eager Initialization:

   hh_server can initialize either by typechecking the entire project (aka
   starting from a "fresh state") or by loading from a saved state and
   typechecking what has changed. Historically, eager initialization was able to
   do both, but we had little need for eager init from saved state, and removed
   it.

   If we perform an eager init from a fresh state, we run the following phases:

     Parsing -> Naming -> Type-decl -> Type-check

   If we are initializing lazily from a saved state, we do

     Run load script and parsing concurrently -> Naming -> Type-check

   Then we typecheck only the files that have changed since the state was saved.
   Type-decl is performed lazily as needed.
*)

open Core_kernel
open Result.Export
open SearchServiceRunner
open ServerEnv
open ServerInitCommon
open ServerInitTypes

module SLC = ServerLocalConfig

let type_decl
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (fast: FileInfo.fast)
    (t: float)
  : ServerEnv.env * float =
  let logstring = "Type-decl" in
  Hh_logger.log "Begin %s" logstring;
  let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
  let errorl =
    Decl_service.go ~bucket_size genv.workers env.tcopt fast in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  Stats.(stats.init_heap_size <- hs);
  HackEventLogger.type_decl_end t;
  let t = Hh_logger.log_duration logstring t in
  let env = {
    env with
    errorl = Errors.merge errorl env.errorl;
  } in
  env, t

let init
    ~(load_mini_approach: load_mini_approach option)
    (genv: ServerEnv.genv)
    (lazy_level: lazy_level)
    (env: ServerEnv.env)
    (root: Path.t)
  : (ServerEnv.env * float) * (loaded_info * Relative_path.Set.t, error) result =
  (* We don't support a saved state for eager init. *)
  ignore (load_mini_approach, root);
  let get_next, t = indexing genv in
  let lazy_parse = lazy_level = Parse in
  (* Parsing entire repo, too many files to trace. TODO: why do we parse
   * entire repo WHILE loading saved state that is supposed to prevent having
   * to do that? *)
  let trace = false in
  let env, t = parsing ~lazy_parse genv env ~get_next t ~trace in
  if not (ServerArgs.check_mode genv.options) then
    SearchServiceRunner.update_fileinfo_map env.files_info;

  let t = update_files genv env.files_info t in
  let env, t = naming env t in
  let fast = FileInfo.simplify_fast env.files_info in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
  let fast = Relative_path.Set.fold failed_parsing
    ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in
  let env, t =
    if lazy_level <> Off then env, t
    else type_decl genv env fast t in

  (* Type-checking everything *)
  SharedMem.cleanup_sqlite ();
  type_check genv env fast t, Error Eager_init_saved_state_not_supported
