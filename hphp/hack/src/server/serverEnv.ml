(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

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

type recheck_iteration_flag =
  (** On next iteration of recheck loop, force a flush of the notififer. *)
  | Force_flush

(*****************************************************************************)
(* The "static" environment, initialized first and then doesn't change *)
(*****************************************************************************)

type genv = {
    options          : ServerArgs.options;
    config           : ServerConfig.t;
    local_config     : ServerLocalConfig.t;
    debug_port       : Debug_port.out_port option;
    workers          : Worker.t list option;
    (* Returns the list of files under .hhconfig, subject to a filter *)
    indexer          : (string -> bool) -> (unit -> string list);
    (* Each time this is called, it should return the files that have changed
     * since the last invocation *)
    notifier_async   : unit -> ServerNotifierTypes.notifier_changes;
    notifier         : unit -> SSet.t;
    (* If daemons are spawned as part of the init process, wait for them here *)
    wait_until_ready : unit -> unit;
    mutable debug_channels   : (Timeout.in_channel * out_channel) option;
  }

(*****************************************************************************)
(* The environment constantly maintained by the server *)
(*****************************************************************************)

(* In addition to this environment, many functions are storing and
 * updating ASTs, NASTs, and types in a shared space
 * (see respectively Parser_heap, Naming_heap, Typing_env).
 * The Ast.id are keys to index this shared space.
 *)
type env = {
    files_info     : FileInfo.t Relative_path.Map.t;
    tcopt          : TypecheckerOptions.t;
    popt           : ParserOptions.t;
    errorl         : Errors.t;
    (* Sets of files that were known to GENERATE errors in corresponding phases.
     * Note that this is different from HAVING errors - it's possible for
     * checking of A to generate error in B - in this case failed_check
     * should contain A, not B.
     * Conversly, if declaring A will require declaring B, we should put
     * B in failed_decl. Same if checking A will cause declaring B (via lazy
     * decl).
     *
     * During recheck, we add those files to the set of files to reanalyze
     * at each stage in order to regenerate their error lists. So those
     * failed_ sets are the main piece of mutable state that incremental mode
     * needs to maintain - ServerEnv.errorl is more of a cache, and should
     * always be possible to be regenerated based on those sets.
     *
     * failed_naming is also used as kind of a dependency tracking mechanism:
     * if files A.php and B.php both define class C, then those files are
     * mutually depending on each other (edit to one might resolve naming
     * ambiguity and change the interpretation of the other). Both of those
     * files being inside failed_naming is how we track the need to
     * check for this condition.
     *
     * See test_naming_errors.ml and test_failed_naming.ml
     *)
    failed_parsing : Relative_path.Set.t;
    failed_naming  : Relative_path.Set.t;
    failed_decl    : Relative_path.Set.t;
    failed_check   : Relative_path.Set.t;
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
    (* Definitions that became invalidated and removed from heap. Depending
     * on lazy decl to update them on as-needed basis. Things that require
     * entire global state to be up to date (like global list of errors, build,
     * or find all references) must be preceded by Full_check. *)
    needs_phase2_redecl : Relative_path.Set.t;
    (* A set of files that we need to still redeclare during a Full Check, but
      does not need to go through decl_redecl fanout. This contains files
      that were unchanged, but had a parent class's declarations
      change unmeaningfully(positional change only).
    *)
    needs_redecl : Relative_path.Set.t;
    needs_recheck : Relative_path.Set.t;
    needs_full_check : bool;
    (* The diagnostic subscription information of the current client *)
    diag_subscribe : Diagnostic_subscription.t option;
    recent_recheck_loop_stats : recheck_loop_stats;
  }

let file_filter f =
  (FindUtils.is_php f && not (FilesToIgnore.should_ignore f))
  || FindUtils.is_js f

let list_files env oc =
  let acc = List.fold_right
    ~f:begin fun error acc ->
      let pos = Errors.get_pos error in
      Relative_path.Set.add acc (Pos.filename pos)
    end
    ~init:Relative_path.Set.empty
    (Errors.get_error_list env.errorl) in
  Relative_path.Set.iter acc (fun s ->
    Printf.fprintf oc "%s\n" (Relative_path.to_absolute s));
  flush oc
