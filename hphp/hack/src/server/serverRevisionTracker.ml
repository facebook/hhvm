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

type watchman_event = {
  timestamp: float;
  source: string;
  is_enter: bool;
}

(** A list of all unpaired "hg.update.enter", "hg.update.leave", "hg.transaction.enter",
"hg.transaction.leave" events that we have encountered, sorted by time, most recent first.
Because they are unpaired, the list will never contain both X.enter and X.leave for a given source.
Meaning, from the perspective of consulting the list to see whether we are in a state,
or amending the list because we just received a state-transition event, is the same
1. We are "in a state" if there exist any enter events
2. A new leave will be paired to the most recent enter, if present.
3. A new enter will be paired to the most recent leave only if it is recent; otherwise we ditch the leave, and log.
4. Observe the consequence, that existence of "enter" events can only be discharged either by
a subsequent leave, or an outstanding one from the recent past.
*)
type event_list = watchman_event list

type tracker_state = {
  mutable is_enabled: bool;
  mutable current_mergebase: Hg.global_rev option;
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
  pending_queries: Hg.Rev.t Queue.t;
      (** Keys from mergebase_queries that contain futures that were not resolved yet *)
  mergebase_queries: (Hg.Rev.t, Hg.global_rev Future.t) Stdlib.Hashtbl.t;
}

type state_handler = {
  is_hg_updating: unit -> bool;
  on_state_enter: string -> unit;
  on_state_leave:
    Path.t ->
    (* project root *)
    string ->
    (* state name *)
    Hh_json.json option ->
    (* state metadata *)
    unit;
}

type tracker_v1_event_state = {
  mutable is_in_hg_update_state: bool;
  mutable is_in_hg_transaction_state: bool;
}

type tracker_v2_event_state = { mutable outstanding_events: event_list }

let tracker_state =
  {
    is_enabled = false;
    current_mergebase = None;
    did_change_mergebase = false;
    pending_queries = Queue.create ();
    mergebase_queries = Stdlib.Hashtbl.create 200;
  }

let initialize mergebase =
  Hh_logger.log "ServerRevisionTracker: Initializing mergebase to r%d" mergebase;
  tracker_state.is_enabled <- true;
  tracker_state.current_mergebase <- Some mergebase

let add_query ~(hg_rev : Hg.Rev.t) root =
  if Stdlib.Hashtbl.mem tracker_state.mergebase_queries hg_rev then
    ()
  else (
    Hh_logger.log
      "ServerRevisionTracker: Seen new HG revision: %s"
      (Hg.Rev.to_string hg_rev);
    let future = Hg.get_closest_global_ancestor hg_rev (Path.to_string root) in
    Stdlib.Hashtbl.add tracker_state.mergebase_queries hg_rev future;
    Queue.enqueue tracker_state.pending_queries hg_rev
  )

let v1_handler_fn () : state_handler =
  let state =
    { is_in_hg_update_state = false; is_in_hg_transaction_state = false }
  in
  let on_state_enter state_name =
    match state_name with
    | "hg.update" -> state.is_in_hg_update_state <- true
    | "hg.transaction" -> state.is_in_hg_transaction_state <- true
    | _ -> ()
  in
  let on_state_leave root state_name state_metadata =
    match state_name with
    | "hg.update" ->
      if not state.is_in_hg_update_state then
        HackEventLogger.invalid_mercurial_state_transition ~state:state_name;
      state.is_in_hg_update_state <- false;
      Hh_logger.log "ServerRevisionTracker: leaving hg.update";
      Option.Monad_infix.(
        Option.iter
          (state_metadata >>= Watchman_utils.rev_in_state_change)
          ~f:(fun hg_rev ->
            match state_metadata >>= Watchman_utils.merge_in_state_change with
            | Some true ->
              Hh_logger.log
                "ServerRevisionTracker: Ignoring merge rev %s"
                (Hg.Rev.to_string hg_rev)
            | _ -> add_query ~hg_rev root))
    | "hg.transaction" ->
      if not state.is_in_hg_transaction_state then
        HackEventLogger.invalid_mercurial_state_transition ~state:state_name;
      state.is_in_hg_transaction_state <- false
    | _ -> ()
  in
  let is_hg_updating () =
    state.is_in_hg_transaction_state || state.is_in_hg_update_state
  in
  { on_state_enter; on_state_leave; is_hg_updating }

