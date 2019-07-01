(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Result.Export
open SearchServiceRunner
open ServerEnv

module SLC = ServerLocalConfig

include ServerInitTypes

let run_search
    (genv: ServerEnv.genv)
    (t: float)
    (sienv: SearchUtils.si_env ref): unit =
  if SearchServiceRunner.should_run_completely genv
    !sienv.SearchUtils.sie_provider
  then begin
    (* The duration is already logged by SearchServiceRunner *)
    SearchServiceRunner.run_completely genv sienv;
    HackEventLogger.update_search_end t
  end
  else ()

let save_state
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (output_filename: string) : ServerEnv.env * SaveStateServiceTypes.save_state_result option =
  let ignore_errors = match (ServerArgs.save_with_spec genv.ServerEnv.options) with
  | None -> false
  | Some (spec: ServerArgs.save_state_spec_info) -> spec.ServerArgs.gen_with_errors
  in
  let ignore_errors = ignore_errors ||
    ServerArgs.gen_saved_ignore_type_errors genv.ServerEnv.options in
  let has_errors = not (Errors.is_empty env.errorl) in
  let do_save_state =
    if ignore_errors then begin
      if has_errors then
        Printf.eprintf
          "WARNING: BROKEN SAVED STATE! Generating saved state. Ignoring type errors.\n%!"
      else
        Printf.eprintf "Generating saved state and ignoring type errors, but there were none.\n%!";
      true
    end else begin
      if has_errors then begin
        Printf.eprintf "Refusing to generate saved state. There are type errors\n%!";
        Printf.eprintf "and --gen-saved-ignore-type-errors was not provided.\n%!";
        false
      end else
        true
    end in

  if not do_save_state then env, None
  else begin
    let enable_naming_table_fallback =
      genv.ServerEnv.local_config.ServerLocalConfig.enable_naming_table_fallback
    in
    let save_decls = genv.local_config.ServerLocalConfig.store_decls_in_saved_state in
    let dep_table_as_blob = match (ServerArgs.save_with_spec genv.ServerEnv.options) with
    | None -> false
    | Some _ -> true
    in
    let replace_state_after_saving = ServerArgs.replace_state_after_saving genv.ServerEnv.options in
    let (env, result) = SaveStateService.save_state
        ~enable_naming_table_fallback
        ~dep_table_as_blob
        ~save_decls
        env
        output_filename
        ~replace_state_after_saving in
    env, Some result
  end

let get_lazy_level (genv: ServerEnv.genv) : lazy_level =
  let lazy_decl = Option.is_none (ServerArgs.ai_mode genv.options) in
  let lazy_parse = genv.local_config.SLC.lazy_parse in
  let lazy_initialize = genv.local_config.SLC.lazy_init in
  match lazy_decl, lazy_parse, lazy_initialize with
  | true, false, false -> Decl
  | true, true, false -> Parse
  | true, true, true -> Init
  | _ -> Off

(* entry point *)
let init
    ~(init_approach: init_approach)
    (genv: ServerEnv.genv)
  : ServerEnv.env * init_result =
  let lazy_lev = get_lazy_level genv in
  let env = ServerEnvBuild.make_env genv.config in
  (* Save the global settings for parsing, naming, and declaration.
     These settings cannot be changed during the lifetime of the server. *)
  GlobalParserOptions.set env.popt;
  GlobalNamingOptions.set env.tcopt;
  let root = ServerArgs.root genv.options in
  let (env, t), init_result, skip_post_init = match lazy_lev, init_approach with
    | Init, Full_init ->
      ServerLazyInit.full_init genv env,
      Load_state_declined "No saved-state requested (for lazy init)",
      false

    | Init, Parse_only_init ->
      ServerLazyInit.parse_only_init genv env,
      Load_state_declined "No saved-state requested (for lazy parse-only init)",
      true

    | Init, Saved_state_init load_state_approach -> begin
      let result = ServerLazyInit.saved_state_init ~load_state_approach genv env root in
      (* Saved-state init is the only kind of init that might error... *)
      match result with
      | Ok ((env, t), ({state_distance; _}, _)) ->
        (env, t), Load_state_succeeded state_distance, false
      | Error err ->
        let (msg, retry, Utils.Callstack stack) = load_state_error_to_verbose_string err in
        let next_step_descr, next_step =
          match genv.local_config.SLC.require_saved_state, retry with
          | true, true -> "retry", Exit_status.Failed_to_load_should_retry
          | true, false -> "fatal", Exit_status.Failed_to_load_should_abort
          | false, _ -> "fallback", Exit_status.No_error in
        let msg = Printf.sprintf "%s [%s]" msg next_step_descr in
        let msg_verbose = Printf.sprintf "%s\n%s" msg stack in
        HackEventLogger.load_state_exn msg_verbose;
        Hh_logger.log "Could not load saved state: %s" msg_verbose;
        if next_step = Exit_status.No_error then begin
          ServerProgress.send_to_monitor (MonitorRpc.PROGRESS_WARNING (Some msg));
          ServerLazyInit.full_init genv env, Load_state_failed msg_verbose, false
        end else begin
          let finale_data = { ServerCommandTypes.
            exit_status = next_step;
            msg;
            stack = Utils.Callstack stack;
          } in
          let finale_file = ServerFiles.server_finale_file (Unix.getpid ()) in
          begin try
            let oc = Pervasives.open_out_bin finale_file in
            Marshal.to_channel oc finale_data [];
            Pervasives.close_out oc
          with _ -> () end;
          Exit_status.exit next_step
        end
      end
    | Off, Full_init
    | Decl, Full_init
    | Parse, Full_init ->
      ServerEagerInit.init genv lazy_lev env,
      Load_state_declined "No saved-state requested",
      false
    | Off, _
    | Decl, _
    | Parse, _ ->
      Hh_logger.log "Saved-state requested, but overridden by eager init";
      ServerEagerInit.init genv lazy_lev env,
      Load_state_declined "Saved-state requested, but overridden by eager init",
      false
    | _, Write_symbol_info ->
      ServerLazyInit.write_symbol_info_init genv env,
      Load_state_declined "Write Symobl info state", false

  in
  if skip_post_init then env, init_result else
  let env, t = ServerAiInit.ai_check genv env.naming_table env t in

  (* Configure symbol index settings *)
  let namespace_map = GlobalOptions.po_auto_namespace_map env.tcopt in
  env.local_symbol_table := (SymbolIndex.initialize
      ~quiet:genv.local_config.ServerLocalConfig.symbolindex_quiet
      ~provider_name:genv.local_config.ServerLocalConfig.symbolindex_search_provider
      ~namespace_map
      ~savedstate_file_opt:genv.local_config.ServerLocalConfig.symbolindex_file
      ~workers:genv.workers);
  run_search genv t env.ServerEnv.local_symbol_table;
  SharedMem.init_done ();
  ServerUtils.print_hash_stats ();
  env, init_result
