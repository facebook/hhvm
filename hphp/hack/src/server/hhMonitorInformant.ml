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

module Prefetcher = struct
  type t = Path.t option
  type svn_rev = int
  let dummy = None
  let of_script script = Some script

  let run_and_log (script, svn_rev) _ic_oc =
    HackEventLogger.init_informant_prefetcher_runner (Unix.time ());
    let start_t = Unix.time () in
    let process = Process.exec
      (Path.to_string script) [(string_of_int svn_rev)] in
    (** We can block since this is already daemonized in a separate process. *)
    match Process.read_and_wait_pid ~timeout:30 process with
    | Result.Ok _ ->
      Hh_logger.log "Prefetcher finished successfully.";
      HackEventLogger.informant_prefetcher_success start_t;
      exit 0
    | Result.Error (Process_types.Process_exited_abnormally (es, _, stderr)) ->
      let exit_kind, exit_code = Exit_status.unpack es in
      let msg = Printf.sprintf "%s %d. stderr: %s" exit_kind exit_code stderr in
      Hh_logger.log "Prefetcher failed. %s" msg;
      HackEventLogger.informant_prefetcher_failed start_t msg;
      exit exit_code
    | Result.Error (Process_types.Timed_out (_, stderr)) ->
      let msg = Printf.sprintf "Timed out. stderr: %s" stderr in
      Hh_logger.log "Prefetcher timed out. %s" msg;
      HackEventLogger.informant_prefetcher_failed start_t msg;
      exit 1

  let run_and_log_entry = Daemon.register_entry_point
    "Prefetcher_run_and_log" run_and_log

  let run svn_rev t  = match t with
    | None -> Process_types.dummy
    | Some script ->
      Process.run_daemon run_and_log_entry
        (script, svn_rev)

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
 *)
