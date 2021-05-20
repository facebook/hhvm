(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
include HhMonitorInformant_sig.Types
module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module type State_loader_prefetcher_sig = sig
  val fetch :
    hhconfig_hash:string ->
    cache_limit:int ->
    State_loader.saved_state_handle ->
    unit Future.t
end

module State_loader_prefetcher_real = struct
  let fetch ~hhconfig_hash ~cache_limit handle =
    let error_to_string (error : State_loader.error) : string =
      let State_loader.
            {
              message;
              auto_retry = _;
              stack = Utils.Callstack stack;
              environment;
            } =
        State_loader.error_string_verbose error
      in
      message
      ^ "\n"
      ^ stack
      ^ "\nEnvironment:\b"
      ^ Option.value environment ~default:"N/A"
    in
    let future =
      (* TODO(hverr): Support 64-bit *)
      State_loader.fetch_saved_state
        ~load_64bit:false
        ~cache_limit
        ~config:State_loader_config.default_timeouts
        ~config_hash:hhconfig_hash
        handle
    in
    Future.continue_with future @@ function
    | Ok result -> ignore result
    | Error error -> failwith (error_to_string error)
end

module State_loader_prefetcher_fake = struct
  let fetch ~hhconfig_hash:_ ~cache_limit:_ _handle = Future.of_value ()
end

module State_loader_prefetcher =
( val if Injector_config.use_test_stubbing then
        (module State_loader_prefetcher_fake : State_loader_prefetcher_sig)
      else
        (module State_loader_prefetcher_real : State_loader_prefetcher_sig) )

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
    global_rev_queries: (Hg.hg_rev, Hg.global_rev Future.t) Caml.Hashtbl.t;
    xdb_queries: (Hg.global_rev, query) Caml.Hashtbl.t;
    use_xdb: bool;
    ignore_hh_version: bool;
    ignore_hhconfig: bool;
    saved_state_cache_limit: int;
  }

  and query = {
    query: Xdb.sql_result list Future.t;
    prefetcher: unit Future.t option ref;
  }

  let create
      ~saved_state_cache_limit ~use_xdb ~ignore_hh_version ~ignore_hhconfig =
    {
      global_rev_queries = Caml.Hashtbl.create 200;
      xdb_queries = Caml.Hashtbl.create 200;
      use_xdb;
      ignore_hh_version;
      ignore_hhconfig;
      saved_state_cache_limit;
    }

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

  (* XDB table changes over time. Prior queries should be cleared out after
   * we'v finished using the result completely (i.e. also allowed the
   * Prefetcher to finish) so that we don't reuse old results. *)
  let clear_xdb_query ~global_rev t =
    Caml.Hashtbl.remove t.xdb_queries global_rev

  (**
     * Does an async query to XDB to find nearest saved state match.
     * If no match is found, returns an empty list.
     * If query is not ready returns None.
     *
     * Non-blocking.
     *)
  let find_xdb_match global_rev t =
    let query =
      let query = Caml.Hashtbl.find_opt t.xdb_queries global_rev in
      if Option.is_some query then
        query
      else
        let (hhconfig_hash, _config) =
          Config_file.parse_hhconfig
            ~silent:false
            (Relative_path.to_absolute ServerConfig.filename)
        in
        let hhconfig_hash =
          if t.ignore_hhconfig then
            None
          else
            Some hhconfig_hash
        in
        (* Query doesn't exist yet, so we create one and consume it when
         * it's ready. *)
        let future =
          let hh_version =
            if t.ignore_hh_version then
              None
            else
              Some Build_id.build_revision
          in
          (* TODO(hverr): Support 64-bit state *)
          Xdb.find_nearest
            ~db:Xdb.hack_db_name
            ~db_table:Xdb.saved_states_table
            ~load_64bit:false
            ~global_rev
            ~hh_version
            ~hhconfig_hash
          |> fst
        in
        let () =
          Caml.Hashtbl.add
            t.xdb_queries
            global_rev
            { query = future; prefetcher = ref None }
        in
        None
    in
    let query_to_result_list future =
      Future.get future
      |> Result.map_error ~f:Future.error_to_string
      |> Result.map_error
           ~f:(HackEventLogger.find_xdb_match_failed (Future.start_t future))
      |> Result.ok
      |> Option.value ~default:[]
    in
    let prefetch_package xdb_result =
      let rev = Hg.Global_rev xdb_result.Xdb.global_rev in
      let config_hash = xdb_result.Xdb.hhconfig_hash in
      let saved_state_handle =
        {
          State_loader.saved_state_for_rev = rev;
          saved_state_everstore_handle = xdb_result.Xdb.everstore_handle;
          watchman_mergebase = None;
        }
      in
      let cached_state =
        (* TODO(hverr): Support 64-bit state *)
        State_loader.cached_state
          ~load_64bit:false
          ~saved_state_handle
          ~config_hash
          ~rev
      in
      match cached_state with
      | Some _ ->
        Hh_logger.log
          "Informant found cached saved state for %s"
          (Hg.show_rev rev);
        Future.of_value ()
      | None ->
        State_loader_prefetcher.fetch
          ~hhconfig_hash:config_hash
          ~cache_limit:t.saved_state_cache_limit
          saved_state_handle
    in
    let not_yet_ready =
      (* We use None to represent a not yet ready result. Check again later *)
      None
    in
    let no_good_xdb_result () =
      let () = clear_xdb_query ~global_rev t in
      Some []
    in
    let good_xdb_result result =
      let () = clear_xdb_query ~global_rev t in
      Some [result]
    in
    Option.(
      (* We run the prefetcher after the XDB lookup (because we need the XDB
       * result to run the prefetcher). *)
      query >>= fun { query; prefetcher } ->
      match (query, !prefetcher) with
      | (query, Some prefetcher) ->
        begin
          match Future.check_status prefetcher with
          | Future.In_progress { age } when Float.(age > 90.0) ->
            (* If prefetcher has taken longer than 90 seconds, we consider
             * this as having no saved states. *)
            let () = Hh_logger.log "Informant prefetcher timed out" in
            let () =
              HackEventLogger.informant_prefetcher_timed_out
                (Future.start_t prefetcher)
            in
            no_good_xdb_result ()
          | Future.In_progress _ ->
            (* Prefetcher is still running. "Not yet ready, check later." *)
            not_yet_ready
          | Future.Complete_with_result (Ok ()) ->
            (* This is the only case where we produce a positive result
             * (where the prefetcher succeeded). Prefetchr is only run after
             * XDB lookup finishes and produces a non-empty result list,
             * so we know the XDB list is non-empty here. *)
            let () =
              HackEventLogger.informant_prefetcher_success
                (Future.start_t prefetcher)
            in
            good_xdb_result (List.hd_exn (query_to_result_list query))
          | Future.Complete_with_result (Error e) ->
            Hh_logger.log
              "Informant prefetcher failed with error: %s"
              (Future.error_to_string e);
            let () =
              HackEventLogger.informant_prefetcher_failed
                (Future.start_t prefetcher)
                (Future.error_to_string e)
            in
            no_good_xdb_result ()
        end
      | (query, None) ->
        begin
          match Future.check_status query with
          | Future.In_progress { age } when Float.(age > 15.0) ->
            (* If lookup in XDB table has taken more than 15 seconds, we
             * we consider this as having no saved state. *)
            let () =
              HackEventLogger.find_xdb_match_timed_out (Future.start_t query)
            in
            no_good_xdb_result ()
          | Future.In_progress _ ->
            (* XDB lookup still in progress. "Not yet ready, check later." *)
            not_yet_ready
          | Future.Complete_with_result _ ->
            let result = query_to_result_list query in
            if List.is_empty result then
              let () =
                Hh_logger.log
                  "Got no XDB results on merge base change to %d"
                  global_rev
              in
              let () = HackEventLogger.informant_no_xdb_result () in
              no_good_xdb_result ()
            else
              let () =
                HackEventLogger.find_xdb_match_success (Future.start_t query)
              in
              (* XDB looup is done, so we need to fire up the prefetcher.
               * The prefetcher's status will be checked on the next loop. *)
              let () =
                prefetcher := Some (prefetch_package (List.hd_exn result))
              in
              not_yet_ready
        end)

  (**
    * Looks up the global revision for this hg_rev, and prefetches the Saved State for that global
    * rev. Converting hg_rev to an global rev takes on the order of seconds, and prefetching
    * 10-20 seconds. This will run those operations asynchronously, stash those operations
    * away for later checking.
    *
    * Returns the actual result after both operations have completed.
    *
    * Returns None while those operations are still in progress (on a None
    * result, you should check again later).
    * *)
  let find_and_prefetch ~start_t hg_rev t =
    let global_rev = find_global_rev hg_rev t in
    Option.(
      global_rev >>= fun global_rev ->
      if t.use_xdb then
        find_xdb_match global_rev t >>= fun xdb_results ->
        (* We log the mergebase after the XDB lookup and prefetch has
         * completed to avoid log spam, since the "find_global_rev" result
         * is pinged once per second until completion. *)
        let () =
          Hh_logger.log "Informant Mergebase: %s -> %d" hg_rev global_rev
        in
        let () =
          match xdb_results with
          | [] ->
            let () =
              Hh_logger.log
                "Informant Saved State not found or prefetcher failed for global_rev %d"
                global_rev
            in
            HackEventLogger.informant_find_saved_state_failed start_t
          | result :: _ ->
            let distance = abs (global_rev - result.Xdb.global_rev) in
            let () =
              Hh_logger.log
                "Informant found Saved State and prefetched for global_rev %d at distance %d"
                global_rev
                distance
            in
            HackEventLogger.informant_find_saved_state_success ~distance start_t
        in
        Some (global_rev, xdb_results)
      else
        Some (global_rev, []))
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
    | State_enter of Hg.hg_rev
    | State_leave of Hg.hg_rev
    | Changed_merge_base of
        Hg.hg_rev * (SSet.t[@printer SSet.pp_large]) * Watchman.clock
  [@@deriving show]

  let _ = show_repo_transition (* allow unused show *)

  type init_settings = {
    watchman: Watchman.watchman_instance ref;
    root: Path.t;
    min_distance_restart: int;
    saved_state_cache_limit: int;
    use_xdb: bool;
    ignore_hh_version: bool;
    ignore_hhconfig: bool;
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

  let init
      ~min_distance_restart
      ~use_xdb
      ~ignore_hh_version
      ~ignore_hhconfig
      ~saved_state_cache_limit
      ~is_saved_state_precomputed
      watchman
      root =
    let init_settings =
      {
        watchman = ref watchman;
        root;
        min_distance_restart;
        saved_state_cache_limit;
        use_xdb;
        ignore_hh_version;
        ignore_hhconfig;
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
      rev_map =
        Revision_map.create
          ~saved_state_cache_limit:init_settings.saved_state_cache_limit
          ~use_xdb:init_settings.use_xdb
          ~ignore_hh_version:init_settings.ignore_hh_version
          ~ignore_hhconfig:init_settings.ignore_hhconfig;
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
   * xdb_results: The nearest saved states for this global_rev provided by the XDB table.
   *)
  let form_decision
      ~start_t ~significant transition server_state xdb_results global_rev env =
    let use_xdb = env.inits.use_xdb in
    Informant_sig.(
      match (significant, transition, server_state, xdb_results) with
      | (_, State_leave _, Server_not_yet_started, _) ->
        (* This is reachable when server stopped in the middle of rebase. Instead
      * of restarting immediately, we go back to Server_not_yet_started, and want
      * to restart only when hg.update state is vacated *)
        Restart_server None
      | (_, State_leave _, Server_dead, _) ->
        (* Regardless of whether we had a significant change or not, when the
         * server is not alive, we restart it on a state leave.*)
        Restart_server None
      | (false, _, _, _) ->
        let () = Hh_logger.log "Informant insignificant transition" in
        Move_along
      | (true, State_enter _, _, _)
      | (true, State_leave _, _, _) ->
        (* We use the State enter and leave events to kick off asynchronous
         * computations off the hg revisions when they arrive (during preprocess)
         * But actual actions are taken only on changed_merge_base below. *)
        Move_along
      | (true, Changed_merge_base _, _, []) when use_xdb ->
        (* No XDB results, so w don't restart. *)
        Move_along
      | ( true,
          Changed_merge_base (_rev, files_changed, watchman_clock),
          _,
          nearest_xdb_result :: _ )
        when use_xdb ->
        let state_distance =
          abs @@ (nearest_xdb_result.Xdb.global_rev - global_rev)
        in
        let incremental_distance =
          abs @@ (global_rev - !(env.current_base_revision))
        in
        let () =
          HackEventLogger.informant_decision_on_saved_state
            ~start_t
            ~state_distance
            ~incremental_distance
        in
        if incremental_distance > state_distance then
          let watchman_mergebase =
            {
              ServerMonitorUtils.mergebase_global_rev = global_rev;
              files_changed;
              watchman_clock;
            }
          in
          let target_state =
            {
              ServerMonitorUtils.saved_state_everstore_handle =
                nearest_xdb_result.Xdb.everstore_handle;
              target_global_rev = nearest_xdb_result.Xdb.global_rev;
              watchman_mergebase = Some watchman_mergebase;
            }
          in
          Restart_server (Some target_state)
        else
          let () =
            Hh_logger.log
              "Informant: Incremental distance <= state distance. Ignoring fetched Saved State."
          in
          Move_along
      | (true, Changed_merge_base _, _, _) ->
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
          Restart_server None)

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
    match
      Revision_map.find_and_prefetch ~start_t:timestamp hg_rev env.rev_map
    with
    | None -> None
    | Some (global_rev, xdb_results) ->
      let jump_distance = get_jump_distance global_rev env in
      let elapsed_t = Unix.time () -. timestamp in
      let significant =
        is_significant
          ~min_distance_restart:env.inits.min_distance_restart
          ~jump_distance
          elapsed_t
      in
      Some
        ( form_decision
            ~start_t:timestamp
            ~significant
            transition
            server_state
            xdb_results
            global_rev
            env,
          global_rev )

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
    Informant_sig.(
      if Queue.is_empty env.state_changes then
        Move_along
      else
        let decisions = churn_ready_changes ~acc:[] env server_state in
        (* left is more recent transition, so we prefer its saved state target. *)
        let select_relevant left right =
          match (left, right) with
          | (Restart_server target, _) -> Restart_server target
          | (_, Restart_server target) -> Restart_server target
          | (_, _) -> Move_along
        in
        List.fold_left ~f:select_relevant ~init:Move_along decisions)

  let get_change env =
    let (watchman, change) = Watchman.get_changes !(env.inits.watchman) in
    env.inits.watchman := watchman;
    match change with
    | Watchman.Watchman_unavailable
    | Watchman.Watchman_synchronous _ ->
      None
    | Watchman.Watchman_pushed (Watchman.Changed_merge_base (rev, files, clock))
      ->
      let () = Hh_logger.log "Changed_merge_base: %s" rev in
      Some (Changed_merge_base (rev, files, clock))
    | Watchman.Watchman_pushed (Watchman.State_enter (state, json))
      when String.equal state "hg.update" ->
      env.is_in_hg_update_state := true;
      Option.(
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
        Hh_logger.log "State_enter: %s" hg_rev;
        Some (State_enter hg_rev))
    | Watchman.Watchman_pushed (Watchman.State_leave (state, json))
      when String.equal state "hg.update" ->
      env.is_in_hg_update_state := false;
      Option.(
        json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
        Hh_logger.log "State_leave: %s" hg_rev;
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
    Informant_sig.(
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
        | Some (Restart_server target, global_rev) ->
          (* Early decision to restart, so the prior state changes don't
           * matter anymore. *)
          Hh_logger.log "Informant early decision: restart server";
          let () = Queue.clear env.state_changes in
          let () = set_base_revision global_rev env in
          Restart_server target
        | Some (Move_along, _)
        | None ->
          handle_change_then_churn server_state change env
      in
      (* All the cases that `(change <> None)` cover should be also covered by
       * has_more_watchman_messages, but this alternate method of pumping messages
       * is heavily used in tests *)
      (report, has_more_watchman_messages env || Option.is_some change))

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
    Informant_sig.(
      let reports = process (server_state, env, []) in
      (* We fold through the reports and take the "most active" one. *)
      let max_report a b =
        match (a, b) with
        | (Restart_server target, _)
        | (_, Restart_server target) ->
          Restart_server target
        | (_, _) -> Move_along
      in
      let default_decision =
        match server_state with
        | Server_not_yet_started when not (is_hg_updating env) ->
          Restart_server None
        | _ -> Move_along
      in
      let decision =
        List.fold_left ~f:max_report ~init:default_decision reports
      in
      match decision with
      | Restart_server _ when is_hg_updating env ->
        Hh_logger.log
          "Ignoring Restart_server because we are already in next hg.update state";
        Move_along
      | decision -> decision)

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
    | Initializing (init_settings, future) ->
      begin
        match check_init_future future with
        | Some global_rev ->
          let env = active_env init_settings global_rev in
          let () = t := Tracking env in
          process server_state env
        | None -> Informant_sig.Move_along
      end
    | Reinitializing (env, future) ->
      begin
        match check_init_future future with
        | Some global_rev ->
          let env = reinitialized_env env global_rev in
          let () = t := Tracking env in
          process server_state env
        | None -> Informant_sig.Move_along
      end
    | Tracking env -> process server_state env
end

type env = {
  (* Reports for an Active informant are made by pinging the
   * revision_tracker. *)
  revision_tracker: Revision_tracker.t;
  watchman_event_watcher: WEWClient.t;
}

type t =
  (* Informant is active. *)
  | Active of env
  (* We don't run the informant if Watchman fails to initialize,
   * or if Watchman subscriptions are disabled in the local config,
   * or if the informant is disabled in the hhconfig. *)
  | Resigned

let watchman_expression_terms =
  let module J = Hh_json_helpers.AdhocJsonHelpers in
  [
    J.pred "not"
    @@ [
         J.pred "anyof"
         @@ [
              J.strlist ["name"; ".hg"];
              J.strlist ["dirname"; ".hg"];
              J.strlist ["name"; ".git"];
              J.strlist ["dirname"; ".git"];
              J.strlist ["name"; ".svn"];
              J.strlist ["dirname"; ".svn"];
            ];
       ];
  ]

let init
    {
      root;
      allow_subscriptions;
      use_dummy;
      min_distance_restart;
      saved_state_cache_limit;
      use_xdb;
      watchman_debug_logging;
      ignore_hh_version;
      ignore_hhconfig;
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
          expression_terms = watchman_expression_terms;
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
              ~use_xdb
              ~ignore_hh_version
              ~ignore_hhconfig
              ~saved_state_cache_limit
              ~is_saved_state_precomputed
              (Watchman.Watchman_alive watchman_env)
              root;
          watchman_event_watcher = WEWClient.init root;
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
    let status = WEWClient.get_status env.watchman_event_watcher in
    begin
      match status with
      | None ->
        (*
         * Watcher is not running, or connection to watcher collapsed.
         * So we let the first server start up.
         *)
        HackEventLogger.informant_watcher_not_available ();
        true
      | Some WEWConfig.Responses.Unknown ->
        (*
         * Watcher doens't know what state the repo is. We don't
         * know when the next "hg update" will happen, so we let the
         * first Hack Server start up to avoid wedging.
         *)
        HackEventLogger.informant_watcher_unknown_state ();
        true
      | Some WEWConfig.Responses.Mid_update ->
        (* Wait until the update is finished  *)
        HackEventLogger.informant_watcher_mid_update_state ();
        false
      | Some WEWConfig.Responses.Settled ->
        HackEventLogger.informant_watcher_settled_state ();
        true
    end

let should_ignore_hh_version init_env = init_env.ignore_hh_version

let is_managing = function
  | Resigned -> false
  | Active _ -> true

let report informant server_state =
  match (informant, server_state) with
  | (Resigned, Informant_sig.Server_not_yet_started) ->
    (* Actually, this case should never happen. But we force a restart
     * to avoid accidental wedged states anyway. *)
    Informant_sig.Restart_server None
  | (Resigned, _) -> Informant_sig.Move_along
  | (Active env, Informant_sig.Server_not_yet_started) ->
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
      | Informant_sig.Restart_server _ ->
        HackEventLogger.informant_watcher_starting_server_from_settling ()
      | Informant_sig.Move_along -> ());
      report
    ) else
      Informant_sig.Move_along
  | (Active env, _) ->
    Revision_tracker.make_report server_state env.revision_tracker
