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

let run_search (genv : ServerEnv.genv) (env : ServerEnv.env) (t : float) :
    SearchUtils.si_env =
  let ctx = Provider_utils.ctx_from_server_env env in
  let sienv = env.local_symbol_table in
  if
    SearchServiceRunner.should_run_completely
      genv
      sienv.SearchUtils.sie_provider
  then (
    (* The duration is already logged by SearchServiceRunner *)
    let sienv = SearchServiceRunner.run_completely ctx sienv in
    HackEventLogger.update_search_end t;
    sienv
  ) else
    sienv

let save_state
    (genv : ServerEnv.genv) (env : ServerEnv.env) (output_filename : string) :
    SaveStateServiceTypes.save_state_result option =
  let ignore_errors =
    match ServerArgs.save_with_spec genv.ServerEnv.options with
    | None -> false
    | Some (spec : ServerArgs.save_state_spec_info) ->
      spec.ServerArgs.gen_with_errors
  in
  let ignore_errors =
    ignore_errors
    || ServerArgs.gen_saved_ignore_type_errors genv.ServerEnv.options
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
    let save_decls =
      genv.local_config.ServerLocalConfig.store_decls_in_saved_state
    in
    let replace_state_after_saving =
      ServerArgs.replace_state_after_saving genv.ServerEnv.options
    in
    let result =
      SaveStateService.save_state
        ~save_decls
        genv
        env
        output_filename
        ~replace_state_after_saving
    in
    Some result

let post_init genv (env, t) =
  let (env, t) = ServerAiInit.ai_check genv env.naming_table env t in
  (* Configure symbol index settings *)
  let namespace_map = GlobalOptions.po_auto_namespace_map env.tcopt in
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
          ~ignore_hh_version:(ServerArgs.ignore_hh_version genv.options)
          ~savedstate_file_opt:
            genv.local_config.ServerLocalConfig.symbolindex_file
          ~workers:genv.workers;
    }
  in
  let env = { env with local_symbol_table = run_search genv env t } in
  SharedMem.init_done ();
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
  let open ServerLocalConfig in
  let { recli_version; remote_transport_channel = transport_channel; _ } =
    genv.local_config
  in
  let { init_id; ci_info; init_start_t; _ } = env.init_env in
  ServerRemoteInit.init
    ctx
    genv.workers
    ~worker_key
    ~nonce
    ~check_id
    ~recli_version
    ~transport_channel
    ~remote_type_check_config:genv.local_config.remote_type_check
    ~ci_info
    ~init_id
    ~init_start_t
    ~bin_root
    ~root;
  (* TODO(milliechen): sample cgorup memory stats during remote init *)
  let _ = Hh_logger.log_duration "Remote type check" t in
  (env, Load_state_declined "Out-of-band naming table initialization only")

let lazy_full_init genv env profiling =
  ( ServerLazyInit.full_init genv env profiling |> post_init genv,
    Load_state_declined "No saved-state requested (for lazy init)" )

let lazy_parse_only_init genv env profiling =
  ( ServerLazyInit.parse_only_init genv env profiling |> fst,
    Load_state_declined "No saved-state requested (for lazy parse-only init)" )

let lazy_saved_state_init genv env root load_state_approach profiling =
  let result =
    ServerLazyInit.saved_state_init ~load_state_approach genv env root profiling
  in
  (* Saved-state init is the only kind of init that might error... *)
  match result with
  | Ok (res, ({ state_distance; _ }, _)) ->
    (post_init genv res, Load_state_succeeded state_distance)
  | Error err ->
    let ( State_loader.
            { message; auto_retry; stack = Utils.Callstack stack; environment }
        as verbose_error ) =
      load_state_error_to_verbose_string err
    in
    let (next_step_descr, next_step) =
      match (genv.local_config.SLC.require_saved_state, auto_retry) with
      | (true, true) -> ("retry", Exit_status.Failed_to_load_should_retry)
      | (true, false) -> ("fatal", Exit_status.Failed_to_load_should_abort)
      | (false, _) -> ("fallback", Exit_status.No_error)
    in
    let user_message = Printf.sprintf "%s [%s]" message next_step_descr in
    let exception_telemetry =
      Telemetry.create ()
      |> Telemetry.string_ ~key:"message" ~value:message
      |> Telemetry.string_ ~key:"stack" ~value:stack
      |> Telemetry.string_
           ~key:"environment"
           ~value:(Option.value environment ~default:"N/A")
    in
    HackEventLogger.load_state_exn exception_telemetry;
    Hh_logger.log
      "Could not load saved state: %s\n%s\n"
      (State_loader.show_verbose_error verbose_error)
      stack;
    (match next_step with
    | Exit_status.No_error ->
      ServerProgress.send_progress_warning_to_monitor (Some user_message);
      (* print the memory stats for saved-state init gathered before it failed *)
      CgroupProfiler.print_summary_memory_table ~event:`Init;
      let fall_back_to_full_init profiling =
        ServerLazyInit.full_init genv env profiling |> post_init genv
      in
      ( CgroupProfiler.profile_memory ~event:(`Init "lazy_full_init")
        @@ fall_back_to_full_init,
        Load_state_failed (user_message, Utils.Callstack stack) )
    | _ -> Exit.exit ~msg:user_message ~stack next_step)

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

let lazy_write_symbol_info_init genv env profiling =
  ( ServerLazyInit.write_symbol_info_init genv env profiling |> post_init genv,
    Load_state_declined "Write Symobl info state" )

(* entry point *)
let init
    ~(init_approach : init_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * init_result =
  Provider_backend.set_shared_memory_backend ();
  let lazy_lev = get_lazy_level genv in
  let root = ServerArgs.root genv.options in
  let (lazy_lev, init_approach) =
    if GlobalOptions.tco_global_inference env.tcopt then (
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
      ( lazy_saved_state_init genv env root load_state_approach,
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
      (lazy_write_symbol_info_init genv env, "lazy_write_symbol_info_init")
  in
  CgroupProfiler.profile_memory ~event:(`Init init_method_name) init_method
