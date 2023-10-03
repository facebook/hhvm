(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type report =
  | Move_along  (** Nothing to see here. *)
  | Restart_server
      (** Kill the server (if one is running) and start a new one. *)
[@@deriving show]

type server_state =
  | Server_not_yet_started
  | Server_alive
  | Server_dead
[@@deriving show]

type options = {
  root: Path.t;
  allow_subscriptions: bool;
      (** Disable the informant - use the dummy instead. *)
  use_dummy: bool;
      (** Don't trigger a server restart if the distance between two
            revisions we are moving between is less than this. *)
  min_distance_restart: int;
  watchman_debug_logging: bool;
      (** Informant should ignore the hh_version when doing version checks. *)
  ignore_hh_version: bool;
      (** Was the server initialized with a precomputed saved-state? *)
  is_saved_state_precomputed: bool;
}

type init_env = options

(** We need to query mercurial to convert an hg revision into a numerical
 * global revision. These queries need to be non-blocking, so we keep a cache
 * of the mapping in here, as well as the Futures corresponding to
 * th queries. *)
module Revision_map = struct
  (*
   * Running and finished queries. A query gets the global_rev for a
   * given HG Revision.
   *)
  type t = {
    global_rev_queries: (Hg.Rev.t, Hg.global_rev Future.t) Caml.Hashtbl.t;
  }

  let create () = { global_rev_queries = Caml.Hashtbl.create 200 }

  let add_query ~hg_rev root t =
    (* Don't add if we already have an entry for this. *)
    if not (Caml.Hashtbl.mem t.global_rev_queries hg_rev) then
      let future =
        Hg.get_closest_global_ancestor hg_rev (Path.to_string root)
      in
      Caml.Hashtbl.add t.global_rev_queries hg_rev future

  let find_global_rev hg_rev t =
    let future = Caml.Hashtbl.find t.global_rev_queries hg_rev in
    match Future.check_status future with
    | Future.In_progress { age } when Float.(age > 60.0) ->
      (* Fail if lookup up global rev number takes more than 60 s.
       * Delete the query so we can retry again if we encounter this hg_rev
       * again. Return fake "0" global rev number. *)
      let () = Caml.Hashtbl.remove t.global_rev_queries hg_rev in
      Some 0
    | Future.In_progress _ -> None
    | Future.Complete_with_result result ->
      let result =
        result
        |> Result.map_error ~f:Future.error_to_string
        |> Result.map_error
             ~f:(HackEventLogger.find_svn_rev_failed (Future.start_t future))
      in
      Some (result |> Result.ok |> Option.value ~default:0)
end

(**
 * The Revision tracker tracks the latest known global revision of the repo,
 * the corresponding global revisions of Hg revisions, and the sequence of
 * revision changes (from hg update). See record type "env" below.
 *
 * This machinery is necessary because Watchman state change events give
 * us only the HG Revisions of hg updates, but we need to make decisions
 * on their global Revision numbers.
 *
 * We want to be able to:
 *  1) Determine when the base global revision (in trunk) has changed
 *     "significantly" so we can inform the server monitor to trigger
 *     a server restart (since incremental typechecks can be slower
 *     than a fresh server using a saved state).
 *  2) Fulfill goal 1 without blocking, and while being "highly responsive"
 *
 * The definition of "significant" can be adjusted according to how fast
 * incremental type checking is compared to a fresh restart.
 *
 * The meaning of "highly responsive" above roughly means using a cache
 * for global revisions (because a Mercurial request to get the global revision
 * number of an HG Revision is very slow, on the order of seconds).
 * Consider the following scenario:
 *
 *   Server is running with Repo at Revision 100. User does hg update to
 *   Rev 500 (which triggers a server restart) and works on that for
 *   an hour. Then the user hg updates back to Rev 100.
 *
 * At this point, the server restart should be triggered because we
 * are moving across hundreds of revisions. Without caching the Revision
 * 100 result, the restart would be delayed by seconds. With a cache, the
 * restart is triggered immediately.
 *
 * We use an SCM Aware Watchman subscription to follow when the merge base
 * changes. We use State Enter and Leave events (which appear before the
 * slower SCM Aware notification) to kick off asynchronous computation. In
 * particular, we kick off prefetching of a saved state, and shelling out
 * to mercurial for mapping the HG Revision to its corresponding global revision
 * (since our "distance" measure uses global_revs which are
 * monotonically increasing)..
 *)
