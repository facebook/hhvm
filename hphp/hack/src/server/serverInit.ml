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
open ServerEnv
module SLC = ServerLocalConfig
include ServerInitTypes

let post_init genv (env, _t) =
  ignore genv;
  SharedMem.SMTelemetry.init_done ();
  env

let full_init genv env profiling =
  ( ServerLazyInit.full_init genv env profiling |> post_init genv,
    Load_state_declined "No saved-state requested (for lazy init)" )

let parse_only_init genv env profiling =
  ( ServerLazyInit.parse_only_init genv env profiling |> fst,
    Load_state_declined "No saved-state requested (for lazy parse-only init)" )

let saved_state_init_error env genv ~do_indexing err =
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
  match next_step with
  | Exit_status.No_error ->
    let fall_back_to_full_init profiling =
      ServerLazyInit.full_init genv env profiling |> post_init genv
    in
    ( CgroupProfiler.step_group "full_init" ~log:true @@ fall_back_to_full_init,
      Load_state_failed (user_message, telemetry) )
  | _ -> Exit.exit ~msg:user_message ~telemetry next_step

let saved_state_init
    ~do_indexing
    genv
    env
    root
    (load_state_approach : load_state_approach)
    profiling =
  let result =
    ServerLazyInit.saved_state_init
      ~do_indexing
      ~load_state_approach
      genv
      env
      root
      profiling
  in
  match result with
  | Ok ((env, t), ({ saved_state_revs_info; _ }, _)) ->
    let env = post_init genv (env, t) in
    (env, Load_state_succeeded saved_state_revs_info)
  | Error err -> saved_state_init_error env genv ~do_indexing err

(** Write symbol info for Glean *)
let write_symbol_info_init genv env root (load_state : _ option) profiling =
  match load_state with
  | None ->
    ( ServerLazyInit.write_symbol_info_full_init genv env profiling
      |> post_init genv,
      Load_state_declined "Write Symobl info state" )
  | Some load_state_approach ->
    saved_state_init
      ~do_indexing:true
      genv
      env
      root
      load_state_approach
      profiling

let possibly_set_rust_provider_backend env genv : unit =
  if genv.local_config.ServerLocalConfig.rust_provider_backend then (
    Hh_logger.log "ServerInit: using rust backend";
    let backend =
      Hh_server_provider_backend.make
        (DeclFoldOptions.from_global_options env.tcopt)
        (DeclParserOptions.from_parser_options env.tcopt.GlobalOptions.po)
    in
    Provider_backend.set_rust_backend backend
  )

let init
    ~(init_approach : init_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env) : ServerEnv.env * init_result =
  possibly_set_rust_provider_backend env genv;
  Hh_logger.log
    "ServerInit: init_approach=%s"
    (show_init_approach init_approach);
  let (init_method, init_method_name) =
    let root = ServerArgs.root genv.options in
    match init_approach with
    | Full_init -> (full_init genv env, "full_init")
    | Saved_state_init load_state_approach ->
      ( saved_state_init ~do_indexing:false genv env root load_state_approach,
        "saved_state_init" )
    | Parse_only_init -> (parse_only_init genv env, "parse_only_init")
    | Write_symbol_info ->
      (write_symbol_info_init genv env root None, "write_symbol_info_init")
    | Write_symbol_info_with_state load_state_approach ->
      ( write_symbol_info_init genv env root (Some load_state_approach),
        "write_symbol_info_with_state" )
  in
  CgroupProfiler.step_group init_method_name ~log:true init_method
