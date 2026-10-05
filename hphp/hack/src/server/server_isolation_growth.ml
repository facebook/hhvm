(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** What a batch of growth spent and where. Indexing and absorption have
    entirely different fixes, and one elapsed figure cannot tell them apart. *)
module Stats = struct
  type t = {
    index_seconds: float;
    absorb_seconds: float;
    candidates_tested: int;
    candidates_skipped: int;
        (** Candidates whose blocker has not moved, so not retested this
            round. *)
    absorbed_by_subset: int;
        (** Taken because everything referencing them was already inside. *)
    absorbed_by_closure: int;
        (** Taken along with everything referencing them — growth getting past a
            narrowly-shared file, which the cheap rule cannot do. *)
  }

  let zero =
    {
      index_seconds = 0.0;
      absorb_seconds = 0.0;
      candidates_tested = 0;
      candidates_skipped = 0;
      absorbed_by_subset = 0;
      absorbed_by_closure = 0;
    }

  let current = ref zero

  let get () = !current

  let reset () = current := zero

  (** Runs [f] and hands its duration to [add]. *)
  let timed add f =
    let t0 = Unix.gettimeofday () in
    let result = f () in
    add (Unix.gettimeofday () -. t0);
    result

  let add_index_seconds s =
    current := { !current with index_seconds = !current.index_seconds +. s }

  let add_absorb_seconds s =
    current := { !current with absorb_seconds = !current.absorb_seconds +. s }

  let record_candidate_tested () =
    current :=
      { !current with candidates_tested = !current.candidates_tested + 1 }

  let add_candidates_skipped n =
    current :=
      { !current with candidates_skipped = !current.candidates_skipped + n }

  let record_absorbed_by_subset () =
    current :=
      { !current with absorbed_by_subset = !current.absorbed_by_subset + 1 }

  let record_absorbed_by_closure () =
    current :=
      { !current with absorbed_by_closure = !current.absorbed_by_closure + 1 }
end

(** What depends on a candidate, minus what it defines itself. Asked once per
    candidate rather than once per cluster reaching it, and memoised: the walk
    is expensive and the files that recur on frontiers recur everywhere. *)
module Dependents = struct
  type t =
    | Outside of Typing_deps.DepSet.t
    | Too_many
        (** Past [cap], and not stored. This does not refuse the candidate: it
            falls through to the closure rule, which may still take it. *)

  (* Bounds both the walk and what the memo holds, so one candidate cannot keep
     thousands of hashes alive for the rest of the batch. Not measured — a round
     number above the clusters a run produces. Too low costs only the cheap
     rule, and with it the witness that would have let the candidate be skipped
     next round. *)
  let cap = 4096

  let cache : t Relative_path.Map.t ref = ref Relative_path.Map.empty

  (** [outside deps_mode candidate own own_set], where [own] is what the
      candidate defines and [own_set] the same as a set. *)
  let outside deps_mode candidate own own_set =
    match Relative_path.Map.find_opt !cache candidate with
    | Some cached -> cached
    | None ->
      let result =
        List.fold_until
          own
          ~init:(Typing_deps.DepSet.make ())
          ~f:(fun acc dep ->
            let acc =
              Typing_deps.DepSet.union
                acc
                (Typing_deps.DepSet.diff
                   (Typing_deps.get_ideps_from_hash deps_mode dep)
                   own_set)
            in
            if Typing_deps.DepSet.cardinal acc > cap then
              Container.Continue_or_stop.Stop Too_many
            else
              Container.Continue_or_stop.Continue acc)
          ~finish:(fun acc -> Outside acc)
      in
      cache := Relative_path.Map.add !cache ~key:candidate ~data:result;
      result

  (** The first dependent lying outside the cluster. Callers keep it as a
      witness: while it stays outside, the candidate cannot have become
      absorbable. *)
  let first_outside cluster_deps dependents =
    Typing_deps.DepSet.fold_result dependents ~init:() ~f:(fun dependent () ->
        if Typing_deps.DepSet.mem cluster_deps dependent then
          Ok ()
        else
          Error dependent)
    |> function
    | Ok () -> None
    | Error dependent -> Some dependent

  let cached_count () = Relative_path.Map.cardinal !cache

  let release () = cache := Relative_path.Map.empty
end

(** Whether one candidate can join one cluster, and what comes with it.
    Two rules, cheapest first; the expensive one runs only on what the cheap
    one turns down. *)
module Absorb = struct
  type absorption =
    | Absorbs of Relative_path.Set.t
    | Blocked_by of Typing_deps.Dep.t
    | Blocked
    | No_room

  (** Room left, or [None] when unbounded. Closure absorption is all-or-nothing, so
      checking the cap afterwards would not bound anything. *)
  let fits room n =
    match room with
    | None -> true
    | Some room -> n <= room

  type batch = {
    ctx: Provider_context.t;
    workers: Multi_worker.worker list option;
    deps_mode: Typing_deps.Mode.t;
    naming_table: Naming_table.t;
    claimed: Relative_path.Set.t;
    max_cluster_size: int option;
    max_dependents: int;
  }

  (* Where the subset rule stands: every dependent already inside, one named
     dependent outside, or too many dependents to have looked at all. *)
  type blocker =
    | Inside
    | Witness of Typing_deps.Dep.t
    | No_witness

  let subset_rule b cluster_deps candidate own own_set =
    match Dependents.outside b.deps_mode candidate own own_set with
    | Dependents.Too_many -> No_witness
    | Dependents.Outside outside ->
      (match Dependents.first_outside cluster_deps outside with
      | None -> Inside
      | Some dependent -> Witness dependent)

  (* [refused] is what the subset rule already concluded: both rules are monotone,
     so a candidate the closure rule also turns down stays blocked by the dependent
     the subset rule named. *)
  let closure_rule b ~held ~room ~refused candidate =
    match
      Server_isolation_inbound.get
        b.ctx
        b.naming_table
        ~max:b.max_dependents
        candidate
    with
    | Some closure
      when Relative_path.Set.is_empty
             (Relative_path.Set.inter closure b.claimed) ->
      (* Only files not already held count against the cap. A candidate is on the
         frontier because a held file references it, so that file depends on it and
         lies in its closure: measuring the whole closure would refuse ones that
         fit. *)
      let joining =
        match room with
        | None -> 0
        | Some _ ->
          Relative_path.Set.cardinal (Relative_path.Set.diff closure held)
      in
      if fits room joining then begin
        Stats.record_absorbed_by_closure ();
        Absorbs closure
      end else
        No_room
    | Some _
    | None ->
      refused

  let absorbable b ~held ~room cluster_deps candidate =
    match Naming_table.get_file_info b.naming_table candidate with
    | None -> Blocked
    | _ when not (fits room 1) -> No_room
    | Some file_info ->
      let own = Typing_deps.deps_of_file_info file_info in
      let own_set = Typing_deps.DepSet.of_list own in
      let closure_rule = closure_rule b ~held ~room in
      (match subset_rule b cluster_deps candidate own own_set with
      | Inside ->
        Stats.record_absorbed_by_subset ();
        Absorbs (Relative_path.Set.singleton candidate)
      | Witness dependent ->
        closure_rule ~refused:(Blocked_by dependent) candidate
      | No_witness -> closure_rule ~refused:Blocked candidate)
end
