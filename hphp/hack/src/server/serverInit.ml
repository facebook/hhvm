(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Result.Export
open SearchServiceRunner
open ServerEnv
module SLC = ServerLocalConfig
include ServerInitTypes

let run_search (genv : ServerEnv.genv) (env : ServerEnv.env) :
    SearchUtils.si_env =
  let ctx = Provider_utils.ctx_from_server_env env in
  let sienv = env.local_symbol_table in
  if
    SearchServiceRunner.should_run_completely
      genv
      sienv.SearchUtils.sie_provider
  then
    SearchServiceRunner.run_completely ctx sienv
  else
    sienv

let save_state
    (genv : ServerEnv.genv) (env : ServerEnv.env) (output_filename : string) :
    SaveStateServiceTypes.save_state_result option =
  let ignore_errors =
    ServerArgs.gen_saved_ignore_type_errors genv.ServerEnv.options
  in
  let has_errors = not (Errors.is_empty env.errorl) in
  let do_save_state =
    if ignore_errors then (
      if has_errors then
        Printf.eprintf
          "WARNING: BROKEN SAVED STATE! Generating saved state. Ignoring type errors.\n%!"
      else
        Printf.eprintf
          "Generating saved state and ignoring type errors, but there were none.\n%!";
      true
    ) else if has_errors then (
      Printf.eprintf
        "Refusing to generate saved state. There are type errors\n%!";
      Printf.eprintf "and --gen-saved-ignore-type-errors was not provided.\n%!";
      false
    ) else
      true
  in
  if not do_save_state then
    None
  else
    let result = SaveStateService.save_state genv env output_filename in
    Some result

let post_init genv (env, _t) =
  (* Configure symbol index settings *)
  ServerProgress.write "updating search index...";
  let namespace_map = ParserOptions.auto_namespace_map env.tcopt in
  let env =
    {
      env with
      local_symbol_table =
        SymbolIndex.initialize
          ~globalrev:None
          ~gleanopt:env.gleanopt
          ~namespace_map
          ~provider_name:
            genv.local_config.ServerLocalConfig.symbolindex_search_provider
          ~quiet:genv.local_config.ServerLocalConfig.symbolindex_quiet
          ~savedstate_file_opt:
            genv.local_config.ServerLocalConfig.symbolindex_file
          ~workers:genv.workers;
    }
  in
  let env = { env with local_symbol_table = run_search genv env } in
  SharedMem.SMTelemetry.init_done ();
  env

let get_lazy_level (genv : ServerEnv.genv) : lazy_level =
  let lazy_decl = Option.is_none (ServerArgs.ai_mode genv.options) in
  let lazy_parse = genv.local_config.SLC.lazy_parse in
  let lazy_initialize = genv.local_config.SLC.lazy_init in
  match (lazy_decl, lazy_parse, lazy_initialize) with
  | (true, false, false) -> Decl
  | (true, true, false) -> Parse
  | (true, true, true) -> Init
  | _ -> Off

let remote_init genv env root worker_key nonce check_id _profiling =
  if not (ServerArgs.check_mode genv.options) then
    failwith "Remote init is only supported in check (run once) mode";
  let bin_root = Path.make (Filename.dirname Sys.argv.(0)) in
  let t = Unix.gettimeofday () in
  let ctx = Provider_utils.ctx_from_server_env env in
  let cache_remote_decls =
    genv.local_config.ServerLocalConfig.cache_remote_decls
  in
  let use_shallow_decls_saved_state =
    genv.local_config.ServerLocalConfig.use_shallow_decls_saved_state
  in
  let open ServerLocalConfig in
  let { init_id; ci_info; init_start_t; _ } = env.init_env in
  ServerRemoteInit.init
    ctx
    genv.workers
    ~worker_key
    ~nonce
    ~check_id
    ~ci_info
    ~init_id
    ~init_start_t
    ~bin_root
    ~root
    ~cache_remote_decls
    ~use_shallow_decls_saved_state
    ~saved_state_manifold_path:
      genv.local_config.remote_worker_saved_state_manifold_path
    ~shallow_decls_manifold_path:genv.local_config.shallow_decls_manifold_path;
  let _ = Hh_logger.log_duration "Remote type check" t in
  (env, Load_state_declined "Out-of-band naming table initialization only")

let lazy_full_init genv env profiling =
  ( ServerLazyInit.full_init genv env profiling |> post_init genv,
    Load_state_declined "No saved-state requested (for lazy init)" )

let lazy_parse_only_init genv env profiling =
  ( ServerLazyInit.parse_only_init genv env profiling |> fst,
    Load_state_declined "No saved-state requested (for lazy parse-only init)" )

let lazy_saved_state_init
    ~do_indexing genv env root load_state_approach profiling =
  let result =
    ServerLazyInit.saved_state_init
      ~do_indexing
      ~load_state_approach
      genv
      env
      root
      profiling
  in
  (* Saved-state init is the only kind of init that might error... *)
  match result with
  | Ok ((env, t), ({ saved_state_delta; _ }, _)) ->
    let env = post_init genv (env, t) in
    (env, Load_state_succeeded saved_state_delta)
  | Error err ->
    let ServerInitTypes.{ message; auto_retry; telemetry } =
      load_state_error_to_verbose_string err
    in
    let (next_step_descr, next_step, user_instructions) =
      if do_indexing then
        (* ServerInit.Write_symbol_info_with_state will never fallback upon saved-state problems *)
        ("fatal", Exit_status.Failed_to_load_should_abort, None)
      else if not genv.local_config.SLC.require_saved_state then
        (* without "--config require_saved_state=true", we're happy to fallback to full init *)
        ("fallback", Exit_status.No_error, None)
      else if auto_retry then
        (* The auto-retry means we'll exit in such a way that find_hh.sh will rerun us *)
        ("retry", Exit_status.Failed_to_load_should_retry, None)
      else
        (* No fallbacks, no retries, no recourse! Let's explain this clearly to the user. *)
        ( "fatal",
          Exit_status.Failed_to_load_should_abort,
          Some ServerInitMessages.messageSavedStateFailedFullInitDisabled )
    in
    let user_message = Printf.sprintf "%s [%s]" message next_step_descr in
    let user_message =
      match user_instructions with
      | None -> user_message
      | Some user_instructions ->
        Printf.sprintf "%s\n\n%s" user_message user_instructions
    in
    HackEventLogger.load_state_exn telemetry;
    Hh_logger.log "LOAD_STATE_EXN %s" (Telemetry.to_string telemetry);
    (match next_step with
    | Exit_status.No_error ->
      let fall_back_to_full_init profiling =
        ServerLazyInit.full_init genv env profiling |> post_init genv
      in
      ( CgroupProfiler.step_group "lazy_full_init" ~log:true
        @@ fall_back_to_full_init,
        Load_state_failed (user_message, telemetry) )
    | _ -> Exit.exit ~msg:user_message ~telemetry next_step)

let eager_init genv env _lazy_lev profiling =
  let init_result =
    Hh_logger.log "Saved-state requested, but overridden by eager init";
    Load_state_declined "Saved-state requested, but overridden by eager init"
  in
  let env =
    ServerEagerInit.init genv _lazy_lev env profiling |> post_init genv
  in
  (env, init_result)

let eager_full_init genv env _lazy_lev profiling =
  let env =
    ServerEagerInit.init genv _lazy_lev env profiling |> post_init genv
  in
  let init_result = Load_state_declined "No saved-state requested" in
  (env, init_result)

let lazy_write_symbol_info_init genv env root (load_state : 'a option) profiling
    =
  match load_state with
  | None ->
    ( ServerLazyInit.write_symbol_info_full_init genv env profiling
      |> post_init genv,
      Load_state_declined "Write Symobl info state" )
  | Some load_state_approach ->
    lazy_saved_state_init
      ~do_indexing:true
      genv
      env
      root
      load_state_approach
      profiling

(* entry point *)
let init
    ~(init_approach : init_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * init_result =
  if genv.local_config.ServerLocalConfig.rust_provider_backend then (
    Hh_logger.log "ServerInit: using rust backend";
    Provider_backend.set_rust_backend env.popt
  );
  let lazy_lev = get_lazy_level genv in
  let root = ServerArgs.root genv.options in
  let (lazy_lev, init_approach) =
    if TypecheckerOptions.global_inference env.tcopt then (
      Typing_global_inference.init ();
      (Off, Full_init)
    ) else
      (lazy_lev, init_approach)
  in
  let (init_method, init_method_name) =
    Hh_logger.log "ServerInit: lazy_lev=%s" (show_lazy_level lazy_lev);
    Hh_logger.log
      "ServerInit: init_approach=%s"
      (show_init_approach init_approach);
    match (lazy_lev, init_approach) with
    | (_, Remote_init { worker_key; nonce; check_id }) ->
      (remote_init genv env root worker_key nonce check_id, "remote_init")
    | (Init, Full_init) -> (lazy_full_init genv env, "lazy_full_init")
    | (Init, Parse_only_init) ->
      (lazy_parse_only_init genv env, "lazy_parse_only_init")
    | (Init, Saved_state_init load_state_approach) ->
      ( lazy_saved_state_init
          ~do_indexing:false
          genv
          env
          root
          load_state_approach,
        "lazy_saved_state_init" )
    | (Off, Full_init)
    | (Decl, Full_init)
    | (Parse, Full_init) ->
      (eager_full_init genv env lazy_lev, "eager full init")
    | (Off, _)
    | (Decl, _)
    | (Parse, _) ->
      (eager_init genv env lazy_lev, "eager_init")
    | (_, Write_symbol_info) ->
      ( lazy_write_symbol_info_init genv env root None,
        "lazy_write_symbol_info_init" )
    | (_, Write_symbol_info_with_state load_state_approach) ->
      ( lazy_write_symbol_info_init genv env root (Some load_state_approach),
        "lazy_write_symbol_info_init with state" )
  in
  CgroupProfiler.step_group init_method_name ~log:true init_method
