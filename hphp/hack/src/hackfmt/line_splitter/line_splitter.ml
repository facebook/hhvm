(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

let expand_state state_queue state =
  let { Solve_state.chunk_group; rbm; _ } = state in
  let rule_ids = ISet.elements @@ Solve_state.get_candidate_rules state in

  let _, next_rbms = List.map_env rbm rule_ids ~f:(fun env_rbm rule_id ->
    if Solve_state.is_rule_bound state rule_id
    then env_rbm, None
    else begin
      let next_rbm_opt =
        Some (IMap.add rule_id true env_rbm)
          |> Option.filter ~f:(Chunk_group.are_rule_bindings_valid chunk_group)
      in
      let env_rbm = if Option.is_some next_rbm_opt
        then IMap.add rule_id false env_rbm
        else env_rbm
      in
      env_rbm, next_rbm_opt
    end
  ) in
  let next_rbms = List.filter_opt next_rbms in

  List.iter next_rbms ~f:(fun rbm ->
    let st = Solve_state.make state.Solve_state.chunk_group rbm in
    State_queue.push state_queue st;
  );
  state_queue


let find_best_state queue =
  let best = State_queue.pop queue in
  let queue = expand_state queue best in
  let rec aux count acc queue =
    if
      State_queue.is_empty queue ||
      count > 2000 ||
      acc.Solve_state.overflow = 0
    then acc
    else
      let state = State_queue.pop queue in
      let best = Solve_state.pick_best_state state acc in
      let queue = expand_state queue state in
      aux (count + 1) best queue;
  in
  aux 0 best queue

let find_solve_states ?range chunk_groups =
  let chunk_groups = match range with
    | None -> chunk_groups
    | Some range ->
      List.filter chunk_groups ~f:(fun chunk_group ->
        let group_range = Chunk_group.get_char_range chunk_group in
        Interval.intervals_overlap range group_range
      )
  in
  chunk_groups |> List.map ~f:(fun chunk_group ->
    let rbm = Chunk_group.get_initial_rule_bindings chunk_group in
    let init_state = Solve_state.make chunk_group rbm in
    let state_queue = State_queue.make init_state in
    find_best_state state_queue
  )

let print ?range solve_states =
  let formatted = solve_states
    |> List.map ~f:(State_printer.print_state ?range)
    |> String.concat ""
  in
  match range with
    | None -> formatted
    (* FIXME: This is a hack to work around the bizarre situation we're in,
     * where chunks are associated with the newline preceding them (because Bob
     * Nystrom suggested that that approach might be nicer than the
     * alternative), but chunk ranges associate tokens with the newline that
     * followed them (since that newline is in the token's trailing trivia).
     * Because chunks are associated with the split preceding them, printing a
     * range of chunks produces a newline before and not after. However, the
     * range we were provided associates newline characters with the line
     * preceding them, so the caller expects a newline after and not before. *)
    | Some _ -> (String_utils.lstrip formatted "\n") ^ "\n"

let solve ?range chunk_groups =
  chunk_groups
  |> find_solve_states ?range
  |> print ?range
