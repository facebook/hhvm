(*
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
  (* Watchman subscription has gone down, so state of the world after the
   * recheck loop may not reflect what is actually on disk. *)
  updates_stale: bool;
  per_batch_telemetry: Telemetry.t list;
  rechecked_count: int;
  (* includes dependencies *)
  total_rechecked_count: int;
  duration: float;
  (* in seconds *)
  recheck_id: string;
  any_full_checks: bool;
}
[@@deriving show]

let empty_recheck_loop_stats ~(recheck_id : string) : recheck_loop_stats =
  {
    updates_stale = false;
    per_batch_telemetry = [];
    rechecked_count = 0;
    total_rechecked_count = 0;
    duration = 0.;
    recheck_id;
    any_full_checks = false;
  }

(** The format of this json is user-facing, returned from 'hh check --json' *)
let recheck_loop_stats_to_user_telemetry (stats : recheck_loop_stats) :
    Telemetry.t =
  Telemetry.create ()
  |> Telemetry.string_ ~key:"id" ~value:stats.recheck_id
  |> Telemetry.float_ ~key:"time" ~value:stats.duration
  |> Telemetry.int_ ~key:"count" ~value:stats.total_rechecked_count
  |> Telemetry.int_ ~key:"reparse_count" ~value:stats.rechecked_count
  |> Telemetry.object_list
       ~key:"per_batch"
       ~value:(List.rev stats.per_batch_telemetry)
  |> Telemetry.bool_ ~key:"updates_stale" ~value:stats.updates_stale
  |> Telemetry.bool_ ~key:"any_full_checks" ~value:stats.any_full_checks

