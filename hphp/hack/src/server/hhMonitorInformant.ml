(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

include HhMonitorInformant_sig.Types

module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig


(** We need to query mercurial to convert an hg revision into a numerical
 * SVN revision. These queries need to be non-blocking, so we keep a cache
 * of the mapping in here, as well as the Futures corresponding to
 * th queries. *)
module Revision_map = struct

    (**
     * Running and finished queries. A query gets the SVN revision for a
     * given HG Revision.
     *)
    type t =
      {
        svn_queries : (Hg.hg_rev, (Hg.svn_rev Future.t)) Hashtbl.t;
        xdb_queries : (int, Xdb.sql_result list Future.t) Hashtbl.t;
        use_xdb : bool;
      }

    let create use_xdb =
      {
        svn_queries = Hashtbl.create 200;
        xdb_queries = Hashtbl.create 200;
        use_xdb;
      }

    let add_query ~hg_rev root t =
      (** Don't add if we already have an entry for this. *)
      try ignore @@ Hashtbl.find t.svn_queries hg_rev
      with
      | Not_found ->
        let future = Hg.get_closest_svn_ancestor hg_rev (Path.to_string root) in
        Hashtbl.add t.svn_queries hg_rev future

    (** Wrap Future.get. If process exits abnormally, returns 0. *)
    let svn_rev_of_future future =
      let parse_svn_rev svn_rev =
        try int_of_string svn_rev
        with Failure "int_of_string" ->
          Hh_logger.log "Revision_tracker failed to parse svn_rev: %s" svn_rev;
          0
      in
      try begin
        let result = Future.get future in
        parse_svn_rev result
      end with
      | Future_sig.Process_failure _ -> 0

    let find_svn_rev hg_rev t =
      let future = Hashtbl.find t.svn_queries hg_rev in
      if Future.is_ready future then
        Some (svn_rev_of_future future)
      else
        None

    (**
     * Does an async query to XDB to find nearest saved state match.
     * If no match is found, returns an empty list.
     * If query is not ready returns None.
     *
     * Non-blocking.
     *)
    let find_xdb_match svn_rev t =
      let query = try Some (Hashtbl.find t.xdb_queries svn_rev) with
        | Not_found ->
          let hhconfig_hash, _config = Config_file.parse
            (Relative_path.to_absolute ServerConfig.filename) in
          (** Query doesn't exist yet, so we create one and consume it when
           * it's ready. *)
          let future = begin match hhconfig_hash with
          | None ->
            (** Have no config file hash.
             * Just return a fake empty list of XDB results. *)
            Future.of_value []
          | Some hhconfig_hash ->
            Xdb.find_nearest
              ~db:Xdb.hack_db_name
              ~db_table:Xdb.mini_saved_states_table
              ~svn_rev
              ~hh_version:Build_id.build_revision
              ~hhconfig_hash
          end in
          let () = Hashtbl.add t.xdb_queries svn_rev future in
          None
      in
      let open Option in
      query >>= fun future ->
        if Future.is_ready future then
          let results = Future.get future in
          Some results
        else
          None

    let find hg_rev t =
      let svn_rev = find_svn_rev hg_rev t in
      let open Option in
      svn_rev >>= fun svn_rev ->
        if t.use_xdb then
          find_xdb_match svn_rev t
          >>= fun xdb_results ->
            Some(svn_rev, xdb_results)
        else
          Some(svn_rev, [])

end


(**
 * The Revision tracker tracks the latest known SVN Revision of the repo,
 * the corresponding SVN revisions of Hg revisions, and the sequence of
 * revision changes (from hg update). See record type "env" below.
 *
 * This machinery is necessary because Watchman state change events give
 * us only the HG Revisions of hg updates, but we need to make decisions
 * on their SVN Revision numbers.
 *
 * We want to be able to:
 *  1) Determine when the base SVN revision (in trunk) has changed
 *     "significantly" so we can inform the server monitor to trigger
 *     a server restart (since incremental typechecks can be slower
 *     than a fresh server using a saved state).
 *  2) Fulfill goal 1 without blocking, and while being "highly responsive"
 *
 * The definition of "significant" can be adjusted according to how fast
 * incremental type checking is compared to a fresh restart.
 *
 * The meaning of "highly responsive" above roughly means using a cache
 * for SVN revisions (because a Mercurial request to get the SVN revision
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
 * to mercurial for mapping the HG Revision to its corresponding SVN Revision
 * (since our "distance" meausure uses svn revisions which are
 * monotonically increasing)..
 *)
module Revision_tracker = struct

  type timestamp = float

  type repo_transition =
    | State_enter of Hg.hg_rev
    | State_leave of Hg.hg_rev
    | Changed_merge_base of Hg.hg_rev

  type init_settings = {
    watchman : Watchman.watchman_instance ref;
    root : Path.t;
    prefetcher : State_prefetcher.t;
    min_distance_restart : int;
    use_xdb : bool;
  }

  type env = {
    inits : init_settings;
    (** The 'current' base revision (from the tracker's perspective of the
     * repo. This is used to make calculations on distance. This is changed
     * when a State_leave is handled. *)
    current_base_revision : int ref;

    rev_map : Revision_map.t;

    (**
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
    state_changes : (repo_transition * timestamp) Queue.t;
  }

  type instance =
    | Initializing of init_settings * (Hg.svn_rev Future.t)
    | Tracking of env

  (** Revision_tracker has lots of mutable state anyway, so might as well
   * make it responsible for maintaining its own instance. *)
  type t = instance ref

  let init ~min_distance_restart ~use_xdb watchman prefetcher root =
    let init_settings = {
      watchman = ref watchman;
      prefetcher;
      root;
      min_distance_restart;
      use_xdb;
  } in
    ref @@ Initializing (init_settings,
      Hg.current_working_copy_base_rev (Path.to_string root))

  let set_base_revision svn_rev env =
    let () = Hh_logger.log "Revision_tracker setting base rev: %d" svn_rev in
    env.current_base_revision := svn_rev

  let active_env init_settings base_svn_rev =
    {
      inits = init_settings;
      current_base_revision = ref base_svn_rev;
      rev_map = Revision_map.create init_settings.use_xdb;
      state_changes = Queue.create() ;
    }

  let get_distance svn_rev env =
    abs @@ svn_rev - !(env.current_base_revision)

  let is_significant ~min_distance_restart distance elapsed_t =
    (** Allow up to 2 revisions per second for incremental. More than that,
     * prefer a server restart. *)
    distance > (float_of_int min_distance_restart)
      && (elapsed_t <= 0.0 || ((distance /. elapsed_t) > 2.0))

  let cached_svn_rev revision_map hg_rev =
    Revision_map.find hg_rev revision_map

  let form_decision ~use_xdb is_significant transition server_state xdb_results =
    let open Informant_sig in
    match is_significant, transition, server_state, xdb_results with
    | _, State_leave _, Server_not_yet_started, _ ->
     (** This case should be unreachable since Server_not_yet_started
      * should be handled by "should_start_first_server" and not by the
      * revision tracker. Restart anyway which, at worst, could result in a
      * slow init. *)
      Hh_logger.log "Hit unreachable Server_not_yet_started match in %s"
        "Revision_tracker.form_decision";
      Restart_server
    | _, State_leave _, Server_dead, _ ->
      (** Regardless of whether we had a significant change or not, when the
       * server is not alive, we restart it on a state leave.*)
      Restart_server
    | false, _, _, _ ->
      Move_along
    | true, State_enter _, _, _
    | true, State_leave _, _, _ ->
      (** We use the State enter and leave events to kick off asynchronous
       * computations off the hg revisions when they arrive (during preprocess)
       * But actual actions are taken only on changed_merge_base below. *)
      Move_along
    | true, Changed_merge_base _, _, [] when use_xdb ->
      (** No XDB results, so w don't restart. *)
      let () = Printf.eprintf "Got no XDB results on merge base change\n" in
      Move_along
    | true, Changed_merge_base _, _, _ ->
      Restart_server

  (**
   * If we have a cached svn_rev for this hg_rev, make a decision on
   * this transition and returns that decision.  If not, returns None.
   *
   * Nonblocking.
   *)
  let make_decision timestamp transition server_state env =
    let hg_rev = match transition with
      | State_enter hg_rev
      | State_leave hg_rev
      | Changed_merge_base hg_rev -> hg_rev
    in
    match cached_svn_rev env.rev_map hg_rev with
    | None ->
      None
    | Some (svn_rev, xdb_results) ->
      let distance = float_of_int @@ get_distance svn_rev env in
      let elapsed_t = (Unix.time () -. timestamp) in
      let significant = is_significant
        ~min_distance_restart:env.inits.min_distance_restart
        distance elapsed_t in
      Some (form_decision ~use_xdb:env.inits.use_xdb significant
        transition server_state xdb_results, svn_rev)

  (**
   * Keep popping state_changes queue until we reach a non-ready result.
   *
   * Returns a list of the decisions from processing each ready change
   * in the queue; the list is ordered most-recent to oldest.
   *
   * Non-blocking.
   *)
  let rec churn_ready_changes ~acc env server_state =
    let maybe_set_base_rev transition svn_rev env = match transition with
      | State_enter _
      | State_leave _ ->
        ()
      | Changed_merge_base _ ->
        set_base_revision svn_rev env
    in
    if Queue.is_empty env.state_changes then
      acc
    else
      let transition, timestamp = Queue.peek env.state_changes in
      match make_decision timestamp transition server_state env with
      | None ->
        acc
      | Some (decision, svn_rev) ->
        (** We already peeked the value above. Can ignore here. *)
        let _ = Queue.pop env.state_changes in
        let _ = State_prefetcher.run svn_rev env.inits.prefetcher in
        (** Maybe setting the base revision must be done after
         * computing distance. *)
        maybe_set_base_rev transition svn_rev env;
        churn_ready_changes ~acc:(decision :: acc) env server_state

  (**
   * Run through the ready changs in the queue; then make a decision.
   *)
  let churn_changes server_state env =
    let open Informant_sig in
    if Queue.is_empty env.state_changes then
      Move_along
    else
      let decisions = churn_ready_changes ~acc:[] env server_state in
      let select_relevant left right = match left, right with
        | Restart_server, _ -> Restart_server
        | _, Restart_server -> Restart_server
        | _, _ -> Move_along
      in
      List.fold_left select_relevant Move_along decisions

  let get_change env =
    let watchman, change = Watchman.get_changes !(env.inits.watchman) in
    env.inits.watchman := watchman;
    match change with
    | Watchman.Watchman_unavailable
    | Watchman.Watchman_synchronous _ ->
      None
    | Watchman.Watchman_pushed (Watchman.Changed_merge_base (rev, _)) ->
      let () = Hh_logger.log "Changed_merge_base: %s" rev in
      Some (Changed_merge_base rev)
    | Watchman.Watchman_pushed (Watchman.State_enter (state, json))
        when state = "hg.update" ->
        let open Option in
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev -> begin
          Hh_logger.log "State_enter: %s" hg_rev;
          Some (State_enter hg_rev)
        end
    | Watchman.Watchman_pushed (Watchman.State_leave (state, json))
        when state = "hg.update" ->
        let open Option in
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev -> begin
          Hh_logger.log "State_leave: %s" hg_rev;
          Some (State_leave hg_rev)
        end
    | Watchman.Watchman_pushed (Watchman.Files_changed _)
    | Watchman.Watchman_pushed (Watchman.State_enter _)
    | Watchman.Watchman_pushed (Watchman.State_leave _) ->
      None

  let preprocess server_state transition env =
    match make_decision (Unix.time ()) transition server_state env with
    | None ->
      None
    | Some (decision, svn_rev) ->
      if decision <> Informant_sig.Move_along
        then ignore @@ State_prefetcher.run svn_rev env.inits.prefetcher;
      Some (decision, svn_rev)

  let handle_change_then_churn server_state change env = begin
    let () = match change with
    | None -> ()
    | Some (State_enter hg_rev) ->
      Queue.add (State_enter hg_rev, Unix.time ()) env.state_changes
    | Some (State_leave hg_rev) ->
      Queue.add (State_leave hg_rev, Unix.time ()) env.state_changes
    | Some (Changed_merge_base hg_rev) ->
      Queue.add (Changed_merge_base hg_rev, Unix.time ()) env.state_changes
    in
    churn_changes server_state env
  end

  (**
   * This must be a non-blocking call, so it creates Futures and consumes ready
   * Futures.
   *
   * The steps are:
   *   1) Get state change event from Watchman.
   *   3) Maybe add a needed query
   *      (if we don't already know the SVN rev for this hg rev)
   *   4) Preprocess new incoming change - this might result in an early
   *      decision to restart or kill the server, in which case we can dump
   *      out all the older changes that still need to be handled.
   *   5) If no early decision, handle the new change and churn through the
   *      queue of pending changes.
   * *)
  let process_once server_state env =
    let open Informant_sig in
    let change = get_change env in
    let early_decision = match change with
    | None -> None
    | Some (State_enter hg_rev) ->
      let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
      preprocess server_state (State_enter hg_rev) env
    | Some (State_leave hg_rev) ->
      let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
      preprocess server_state (State_leave hg_rev) env
    | Some (Changed_merge_base hg_rev) ->
      let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
      preprocess server_state (Changed_merge_base hg_rev) env
    in
    (** If we make an "early" decision to either kill or restart, we toss
     * out earlier state changes and don't need to pump the queue anymore.
     *
     * This is an optimization.
     *
     * Otherwise, we continue as per usual. *)
    let report = match early_decision with
      | Some (Restart_server, svn_rev) ->
        (** Early decision to restart, so the prior state changes don't
         * matter anymore. *)
        Hh_logger.log "Informant early decision: restart server";
        let () = Queue.clear env.state_changes in
        let () = set_base_revision svn_rev env in
        Restart_server
      | Some (Move_along, _) | None ->
        handle_change_then_churn server_state change env
    in
    report, (change <> None)

  let rec process (server_state, env, reports_acc) =
    (** Sometimes Watchman pushes many file changes as many sequential
     * notifications instead of all at once. Since make_report is
     * only called once per tick, we don't want to take many ticks
     * to consume this queue of notifications. So instead we repeatedly consume
     * the Watchman pipe here until it has no more changes. *)
    let report, had_watchman_changes = process_once server_state env in
    let reports_acc = report :: reports_acc in
    if had_watchman_changes then
      process (server_state, env, reports_acc)
    else
      reports_acc

  let process server_state env =
    let open Informant_sig in
    let reports = process (server_state, env, []) in
    (** We fold through the reports and take the "most active" one. *)
    let max_report a b = match a, b with
      | Restart_server, _
      | _, Restart_server ->
        Restart_server
      | _, _ ->
        Move_along
    in
    List.fold_left max_report Move_along reports

  let make_report server_state t = match !t with
    | Initializing (init_settings, future) ->
      if Future.is_ready future
      then
        let svn_rev = Revision_map.svn_rev_of_future future in
        let () = Hh_logger.log "Initialized Revision_tracker to SVN rev: %d"
          svn_rev in
        let env = active_env init_settings svn_rev in
        let () = t := Tracking env in
        process server_state env
      else
        Informant_sig.Move_along
    | Tracking env ->
      process server_state env

end

type env = {
  (** Reports for an Active informant are made by pinging the
   * revision_tracker. *)
  revision_tracker : Revision_tracker.t;
  watchman_event_watcher : WEWClient.t;
}

type t =
  (** Informant is active. *)
  | Active of env
  (** We don't run the informant if Watchman fails to initialize,
   * or if Watchman subscriptions are disabled in the local config,
   * or if the informant is disabled in the hhconfig. *)
  | Resigned

let watchman_expression_terms =
  let module J = Hh_json_helpers in
  [
    J.pred "not" @@ [
      J.pred "anyof" @@ [
        J.strlist ["dirname"; ".hg"];
        J.strlist ["dirname"; ".git"];
        J.strlist ["dirname"; ".svn"];
      ]
    ]
  ]

let init {
  root;
  allow_subscriptions;
  state_prefetcher;
  use_dummy;
  min_distance_restart;
  use_xdb;
} =
  if use_dummy then
    Resigned
  (** Active informant requires Watchman subscriptions. *)
  else if not allow_subscriptions then
    Resigned
  else
    let watchman = Watchman.init {
      Watchman.subscribe_mode = Some Watchman.Scm_aware;
      init_timeout = 30;
      sync_directory = "";
      expression_terms = watchman_expression_terms;
      root;
    } in
    match watchman with
    | None -> Resigned
    | Some watchman_env ->
      Active
      {
        revision_tracker = Revision_tracker.init
          ~min_distance_restart
          ~use_xdb
          (Watchman.Watchman_alive watchman_env)
          (** TODO: Put the prefetcher here. *)
          state_prefetcher root;
        watchman_event_watcher = WEWClient.init root;
      }

let should_start_first_server t = match t with
  | Resigned ->
    (** If Informant doesn't control the server lifetime, then the first server
     * instance should always be started during startup. *)
    true
  | Active env ->
    let status = WEWClient.get_status env.watchman_event_watcher in
    begin match status with
    | None ->
      (**
       * Watcher is not running, or connection to watcher collapsed.
       * So we let the first server start up.
       *)
      HackEventLogger.informant_watcher_not_available ();
      true
    | Some WEWConfig.Responses.Unknown ->
      (**
       * Watcher doens't know what state the repo is. We don't
       * know when the next "hg update" will happen, so we let the
       * first Hack Server start up to avoid wedging.
       *)
      HackEventLogger.informant_watcher_unknown_state ();
      true
    | Some WEWConfig.Responses.Mid_update ->
      (** Wait until the update is finished  *)
      HackEventLogger.informant_watcher_mid_update_state ();
      false
    | Some WEWConfig.Responses.Settled ->
      HackEventLogger.informant_watcher_settled_state ();
      true
  end

let is_managing = function
  | Resigned -> false
  | Active _ -> true

let report informant server_state = match informant, server_state with
  | Resigned, Informant_sig.Server_not_yet_started ->
    (** Actually, this case should never happen. But we force a restart
     * to avoid accidental wedged states anyway. *)
    Informant_sig.Restart_server
  | Resigned, _ ->
    Informant_sig.Move_along
  | Active _, Informant_sig.Server_not_yet_started ->
    if should_start_first_server informant then begin
      HackEventLogger.informant_watcher_starting_server_from_settling ();
      Informant_sig.Restart_server
    end
    else
      Informant_sig.Move_along
  | Active env, _ ->
    Revision_tracker.make_report server_state env.revision_tracker
