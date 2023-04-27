(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix

(*****************************************************************************)
(* Recheck loop types. *)
(*****************************************************************************)
type seconds = float

type seconds_since_epoch = float

module RecheckLoopStats = struct
  type t = {
    updates_stale: bool;
        (** Watchman subscription has gone down, so state of the world after the
          recheck loop may not reflect what is actually on disk. *)
    per_batch_telemetry: Telemetry.t list;
    total_changed_files_count: int;
        (** Count of files changed on disk and potentially also in the IDE. *)
    total_rechecked_count: int;
    last_iteration_start_time: seconds_since_epoch;
    duration: seconds;
    time_first_result: seconds_since_epoch option;
    recheck_id: string;
    any_full_checks: bool;
  }

  let empty ~(recheck_id : string) : t =
    {
      updates_stale = false;
      per_batch_telemetry = [];
      total_changed_files_count = 0;
      total_rechecked_count = 0;
      last_iteration_start_time = 0.;
      duration = 0.;
      time_first_result = None;
      recheck_id;
      any_full_checks = false;
    }

  (** The format of this json is user-facing, returned from 'hh check --json' *)
  let to_user_telemetry (stats : t) : Telemetry.t =
    let {
      updates_stale;
      per_batch_telemetry;
      total_changed_files_count;
      total_rechecked_count;
      last_iteration_start_time;
      duration;
      time_first_result;
      recheck_id;
      any_full_checks;
    } =
      stats
    in
    Telemetry.create ()
    |> Telemetry.string_ ~key:"id" ~value:recheck_id
    |> Telemetry.float_ ~key:"time" ~value:duration
    |> Telemetry.int_ ~key:"count" ~value:total_rechecked_count
    |> Telemetry.int_
         ~key:"total_changed_files_count"
         ~value:total_changed_files_count
    |> Telemetry.object_list
         ~key:"per_batch"
         ~value:(List.rev per_batch_telemetry)
    |> Telemetry.bool_ ~key:"updates_stale" ~value:updates_stale
    |> Telemetry.bool_ ~key:"any_full_checks" ~value:any_full_checks
    |> Telemetry.int_opt
         ~key:"time_to_first_result_ms"
         ~value:
           ( time_first_result >>| fun time ->
             int_of_float ((time -. last_iteration_start_time) *. 1000.) )

  (** Update field [time_first_result] if given timestamp is the
      first sent result. *)
  let record_result_sent_ts stats new_result_sent_ts =
    {
      stats with
      time_first_result =
        Option.first_some stats.time_first_result new_result_sent_ts;
    }
end

(*****************************************************************************)

(** The "static" environment, initialized first and then doesn't change *)
type genv = {
  options: ServerArgs.options;
  config: ServerConfig.t;
  local_config: ServerLocalConfig.t;
  workers: MultiWorker.worker list option;
  notifier: ServerNotifier.t;
  indexer: (string -> bool) -> unit -> string list;
      (** Returns the list of files under .hhconfig, subject to a filter *)
  mutable debug_channels: (Timeout.in_channel * Out_channel.t) option;
}

(*****************************************************************************)
(** The environment constantly maintained by the server *)

(*****************************************************************************)

type full_check_status =
  | Full_check_needed
      (** Some updates have not been fully processed. We get into this state every
          time file contents change (on disk, or through IDE notifications).
          Operations that depend on global state (like taking full error list, or
          looking up things in dependency table) will have stale results. *)
  | Full_check_started
      (** Same as above, except server will actively try to process outstanding
          changes (by going into ServerTypeCheck from main loop - this might need to
          be repeated several times before progressing to Full_check_done, due to
          ability to interrupt typecheck jobs).
          Server starts in this state, and we also enter it from Full_check_needed
          whenever there is a command requiring full check pending, or when user
          saves a file. *)
  | Full_check_done  (** All the changes have been fully processed. *)
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

module Init_telemetry = struct
  type reason =
    | Init_lazy_dirty
    | Init_typecheck_disabled_after_init
    | Init_eager
    | Init_lazy_full
    | Init_prechecked_fanout
  [@@deriving show { with_path = false }]

  type t = {
    reason: reason;
    telemetry: Telemetry.t;
  }

  let make reason telemetry = { reason; telemetry }

  let get { reason; telemetry } =
    telemetry |> Telemetry.string_ ~key:"reason" ~value:(show_reason reason)

  let get_reason { reason; _ } = show_reason reason
end

(** How "old" a given saved state is relative to the working revision.
    That is, a saved state has some corresponding revision X, and `hh` is invoked on a commit
    of mergebase Y.

    Distance tracks the number of revisions between X and Y, using globalrev.
    Age tracks the time elapsed between X and Y in seconds, according to hg log data.
*)
type saved_state_delta = {
  distance: int;
  age: int;
}
[@@deriving show]

(** In addition to this environment, many functions are storing and
    updating ASTs, NASTs, and types in a shared space
    (see respectively Parser_heap, Naming_table, Typing_env).
    The Ast.id are keys to index this shared space. *)
type env = {
  naming_table: Naming_table.t;
  deps_mode: Typing_deps_mode.t; [@opaque]
  typing_service: typing_service;
  tcopt: TypecheckerOptions.t;
  popt: ParserOptions.t;
  gleanopt: GleanOptions.t;
  swriteopt: SymbolWriteOptions.t;
  errorl: Errors.t; [@opaque]
      (** Errors are indexed by files that were known to GENERATE errors in
          corresponding phases. Note that this is different from HAVING errors -
          it's possible for checking of A to generate error in B - in this case
          Errors.get_failed_files Typing should contain A, not B.
          Conversly, if declaring A will require declaring B, we should put
          B in failed decl. Same if checking A will cause declaring B (via lazy
          decl).

          During recheck, we add those files to the set of files to reanalyze
          at each stage in order to regenerate their error lists. So those
          failed_ sets are the main piece of mutable state that incremental mode
          needs to maintain - the errors themselves are more of a cache, and should
          always be possible to be regenerated based on those sets. *)
  failed_naming: Relative_path.Set.t;
      (** failed_naming is used as kind of a dependency tracking mechanism:
          if files A.php and B.php both define class C, then those files are
          mutually depending on each other (edit to one might resolve naming
          ambiguity and change the interpretation of the other). Both of those
          files being inside failed_naming is how we track the need to
          check for this condition.

          See test_naming_errors.ml and test_failed_naming.ml *)
  ide_idle: bool;  (** Whether last received IDE command was IDE_IDLE *)
  last_command_time: float;
      (** Timestamp of last IDE file synchronization command *)
  last_notifier_check_time: float;
      (** Timestamp of last query for disk changes *)
  last_idle_job_time: float;  (** Timestamp of last ServerIdle.go run *)
  editor_open_files: Relative_path.Set.t;
      (** The map from full path to synchronized file contents *)
  ide_needs_parsing: Relative_path.Set.t;
      (** Files which parse trees were invalidated (because they changed on disk
          or in editor) and need to be re-parsed *)
  disk_needs_parsing: Relative_path.Set.t;
  clock: Watchman.clock option;
      (** This is the clock as of when disk_needs_parsing was last updated.
      None if not using Watchman. *)
  needs_phase2_redecl: Relative_path.Set.t;
      (** Declarations that became invalidated and moved to "old" part of the heap.
          We keep them there to be used in "determining changes" step of recheck.
          (when they are compared to "new" versions). Depending on lazy decl to
          compute "new" versions in all the other scenarios (like IDE queries) *)
  needs_recheck: Relative_path.Set.t;
      (** Files that need to be typechecked before commands that depend on global
          state (like full list of errors, build, or find all references) can be
          executed . After full check this should be empty, unless that check was
          cancelled mid-flight, in which case full_check_status will be set to
          Full_check_started and entire thing will be retried on next iteration. *)
  why_needs_server_type_check: string * string;
  init_env: init_env;
  full_recheck_on_file_changes: full_recheck_on_file_changes;
      (** Set by `hh --pause` or `hh --resume`. Indicates whether full/global recheck
          should be triggered on file changes. If paused, it would still be triggered
          by commands that require a full recheck, such as STATUS, i.e., `hh`
          on the command line. *)
  full_check_status: full_check_status;
  prechecked_files: prechecked_files_status;
  changed_files: Relative_path.Set.t;
  can_interrupt: bool;
      (** Not every caller of rechecks expects that they can be interrupted,
          so making it opt-in by setting this flag at call site *)
  interrupt_handlers:
    genv ->
    env ->
    (Unix.file_descr * env MultiThreadedCall.interrupt_handler) list;
  remote: bool;  (** Whether we should force remote type checking or not *)
  pending_command_needs_writes: (env -> env) option;
      (** When persistent client sends a command that cannot be handled (due to
          thread safety, e.g. opening, closing, editing files)
          we put the continuation that finishes handling it here. *)
  persistent_client_pending_command_needs_full_check:
    ((env -> env) * string) option;
      (** When the persistent client sends a command that cannot be immediately handled
          (due to needing full check) we put the continuation that finishes handling
          it here. The string specifies a reason why this command needs full
          recheck (for logging/debugging purposes) *)
  nonpersistent_client_pending_command_needs_full_check:
    ((env -> env) * string * ClientProvider.client) option;
      [@opaque]
      (** When a non-persistent client sends a command that cannot be immediately handled
          (due to needing full check) we put the continuation that finishes handling
          it here. The string specifies a reason why this command needs full
          recheck (for logging/debugging purposes) *)
  diagnostic_pusher: Diagnostic_pusher.t;
      (** Structure tracking errors to push to LSP client. *)
  last_recheck_loop_stats: RecheckLoopStats.t; [@opaque]
  last_recheck_loop_stats_for_actual_work: RecheckLoopStats.t option; [@opaque]
  local_symbol_table: SearchUtils.si_env; [@opaque]
      (** Symbols for locally changed files *)
  package_info: Package.Info.t; [@opaque]
      (** Function for determining which package a module belongs to *)
}
[@@deriving show]

(** Global rechecks in response to file changes can be paused. If the user
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
  dirty_local_deps: Typing_deps.DepSet.t;
      (** We are rechecking dirty files to bootstrap the dependency graph.
          After this is done we need to also recheck full fan-out (in this updated
          graph) of provided set. *)
  dirty_master_deps: Typing_deps.DepSet.t;
      (** The fan-outs of those nodes were not expanded yet. *)
  rechecked_files: Relative_path.Set.t;
      (** Files that have been rechecked since server startup *)
  clean_local_deps: Typing_deps.DepSet.t;
      (** Those deps have already been checked against their interaction with
          dirty_master_deps. Storing them here to avoid checking it over and over *)
}

(** Remote typing service. *)
and typing_service = {
  delegate_state: Typing_service_delegate_types.state; [@opaque]
  enabled: bool;
}

(** When using prechecked files we split initial typechecking in two phases
    (dirty files and a subset of their fan-out). Other init types compute the
    full fan-out up-front. *)
and prechecked_files_status =
  | Prechecked_files_disabled
  | Initial_typechecking of dirty_deps
  | Prechecked_files_ready of dirty_deps

and init_env = {
  approach_name: string;
  ci_info: Ci_util.info option Future.t option; [@opaque]
      (** The info describing the CI job the server is a part of, if any *)
  init_error: string option;
  init_id: string;
  init_start_t: float;
  init_type: string;
  mergebase: string option;
  why_needed_full_check: Init_telemetry.t option; [@opaque]
      (** This is about the first full check (if any) which was deferred after init.
      It gets reset after that first full check is completed.
      First parameter is a string label used in telemetry. Second is opaque telemetry. *)
  recheck_id: string option;
  (* Additional data associated with init that we want to log when a first full
   * check completes. *)
  saved_state_delta: saved_state_delta option;
  naming_table_manifold_path: string option;
      (** The manifold path for remote typechecker workers to download the naming table
          saved state. This value will be None in the case of full init *)
}

let list_files env =
  let acc =
    List.fold_right
      ~f:
        begin
          fun error (acc : SSet.t) ->
            let pos = User_error.get_pos error in
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

let show_clock (clock : Watchman.clock option) : string =
  Option.value clock ~default:"[noclock]"
