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
 *)
module Revision_tracker = struct

  (** This is just a heuristic taken by eye-balling some graphs. Can turn
   * up the sensitivity as the advantage of a fresh server over incremental
   * typechecking improves. *)
  let restart_min_svn_distance = 100

  type timestamp = float

  type env = {
    watchman : Watchman.watchman_instance ref;
    root : Path.t;
    current_base_revision : int ref;

    (**
     * Running queries and cached results. A query gets the SVN revision for a
     * given HG Revision.
     *)
    queries : (Hg.hg_rev, (Hg.svn_rev Future.t)) Hashtbl.t;

    (**
     * Timestamp and HG revision of state exit events.
     *
     * Why do we keep the entire sequence? It seems like it would be sufficient
     * to just consume from the Watchman subscription one-at-a-time,
     * processing at most one state exit at a time. But consider the case
     * of many sequential hg updates, the last of which is very distant
     * (i.e. significant), and we happen to have a cached value only for
     * the last hg update.
     *
     * If we processed one-at-a-time as the updates came in, it would be many
     * seconds of processing each hg query before deciding at the final one
     * to restart the server (so the server restart is delayed by many
     * seconds).
     *
     * By keeping a running queue of state exits and "preprocessing" new
     * exits from the watchman subscription (before appending them to this
     * queue), we can catch that final hg update early on and proactively
     * trigger a server restart.
     *)
    state_exits : (timestamp * Hg.hg_rev) Queue.t;
  }

  type instance =
    | Initializing of Watchman.watchman_instance * Path.t * (Hg.svn_rev Future.t)
    | Tracking of env

  type change =
    | Hg_update_enter of Hg.hg_rev
    | Hg_update_exit of Hg.hg_rev

  (** Revision_tracker has lots of mutable state anyway, so might as well
   * make it responsible for maintaining its own instance. *)
  type t = instance ref

  let init watchman root =
    ref @@ Initializing (watchman, root,
      Hg.current_working_copy_base_rev (Path.to_string root))

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

  let active_env watchman root base_svn_rev =
    {
      watchman = ref @@ watchman;
      root;
      current_base_revision = ref base_svn_rev;
      queries = Hashtbl.create 200;
      state_exits = Queue.create() ;
    }

  let get_distance svn_rev env =
    abs @@ svn_rev - !(env.current_base_revision)

  (** See docs on non-recursive version below. *)
  let rec churn_exits env acc =
    try begin
      let timestamp, hg_rev = Queue.peek env.state_exits in
      (** Hashtable always has an entry, since it is added before
       * being put on the state_exits queue. *)
      let future = Hashtbl.find env.queries hg_rev in
      if Future.is_ready future then
        let _ = Queue.pop env.state_exits in
        let svn_rev = svn_rev_of_future future in
        let distance = float_of_int @@ get_distance svn_rev env in
        let elapsed = (Unix.time () -. timestamp) in
        (** Allow up to 2 revisions per second for incremental. More than that,
         * prefer a server restart. *)
        let should_restart = distance > (float_of_int restart_min_svn_distance)
          && (distance /. elapsed) > 2.0 in
        (** Repo has been moved to a new SVN Rev, so we set this mutable
         * reference. This must be done after computing distance. *)
        let () = env.current_base_revision := svn_rev in
        churn_exits env (acc || should_restart)
      else
        acc
    end
    with
    | Queue.Empty -> acc

  (**
   * Keep popping state_exits queue until we reach a non-ready result.
   *
   * Returns true if a state exit is encountered that should trigger a
   * restart.
   *
   * Non-blocking.
   *)
  let churn_exits env = churn_exits env false

  let maybe_add_query hg_rev env =
    (** Don't add if we already have an entry for this. *)
    try ignore @@ Hashtbl.find env.queries hg_rev
    with
    | Not_found ->
      let future = Hg.get_closest_svn_ancestor
        hg_rev (Path.to_string env.root) in
      Hashtbl.add env.queries hg_rev future

  let purge_exits env = Queue.clear env.state_exits

  let handle_change change env =
    let should_exit = churn_exits env in
    if should_exit
    then
      let () = purge_exits env in
      Informant_sig.Restart_server
    else
      match change with
      | None ->
        Informant_sig.Move_along
      | Some (Hg_update_enter hg_rev) ->
        let () = maybe_add_query hg_rev env in
        Informant_sig.Move_along
      | Some (Hg_update_exit hg_rev) ->
        let () = maybe_add_query hg_rev env in
        let () = Queue.push (Unix.time (), hg_rev) env.state_exits in
        Informant_sig.Move_along

  let parse_json json = match json with
    | None -> None
    | Some json ->
      let open Hh_json.Access in
      (return json) >>=
        get_string "rev" |> function
        | Result.Error _ ->
          let () = Hh_logger.log
            "Revision_tracker failed to get rev in json: %s"
            (Hh_json.json_to_string json) in
          None
        | Result.Ok (v, _) -> Some v

  let get_change env =
    let watchman, change = Watchman.get_changes !(env.watchman) in
    env.watchman := watchman;
    match change with
    | Watchman.Watchman_unavailable
    | Watchman.Watchman_synchronous _ ->
      None
    | Watchman.Watchman_pushed (Watchman.State_enter (state, json))
        when state = "hg.update" ->
        let open Option in
        parse_json json >>= fun hg_rev ->
          Some (Hg_update_enter hg_rev)
    | Watchman.Watchman_pushed (Watchman.State_leave (state, json))
        when state = "hg.update" ->
        let open Option in
        parse_json json >>= fun hg_rev ->
          Some (Hg_update_exit hg_rev)
    | Watchman.Watchman_pushed _ ->
      None

  (**
   * This "catches" significant state exits early on, before adding them
   * to state_exits queue. Returns true if we want to
   *
   * See docs on state_exits queue above.
   *)
  let preprocess change env = match change with
    | None -> false
    | Some (Hg_update_enter _hg_rev) ->
      false
    | Some (Hg_update_exit hg_rev) ->
      try begin
        let future = Hashtbl.find env.queries hg_rev in
        if Future.is_ready future then
          let svn_rev = svn_rev_of_future future in
          let distance = get_distance svn_rev env in
          (** Just a heuristic - restart if we're crossing more than
           * 30 revisions. *)
          let () = env.current_base_revision := svn_rev in
          distance > restart_min_svn_distance
        else
          false
      end with
      | Not_found -> false

  (**
   * This must be a non-blocking call, so it creates Futures and consumes ready
   * Futures.
   *
   * The steps are:
   *   1) Get state change event from Watchman.
   *   2) Pre-process event to maybe early trigger a server restart
   *   3) Maybe add a needed query
   *   4) Append state exit to state_exits queue
   *   5) Check state_exits queue processing all ready results.
   * *)
  let process env =
    let change = get_change env in
    let should_restart = preprocess change env in
    if should_restart
    then
      let () = purge_exits env in
      Informant_sig.Restart_server
    else
      handle_change change env

  let make_report t = match !t with
    | Initializing (watchman, root, future) ->
      if Future.is_ready future
      then
        let svn_rev = svn_rev_of_future future in
        let () = Hh_logger.log "Initialized Revision_tracker to SVN rev: %d"
          svn_rev in
        let env = active_env watchman root svn_rev in
        let () = t := Tracking env in
        process env
      else
        Informant_sig.Move_along
    | Tracking env ->
      process env

end

type env = {
  (** Reports for an Active informant are made by pinging the
   * revision_tracker. *)
  revision_tracker : Revision_tracker.t;
}

type t =
  (** Informant is active. *)
  | Active of env
  (** We don't run the informant if Watchman fails to initialize,
   * or if Watchman subscriptions are disabled in the local config,
   * or if the informant is disabled in the hhconfig. *)
  | Resigned

let init { root; allow_subscriptions; use_dummy } =
  if use_dummy then
    Resigned
  (** Active informant requires Watchman subscriptions. *)
  else if not allow_subscriptions then
    Resigned
  else
    let watchman = Watchman.init {
      Watchman.subscribe_mode = Some Watchman.Drop_changes;
      init_timeout = 30;
      sync_directory = "";
      root;
    } in
    match watchman with
    | None -> Resigned
    | Some watchman_env ->
      Active
      {
        revision_tracker = Revision_tracker.init
          (Watchman.Watchman_alive watchman_env) root;
      }

let report informant = match informant with
  | Resigned -> Informant_sig.Move_along
  | Active env ->
    Revision_tracker.make_report env.revision_tracker