(*****************************************************************************)
(* The "static" environment, initialized first and then doesn't change *)
(*****************************************************************************)

type genv = {
  options: ServerArgs.options;
  config: ServerConfig.t;
  local_config: ServerLocalConfig.t;
  (* Early-initialized workers to be used in MultiWorker jobs
   * They are initialized early to keep their heaps as empty as possible. *)
  workers: MultiWorker.worker list option;
  (* Returns the list of files under .hhconfig, subject to a filter *)
  indexer: (string -> bool) -> unit -> string list;
  (* Each time this is called, it should return the files that have changed
   * since the last invocation *)
  notifier_async: unit -> ServerNotifierTypes.notifier_changes;
  (* If this FD is readable, next call to notifier_async () should read
   * something from it. *)
  notifier_async_reader: unit -> Buffered_line_reader.t option;
  notifier: unit -> SSet.t;
  (* If daemons are spawned as part of the init process, wait for them here
   * e.g. wait until dfindlib is ready (in the case that watchman is absent) *)
  wait_until_ready: unit -> unit;
  mutable debug_channels: (Timeout.in_channel * out_channel) option;
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
[@@deriving show]

let is_full_check_done = function
  | Full_check_done -> true
  | _ -> false

let is_full_check_needed = function
  | Full_check_needed -> true
  | _ -> false

let is_full_check_started = function
  | Full_check_started -> true
  | _ -> false

(* In addition to this environment, many functions are storing and
 * updating ASTs, NASTs, and types in a shared space
 * (see respectively Parser_heap, Naming_table, Typing_env).
 * The Ast.id are keys to index this shared space.
 *)
type env = {
  naming_table: Naming_table.t;
  typing_service: typing_service;
  tcopt: TypecheckerOptions.t;
  popt: ParserOptions.t;
  gleanopt: GleanOptions.t;
  swriteopt: SymbolWriteOptions.t;
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
  errorl: Errors.t; [@opaque]
  (* failed_naming is used as kind of a dependency tracking mechanism:
   * if files A.php and B.php both define class C, then those files are
   * mutually depending on each other (edit to one might resolve naming
   * ambiguity and change the interpretation of the other). Both of those
   * files being inside failed_naming is how we track the need to
   * check for this condition.
   *
   * See test_naming_errors.ml and test_failed_naming.ml
   *)
  failed_naming: Relative_path.Set.t;
  persistent_client: ClientProvider.client option; [@opaque]
  (* Whether last received IDE command was IDE_IDLE *)
  ide_idle: bool;
  (* Timestamp of last IDE file synchronization command *)
  last_command_time: float;
  (* Timestamp of last query for disk changes *)
  last_notifier_check_time: float;
  (* Timestamp of last ServerIdle.go run *)
  last_idle_job_time: float;
  (* The map from full path to synchronized file contents *)
  editor_open_files: Relative_path.Set.t;
  (* Files which parse trees were invalidated (because they changed on disk
   * or in editor) and need to be re-parsed *)
  ide_needs_parsing: Relative_path.Set.t;
  disk_needs_parsing: Relative_path.Set.t;
  (* Declarations that became invalidated and moved to "old" part of the heap.
   * We keep them there to be used in "determining changes" step of recheck.
   * (when they are compared to "new" versions). Depending on lazy decl to
   * compute "new" versions in all the other scenarios (like IDE queries) *)
  needs_phase2_redecl: Relative_path.Set.t;
  (* Files that need to be typechecked before commands that depend on global
   * state (like full list of errors, build, or find all references) can be
   * executed . After full check this should be empty, unless that check was
   * cancelled mid-flight, in which case full_check will be set to
   * Full_check_started and entire thing will be retried on next iteration. *)
  needs_recheck: Relative_path.Set.t;
  init_env: init_env;
  (* Set by `hh --pause` or `hh --resume`. Indicates whether full/global recheck
    should be triggered on file changes. If paused, it would still be triggered
    by commands that require a full recheck, such as STATUS, i.e., `hh`
    on the command line. *)
  full_recheck_on_file_changes: full_recheck_on_file_changes;
  full_check: full_check_status;
  prechecked_files: prechecked_files_status;
  changed_files: Relative_path.Set.t;
  (* Not every caller of rechecks expects that they can be interrupted,
   * so making it opt-in by setting this flag at call site *)
  can_interrupt: bool;
  interrupt_handlers:
    genv ->
    env ->
    (Unix.file_descr * env MultiThreadedCall.interrupt_handler) list;
  (* Whether we should force remote type checking or not *)
  remote: bool;
  (* When persistent client sends a command that cannot be handled (due to
   * thread safety) we put the continuation that finishes handling it here. *)
  pending_command_needs_writes: (env -> env) option;
  (* When persistent client sends a command that cannot be immediately handled
   * (due to needing full check) we put the continuation that finishes handling
   * it here. The string specifies a reason why this command needs full
   * recheck (for logging/debugging purposes) *)
  persistent_client_pending_command_needs_full_check:
    ((env -> env) * string) option;
  (* Same as above, but for non-persistent clients *)
  default_client_pending_command_needs_full_check:
    ((env -> env) * string * ClientProvider.client) option;
      [@opaque]
  (* The diagnostic subscription information of the current client *)
  diag_subscribe: Diagnostic_subscription.t option;
  last_recheck_loop_stats: recheck_loop_stats;
  last_recheck_loop_stats_for_actual_work: recheck_loop_stats option;
  (* Symbols for locally changed files *)
  local_symbol_table: SearchUtils.si_env; [@opaque]
}
[@@deriving show]

(* Global rechecks in response to file changes can be paused. If the user
  changes the state to `Paused` during an ongoing recheck, we should cancel
  that recheck.

  The effect of the `PAUSE true` RPC during a recheck is that the recheck will
  be canceled, while the result of `PAUSE false` is that the client will wait
  for the recheck to be finished.

  NOTE:
  Interrupt handlers are currently set up and used during type checking.
  MultiWorker executor (MultiThreadedCall) selects worker file descriptors as
  well as some input channels from clients. If a client is sending an RPC over
  such a channel, the executor will call that channel's designated handler.

  `ServerMain` sets up a few of these handlers, and the one we're interested in
  for this change is the __priority__ client interrupt handler. This handler is
  only listening to RPCs sent over the priority channel. The client decides
  which RPCs are sent over the priority channel vs. the default channel.

  In the priority client interrupt handler, we actually handle the RPC that
  the client is sending. We then return the updated environment to
  the MultiWorker executor, along with the decision on whether it should
  continue executing or cancel. We examine the environment after handling
  the RPC to check whether the user paused file changes-driven global rechecks
  during the *current* recheck. If that is the case, then the current recheck
  will be canceled.

  The reason we care about whether it's the current recheck that's paused or
  some other one is because global rechecks can still happen even if
  *file changes-driven* global rechecks are paused. This is because the user
  can explicitly request a full recheck by running an `hh_client` command that
  *requires* a full recheck. There are a number of such commands, but the most
  obvious one is just `hh` (defaults to `hh check`).

  It is possible to immediately set the server into paused mode AND cancel
  the current recheck. There are two cases the user might wish to do this in:
    1) The user notices that some change they made is taking a while to
      recheck, so they want to cancel the recheck because they want to make
      further changes, or retry the recheck with the `--remote` argument
    2) While the server is in the paused state, the user explicitly starts
      a full recheck, but then decides that they want to cancel it

  In both cases, running `hh --pause` on the command line should stop
  the recheck if it's in the middle of the type checking phase.

  Note that interrupts are only getting set up for the type checking phase,
  so if the server is in the middle of, say, the redecl phase, it's not going
  to be interrupted until it gets to the type checking itself. In some cases,
  redecling can be very costly. For example, if we redecl using the folded decl
  approach, adding `extends ISomeInterface` to `IBaseInterface` that has many
  descendants (implementers and their descendants) requires redeclaring all
  the descendants of `IBaseInterface`. However, redecling using the shallow
  decl approach should be considerably less costly, therefore it may not be
  worth it to support interrupting redecling. *)
