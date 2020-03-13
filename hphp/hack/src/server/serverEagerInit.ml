(*
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
open SearchServiceRunner
open ServerEnv
open ServerInitCommon
open ServerInitTypes
module SLC = ServerLocalConfig

let type_decl
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (fast : Naming_table.fast)
    (t : float) : ServerEnv.env * float =
  ServerProgress.send_progress_to_monitor "evaluating type declarations";
  let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
  let ctx = Provider_utils.ctx_from_server_env env in
  let errorl = Decl_service.go ~bucket_size ctx genv.workers fast in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  Stats.(stats.init_heap_size <- hs);
  HackEventLogger.type_decl_end t;
  let t = Hh_logger.log_duration "Type-decl" t in
  let env = { env with errorl = Errors.merge errorl env.errorl } in
  (env, t)

let init (genv : ServerEnv.genv) (lazy_level : lazy_level) (env : ServerEnv.env)
    : ServerEnv.env * float =
  (* We don't support a saved state for eager init. *)
  let (get_next, t) = indexing genv in
  let lazy_parse = lazy_level = Parse in
  (* Parsing entire repo, too many files to trace *)
  let trace = false in
  let (env, t) = parsing ~lazy_parse genv env ~get_next t ~trace in
  if not (ServerArgs.check_mode genv.options) then
    SearchServiceRunner.update_fileinfo_map env.naming_table SearchUtils.Init;

  let t = update_files genv env.naming_table t in
  let (env, t) = naming env t in
  let fast = Naming_table.to_fast env.naming_table in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
  let fast =
    Relative_path.Set.fold
      failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x)
      ~init:fast
  in
  let (env, t) =
    if lazy_level <> Off then
      (env, t)
    else
      type_decl genv env fast t
  in
  (* Type-checking everything *)
  SharedMem.cleanup_sqlite ();
  type_check genv env (Relative_path.Map.keys fast) t