module Revision_tracker = struct
  type timestamp = float

  type repo_transition =
    | State_enter of Hg.Rev.t
    | State_leave of Hg.Rev.t
    | Changed_merge_base of
        Hg.Rev.t * (SSet.t[@printer SSet.pp_large]) * Watchman.clock
  [@@deriving show]

  let _ = show_repo_transition (* allow unused show *)

  type init_settings = {
    watchman: Watchman.watchman_instance ref;
    root: Path.t;
    min_distance_restart: int;
    is_saved_state_precomputed: bool;
  }

  type env = {
    inits: init_settings;
    (* The 'current' base revision (from the tracker's perspective of the
     * repo. This is used to make calculations on distance. This is changed
     * when a State_leave is handled. *)
    current_base_revision: int ref;
    is_in_hg_update_state: bool ref;
    is_in_hg_transaction_state: bool ref;
    rev_map: Revision_map.t;
    (*
     * Timestamp and HG revision of state change events.
     *
     * Why do we keep the entire sequence? It seems like it would be sufficient
     * to just consume from the Watchman subscription one-at-a-time,
     * processing at most one state change at a time. But consider the case
     * of many sequential hg updates, the last of which is very distant
     * (i.e. significant), and we happen to have a cached value only for
     * the last hg update.
     *
     * If we processed one-at-a-time as the updates came in, it would be many
     * seconds of processing each hg query before deciding at the final one
     * to restart the server (so the server restart is delayed by many
     * seconds).
     *
     * By keeping a running queue of state changes and "preprocessing" new
     * changes from the watchman subscription (before appending them to this
     * queue), we can catch that final hg update early on and proactively
     * trigger a server restart.
     *)
    state_changes: (repo_transition * timestamp) Queue.t;
  }

  type instance =
    | Initializing of init_settings * Hg.global_rev Future.t
    | Reinitializing of env * Hg.global_rev Future.t
    | Tracking of env

  (* Revision_tracker has lots of mutable state anyway, so might as well
   * make it responsible for maintaining its own instance. *)
  type t = instance ref

  let is_hg_updating env =
    !(env.is_in_hg_update_state) || !(env.is_in_hg_transaction_state)

  let init ~min_distance_restart ~is_saved_state_precomputed watchman root =
    let init_settings =
      {
        watchman = ref watchman;
        root;
        min_distance_restart;
        is_saved_state_precomputed;
      }
    in
    ref
    @@ Initializing
         (init_settings, Hg.current_working_copy_base_rev (Path.to_string root))

  let set_base_revision global_rev env =
    if global_rev = !(env.current_base_revision) then
      ()
    else
      let () =
        Hh_logger.log "Revision_tracker setting base rev: %d" global_rev
      in
      env.current_base_revision := global_rev

  let active_env init_settings base_global_rev =
    {
      inits = init_settings;
      current_base_revision = ref base_global_rev;
      rev_map = Revision_map.create ();
      state_changes = Queue.create ();
      is_in_hg_update_state = ref false;
      is_in_hg_transaction_state = ref false;
    }

  let reinitialized_env env base_global_rev =
    {
      env with
      current_base_revision = ref base_global_rev;
      state_changes = Queue.create ();
    }

  let get_jump_distance global_rev env =
    abs @@ (global_rev - !(env.current_base_revision))

  let is_significant ~min_distance_restart ~jump_distance elapsed_t =
    let () =
      Hh_logger.log
        "Informant: jump distance %d. elapsed_t: %2f"
        jump_distance
        elapsed_t
    in
    (* Allow up to 2 revisions per second for incremental. More than that,
     * prefer a server restart. *)
    let result =
      jump_distance > min_distance_restart
      && Float.(
           elapsed_t <= 0.0 || float_of_int jump_distance /. elapsed_t > 2.0)
    in
    result

  (** Form a decision about whether or not we'd like to start a new server.
   * transition: The state transition for which we are forming a decision
   * global_rev: The corresponding global rev for this transition's hg rev.
   *)
  let form_decision ~significant transition server_state env =
    match (significant, transition, server_state) with
    | (_, State_leave _, Server_not_yet_started) ->
      (* This is reachable when server stopped in the middle of rebase. Instead
         * of restarting immediately, we go back to Server_not_yet_started, and want
         * to restart only when hg.update state is vacated *)
      Restart_server
    | (_, State_leave _, Server_dead) ->
      (* Regardless of whether we had a significant change or not, when the
       * server is not alive, we restart it on a state leave.*)
      Restart_server
    | (false, _, _) ->
      let () = Hh_logger.log "Informant insignificant transition" in
      Move_along
    | (true, State_enter _, _)
    | (true, State_leave _, _) ->
      (* We use the State enter and leave events to kick off asynchronous
       * computations off the hg revisions when they arrive (during preprocess)
       * But actual actions are taken only on changed_merge_base below. *)
      Move_along
    | (true, Changed_merge_base _, _) ->
      (* If the current server was started using a precomputed saved-state,
       * we don't want to relaunch the server. If we do, it'll reuse
       * the previously used saved-state for the new mergebase and more
       * importantly the **same list of changed files!!!** which is
       * totally wrong *)
      if env.inits.is_saved_state_precomputed then begin
        Hh_logger.log
          "Server was started using a precomputed saved-state, not restarting.";
        Move_along
      end else
        Restart_server

  (**
   * If we have a cached global_rev for this hg_rev, make a decision on
   * this transition and returns that decision.  If not, returns None.
   *
   * Nonblocking.
   *)
  let make_decision timestamp transition server_state env =
    let hg_rev =
      match transition with
      | State_enter hg_rev
      | State_leave hg_rev
      | Changed_merge_base (hg_rev, _, _) ->
        hg_rev
    in
    match Revision_map.find_global_rev hg_rev env.rev_map with
    | None -> None
    | Some global_rev ->
      let jump_distance = get_jump_distance global_rev env in
      let elapsed_t = Unix.time () -. timestamp in
      let significant =
        is_significant
          ~min_distance_restart:env.inits.min_distance_restart
          ~jump_distance
          elapsed_t
      in
      Some (form_decision ~significant transition server_state env, global_rev)

  (**
   * Keep popping state_changes queue until we reach a non-ready result.
   *
   * Returns a list of the decisions from processing each ready change
   * in the queue; the list is ordered most-recent to oldest.
   *
   * Non-blocking.
   *)
  let rec churn_ready_changes ~acc env server_state =
    let maybe_set_base_rev transition global_rev env =
      match transition with
      | State_enter _
      | State_leave _ ->
        ()
      | Changed_merge_base _ -> set_base_revision global_rev env
    in
    if Queue.is_empty env.state_changes then
      acc
    else
      let (transition, timestamp) = Queue.peek_exn env.state_changes in
      match make_decision timestamp transition server_state env with
      | None -> acc
      | Some (decision, global_rev) ->
        (* We already peeked the value above. Can ignore here. *)
        let _ = Queue.dequeue_exn env.state_changes in
        (* Maybe setting the base revision must be done after
         * computing distance. *)
        maybe_set_base_rev transition global_rev env;
        churn_ready_changes ~acc:(decision :: acc) env server_state

  (**
   * Run through the ready changs in the queue; then make a decision.
   *)
  let churn_changes server_state env =
    if Queue.is_empty env.state_changes then
      Move_along
    else
      let decisions = churn_ready_changes ~acc:[] env server_state in
      (* left is more recent transition, so we prefer its saved state target. *)
      let select_relevant left right =
        match (left, right) with
        | (Restart_server, _)
        | (_, Restart_server) ->
          Restart_server
        | (_, _) -> Move_along
      in
      List.fold_left ~f:select_relevant ~init:Move_along decisions

  let get_change env =
    let (watchman, change) = Watchman.get_changes !(env.inits.watchman) in
    env.inits.watchman := watchman;
    match change with
    | Watchman.Watchman_unavailable
    | Watchman.Watchman_synchronous _ ->
      None
    | Watchman.Watchman_pushed (Watchman.Changed_merge_base (rev, files, clock))
      ->
      let () = Hh_logger.log "Changed_merge_base: %s" (Hg.Rev.to_string rev) in
      Some (Changed_merge_base (rev, files, clock))
    | Watchman.Watchman_pushed (Watchman.State_enter (state, json))
      when String.equal state "hg.update" ->
      env.is_in_hg_update_state := true;
      Option.(
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
        Hh_logger.log "State_enter: %s" (Hg.Rev.to_string hg_rev);
        Some (State_enter hg_rev))
    | Watchman.Watchman_pushed (Watchman.State_leave (state, json))
      when String.equal state "hg.update" ->
      env.is_in_hg_update_state := false;
      Option.(
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
        Hh_logger.log "State_leave: %s" (Hg.Rev.to_string hg_rev);
        Some (State_leave hg_rev))
    | Watchman.Watchman_pushed (Watchman.State_enter (state, _))
      when String.equal state "hg.transaction" ->
      env.is_in_hg_transaction_state := true;
      None
    | Watchman.Watchman_pushed (Watchman.State_leave (state, _))
      when String.equal state "hg.transaction" ->
      env.is_in_hg_transaction_state := false;
      None
    | Watchman.Watchman_pushed (Watchman.Files_changed _)
    | Watchman.Watchman_pushed (Watchman.State_enter _)
    | Watchman.Watchman_pushed (Watchman.State_leave _) ->
      None

  let preprocess server_state transition env =
    make_decision (Unix.time ()) transition server_state env

  let handle_change_then_churn server_state change env =
    let () =
      match change with
      | None -> ()
      | Some (State_enter hg_rev) ->
        Queue.enqueue env.state_changes (State_enter hg_rev, Unix.time ())
      | Some (State_leave hg_rev) ->
        Queue.enqueue env.state_changes (State_leave hg_rev, Unix.time ())
      | Some (Changed_merge_base _ as change) ->
        Queue.enqueue env.state_changes (change, Unix.time ())
    in
    churn_changes server_state env

  let has_more_watchman_messages env =
    match Watchman.get_reader !(env.inits.watchman) with
    | None -> false
    | Some reader -> Buffered_line_reader.is_readable reader

  (**
   * This must be a non-blocking call, so it creates Futures and consumes ready
   * Futures.
   *
   * The steps are:
   *   1) Get state change event from Watchman.
   *   3) Maybe add a needed query
   *      (if we don't already know the global rev for this hg rev)
   *   4) Preprocess new incoming change - this might result in an early
   *      decision to restart or kill the server, in which case we can dump
   *      out all the older changes that still need to be handled.
   *   5) If no early decision, handle the new change and churn through the
   *      queue of pending changes.
   * *)
  let process_once server_state env =
    let change = get_change env in
    let early_decision =
      match change with
      | None -> None
      | Some (State_enter hg_rev) ->
        let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
        preprocess server_state (State_enter hg_rev) env
      | Some (State_leave hg_rev) ->
        let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
        preprocess server_state (State_leave hg_rev) env
      | Some (Changed_merge_base (hg_rev, _, _) as change) ->
        let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
        preprocess server_state change env
    in
    (* If we make an "early" decision to either kill or restart, we toss
     * out earlier state changes and don't need to pump the queue anymore.
     *
     * This is an optimization.
     *
     * Otherwise, we continue as per usual. *)
    let report =
      match early_decision with
      | Some (Restart_server, global_rev) ->
        (* Early decision to restart, so the prior state changes don't
         * matter anymore. *)
        Hh_logger.log "Informant early decision: restart server";
        let () = Queue.clear env.state_changes in
        let () = set_base_revision global_rev env in
        Restart_server
      | Some (Move_along, _)
      | None ->
        handle_change_then_churn server_state change env
    in
    (* All the cases that `(change <> None)` cover should be also covered by
     * has_more_watchman_messages, but this alternate method of pumping messages
     * is heavily used in tests *)
    (report, has_more_watchman_messages env || Option.is_some change)

  let rec process (server_state, env, reports_acc) =
    (* Sometimes Watchman pushes many file changes as many sequential
     * notifications instead of all at once. Since make_report is
     * only called once per tick, we don't want to take many ticks
     * to consume this queue of notifications. So instead we repeatedly consume
     * the Watchman pipe here until it has no more changes. *)
    let (report, had_watchman_changes) = process_once server_state env in
    let reports_acc = report :: reports_acc in
    if had_watchman_changes then
      process (server_state, env, reports_acc)
    else
      reports_acc

  let process server_state env =
    let reports = process (server_state, env, []) in
    (* We fold through the reports and take the "most active" one. *)
    let max_report a b =
      match (a, b) with
      | (Restart_server, _)
      | (_, Restart_server) ->
        Restart_server
      | (_, _) -> Move_along
    in
    let default_decision =
      match server_state with
      | Server_not_yet_started when not (is_hg_updating env) -> Restart_server
      | _ -> Move_along
    in
    let decision =
      List.fold_left ~f:max_report ~init:default_decision reports
    in
    match decision with
    | Restart_server when is_hg_updating env ->
      Hh_logger.log
        "Ignoring Restart_server because we are already in next hg.update state";
      Move_along
    | decision -> decision

  let reinit t =
    (* The results of old initialization query might be stale by now, so we need
     * to cancel it and issue a new one *)
    let cancel_future future =
      (* timeout=0 should immediately kill and cleanup all queries that were not ready by now *)
      let (_ : ('a, Future.error) result) = Future.get future ~timeout:0 in
      ()
    in
    let make_new_future root =
      Hg.current_working_copy_base_rev (Path.to_string root)
    in
    match !t with
    | Initializing (init_settings, future) ->
      cancel_future future;
      t := Initializing (init_settings, make_new_future init_settings.root)
    | Reinitializing (env, future) ->
      cancel_future future;
      t := Reinitializing (env, make_new_future env.inits.root)
    | Tracking env -> t := Reinitializing (env, make_new_future env.inits.root)

  let check_init_future future =
    if not @@ Future.is_ready future then
      None
    else
      let global_rev =
        Future.get future
        |> Result.map_error ~f:Future.error_to_string
        |> Result.map_error
             ~f:HackEventLogger.revision_tracker_init_svn_rev_failed
        |> Result.ok
        |> Option.value ~default:0
      in
      let () =
        Hh_logger.log
          "Initialized Revision_tracker to global rev: %d"
          global_rev
      in
      Some global_rev

  let make_report server_state t =
    match !t with
    | Initializing (init_settings, future) -> begin
      match check_init_future future with
      | Some global_rev ->
        let env = active_env init_settings global_rev in
        let () = t := Tracking env in
        process server_state env
      | None -> Move_along
    end
    | Reinitializing (env, future) -> begin
      match check_init_future future with
      | Some global_rev ->
        let env = reinitialized_env env global_rev in
        let () = t := Tracking env in
        process server_state env
      | None -> Move_along
    end
    | Tracking env -> process server_state env
end

type env = {
  (* Reports for an Active informant are made by pinging the
   * revision_tracker. *)
  revision_tracker: Revision_tracker.t;
  watchman_event_watcher: WatchmanEventWatcherClient.t;
}

type t =
  (* Informant is active. *)
  | Active of env
  (* We don't run the informant if Watchman fails to initialize,
   * or if Watchman subscriptions are disabled in the local config,
   * or if the informant is disabled in the hhconfig. *)
  | Resigned

let init
    {
      root;
      allow_subscriptions;
      use_dummy;
      min_distance_restart;
      watchman_debug_logging;
      ignore_hh_version = _;
      is_saved_state_precomputed;
    } =
  if use_dummy then
    let () = Printf.eprintf "Informant using dummy - resigning\n" in
    Resigned
  (* Active informant requires Watchman subscriptions. *)
  else if not allow_subscriptions then
    let () = Printf.eprintf "Not using subscriptions - Informant resigning\n" in
    Resigned
  else
    let watchman =
      Watchman.init
        {
          Watchman.subscribe_mode = Some Watchman.Scm_aware;
          init_timeout = Watchman.Explicit_timeout 30.;
          expression_terms = FilesToIgnore.watchman_monitor_expression_terms;
          debug_logging = watchman_debug_logging;
          (* Should also take an arg *)
          sockname = None;
          subscription_prefix = "hh_informant_watcher";
          roots = [root];
        }
        ()
    in
    match watchman with
    | None ->
      let () =
        Printf.eprintf "Watchman failed to init - Informant resigning\n"
      in
      Resigned
    | Some watchman_env ->
      Active
        {
          revision_tracker =
            Revision_tracker.init
              ~min_distance_restart
              ~is_saved_state_precomputed
              (Watchman.Watchman_alive watchman_env)
              root;
          watchman_event_watcher = WatchmanEventWatcherClient.init root;
        }

let reinit t =
  match t with
  | Resigned -> ()
  | Active env ->
    Hh_logger.log "Reinitializing Informant";
    Revision_tracker.reinit env.revision_tracker

let should_start_first_server t =
  match t with
  | Resigned ->
    (* If Informant doesn't control the server lifetime, then the first server
     * instance should always be started during startup. *)
    true
  | Active env ->
    let status =
      WatchmanEventWatcherClient.get_status env.watchman_event_watcher
    in
    begin
      match status with
      | None ->
        (*
         * Watcher is not running, or connection to watcher collapsed.
         * So we let the first server start up.
         *)
        HackEventLogger.informant_watcher_not_available ();
        true
      | Some WatchmanEventWatcherConfig.Responses.Unknown ->
        (*
         * Watcher doens't know what state the repo is. We don't
         * know when the next "hg update" will happen, so we let the
         * first Hack Server start up to avoid wedging.
         *)
        HackEventLogger.informant_watcher_unknown_state ();
        true
      | Some WatchmanEventWatcherConfig.Responses.Mid_update ->
        (* Wait until the update is finished  *)
        HackEventLogger.informant_watcher_mid_update_state ();
        false
      | Some WatchmanEventWatcherConfig.Responses.Settled ->
        HackEventLogger.informant_watcher_settled_state ();
        true
    end

let should_ignore_hh_version init_env = init_env.ignore_hh_version

let is_managing = function
  | Resigned -> false
  | Active _ -> true

let report informant server_state =
  match (informant, server_state) with
  | (Resigned, Server_not_yet_started) ->
    (* Actually, this case should never happen. But we force a restart
     * to avoid accidental wedged states anyway. *)
    Hh_logger.log
      "Unexpected Resigned informant without a server started, restarting server";
    Restart_server
  | (Resigned, _) -> Move_along
  | (Active env, Server_not_yet_started) ->
    if should_start_first_server informant then (
      (* should_start_first_server (as name implies) prevents us from starting first (since
       * monitor start) server in the middle of update, when Revision_tracker is not reliable.
       * When server crashes/exits, it will go back to Server_not_yet_started, and
       * once again we need to avoid starting it during update. This time we can
       * defer to Revision_tracker safely. *)
      let report =
        Revision_tracker.make_report server_state env.revision_tracker
      in
      (match report with
      | Restart_server ->
        Hh_logger.log "Informant watcher starting server from settling";
        HackEventLogger.informant_watcher_starting_server_from_settling ()
      | Move_along -> ());
      report
    ) else
      Move_along
  | (Active env, _) ->
    Revision_tracker.make_report server_state env.revision_tracker
