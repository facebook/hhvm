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
  let { Solve_state.chunk_group; rvm; _ } = state in
  let rule_ids = ISet.elements @@ Solve_state.get_candidate_rules state in

  let _, next_rvms = List.map_env rvm rule_ids ~f:(fun env_rvm rule_id ->
    if Solve_state.is_rule_bound state rule_id
    then env_rvm, []
    else begin
      let possible_values = Rule.get_possible_values rule_id  in
      let next_rvms = List.filter_map possible_values ~f:(fun v ->
        let next_rvm = IMap.add rule_id v env_rvm in
        if Chunk_group.is_rule_value_map_valid chunk_group next_rvm
        then Some next_rvm
        else None
      ) in
      let env_rvm = if List.length next_rvms = List.length possible_values
        then IMap.add rule_id 0 env_rvm
        else env_rvm
      in
      env_rvm, next_rvms
    end
  ) in
  let next_rvms = List.concat next_rvms in

  List.iter next_rvms ~f:(fun rvm ->
    let st = Solve_state.make state.Solve_state.chunk_group rvm in
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

let solve chunk_groups =
  let best_states = List.map chunk_groups ~f:(fun chunk_group ->
    let rvm = Chunk_group.get_initial_rule_value_map chunk_group in
    let init_state = Solve_state.make chunk_group rvm in
    let state_queue = State_queue.make init_state in
    find_best_state state_queue
  ) in
  let strings = List.map best_states ~f:(fun ss ->
    Printf.sprintf "%s" (State_printer.print_state ss)
  ) in
  String.concat "\n" strings ^ "\n"