module Revision_tracker = struct

  (** This is just a heuristic taken by eye-balling some graphs. Can turn
   * up the sensitivity as the advantage of a fresh server over incremental
   * typechecking improves. *)
  let restart_min_svn_distance = 100

  type timestamp = float

  type repo_transition =
    | State_enter
    | State_leave

  type env = {
    watchman : Watchman.watchman_instance ref;
    root : Path.t;
    prefetcher : Prefetcher.t;
    (** The 'current' base revision (from the tracker's perspective of the
     * repo. This is used to make calculations on distance. This is changed
     * when a State_leave is handled. *)
    current_base_revision : int ref;

    (**
     * Running queries and cached results. A query gets the SVN revision for a
     * given HG Revision.
     *)
    queries : (Hg.hg_rev, (Hg.svn_rev Future.t)) Hashtbl.t;

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
    state_changes : (repo_transition * timestamp * Hg.hg_rev) Queue.t;
  }

  type instance =
    | Initializing of Watchman.watchman_instance *
      Prefetcher.t * Path.t * (Hg.svn_rev Future.t)
    | Tracking of env

  type change =
    | Hg_update_enter of Hg.hg_rev
    | Hg_update_exit of Hg.hg_rev

  (** Revision_tracker has lots of mutable state anyway, so might as well
   * make it responsible for maintaining its own instance. *)
  type t = instance ref

  let init watchman prefetcher root =
    ref @@ Initializing (watchman, prefetcher, root,
      Hg.current_working_copy_base_rev (Path.to_string root))

  let set_base_revision svn_rev env =
    let () = Hh_logger.log "Revision_tracker setting base rev: %d" svn_rev in
    env.current_base_revision := svn_rev

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

  let active_env watchman prefetcher root base_svn_rev =
    {
      watchman = ref @@ watchman;
      root;
      prefetcher;
      current_base_revision = ref base_svn_rev;
      queries = Hashtbl.create 200;
      state_changes = Queue.create() ;
    }

  let get_distance svn_rev env =
    abs @@ svn_rev - !(env.current_base_revision)

  let is_significant distance elapsed_t =
    (** Allow up to 2 revisions per second for incremental. More than that,
     * prefer a server restart. *)
    distance > (float_of_int restart_min_svn_distance)
      && (elapsed_t <= 0.0 || ((distance /. elapsed_t) > 2.0))

  let cached_svn_rev queries hg_rev =
    let future = Hashtbl.find queries hg_rev in
    if Future.is_ready future then
      Some (svn_rev_of_future future)
    else
      None

  (**
   * If we have a cached svn_rev for this hg_rev, returns whether or not this
   * hg_rev is a significant distance away, and its svn_rev.
   *
   * Nonblocking.
   *)
  let maybe_significant timestamp hg_rev env =
    match cached_svn_rev env.queries hg_rev with
    | None ->
      None
    | Some svn_rev ->
      let distance = float_of_int @@ get_distance svn_rev env in
      let elapsed_t = (Unix.time () -. timestamp) in
      let significant = is_significant distance elapsed_t in
      Some (significant, svn_rev)

  (**
   * Keep popping state_changes queue until we reach a non-ready result.
   *
   * Returns if any of the popped changes resulted in a "significant" change
   *
   * Non-blocking.
   *)
  let rec churn_ready_changes ~acc env =
    if Queue.is_empty env.state_changes then
      acc
    else
      let transition, timestamp, hg_rev = Queue.peek env.state_changes in
      match maybe_significant timestamp hg_rev env with
      | None ->
        acc
      | Some (significant, svn_rev) ->
        (** We already peeked the value above. Can ignore here. *)
        let _ = Queue.pop env.state_changes in
        let _ = Prefetcher.run svn_rev env.prefetcher in
        if transition = State_leave
          (** Repo has been moved to a new SVN Rev, so we set this mutable
           * reference. This must be done after computing distance. *)
          then set_base_revision svn_rev env;
        churn_ready_changes ~acc:(significant || acc) env

  let form_decision has_significant last_transition server_state =
    let open Informant_sig in
    match has_significant, last_transition, server_state with
    | _, State_leave, Server_dead ->
      (** Regardless of whether we had a significant change or not, when the
       * server is not alive, we restart it on a state leave.*)
      Restart_server
    | false, _, _ ->
      Move_along
    | true, State_enter, _ ->
      (** We don't want a server to be running while transitioning to a
       * distant revision. Kill it, and start a fresh one when the
       * transition is done (State_leave below). *)
      Kill_server
    | true, State_leave, _ ->
      Restart_server

  let queue_peek_last q =
    let first = Queue.peek q in
    Queue.fold (fun _ v -> v) first q

  (**
   * Run through the ready changs in the queue; then make a decision.
   *)
  let churn_changes server_state env =
    let open Informant_sig in
    if Queue.is_empty env.state_changes then
      Move_along
    else
      let last_transition, _, _ = queue_peek_last env.state_changes in
      let has_significant = churn_ready_changes ~acc:false env in
      form_decision has_significant last_transition server_state

  let maybe_add_query hg_rev env =
    (** Don't add if we already have an entry for this. *)
    try ignore @@ Hashtbl.find env.queries hg_rev
    with
    | Not_found ->
      let future = Hg.get_closest_svn_ancestor
        hg_rev (Path.to_string env.root) in
      Hashtbl.add env.queries hg_rev future

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

  let preprocess server_state transition hg_rev env =
    match maybe_significant (Unix.time ()) hg_rev env with
    | None ->
      None
    | Some (significant, svn_rev) ->
      if significant
        then ignore @@ Prefetcher.run svn_rev env.prefetcher;
      let decision = form_decision significant transition server_state in
      Some (decision, svn_rev)

  let handle_change_then_churn server_state change env = begin
    let () = match change with
    | None -> ()
    | Some Hg_update_enter hg_rev ->
      Queue.add (State_enter, Unix.time (), hg_rev) env.state_changes
    | Some Hg_update_exit hg_rev ->
      Queue.add (State_leave, Unix.time (), hg_rev) env.state_changes
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
  let process server_state env =
    let open Informant_sig in
    let change = get_change env in
    let early_decision = match change with
    | None -> None
    | Some (Hg_update_enter hg_rev) ->
      let () = maybe_add_query hg_rev env in
      preprocess server_state State_enter hg_rev env
    | Some (Hg_update_exit hg_rev) ->
      let () = maybe_add_query hg_rev env in
      preprocess server_state State_leave hg_rev env
    in
    (** If we make an "early" decision to either kill or restart, we toss
     * out earlier state changes and don't need to pump the queue anymore.
     *
     * This is an optimization.
     *
     * Otherwise, we continue as per usual. *)
    match early_decision with
    | Some (Kill_server, _svn_rev) ->
      (** Early decision to kill the server, so prior state changes don't
       * matter anymore. We go to a dead state awaiting the following
       * state_leave to trigger a restart. *)
      Hh_logger.log "Informant early decision: kill server";
      Queue.clear env.state_changes;
      Kill_server
    | Some (Restart_server, svn_rev) ->
      (** Early decision to restart, so the prior state changes don't
       * matter anymore. *)
      Hh_logger.log "Informant early decision: restart server";
      let () = Queue.clear env.state_changes in
      let () = set_base_revision svn_rev env in
      Restart_server
    | Some (Move_along, _) | None ->
      handle_change_then_churn server_state change env

  let make_report server_state t = match !t with
    | Initializing (watchman, prefetcher, root, future) ->
      if Future.is_ready future
      then
        let svn_rev = svn_rev_of_future future in
        let () = Hh_logger.log "Initialized Revision_tracker to SVN rev: %d"
          svn_rev in
        let env = active_env watchman prefetcher root svn_rev in
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
          (Watchman.Watchman_alive watchman_env)
          (** TODO: Put the prefetcher here. *)
          Prefetcher.dummy root;
      }

let report informant server_state = match informant with
  | Resigned -> Informant_sig.Move_along
  | Active env ->
    Revision_tracker.make_report server_state env.revision_tracker


(** This is useful only for testing.
 *
 * It oscillates between Move_along, Kill_server and Restart_server
 * every 6 seconds.
 * TODO: Consider injecting this version when injector config is improved. *)
module Fake_informant = struct
  type t = {
    init_time : float;
  }

  include HhMonitorInformant_sig.Types

  let init _ = {
    init_time = Unix.time ();
  }

  let report env _server_state =
    let elapsed = Unix.time () -. env.init_time in
    let multiple = int_of_float @@ floor @@ elapsed /. 6.0 in
    let bucket = multiple mod 3 in
    match bucket with
      | 0 -> Informant_sig.Move_along
      | 1 -> Informant_sig.Kill_server
      | 2 -> Informant_sig.Restart_server
      | _ ->
        (* Actually unreachable by modulus. *)
        Informant_sig.Move_along
end
