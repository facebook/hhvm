(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)


(** Note: the tracking in this module is for approximate logging purposes only;
 * it's not guaranteed to always reflect accurate merge base transitions:
 * - in some init types, initial merge base is not known so we will only notice
 *   the second transition
 * - to avoid blocking rest of the system, mergebase queries time out after 30
 *   seconds and are not retried in case of errors
 * - we only record "new" mergebases as we see them, not detecting transitions
 *   between already visited revisions
 *
 * In other words: do not depend on anything in this module for things more
 * critical than logging (like HackEventLogger.set_changed_mergebase ())
 **)

(* This will be None after init in case of canaries and Precomputed loads *)
let current_mergebase : Hg.svn_rev option ref = ref None
let mergebase_queries : (Hg.hg_rev, (Hg.svn_rev Future.t)) Hashtbl.t = Hashtbl.create 200
(* Keys from mergebase_queries that contain futures that were not resolved yet *)
let pending_queries : Hg.hg_rev Queue.t = Queue.create ()

let initialize mergebase =
  Hh_logger.log "ServerRevisionTracker: Initializing mergebase to r%d" mergebase;
  current_mergebase := Some mergebase

let add_query ~hg_rev root =
  if Hashtbl.mem mergebase_queries hg_rev then
    ()
  else begin
    Hh_logger.log "ServerRevisionTracker: Seen new HG revision: %s" hg_rev;
    let future = Hg.get_closest_svn_ancestor hg_rev (Path.to_string root) in
    Hashtbl.add mergebase_queries hg_rev future;
    Queue.add hg_rev pending_queries
  end

let on_state_leave root state_name state_metadata =
  if state_name <> "hg.update" then () else
  let open Option.Monad_infix in
  Option.iter (state_metadata >>= Watchman_utils.rev_in_state_change)
    ~f:(fun hg_rev -> add_query ~hg_rev root);
  ()

let check_changes start_t =
  if Queue.is_empty pending_queries then
    start_t
  else begin
    Hh_logger.log "Querying Mercurial for mergebase changes";
    Queue.iter begin fun hg_rev ->
      let elapsed_t = (Unix.gettimeofday ()) -. start_t in
      let timeout = max 0 (int_of_float (30.0 -. elapsed_t)) in
      let future = Hashtbl.find mergebase_queries hg_rev in
      match Future.get ~timeout future with
      | Error e ->
        let e = Future.error_to_string e in
        HackEventLogger.check_mergebase_failed (Future.start_t future) e;
        Hh_logger.log "ServerRevisionTracker: %s" e;
      | Ok new_svn_rev ->
        HackEventLogger.check_mergebase_success (Future.start_t future);
        match !current_mergebase with
        | Some svn_rev when svn_rev <> new_svn_rev ->
            current_mergebase := Some new_svn_rev;
            HackEventLogger.set_changed_mergebase ();
            Hh_logger.log "ServerRevisionTracker: Changing mergebase from r%d to r%d"
              svn_rev new_svn_rev;
            ()
        | Some _ -> ()
        | None -> initialize new_svn_rev
    end pending_queries;
    Queue.clear pending_queries;
    Hh_logger.log_duration "Finished querying Mercurial" start_t
  end
