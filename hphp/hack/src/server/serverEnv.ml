(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

(*****************************************************************************)
(* Recheck loop types. *)
(*****************************************************************************)

type recheck_loop_stats = {
  (** Watchman subscription has gone down, so state of the world after the
   * recheck loop may not reflect what is actually on disk. *)
  updates_stale : bool;
  rechecked_batches : int;
  rechecked_count : int;
  (* includes dependencies *)
  total_rechecked_count : int;
}

let empty_recheck_loop_stats = {
  updates_stale = false;
  rechecked_batches = 0;
  rechecked_count = 0;
  total_rechecked_count = 0;
}

(*****************************************************************************)
(* The "static" environment, initialized first and then doesn't change *)
(*****************************************************************************)

type genv = {
    options          : ServerArgs.options;
    config           : ServerConfig.t;
    local_config     : ServerLocalConfig.t;
    workers          : MultiWorker.worker list option;
    (* Returns the list of files under .hhconfig, subject to a filter *)
    indexer          : (string -> bool) -> (unit -> string list);
    (* Each time this is called, it should return the files that have changed
     * since the last invocation *)
    notifier_async   : unit -> ServerNotifierTypes.notifier_changes;
    (* If this FD is readable, next call to notifier_async () should read
     * something from it. *)
    notifier_async_reader : unit -> Buffered_line_reader.t option;
    notifier         : unit -> SSet.t;
    (* If daemons are spawned as part of the init process, wait for them here
     * e.g. wait until dfindlib is ready (in the case that watchman is absent) *)
    wait_until_ready : unit -> unit;
    mutable debug_channels   : (Timeout.in_channel * out_channel) option;
  }

(*****************************************************************************)
(* The environment constantly maintained by the server *)
(*****************************************************************************)

type full_check_status =
  (* Some updates have not been fully processed. We get into this state every
   * time file contents change (on disk, or through IDE notifications).
   * Operations that depend on global state (like taking full error list, or
   * looking up things in dependency table) will have stale results. *)
  | Full_check_needed
  (* Same as above, except server will actively try to process outstanding
   * changes (by going into ServerTypeCheck from main loop - this might need to
   * be repeated several times before progressing to Full_check_done, due to
   * ability to interrupt typecheck jobs).
   * Server starts in this state, and we also enter it from Full_check_needed
   * whenever there is a command requiring full check pending, or when user
   * saves a file. *)
  | Full_check_started
  (* All the changes have been fully processed. *)
  | Full_check_done

(* In addition to this environment, many functions are storing and
 * updating ASTs, NASTs, and types in a shared space
 * (see respectively Parser_heap, Naming_table, Typing_env).
 * The Ast.id are keys to index this shared space.
 *)
type env = {
    naming_table   : Naming_table.t;
    tcopt          : TypecheckerOptions.t;
    popt           : ParserOptions.t;
    (* Errors are indexed by files that were known to GENERATE errors in
     * corresponding phases. Note that this is different from HAVING errors -
     * it's possible for checking of A to generate error in B - in this case
     * Errors.get_failed_files Typing should contain A, not B.
     * Conversly, if declaring A will require declaring B, we should put
     * B in failed decl. Same if checking A will cause declaring B (via lazy
     * decl).
     *
     * During recheck, we add those files to the set of files to reanalyze
     * at each stage in order to regenerate their error lists. So those
     * failed_ sets are the main piece of mutable state that incremental mode
     * needs to maintain - the errors themselves are more of a cache, and should
     * always be possible to be regenerated based on those sets. *)
    errorl         : Errors.t;
    (* failed_naming is used as kind of a dependency tracking mechanism:
     * if files A.php and B.php both define class C, then those files are
     * mutually depending on each other (edit to one might resolve naming
     * ambiguity and change the interpretation of the other). Both of those
     * files being inside failed_naming is how we track the need to
     * check for this condition.
     *
     * See test_naming_errors.ml and test_failed_naming.ml
     *)
    failed_naming : Relative_path.Set.t;
    persistent_client : ClientProvider.client option;
    (* Whether last received IDE command was IDE_IDLE *)
    ide_idle : bool;
    (* Timestamp of last IDE file synchronization command *)
    last_command_time : float;
    (* Timestamp of last query for disk changes *)
    last_notifier_check_time : float;
    (* Timestamp of last ServerIdle.go run *)
    last_idle_job_time : float;
    (* The map from full path to synchronized file contents *)
    editor_open_files : Relative_path.Set.t;
    (* Files which parse trees were invalidated (because they changed on disk
     * or in editor) and need to be re-parsed *)
    ide_needs_parsing : Relative_path.Set.t;
    disk_needs_parsing : Relative_path.Set.t;
    (* Declarations that became invalidated and moved to "old" part of the heap.
     * We keep them there to be used in "determining changes" step of recheck.
     * (when they are compared to "new" versions). Depending on lazy decl to
     * compute "new" versions in all the other scenarios (like IDE queries) *)
    needs_phase2_redecl : Relative_path.Set.t;
    (* Files that need to be typechecked before commands that depend on global
     * state (like full list of errors, build, or find all references) can be
     * executed . After full check this should be empty, unless that check was
     * cancelled mid-flight, in which case full_check will be set to
     * Full_check_started and entire thing will be retried on next iteration. *)
    needs_recheck : Relative_path.Set.t;
    init_env : init_env;
    full_check : full_check_status;
    prechecked_files : prechecked_files_status;
    (* Not every caller of rechecks expects that they can be interrupted,
     * so making it opt-in by setting this flag at call site *)
    can_interrupt : bool;
    interrupt_handlers: genv -> env ->
      (Unix.file_descr * env MultiThreadedCall.interrupt_handler) list;
    (* When persistent client sends a command that cannot be handled (due to
     * thread safety) we put the continuation that finishes handling it here. *)
    pending_command_needs_writes : (env -> env) option;
    (* When persistent client sends a command that cannot be immediately handled
     * (due to needing full check) we put the continuation that finishes handling
     * it here. The string specifies a reason why this command needs full
     * recheck (for logging/debugging purposes) *)
    persistent_client_pending_command_needs_full_check:
      ((env -> env) * string) option;
    (* Same as above, but for non-persistent clients *)
    default_client_pending_command_needs_full_check:
      ((env -> env) * string * ClientProvider.client) option;
    (* The diagnostic subscription information of the current client *)
    diag_subscribe : Diagnostic_subscription.t option;
    recent_recheck_loop_stats : recheck_loop_stats;
  }

and dirty_deps = {
  (* We are rechecking dirty files to bootstrap the dependency graph.
   * After this is done we need to also recheck full fan-out (in this updated
   * graph) of provided set. *)
  dirty_local_deps : Typing_deps.DepSet.t;
  (* The fan-outs of those nodes were not expanded yet. *)
  dirty_master_deps : Typing_deps.DepSet.t;
  (* Files that have been rechecked since server startup *)
  rechecked_files : Relative_path.Set.t;
  (* Those deps have already been checked against their interaction with
   * dirty_master_deps. Storing them here to avoid checking it over and over *)
  clean_local_deps : Typing_deps.DepSet.t;
}

(* When using prechecked files we split initial typechecking in two phases
 * (dirty files and a subset of their fan-out). Other init types compute the
 * full fan-out up-front. *)
and prechecked_files_status =
  | Prechecked_files_disabled
  | Initial_typechecking of dirty_deps
  | Prechecked_files_ready of dirty_deps

and init_env = {
  init_start_t : float;
  (* Whether a full check was ever completed since init. *)
  needs_full_init : bool;
  (* Additional data associated with init that we want to log when a first full
   * check completes. *)
  state_distance : int option;
  approach_name : string;
  init_error : string option;
  init_type : string;
}

let list_files env =
  let acc = List.fold_right
    ~f:begin fun error (acc : SSet.t) ->
      let pos = Errors.get_pos error in
      SSet.add (Relative_path.to_absolute (Pos.filename pos)) acc
    end
    ~init:SSet.empty
    (Errors.get_error_list env.errorl) in
  SSet.elements acc