let v2_handler_fn () =
  let state = { outstanding_events = [] } in
  let is_in_state (outstanding : event_list) : bool =
    List.exists outstanding ~f:(fun e -> e.is_enter)
  in

  let transition (outstanding : event_list) (e : watchman_event) : event_list =
    (* Note: we blindly trusting that e.timestamp is newer than anything in the list.
       The consequence if not is mild; it just means the 10s criterion will be slightly off. *)

    (* consider S301212, which manifests as the following
         transaction/leave
         transaction/leave
         transaction/leave
         transaction/enter
         transaction/enter

       all with the same timestamp. How do we resolve a stale leave at the bottom?
       Simply throw it away if another event comes in 10+ seconds after,
    *)
    let outstanding =
      if is_in_state outstanding then
        outstanding
      else
        match List.filter outstanding ~f:(fun e -> not e.is_enter) with
        | { timestamp; _ } :: _ when Float.(timestamp < e.timestamp -. 10.) ->
          (* the most recent event (which must be a leave based on our if-branch)
              is old. Throw away current state of events
          *)
          let telemetry =
            Telemetry.create ()
            |> Telemetry.string_opt ~key:"event_source" ~value:(Some e.source)
            |> Telemetry.bool_ ~key:"flushed_by_new_event" ~value:true
          in
          HackEventLogger.server_revision_tracker_forced_reset ~telemetry;
          []
        | _ -> outstanding
    in

    (* Strip the first pair, if there is one *)
    let rec strip_first_match outstanding =
      match outstanding with
      (* if there is an event of matching source and opposite is_enter,
         assume that's the pair to our incoming event and remove it *)
      | { source; is_enter; _ } :: rest
        when String.equal source e.source && Bool.(is_enter = not e.is_enter) ->
        rest
      | olde :: rest -> olde :: strip_first_match rest
      | [] -> []
    in
    let new_outstanding = strip_first_match outstanding in

    (* Otherwise prepend our new event
       In other words, if we were able to remove a pair to the incoming event,
       then the new event list is new_outstanding.

       If we didn't find a pair, then the new event list is the old list + our new one
    *)
    if List.length new_outstanding < List.length outstanding then
      new_outstanding
    else
      e :: outstanding
  in
  let on_state_enter state_name =
    let event =
      { source = state_name; timestamp = Unix.gettimeofday (); is_enter = true }
    in
    match state_name with
    | "hg.update"
    | "hg.transaction" ->
      state.outstanding_events <- transition state.outstanding_events event
    | _ -> ()
  in
  let on_state_leave root state_name state_metadata =
    let event =
      {
        source = state_name;
        timestamp = Unix.gettimeofday ();
        is_enter = false;
      }
    in
    match state_name with
    | "hg.update" ->
      let _ =
        state.outstanding_events <- transition state.outstanding_events event
      in
      Hh_logger.log "ServerRevisionTracker: leaving hg.update";
      Option.Monad_infix.(
        Option.iter
          (state_metadata >>= Watchman_utils.rev_in_state_change)
          ~f:(fun hg_rev ->
            match state_metadata >>= Watchman_utils.merge_in_state_change with
            | Some true ->
              Hh_logger.log
                "ServerRevisionTracker: Ignoring merge rev %s"
                (Hg.Rev.to_string hg_rev)
            | _ -> add_query ~hg_rev root))
    | "hg.transaction" ->
      state.outstanding_events <- transition state.outstanding_events event
    | _ -> ()
  in
  let is_hg_updating () = is_in_state state.outstanding_events in
  { on_state_enter; on_state_leave; is_hg_updating }

let v1_handler = v1_handler_fn ()

let v2_handler = v2_handler_fn ()

let on_state_enter state_name use_tracker_v2 =
  if use_tracker_v2 then
    v2_handler.on_state_enter state_name
  else
    v1_handler.on_state_enter state_name

let on_state_leave root state_name state_metadata use_tracker_v2 =
  if use_tracker_v2 then
    v2_handler.on_state_leave root state_name state_metadata
  else
    v1_handler.on_state_leave root state_name state_metadata

let is_hg_updating use_tracker_v2 =
  if use_tracker_v2 then
    v2_handler.is_hg_updating ()
  else
    v1_handler.is_hg_updating ()

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
              Stdlib.Hashtbl.find tracker_state.mergebase_queries hg_rev
            in
            check_query future ~timeout ~current_t
        end
      tracker_state.pending_queries;
    Queue.clear tracker_state.pending_queries;
    let (_ : float) =
      Hh_logger.log_duration "Finished querying Mercurial" start_t
    in
    ()

let rec check_non_blocking ~is_full_check_done =
  if Queue.is_empty tracker_state.pending_queries then (
    if is_full_check_done && tracker_state.did_change_mergebase then (
      Hh_logger.log
        "ServerRevisionTracker: Full check completed despite mergebase changes";

      (* Clearing this flag because we somehow managed to get through this rebase,
       * so no need to restart anymore *)
      tracker_state.did_change_mergebase <- false;
      HackEventLogger.set_changed_mergebase false
    )
  ) else
    let hg_rev = Queue.peek_exn tracker_state.pending_queries in
    let future = Stdlib.Hashtbl.find tracker_state.mergebase_queries hg_rev in
    if Future.is_ready future then (
      let (_ : Hg.Rev.t) = Queue.dequeue_exn tracker_state.pending_queries in
      check_query future ~timeout:30 ~current_t:(Unix.gettimeofday ());
      check_non_blocking ~is_full_check_done
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