and full_recheck_on_file_changes =
  | Not_paused
  | Paused of paused_env
  | Resumed

and paused_env = { paused_recheck_id: string option }

and dirty_deps = {
  (* We are rechecking dirty files to bootstrap the dependency graph.
   * After this is done we need to also recheck full fan-out (in this updated
   * graph) of provided set. *)
  dirty_local_deps: Typing_deps.DepSet.t;
  (* The fan-outs of those nodes were not expanded yet. *)
  dirty_master_deps: Typing_deps.DepSet.t;
  (* Files that have been rechecked since server startup *)
  rechecked_files: Relative_path.Set.t;
  (* Those deps have already been checked against their interaction with
   * dirty_master_deps. Storing them here to avoid checking it over and over *)
  clean_local_deps: Typing_deps.DepSet.t;
}

and typing_service = {
  delegate_state: Typing_service_delegate.state; [@opaque]
  enabled: bool;
}

(* When using prechecked files we split initial typechecking in two phases
 * (dirty files and a subset of their fan-out). Other init types compute the
 * full fan-out up-front. *)
and prechecked_files_status =
  | Prechecked_files_disabled
  | Initial_typechecking of dirty_deps
  | Prechecked_files_ready of dirty_deps

and init_env = {
  approach_name: string;
  (* The info describing the CI job the server is a part of, if any *)
  ci_info: Ci_util.info option Future.t option; [@opaque]
  init_error: string option;
  init_id: string;
  init_start_t: float;
  init_type: string;
  mergebase: string option;
  (* Whether a full check was ever completed since init, and why it was needed *)
  why_needed_full_init: Telemetry.t option;
  recheck_id: string option;
  (* Additional data associated with init that we want to log when a first full
   * check completes. *)
  state_distance: int option;
}

let list_files env =
  let acc =
    List.fold_right
      ~f:
        begin
          fun error (acc : SSet.t) ->
          let pos = Errors.get_pos error in
          SSet.add (Relative_path.to_absolute (Pos.filename pos)) acc
        end
      ~init:SSet.empty
      (Errors.get_error_list env.errorl)
  in
  SSet.elements acc

let add_changed_files env changed_files =
  {
    env with
    changed_files = Relative_path.Set.union env.changed_files changed_files;
  }
