(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Utils
open ServerCommandTypes

exception Nonfatal_rpc_exception of exn * string * ServerEnv.env

(* Some client commands require full check to be run in order to update global
 * state that they depend on *)
let rpc_command_needs_full_check : type a. a t -> bool =
 fun msg ->
  match msg with
  (* global error list is not updated during small checks *)
  | STATUS _ -> true
  | LIST_FILES_WITH_ERRORS -> true (* Same as STATUS *)
  | REMOVE_DEAD_FIXMES _ -> true (* needs same information as STATUS *)
  | REWRITE_LAMBDA_PARAMETERS _ -> true
  | REWRITE_TYPE_PARAMS_TYPE _ -> true
  (* some Ai stuff - calls to those will likely never be interleaved with IDE
   * file sync commands (and resulting small checks), but putting it here just
   * to be safe *)
  | AI_QUERY _ -> true
  (* Finding references/implementations uses global dependency table *)
  | FIND_REFS _ -> true
  | GO_TO_IMPL _ -> true
  | IDE_FIND_REFS _ -> true
  | IDE_GO_TO_IMPL _ -> true
  | METHOD_JUMP (_, _, find_children) -> find_children (* uses find refs *)
  | SAVE_NAMING _ -> false
  | SAVE_STATE _ -> true
  (* COVERAGE_COUNTS (unnecessarily) uses GlobalStorage, so it cannot safely run
   * during interruptions *)
  | COVERAGE_COUNTS _ -> true
  (* Codebase-wide rename, uses find references *)
  | REFACTOR _ -> true
  | IDE_REFACTOR _ -> true
  (* Same case as Ai commands *)
  | CREATE_CHECKPOINT _ -> true
  | RETRIEVE_CHECKPOINT _ -> true
  | DELETE_CHECKPOINT _ -> true
  | IN_MEMORY_DEP_TABLE_SIZE -> true
  | NO_PRECHECKED_FILES -> true
  (* Dump codebase-wide dependency graph information *)
  | GEN_HOT_CLASSES _ -> true
  | STATS -> false
  | DISCONNECT -> false
  | STATUS_SINGLE _ -> false
  | INFER_TYPE _ -> false
  | INFER_TYPE_BATCH _ -> false
  | INFER_TYPE_ERROR _ -> false
  | IDE_HOVER _ -> false
  | DOCBLOCK_AT _ -> false
  | DOCBLOCK_FOR_SYMBOL _ -> false
  | IDE_SIGNATURE_HELP _ -> false
  | COVERAGE_LEVELS _ -> false
  | COMMANDLINE_AUTOCOMPLETE _ -> false
  | IDENTIFY_FUNCTION _ -> false
  | IDENTIFY_SYMBOL _ -> false
  | METHOD_JUMP_BATCH _ -> false
  | IDE_HIGHLIGHT_REFS _ -> false
  | DUMP_SYMBOL_INFO _ -> false
  | LINT _ -> false
  | LINT_STDIN _ -> false
  | LINT_ALL _ -> false
  | LINT_XCONTROLLER _ -> false
  | FORMAT _ -> false
  | DUMP_FULL_FIDELITY_PARSE _ -> false
  | IDE_AUTOCOMPLETE _ -> false
  | IDE_FFP_AUTOCOMPLETE _ -> false
  | SUBSCRIBE_DIAGNOSTIC _ -> false
  | UNSUBSCRIBE_DIAGNOSTIC _ -> false
  | OUTLINE _ -> false
  | IDE_IDLE -> false
  | RAGE -> false
  | DYNAMIC_VIEW _ -> false
  | CST_SEARCH _ -> false
  | SEARCH _ -> false
  | OPEN_FILE _ -> false
  | CLOSE_FILE _ -> false
  | EDIT_FILE _ -> false
  | FUN_DEPS_BATCH _ -> false
  | FILE_DEPENDENTS _ -> true
  | IDENTIFY_TYPES _ -> false
  | EXTRACT_STANDALONE _ -> false
  | CONCATENATE_ALL _ -> true
  | GO_TO_DEFINITION _ -> false
  | BIGCODE _ -> false
  | PAUSE true -> false
  (* when you unpause, then it will catch up *)
  | PAUSE false -> true
  | GLOBAL_INFERENCE _ -> true
  | VERBOSE _ -> false

let command_needs_full_check = function
  | Rpc (_metadata, x) -> rpc_command_needs_full_check x
  | Debug -> false

let is_edit : type a. a command -> bool = function
  | Rpc (_metadata, EDIT_FILE _) -> true
  | _ -> false

let rpc_command_needs_writes : type a. a t -> bool = function
  | OPEN_FILE _ -> true
  | EDIT_FILE _ -> true
  | CLOSE_FILE _ -> true
  (* DISCONNECT involves CLOSE-ing all previously opened files *)
  | DISCONNECT -> true
  | _ -> false

let commands_needs_writes = function
  | Rpc (_metadata, x) -> rpc_command_needs_writes x
  | _ -> false

let full_recheck_if_needed' genv env reason profiling =
  if
    ServerEnv.(is_full_check_done env.full_check_status)
    && Relative_path.Set.is_empty env.ServerEnv.ide_needs_parsing
  then
    env
  else
    let () = Hh_logger.log "Starting a blocking type-check due to %s" reason in
    let start_time = Unix.gettimeofday () in
    let env = { env with ServerEnv.can_interrupt = false } in
    let (env, _res, _telemetry) =
      ServerTypeCheck.(type_check genv env Full_check start_time profiling)
    in
    let env = { env with ServerEnv.can_interrupt = true } in
    assert (ServerEnv.(is_full_check_done env.full_check_status));
    env

let force_remote = function
  | Rpc (_metadata, STATUS status) -> status.remote
  | _ -> false

let ignore_ide = function
  | Rpc (_metadata, STATUS status) -> status.ignore_ide
  | _ -> false

let apply_changes env changes =
  Relative_path.Map.fold changes ~init:env ~f:(fun path content env ->
      ServerFileSync.open_file
        ~predeclare:false
        env
        (Relative_path.to_absolute path)
        content)

let get_unsaved_changes env =
  let changes = ServerFileSync.get_unsaved_changes env in
  Relative_path.Map.(map ~f:fst changes, map ~f:snd changes)

let reason = ServerCommandTypesUtils.debug_describe_cmd

let full_recheck_if_needed genv env msg =
  if ignore_ide msg then
    let (ide, disk) = get_unsaved_changes env in
    let env = apply_changes env disk in
    let env =
      CgroupProfiler.profile_memory ~event:(`Recheck "Full_check")
      @@ full_recheck_if_needed'
           genv
           { env with ServerEnv.remote = force_remote msg }
           (reason msg)
    in
    apply_changes env ide
  else
    env

(****************************************************************************)
(* Called by the server *)
(****************************************************************************)

(* Only grant access to dependency table to commands that declared that they
 * need full check - without full check, there are no guarantees about
 * dependency table being up to date. *)
let with_dependency_table_reads mode full_recheck_needed f =
  let deptable_unlocked =
    if full_recheck_needed then
      Some (Typing_deps.allow_dependency_table_reads mode true)
    else
      None
  in
  try_finally ~f ~finally:(fun () ->
      Option.iter deptable_unlocked ~f:(fun deptable_unlocked ->
          ignore
            ( Typing_deps.allow_dependency_table_reads mode deptable_unlocked
              : bool )))

(* Construct a continuation that will finish handling the command and update
 * the environment. Server can execute the continuation immediately, or store it
 * to be completed later (when full recheck is completed, when workers are
 * available, when current recheck is cancelled... *)
let actually_handle genv client msg full_recheck_needed ~is_stale env =
  Hh_logger.debug "SeverCommand.actually_handle preamble";
  with_dependency_table_reads env.ServerEnv.deps_mode full_recheck_needed
  @@ fun () ->
  Errors.ignore_ @@ fun () ->
  assert (
    (not full_recheck_needed)
    || ServerEnv.(is_full_check_done env.full_check_status) );

  (* There might be additional rechecking required when there are unsaved IDE
   * changes and we asked for an answer that requires ignoring those.
   * This is very rare. *)
  let env = full_recheck_if_needed genv env msg in
  ClientProvider.track client ~key:Connection_tracker.Server_done_full_recheck;

  match msg with
  | Rpc ({ ServerCommandTypes.from; _ }, cmd) ->
    let cmd_string = ServerCommandTypesUtils.debug_describe_t cmd in
    Hh_logger.debug
      "ServerCommand.actually_handle rpc %s, --from %s"
      cmd_string
      from;
    ClientProvider.ping client;
    let t_start = Unix.gettimeofday () in
    ClientProvider.track
      client
      ~key:Connection_tracker.Server_start_handle
      ~time:t_start;
    Sys_utils.start_gc_profiling ();
    Full_fidelity_parser_profiling.start_profiling ();
    let (new_env, response) =
      try ServerRpc.handle ~is_stale genv env cmd
      with e ->
        let stack = Caml.Printexc.get_raw_backtrace () in
        if ServerCommandTypes.is_critical_rpc cmd then
          Caml.Printexc.raise_with_backtrace e stack
        else
          raise
            (Nonfatal_rpc_exception
               (e, Caml.Printexc.raw_backtrace_to_string stack, env))
    in
    let parsed_files = Full_fidelity_parser_profiling.stop_profiling () in
    ClientProvider.track client ~key:Connection_tracker.Server_end_handle;
    let t_end = Unix.gettimeofday () in
    ClientProvider.track
      client
      ~key:Connection_tracker.Server_end_handle2
      ~time:t_end;
    let (major_gc_time, minor_gc_time) = Sys_utils.get_gc_time () in
    let lvl =
      if ClientProvider.is_persistent client then
        Hh_logger.Level.Debug
      else
        Hh_logger.Level.Info
    in
    Hh_logger.log
      ~lvl
      "Handled %s priority: %s [%f ms]"
      (ClientProvider.priority_to_string client)
      cmd_string
      (t_end -. t_start);
    HackEventLogger.handled_command
      cmd_string
      ~start_t:t_start
      ~major_gc_time
      ~minor_gc_time
      ~parsed_files;
    ClientProvider.send_response_to_client client response;
    if
      ServerCommandTypes.is_disconnect_rpc cmd
      || (not @@ ClientProvider.is_persistent client)
    then
      ClientProvider.shutdown_client client;
    new_env
  | Debug ->
    let (ic, oc) = ClientProvider.get_channels client in
    genv.ServerEnv.debug_channels <- Some (ic, oc);
    ServerDebug.say_hello genv;
    env

let handle
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (client : ClientProvider.client) :
    ServerEnv.env ServerUtils.handle_command_result =
  ClientProvider.track client ~key:Connection_tracker.Server_waiting_for_cmd;
  let msg = ClientProvider.read_client_msg client in
  ClientProvider.track client ~key:Connection_tracker.Server_got_cmd;
  ServerProgress.send_progress_to_monitor_w_timeout
    "%s"
    (ServerCommandTypesUtils.status_describe_cmd msg);
  let env = { env with ServerEnv.remote = force_remote msg } in
  let full_recheck_needed = command_needs_full_check msg in
  let is_stale = ServerEnv.(env.last_recheck_loop_stats.updates_stale) in
  let continuation =
    actually_handle genv client msg full_recheck_needed ~is_stale
  in
  if commands_needs_writes msg then
    (* IDE edits can come in quick succession and be immediately followed
     * by time sensitivie queries (like autocomplete). There is a constant cost
     * to stopping and resuming the global typechecking jobs, which leads to
     * flaky experience. To avoid this, we don't restart the global rechecking
     * after IDE edits - you need to save the file again to restart it. *)
    ServerUtils.Needs_writes
      ( env,
        continuation,
        not (is_edit msg),
        ServerCommandTypesUtils.debug_describe_cmd msg )
  else if full_recheck_needed then
    ServerUtils.Needs_full_recheck (env, continuation, reason msg)
  else
    ServerUtils.Done (continuation env)
