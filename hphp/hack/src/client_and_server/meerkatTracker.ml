(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
  Note: the tracking in this module is best effort only;
  it's not guaranteed to always reflect accurate merge base transitions:
  - in some init types, initial merge base is not known so we will only notice
    the second transition
  - to avoid blocking rest of the system, mergebase queries time out after 30
    seconds and are not retried in case of errors
  - we only record "new" mergebases as we see them, not detecting transitions
    between already visited revisions
 **)
open Hh_prelude

type watchman_event = {
  timestamp: float;
  source: string;
  is_enter: bool;
}

(** A list of all unpaired "meerkat-build.enter" and "meerkat-build.leave",
events that we have encountered, sorted by time, most recent first.
Because they are unpaired, the list will never contain both X.enter and X.leave.
Meaning, from the perspective of consulting the list to see whether we are in a state,
or amending the list because we just received a state-transition event, is the same
1. We are "in a state" if there exist any enter events
2. A new leave will be paired to the most recent enter, if present.
3. A new enter will be paired to the most recent leave only if it is recent; otherwise we ditch the leave, and log.
4. Observe the consequence, that existence of "enter" events can only be discharged either by
a subsequent leave, or an outstanding one from the recent past.
*)
type event_list = watchman_event list

type state_handler = {
  is_meerkat_running: unit -> bool;
  on_state_enter: string -> unit;
  on_state_leave: string -> unit;
}

type tracker_event_state = { mutable outstanding_events: event_list }

let handler_fn () =
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
    | "meerkat-build" ->
      state.outstanding_events <- transition state.outstanding_events event
    | _ -> ()
  in
  let on_state_leave state_name =
    let event =
      {
        source = state_name;
        timestamp = Unix.gettimeofday ();
        is_enter = false;
      }
    in
    match state_name with
    | "meerkat-build" ->
      state.outstanding_events <- transition state.outstanding_events event
    | _ -> ()
  in
  let is_meerkat_running () = is_in_state state.outstanding_events in
  { on_state_enter; on_state_leave; is_meerkat_running }

let handler = handler_fn ()

let on_state_enter state_name = handler.on_state_enter state_name

let on_state_leave state_name = handler.on_state_leave state_name

let is_meerkat_running = handler.is_meerkat_running
