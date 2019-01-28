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

let tls_bug_re = Str.regexp_string "fburl.com/tls_debug"

let matches_re re s =
  let pos = try Str.search_forward re s 0 with Caml.Not_found -> -1 in
  pos > -1

let run_search (genv: ServerEnv.genv) (t: float) : unit =
  if SearchServiceRunner.should_run_completely genv
  then begin
    (* The duration is already logged by SearchServiceRunner *)
    SearchServiceRunner.run_completely genv;
    HackEventLogger.update_search_end t
  end
  else ()

let save_state
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (output_filename: string) : int option =
  let ignore_errors =
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

  if not do_save_state then None
  else begin
    let tcopt = env.ServerEnv.tcopt in
    let file_info_on_disk = ServerArgs.file_info_on_disk genv.ServerEnv.options in
    let save_decls = genv.local_config.ServerLocalConfig.store_decls_in_saved_state in
    let replace_state_after_saving = ServerArgs.replace_state_after_saving genv.ServerEnv.options in
    let edges_added: int = SaveStateService.save_state
        ~tcopt
        ~file_info_on_disk
        ~save_decls
        env.ServerEnv.files_info
        env.errorl
        output_filename
        ~replace_state_after_saving in
    Some edges_added
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
    ?(load_state_approach: load_state_approach option)
    (genv: ServerEnv.genv)
  : ServerEnv.env * init_result =
  let lazy_lev = get_lazy_level genv in
  let env = ServerEnvBuild.make_env genv.config in
  let init_errors, () = Errors.do_with_context ServerConfig.filename Errors.Init begin fun() ->
      let fcl = ServerConfig.forward_compatibility_level genv.config in
      let older_than = ForwardCompatibilityLevel.greater_than fcl in
      if older_than ForwardCompatibilityLevel.current then
        let pos = Pos.make_from ServerConfig.filename in
        if older_than ForwardCompatibilityLevel.minimum
        then Errors.forward_compatibility_below_minimum pos fcl
        else Errors.forward_compatibility_not_current pos fcl
    end in
  let env = { env with
              errorl = init_errors
            } in
  let root = ServerArgs.root genv.options in
  let (env, t), init_result = match lazy_lev, load_state_approach with
    | Init, None ->
      ServerLazyInit.full_init genv env,
      Load_state_declined "No saved-state requested (for lazy init)"

    | Init, Some load_state_approach -> begin
      let result = ServerLazyInit.saved_state_init ~load_state_approach genv env root in
      (* Saved-state init is the only kind of init that might error... *)
      match result with
      | Ok ((env, t), ({state_distance; _}, _)) -> (env, t), Load_state_succeeded state_distance
      | Error err ->
        let err_str = load_state_error_to_verbose_string err in
        HackEventLogger.load_state_exn err_str;
        Hh_logger.log "Could not load saved state: %s" err_str;
        let warning = if matches_re tls_bug_re err_str
          then ClientMessages.tls_bug_msg
          else ClientMessages.load_state_not_found_msg
        in
        ServerProgress.send_to_monitor (MonitorRpc.PROGRESS_WARNING (Some warning));
        (* Fall back to type-checking everything *)
        ServerLazyInit.full_init genv env, Load_state_failed err_str
      end

    | Off, Some _
    | Decl, Some _
    | Parse, Some _ ->
      Hh_logger.log "Saved-state requested, but overridden by eager init";
      ServerEagerInit.init genv lazy_lev env,
      Load_state_declined "Saved-state requested, but overridden by eager init"

    | Off, None
    | Decl, None
    | Parse, None ->
      ServerEagerInit.init genv lazy_lev env,
      Load_state_declined "No saved-state requested"

  in
  let env, t = ServerAiInit.ai_check genv env.files_info env t in
  run_search genv t;
  SharedMem.init_done ();
  ServerUtils.print_hash_stats ();
  env, init_result
