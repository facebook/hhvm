(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Format_env
open Hh_prelude
open Common

let expand_state env state =
  let { Solve_state.chunk_group; rbm; _ } = state in
  let rule_ids = ISet.elements @@ Solve_state.get_candidate_rules state in
  let (_, next_rbms) =
    List.map_env rbm rule_ids ~f:(fun env_rbm rule_id ->
        if Solve_state.is_rule_bound state rule_id then
          (env_rbm, None)
        else
          let next_rbm_opt =
            Some (IMap.add rule_id true env_rbm)
            |> Option.filter
                 ~f:(Chunk_group.are_rule_bindings_valid chunk_group)
          in
          let env_rbm =
            if Option.is_some next_rbm_opt then
              IMap.add rule_id false env_rbm
            else
              env_rbm
          in
          (env_rbm, next_rbm_opt))
  in
  next_rbms |> List.filter_opt |> List.map ~f:(Solve_state.make env chunk_group)

let find_best_state env init_state =
  let queue = State_queue.make_empty 7 in
  List.iter (expand_state env init_state) ~f:(State_queue.push queue);
  let rec aux count best =
    if count > 100 then
      None
    else if State_queue.is_empty queue || best.Solve_state.overflow = 0 then
      Some best
    else
      let next_state = State_queue.pop queue in
      List.iter (expand_state env next_state) ~f:(State_queue.push queue);
      aux (count + 1) (Solve_state.pick_best_state next_state best)
  in
  aux 0 init_state

let solve_chunk_group env ?range ?source_text chunk_group =
  let rbm =
    match range with
    | Some range
      when let group_range = Chunk_group.get_char_range chunk_group in
           let (st, ed) = range in
           Interval.contains group_range st || Interval.contains group_range ed
      ->
      let source_rbm =
        match source_text with
        | Some st -> Solve_state.rbm_from_source st chunk_group
        | None -> Solve_state.rbm_broken_everywhere chunk_group
      in
      (* Build two lists of rule IDs: the rules associated with at least one
         split outside of the formatting range, and the rules associated with at
         least one split inside the formatting range. A rule may occur in both
         lists if it is associated with splits both inside and outside the
         formatting range. *)
      let (rules_in_range, rules_out_of_range) =
        List.partition_map chunk_group.Chunk_group.chunks ~f:(fun chunk ->
            (* Each chunk is preceded by a split, and contains the ID of the rule
             governing that split. *)
            let rule = chunk.Chunk.rule in
            (* We consider that split to be within the range when the range
             contains the first character in the chunk. *)
            if Interval.contains range chunk.Chunk.start_char then
              First rule
            else
              Second rule)
      in
      let iset_of_list = List.fold_right ~init:ISet.empty ~f:ISet.add in
      let rules_in_range = iset_of_list rules_in_range in
      let rules_out_of_range = iset_of_list rules_out_of_range in
      let always_rules =
        iset_of_list (Chunk_group.get_always_rules chunk_group)
      in
      let rules_entirely_in_range =
        ISet.diff rules_in_range rules_out_of_range
      in
      let always_rules_in_range = ISet.inter always_rules rules_in_range in
      let bindings =
        source_rbm
        (* If we have a rule associated with a split outside of the formatting
           range which was broken in the original source, the output will look
           strange if we don't break all of that rule's associated splits inside
           the formatting range, too. *)
        |> IMap.filter (fun id broke -> broke && ISet.mem id rules_out_of_range)
        (* We should also break any rule which is configured to ALWAYS break
           (such as the rule governing the split after a single-line comment)
           and has a split inside the formatting range. *)
        |> ISet.fold (fun id -> IMap.add id true) always_rules_in_range
      in
      let propagated =
        bindings
        (* Break any Parental rules which contain the rules we decided to break
           above... *)
        |> Chunk_group.propagate_breakage chunk_group
        (* ...But only do this for rules which do not have any associated splits
           outside of the formatting range. *)
        |> IMap.filter (fun id _ -> ISet.mem id rules_entirely_in_range)
      in
      IMap.union bindings propagated
    | _ -> Chunk_group.get_initial_rule_bindings chunk_group
  in
  let init_state = Solve_state.make env chunk_group rbm in
  match find_best_state env init_state with
  | Some state -> state
  | None ->
    begin
      match source_text with
      | Some s -> Solve_state.from_source env s chunk_group
      | None ->
        let rbm = Solve_state.rbm_broken_everywhere chunk_group in
        Solve_state.from_rbm env rbm chunk_group
    end

let find_solve_states
    (env : Env.t)
    ?(range : Interval.t option)
    ?(source_text : string option)
    (chunk_groups : Chunk_group.t list) : Solve_state.t list =
  let chunk_groups =
    match range with
    | None -> chunk_groups
    | Some range ->
      List.filter chunk_groups ~f:(fun chunk_group ->
          let group_range = Chunk_group.get_char_range chunk_group in
          Interval.intervals_overlap range group_range)
  in
  chunk_groups |> List.map ~f:(solve_chunk_group ?range env ?source_text)

let print
    (env : Env.t)
    ?(range : Interval.t option)
    ?(include_leading_whitespace = true)
    ?(include_trailing_whitespace = true)
    (solve_states : Solve_state.t list) : string =
  let filter_to_range subchunks =
    match range with
    | None -> subchunks
    | Some range ->
      Subchunk.subchunks_in_range
        ~include_leading_whitespace
        ~include_trailing_whitespace
        subchunks
        range
  in
  solve_states
  |> List.concat_map ~f:Subchunk.subchunks_of_solve_state
  |> filter_to_range
  |> Subchunk.string_of_subchunks env

let solve
    (env : Env.t)
    ?(range : Interval.t option)
    ?(include_leading_whitespace : bool option)
    ?(include_trailing_whitespace : bool option)
    ?(source_text : string option)
    (chunk_groups : Chunk_group.t list) : string =
  chunk_groups
  |> find_solve_states env ?range ?source_text
  |> print env ?range ?include_leading_whitespace ?include_trailing_whitespace

let unbroken_solve_state env chunk_group =
  let rbm = Chunk_group.get_initial_rule_bindings chunk_group in
  Solve_state.make env chunk_group rbm

let unbroken env chunk_groups =
  chunk_groups |> List.map ~f:(unbroken_solve_state env)
