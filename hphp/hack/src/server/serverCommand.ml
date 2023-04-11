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

exception Nonfatal_rpc_exception of Exception.t * ServerEnv.env

(* Some client commands require full check to be run in order to update global
 * state that they depend on *)
let rpc_command_needs_full_check : type a. a t -> bool =
 fun msg ->
  match msg with
  (* global error list is not updated during small checks *)
  | STATUS _ -> true
  | LIST_FILES_WITH_ERRORS
  | REMOVE_DEAD_FIXMES _
  | REMOVE_DEAD_UNSAFE_CASTS
  | CODEMOD_SDT _ ->
    true (* need same information as STATUS *)
  | REWRITE_LAMBDA_PARAMETERS _ -> true
  | REWRITE_TYPE_PARAMS_TYPE _ -> true
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
  | RENAME _ -> true
  | RENAME_CHECK_SD _ -> true
  | IDE_RENAME _ -> true
  (* Same case as Ai commands *)
  | CREATE_CHECKPOINT _ -> true
  | RETRIEVE_CHECKPOINT _ -> true
  | DELETE_CHECKPOINT _ -> true
  | IN_MEMORY_DEP_TABLE_SIZE -> true
  | NO_PRECHECKED_FILES -> true
  | GEN_PREFETCH_DIR _ -> false
  | GEN_REMOTE_DECLS_FULL -> false
  | GEN_REMOTE_DECLS_INCREMENTAL -> false
  | GEN_SHALLOW_DECLS_DIR _ -> false
  | STATS -> false
  | DISCONNECT -> false
  | STATUS_SINGLE _ -> false
  | INFER_TYPE _ -> false
  | INFER_TYPE_BATCH _ -> false
  | INFER_TYPE_ERROR _ -> false
  | IS_SUBTYPE _ -> false
  | TAST_HOLES _ -> false
  | TAST_HOLES_BATCH _ -> false
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
  | FORMAT _ -> false
  | DUMP_FULL_FIDELITY_PARSE _ -> false
  | IDE_AUTOCOMPLETE _ -> false
  | CODE_ACTIONS _ -> false
  | OUTLINE _ -> false
  | IDE_IDLE -> false
  | RAGE -> false
  | CST_SEARCH _ -> false
  | SEARCH _ -> false
  | OPEN_FILE _ -> false
  | CLOSE_FILE _ -> false
  | EDIT_FILE _ -> false
  | FUN_DEPS_BATCH _ -> false
  | DEPS_OUT_BATCH _ -> false
  | FILE_DEPENDENTS _ -> true
  | IDENTIFY_TYPES _ -> false
  | EXTRACT_STANDALONE _ -> false
  | CONCATENATE_ALL _ -> true
  | GO_TO_DEFINITION _ -> false
  | PREPARE_CALL_HIERARCHY _ -> false
  | CALL_HIERARCHY_INCOMING_CALLS _ -> true
  | CALL_HIERARCHY_OUTGOING_CALLS _ -> false
  | PAUSE true -> false
  (* when you unpause, then it will catch up *)
  | PAUSE false -> true
  | GLOBAL_INFERENCE _ -> true
  | VERBOSE _ -> false
  | DEPS_IN_BATCH _ -> true

