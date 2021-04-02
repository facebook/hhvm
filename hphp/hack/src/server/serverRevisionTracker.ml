(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(** Note: the tracking in this module is best effort only;
 * it's not guaranteed to always reflect accurate merge base transitions:
 * - in some init types, initial merge base is not known so we will only notice
 *   the second transition
 * - to avoid blocking rest of the system, mergebase queries time out after 30
 *   seconds and are not retried in case of errors
 * - we only record "new" mergebases as we see them, not detecting transitions
 *   between already visited revisions
 **)
open Hh_prelude

type tracker_state = {
  mutable is_enabled: bool;
  mutable current_mergebase: Hg.global_rev option;
  mutable is_in_hg_update_state: bool;
  mutable is_in_hg_transaction_state: bool;
  mutable did_change_mergebase: bool;
      (**  Do we think that this server have processed a mergebase change? If we are
        * in this state and get notified about changes to a huge number of files (or
        * even small number of files that fan-out to a huge amount of work), we might
       * decide that restarting the server is a better option than going through with
       * incremental processing (See ServerLocalConfig.hg_aware_*_restart_threshold).
       * It is likely to be faster because:
       * - a better saved state might be available
       * - even with same saved state, during init we can treat all those changes
       *   (if they were indeed due to non-local commits ) as prechecked (see ServerPrecheckedFiles)
       *    and avoid processing them.
       * There is some room for false positives, when small inconsequential rebase is immediately
       * followed by a big local change, but that seems unlikely to happen often compared to
       * the hours we waste processing incremental rebases.
       *)
  pending_queries: Hg.hg_rev Queue.t;
      (** Keys from mergebase_queries that contain futures that were not resolved yet *)
  mergebase_queries: (Hg.hg_rev, Hg.global_rev Future.t) Caml.Hashtbl.t;
}

let tracker_state =
  {
    is_enabled = false;
    current_mergebase = None;
    is_in_hg_update_state = false;
    is_in_hg_transaction_state = false;
    did_change_mergebase = false;
    pending_queries = Queue.create ();
    mergebase_queries = Caml.Hashtbl.create 200;
  }

let initialize mergebase =
  Hh_logger.log "ServerRevisionTracker: Initializing mergebase to r%d" mergebase;
  tracker_state.is_enabled <- true;
  tracker_state.current_mergebase <- Some mergebase

let add_query ~hg_rev root =
  if Caml.Hashtbl.mem tracker_state.mergebase_queries hg_rev then
    ()
  else (
    Hh_logger.log "ServerRevisionTracker: Seen new HG revision: %s" hg_rev;
    let future = Hg.get_closest_global_ancestor hg_rev (Path.to_string root) in
    Caml.Hashtbl.add tracker_state.mergebase_queries hg_rev future;
    Queue.enqueue tracker_state.pending_queries hg_rev
  )

let on_state_enter state_name =
  match state_name with
  | "hg.update" -> tracker_state.is_in_hg_update_state <- true
  | "hg.transaction" -> tracker_state.is_in_hg_transaction_state <- true
  | _ -> ()

let on_state_leave root state_name state_metadata =
  match state_name with
  | "hg.update" ->
    tracker_state.is_in_hg_update_state <- false;
    Hh_logger.log "ServerRevisionTracker: leaving hg.update";
    Option.Monad_infix.(
      Option.iter
        (state_metadata >>= Watchman_utils.rev_in_state_change)
        ~f:(fun hg_rev ->
          match state_metadata >>= Watchman_utils.merge_in_state_change with
          | Some true ->
            Hh_logger.log "ServerRevisionTracker: Ignoring merge rev %s" hg_rev
          | _ -> add_query ~hg_rev root))
  | "hg.transaction" -> tracker_state.is_in_hg_transaction_state <- false
  | _ -> ()

let is_hg_updating () =
  tracker_state.is_in_hg_update_state
  || tracker_state.is_in_hg_transaction_state

let check_query future ~timeout ~current_t =
  match Future.get ~timeout future with
  | Error e ->
    let e = Future.error_to_string e in
    HackEventLogger.check_mergebase_failed current_t e;
    Hh_logger.log "ServerRevisionTracker: %s" e
  | Ok new_global_rev ->
    HackEventLogger.check_mergebase_success current_t;
    (match tracker_state.current_mergebase with
    | Some global_rev when global_rev <> new_global_rev ->
      tracker_state.current_mergebase <- Some new_global_rev;
      tracker_state.did_change_mergebase <- true;
      HackEventLogger.set_changed_mergebase true;
      Hh_logger.log
        "ServerRevisionTracker: Changing mergebase from r%d to r%d"
        global_rev
        new_global_rev;
      ()
    | Some _ -> ()
    | None -> initialize new_global_rev)

let check_blocking () =
  if Queue.is_empty tracker_state.pending_queries then
    ()
  else
    let start_t = Unix.gettimeofday () in
    Hh_logger.log "Querying Mercurial for mergebase changes";
    Queue.iter
      ~f:
        begin
          fun hg_rev ->
          let current_t = Unix.gettimeofday () in
          let elapsed_t = current_t -. start_t in
          let timeout = max 0 (int_of_float (30.0 -. elapsed_t)) in
          let future =
            Caml.Hashtbl.find tracker_state.mergebase_queries hg_rev
          in
          check_query future ~timeout ~current_t
        end
      tracker_state.pending_queries;
    Queue.clear tracker_state.pending_queries;
    let (_ : float) =
      Hh_logger.log_duration "Finished querying Mercurial" start_t
    in
    ()

let rec check_non_blocking env =
  if Queue.is_empty tracker_state.pending_queries then (
    if
      ServerEnv.(is_full_check_done env.full_check_status)
      && tracker_state.did_change_mergebase
    then (
      Hh_logger.log
        "ServerRevisionTracker: Full check completed despite mergebase changes";

      (* Clearing this flag because we somehow managed to get through this rebase,
       * so no need to restart anymore *)
      tracker_state.did_change_mergebase <- false;
      HackEventLogger.set_changed_mergebase false
    )
  ) else
    let hg_rev = Queue.peek_exn tracker_state.pending_queries in
    let future = Caml.Hashtbl.find tracker_state.mergebase_queries hg_rev in
    if Future.is_ready future then (
      let (_ : Hg.hg_rev) = Queue.dequeue_exn tracker_state.pending_queries in
      check_query future ~timeout:30 ~current_t:(Unix.gettimeofday ());
      check_non_blocking env
    )

let make_decision threshold count name =
  if threshold = 0 || count < threshold || not tracker_state.is_enabled then
    ()
  else (
    (* Enough files / declarations / typings have changed to possibly warrant
     * a restart. Let's wait for Mercurial to decide if we want to before
     * proceeding. *)
    check_blocking ();
    if tracker_state.did_change_mergebase then (
      Hh_logger.log "Changed %d %s due to rebase. Restarting!" count name;
      Exit.exit
        ~msg:
          "Hh_server detected a large rebase. Its quickest option now is to restart."
        Exit_status.Big_rebase_detected
    )
  )

let files_changed local_config count =
  make_decision
    local_config.ServerLocalConfig.hg_aware_parsing_restart_threshold
    count
    "files"

let decl_changed local_config count =
  make_decision
    local_config.ServerLocalConfig.hg_aware_redecl_restart_threshold
    count
    "declarations"

let typing_changed local_config count =
  make_decision
    local_config.ServerLocalConfig.hg_aware_recheck_restart_threshold
    count
    "file typings"
