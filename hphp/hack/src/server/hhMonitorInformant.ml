(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include HhMonitorInformant_sig.Types

let exit_on_parent_exit () = Parent.exit_on_parent_exit 10 60

module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module type State_loader_prefetcher_sig = sig
  val fetch :
    hhconfig_hash:string ->
    is_tiny:bool ->
    cache_limit:int ->
    State_loader.mini_state_handle ->
      unit Future.t
end

module State_loader_prefetcher_real = struct

  (** Main entry point for a new package fetcher process. Exits with 0 on success. *)
  let main (hhconfig_hash, handle, is_tiny, cache_limit) =
    EventLogger.init ~exit_on_parent_exit EventLogger.Event_logger_fake 0.0;
    let cached = State_loader.cached_state
      ~mini_state_handle:handle
      ~config_hash:hhconfig_hash
      ~rev:handle.State_loader.mini_state_for_rev
      ~tiny:is_tiny
    in
    if cached <> None then
      (** No need to fetch if catched. *)
      ()
    else
      let result = State_loader.fetch_mini_state
        ~cache_limit
        ~config:(State_loader_config.default_timeouts)
        ~config_hash:hhconfig_hash
        handle in
      result
      |> Core_result.map_error ~f:State_loader.error_string
      |> Core_result.ok_or_failwith
      |> ignore

  let prefetch_package_entry = Process.register_entry_point "State_loader_prefetcher_entry" main

  let fetch ~hhconfig_hash ~is_tiny ~cache_limit handle =
    Future.make (Process.run_entry prefetch_package_entry
      (hhconfig_hash, handle, is_tiny, cache_limit)) ignore

end;;


module State_loader_prefetcher_fake = struct
  let fetch ~hhconfig_hash:_ ~is_tiny:_ ~cache_limit:_ _ = Future.of_value ()
end


module State_loader_prefetcher =
  (val (if Injector_config.use_test_stubbing
  then (module State_loader_prefetcher_fake : State_loader_prefetcher_sig)
  else (module State_loader_prefetcher_real : State_loader_prefetcher_sig)
))


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
        xdb_queries : (int,
          (Xdb.sql_result list Future.t *
          (** is tiny state *) bool *
          (** Prefetcher *) (unit Future.t) option ref)) Hashtbl.t;
        use_xdb : bool;
        ignore_hh_version : bool;
        devinfra_saved_state_lookup: bool;
        saved_state_cache_limit : int;
      }

    let create ~saved_state_cache_limit ~devinfra_saved_state_lookup use_xdb ignore_hh_version =
      {
        svn_queries = Hashtbl.create 200;
        xdb_queries = Hashtbl.create 200;
        use_xdb;
        ignore_hh_version;
        devinfra_saved_state_lookup;
        saved_state_cache_limit;
      }

    let add_query ~hg_rev root t =
      (** Don't add if we already have an entry for this. *)
      try ignore @@ Hashtbl.find t.svn_queries hg_rev
      with
      | Not_found ->
        let future = Hg.get_closest_svn_ancestor hg_rev (Path.to_string root) in
        Hashtbl.add t.svn_queries hg_rev future

    let find_svn_rev hg_rev t =
      let future = Hashtbl.find t.svn_queries hg_rev in
      match Future.check_status future with
      | Future.In_progress age when age > 60.0 ->
        (** Fail if lookup up SVN rev number takes more than 60 s.
         * Delete the query so we can retry again if we encounter this hg_rev
         * again. Return fake "0" SVN rev number. *)
        let () = Hashtbl.remove t.svn_queries hg_rev in
        Some 0
      | Future.In_progress _ ->
        None
      | Future.Complete_with_result result ->
        let result = result
          |> Core_result.map_error ~f:Future.error_to_string
          |> Core_result.map_error ~f:(HackEventLogger.find_svn_rev_failed (Future.start_t future))
        in
        (* TODO (achow): make it log less often
        let () = Core_result.iter result ~f:(fun _ ->
          HackEventLogger.find_svn_rev_success (Future.start_t future)
        )
        in*)
        Some (result
          |> Core_result.ok
          |> Option.value ~default:0)

    (** XDB table changes over time. Prior queries should be cleared out after
     * we'v finished using the result completely (i.e. also allowed the
     * Prefetcher to finish) so that we don't reuse old results. *)
    let clear_xdb_query ~svn_rev t =
      Hashtbl.remove t.xdb_queries svn_rev

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
          let local_config = ServerLocalConfig.load ~silent:true in
          let tiny = local_config.ServerLocalConfig.load_tiny_state in
          (** Query doesn't exist yet, so we create one and consume it when
           * it's ready. *)
          let future = begin match hhconfig_hash with
            | None ->
              (** Have no config file hash.
               * Just return a fake empty list of XDB results. *)
              Future.of_value []
            | Some hhconfig_hash ->
              let hh_version = if t.ignore_hh_version
                then None
                else Some Build_id.build_revision in
              let db_table = if t.devinfra_saved_state_lookup then
                Xdb.devinfra_saved_states_table
              else
                Xdb.mini_saved_states_table
              in
              Xdb.find_nearest
                ~db:Xdb.hack_db_name
                ~db_table
                ~svn_rev
                ~hh_version
                ~hhconfig_hash
                ~tiny
          end in
          let () = Hashtbl.add t.xdb_queries svn_rev (future, tiny, ref None) in
          None
      in
      let query_to_result_list future =
        Future.get future
        |> Core_result.map_error ~f:Future.error_to_string
        |> Core_result.map_error ~f:(HackEventLogger.find_xdb_match_failed (Future.start_t future))
        |> Core_result.ok
        |> Option.value ~default:[] in
      let prefetch_package ~is_tiny xdb_result =
        let handle = {
          State_loader.mini_state_for_rev = Hg.Svn_rev (xdb_result.Xdb.svn_rev);
          mini_state_everstore_handle = xdb_result.Xdb.everstore_handle;
          watchman_mergebase = None;
        } in
        State_loader_prefetcher.fetch
          ~hhconfig_hash:xdb_result.Xdb.hhconfig_hash
          ~is_tiny ~cache_limit:t.saved_state_cache_limit
          handle
      in
      let not_yet_ready =
        (** We use None to represent a not yet ready result. Check again later *)
        None
      in
      let no_good_xdb_result ~is_tiny =
        let () = clear_xdb_query ~svn_rev t in
        Some ([], is_tiny)
      in
      let good_xdb_result ~is_tiny result =
        let () = clear_xdb_query ~svn_rev t in
        Some ([result], is_tiny)
      in
      let open Option in
      (** We run the prefetcher after the XDB lookup (because we need the XDB
       * result to run the prefetcher). *)
      query >>= fun (query, is_tiny, prefetcher) ->
        match query, !prefetcher with
        | query, Some prefetcher -> begin
          match Future.check_status prefetcher with
          | Future.In_progress age when age > 90.0 ->
            (** If prefetcher has taken longer than 90 seconds, we consider
             * this as having no saved states. *)
            let () = Hh_logger.log "Informant prefetcher timed out" in
            let () = HackEventLogger.informant_prefetcher_timed_out (Future.start_t prefetcher) in
            no_good_xdb_result ~is_tiny
          | Future.In_progress _ ->
            (** Prefetcher is still running. "Not yet ready, check later." *)
            not_yet_ready
          | Future.Complete_with_result (Ok _) ->
            (** This is the only case where we produce a positive result
             * (where the prefetcher succeeded). Prefetchr is only run after
             * XDB lookup finishes and produces a non-empty result list,
             * so we know the XDB list is non-empty here. *)
            let () = HackEventLogger.informant_prefetcher_success (Future.start_t prefetcher) in
            good_xdb_result ~is_tiny (List.hd (query_to_result_list query))
          | Future.Complete_with_result (Error e) ->
            let () = HackEventLogger.informant_prefetcher_failed
              (Future.start_t prefetcher) (Future.error_to_string e) in
            no_good_xdb_result ~is_tiny
          end
        | query, None ->
          begin match Future.check_status query with
          | Future.In_progress age when age > 15.0 ->
            (** If lookup in XDB table has taken more than 15 seconds, we
             * we consider this as having no saved state. *)
            let () = HackEventLogger.find_xdb_match_timed_out (Future.start_t query) in
            no_good_xdb_result ~is_tiny
          | Future.In_progress _ ->
            (** XDB lookup still in progress. "Not yet ready, check later." *)
            not_yet_ready
          | Future.Complete_with_result _ ->
            let result = query_to_result_list query in
            if result = [] then
              let () = Hh_logger.log "Got no XDB results on merge base change to %d" svn_rev in
              let () = HackEventLogger.informant_no_xdb_result () in
              no_good_xdb_result ~is_tiny
            else
              let () = HackEventLogger.find_xdb_match_success (Future.start_t query) in
              (** XDB looup is done, so we need to fire up the prefetcher.
               * The prefetcher's status will be checked on the next loop. *)
              let () = prefetcher := Some (prefetch_package ~is_tiny (List.hd result)) in
              not_yet_ready
          end

    let find ~start_t hg_rev t =
      let svn_rev = find_svn_rev hg_rev t in
      let open Option in
      svn_rev >>= fun svn_rev ->
        if t.use_xdb then
          find_xdb_match svn_rev t
          >>= fun (xdb_results, is_tiny) -> begin
            (** We log the mergebase after the XDB lookup and prefetch has
             * completed to avoid log spam, since the "find_svn_rev" result
             * is pinged once per second until completion. *)
            let () = Hh_logger.log "Informant Mergebase: %s -> %d" hg_rev svn_rev in
            let () = match xdb_results with
              | [] ->
                let () = Hh_logger.log
                  "Informant Saved State not found or prefetcher failed for svn_rev %d"
                  svn_rev in
                HackEventLogger.informant_find_saved_state_failed start_t
              | result :: _ ->
                let distance = abs (svn_rev - result.Xdb.svn_rev) in
                let () = Hh_logger.log
                  "Informant found Saved State and prefetched for svn_rev %d at distance %d"
                  svn_rev
                  distance in
                HackEventLogger.informant_find_saved_state_success ~distance start_t
            in
            Some(svn_rev, xdb_results, is_tiny)
          end
        else
          Some(svn_rev, [], false)

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
    | Changed_merge_base of Hg.hg_rev * SSet.t * Watchman.clock

  type init_settings = {
    watchman : Watchman.watchman_instance ref;
    root : Path.t;
    prefetcher : State_prefetcher.t;
    min_distance_restart : int;
    saved_state_cache_limit : int;
    use_xdb : bool;
    devinfra_saved_state_lookup : bool;
    ignore_hh_version : bool;
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

  let init
  ~min_distance_restart
  ~use_xdb ~ignore_hh_version
  ~saved_state_cache_limit
  ~devinfra_saved_state_lookup
  watchman prefetcher root =
    let init_settings = {
      watchman = ref watchman;
      prefetcher;
      root;
      min_distance_restart;
      saved_state_cache_limit;
      use_xdb;
      ignore_hh_version;
      devinfra_saved_state_lookup;
  } in
    ref @@ Initializing (init_settings,
      Hg.current_working_copy_base_rev (Path.to_string root))

  let set_base_revision svn_rev env =
    if svn_rev = !(env.current_base_revision) then
      ()
    else
      let () = Hh_logger.log "Revision_tracker setting base rev: %d" svn_rev in
      env.current_base_revision := svn_rev

  let active_env init_settings base_svn_rev =
    {
      inits = init_settings;
      current_base_revision = ref base_svn_rev;
      rev_map = Revision_map.create
        ~saved_state_cache_limit:init_settings.saved_state_cache_limit
        ~devinfra_saved_state_lookup:init_settings.devinfra_saved_state_lookup
        init_settings.use_xdb
        init_settings.ignore_hh_version;
      state_changes = Queue.create() ;
    }

  let get_jump_distance svn_rev env =
    abs @@ svn_rev - !(env.current_base_revision)

  let is_significant ~min_distance_restart ~jump_distance elapsed_t =
    let () = Hh_logger.log "Informant: jump distance %d. elapsed_t: %2f"
      jump_distance elapsed_t in
    (** Allow up to 2 revisions per second for incremental. More than that,
     * prefer a server restart. *)
    let result = (jump_distance > min_distance_restart)
        && (elapsed_t <= 0.0 || (((float_of_int jump_distance) /. elapsed_t) > 2.0)) in
    result

  let cached_svn_rev ~start_t revision_map hg_rev =
    Revision_map.find ~start_t hg_rev revision_map

  (** Form a decision about whether or not we'd like to start a new server.
   * transition: The state transition for which we are forming a decision
   * svn_rev: The corresponding SVN rev for this transition's hg rev.
   * xdb_results: The nearest saved states for this svn_rev provided by the XDB table.
   *)
  let form_decision ~start_t ~is_tiny ~significant transition
  server_state xdb_results svn_rev env =
    let use_xdb = env.inits.use_xdb in
    let open Informant_sig in
    match significant, transition, server_state, xdb_results with
    | _, State_leave _, Server_not_yet_started, _ ->
     (** This case should be unreachable since Server_not_yet_started
      * should be handled by "should_start_first_server" and not by the
      * revision tracker. Restart anyway which, at worst, could result in a
      * slow init. *)
      Hh_logger.log "Hit unreachable Server_not_yet_started match in %s"
        "Revision_tracker.form_decision";
      Restart_server None
    | _, State_leave _, Server_dead, _ ->
      (** Regardless of whether we had a significant change or not, when the
       * server is not alive, we restart it on a state leave.*)
      Restart_server None
    | false, _, _, _ ->
      let () = Hh_logger.log "Informant insignificant transition" in
      Move_along
    | true, State_enter _, _, _
    | true, State_leave _, _, _ ->
      (** We use the State enter and leave events to kick off asynchronous
       * computations off the hg revisions when they arrive (during preprocess)
       * But actual actions are taken only on changed_merge_base below. *)
      Move_along
    | true, Changed_merge_base _, _, [] when use_xdb ->
      (** No XDB results, so w don't restart. *)
      Move_along
    | true, Changed_merge_base (_rev, files_changed, watchman_clock), _,
      (nearest_xdb_result :: _) when use_xdb ->
      let state_distance = abs @@ nearest_xdb_result.Xdb.svn_rev - svn_rev in
      let incremental_distance = abs @@ svn_rev - !(env.current_base_revision) in
      let () = HackEventLogger.informant_decision_on_saved_state
        ~start_t ~state_distance ~incremental_distance in
      if incremental_distance > state_distance then
        let watchman_mergebase = {
          ServerMonitorUtils.mergebase_svn_rev = svn_rev;
          files_changed;
          watchman_clock;
        } in
        let target_state = {
          ServerMonitorUtils.mini_state_everstore_handle = nearest_xdb_result.Xdb.everstore_handle;
          target_svn_rev = nearest_xdb_result.Xdb.svn_rev;
          is_tiny;
          watchman_mergebase = Some watchman_mergebase;
        } in
        Restart_server (Some target_state)
      else
        let () = Hh_logger.log
          "Informant: Incremental distance <= state distance. Ignoring fetched Saved State." in
        Move_along
    | true, Changed_merge_base _, _, _ ->
      Restart_server None

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
      | Changed_merge_base (hg_rev, _, _) -> hg_rev
    in
    match cached_svn_rev ~start_t:timestamp env.rev_map hg_rev with
    | None ->
      None
    | Some (svn_rev, xdb_results, is_tiny) ->
      let jump_distance = get_jump_distance svn_rev env in
      let elapsed_t = (Unix.time () -. timestamp) in
      let significant = is_significant
        ~min_distance_restart:env.inits.min_distance_restart
        ~jump_distance elapsed_t in
      Some (form_decision ~start_t:timestamp ~is_tiny ~significant
        transition server_state xdb_results svn_rev env, svn_rev)

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
      (** left is more recent transition, so we prefer its saved state target. *)
      let select_relevant left right = match left, right with
        | Restart_server target, _ -> Restart_server target
        | _, Restart_server target -> Restart_server target
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
    | Watchman.Watchman_pushed (Watchman.Changed_merge_base (rev, files, clock)) ->
      let () = Hh_logger.log "Changed_merge_base: %s" rev in
      Some (Changed_merge_base (rev, files, clock))
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
    | Some (Changed_merge_base _ as change) ->
      Queue.add (change, Unix.time ()) env.state_changes
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
    | Some (Changed_merge_base (hg_rev, _, _) as change) ->
      let () = Revision_map.add_query ~hg_rev env.inits.root env.rev_map in
      preprocess server_state change env
    in
    (** If we make an "early" decision to either kill or restart, we toss
     * out earlier state changes and don't need to pump the queue anymore.
     *
     * This is an optimization.
     *
     * Otherwise, we continue as per usual. *)
    let report = match early_decision with
      | Some (Restart_server target, svn_rev) ->
        (** Early decision to restart, so the prior state changes don't
         * matter anymore. *)
        Hh_logger.log "Informant early decision: restart server";
        let () = Queue.clear env.state_changes in
        let () = set_base_revision svn_rev env in
        Restart_server target
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
      | Restart_server target, _
      | _, Restart_server target->
        Restart_server target
      | _, _ ->
        Move_along
    in
    List.fold_left max_report Move_along reports

  let make_report server_state t =
    match !t with
    | Initializing (init_settings, future) ->
      if Future.is_ready future
      then
        let svn_rev = Future.get future
          |> Core_result.map_error ~f:Future.error_to_string
          |> Core_result.map_error ~f:HackEventLogger.revision_tracker_init_svn_rev_failed
          |> Core_result.ok
          |> Option.value ~default:0
        in
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
  let module J = Hh_json_helpers.AdhocJsonHelpers in
  [
    J.pred "not" @@ [
      J.pred "anyof" @@ [
        J.strlist ["name"; ".hg"];
        J.strlist ["dirname"; ".hg"];
        J.strlist ["name"; ".git"];
        J.strlist ["dirname"; ".git"];
        J.strlist ["name"; ".svn"];
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
  saved_state_cache_limit;
  use_xdb;
  devinfra_saved_state_lookup;
  watchman_debug_logging;
  ignore_hh_version;
} =
  if use_dummy then
    let () = Printf.eprintf "Informant using dummy - resigning\n" in
    Resigned
  (** Active informant requires Watchman subscriptions. *)
  else if not allow_subscriptions then
    let () = Printf.eprintf "Not using subscriptions - Informant resigning\n" in
    Resigned
  else
    let watchman = Watchman.init {
      Watchman.subscribe_mode = Some Watchman.Scm_aware;
      init_timeout = 30;
      expression_terms = watchman_expression_terms;
      debug_logging = watchman_debug_logging;
      subscription_prefix = "hh_informant_watcher";
      roots = [root];
    } () in
    match watchman with
    | None ->
      let () = Printf.eprintf "Watchman failed to init - Informant resigning\n" in
      Resigned
    | Some watchman_env ->
      Active
      {
        revision_tracker = Revision_tracker.init
          ~min_distance_restart
          ~use_xdb
          ~ignore_hh_version
          ~saved_state_cache_limit
          ~devinfra_saved_state_lookup
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

let should_ignore_hh_version init_env =
  init_env.ignore_hh_version

let is_managing = function
  | Resigned -> false
  | Active _ -> true

let report informant server_state = match informant, server_state with
  | Resigned, Informant_sig.Server_not_yet_started ->
    (** Actually, this case should never happen. But we force a restart
     * to avoid accidental wedged states anyway. *)
    Informant_sig.Restart_server None
  | Resigned, _ ->
    Informant_sig.Move_along
  | Active _, Informant_sig.Server_not_yet_started ->
    if should_start_first_server informant then begin
      HackEventLogger.informant_watcher_starting_server_from_settling ();
      Informant_sig.Restart_server None
    end
    else
      Informant_sig.Move_along
  | Active env, _ ->
    Revision_tracker.make_report server_state env.revision_tracker