let command_needs_full_check = function
  | Rpc (_metadata, x) -> rpc_command_needs_full_check x
  | Debug_DO_NOT_USE -> failwith "Debug_DO_NOT_USE"

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
  | Debug_DO_NOT_USE -> failwith "Debug_DO_NOT_USE"

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
      ServerTypeCheck.(type_check genv env CheckKind.Full start_time profiling)
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
      CgroupProfiler.step_group "Full_check" ~log:true
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
            (Typing_deps.allow_dependency_table_reads mode deptable_unlocked
              : bool)))

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
    || ServerEnv.(is_full_check_done env.full_check_status));

  (* There might be additional rechecking required when there are unsaved IDE
   * changes and we asked for an answer that requires ignoring those.
   * This is very rare. *)
  let env = full_recheck_if_needed genv env msg in
  ClientProvider.track
    client
    ~key:Connection_tracker.Server_done_full_recheck
    ~long_delay_okay:true;

  match msg with
  | Rpc (_, cmd) ->
    ClientProvider.ping client;
    let t_start = Unix.gettimeofday () in
    ClientProvider.track
      client
      ~key:Connection_tracker.Server_start_handle
      ~time:t_start;
    Sys_utils.start_gc_profiling ();
    Full_fidelity_parser_profiling.start_profiling ();

    let (new_env, response) =
      try ServerRpc.handle ~is_stale genv env cmd with
      | exn ->
        let e = Exception.wrap exn in
        if ServerCommandTypes.is_critical_rpc cmd then
          Exception.reraise e
        else
          raise (Nonfatal_rpc_exception (e, env))
    in

    let parsed_files = Full_fidelity_parser_profiling.stop_profiling () in
    ClientProvider.track
      client
      ~key:Connection_tracker.Server_end_handle
      ~log:true;
    let (major_gc_time, minor_gc_time) = Sys_utils.get_gc_time () in
    HackEventLogger.handled_command
      (ServerCommandTypesUtils.debug_describe_t cmd)
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
  | Debug_DO_NOT_USE -> failwith "Debug_DO_NOT_USE"

let handle
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (client : ClientProvider.client) :
    ServerEnv.env ServerUtils.handle_command_result =
  (* In the case if LSP, it's normal that this [Server_waiting_for_cmd]
     track happens on a per-message basis, much later than the previous
     [Server_got_connection_type] track that happened when the persistent
     connection was established; the flag [long_delay_okay]
     means that the default behavior, of alarming log messages in case of delays,
     will be suppressed. *)
  ClientProvider.track
    client
    ~key:Connection_tracker.Server_waiting_for_cmd
    ~long_delay_okay:(ClientProvider.is_persistent client);

  let msg = ClientProvider.read_client_msg client in

  (* This is a helper to update progress.json to things like "[HackIDE:idle done]" or "[HackAst:--type-at-pos check]"
     or "[HackAst:--type-at-pos]". We try to balance something useful to the user, with something that helps the hack
     team know what's going on and who is to blame.
     The form is [FROM:CMD PHASE].
     FROM is the --from argument passed at the command-line, or "HackIDE" in case of LSP requests.
     CMD is the "--type-at-pos" or similar command-line argument that gave rise to serverRpc, or something sensible for LSP.
     PHASE is empty at the start, "done" once we've finished handling, "write" if Needs_writes, "check" if Needs_full_recheck. *)
  let send_progress phase =
    ServerProgress.write
      ~include_in_logs:false
      "%s%s"
      (ServerCommandTypesUtils.status_describe_cmd msg)
      phase
  in

  (* Once again, it's expected that [Server_got_cmd] happens a long time
     after we started waiting for one! *)
  ClientProvider.track
    client
    ~key:Connection_tracker.Server_got_cmd
    ~log:true
    ~msg:
      (Printf.sprintf
         "%s [%s]"
         (ServerCommandTypesUtils.debug_describe_cmd msg)
         (ClientProvider.priority_to_string client))
    ~long_delay_okay:(ClientProvider.is_persistent client);
  let env = { env with ServerEnv.remote = force_remote msg } in
  let full_recheck_needed = command_needs_full_check msg in
  let is_stale =
    ServerEnv.(env.last_recheck_loop_stats.RecheckLoopStats.updates_stale)
  in

  let handle_command =
    send_progress "";
    let r = actually_handle genv client msg full_recheck_needed ~is_stale in
    send_progress " done";
    r
  in

  if commands_needs_writes msg then begin
    (* IDE edits can come in quick succession and be immediately followed
     * by time sensitivie queries (like autocomplete). There is a constant cost
     * to stopping and resuming the global typechecking jobs, which leads to
     * flaky experience. To avoid this, we don't restart the global rechecking
     * after IDE edits - you need to save the file again to restart it. *)
    send_progress " write";
    ServerUtils.Needs_writes
      {
        env;
        finish_command_handling = handle_command;
        recheck_restart_is_needed = not (is_edit msg);
        reason = ServerCommandTypesUtils.debug_describe_cmd msg;
      }
  end else if full_recheck_needed then begin
    send_progress " typechecking";
    ServerUtils.Needs_full_recheck
      { env; finish_command_handling = handle_command; reason = reason msg }
  end else
    ServerUtils.Done (handle_command env)
