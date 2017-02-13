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

let _LINE_WIDTH = 80

type t = {
  chunk_group: Chunk_group.t;
  rvm: int IMap.t;
  nesting_set: ISet.t;
  cost: int;
  overflow: int;
}

let has_split_before_chunk c rvm =
  let rule_id = c.Chunk.rule in
  let value = IMap.get rule_id rvm in
  Rule.is_split rule_id value

let has_comma_after_chunk c rvm =
  Option.value_map c.Chunk.comma_rule ~default:false ~f:(fun rule_id ->
    let value = IMap.get rule_id rvm in
    Rule.is_split rule_id value
  )

let get_overflow len = max (len - _LINE_WIDTH) 0

let make chunk_group rvm =
  let { Chunk_group.chunks; block_indentation; _ } = chunk_group in
  let len = 0 in
  let cost = 0 in
  let overflow = 0 in
  let acc = len, cost, overflow in

  let nesting_set, _ =
    List.fold_left chunks ~init:(ISet.empty, ISet.empty)
      (* We only care about the first occurance of each nesting id *)
      ~f:(fun (nset, idset) c ->
        let nid = Chunk.get_nesting_id c in
        if ISet.mem nid idset then
          nset, idset
        else
        if has_split_before_chunk c rvm then
          ISet.add nid nset, ISet.add nid idset
        else
          nset, ISet.add nid idset
      )
  in

  (* keep track of current length, cost of this state, total overflow chars *)
  let (len, cost, overflow)  =
    List.fold_left chunks ~init:acc ~f:(fun (len, cost, overflow) c ->
      let len, cost, overflow = if has_split_before_chunk c rvm then
        let overflow = overflow + (get_overflow len) in
        let len = Nesting.get_indent c.Chunk.nesting nesting_set in
        let len = len + block_indentation in
        len, cost, overflow
      else
        let len = if c.Chunk.space_if_not_split then len + 1 else len in
        len, cost, overflow
      in

      let len = len + (String.length c.Chunk.text) in
      let len = if has_comma_after_chunk c rvm then len + 1 else len in
      len, cost, overflow
    ) in

  (* calculate the overflow of the last chunk *)
  let overflow = overflow + (get_overflow len) in

  (* calculate cost of all of the spans that are split *)
  let span_cost_map = List.fold chunks ~init:IMap.empty ~f:(fun acc c ->
    if has_split_before_chunk c rvm then
      List.fold ~init:acc c.Chunk.spans ~f:(fun acc s ->
        if IMap.mem s.Span.id acc
        then acc
        else IMap.add s.Span.id (Cost.get_cost s.Span.cost) acc
      )
    else acc
  ) in
  let span_cost = IMap.fold (fun _k v acc -> acc + v) span_cost_map 0 in

  (* add to cost the cost of all rules that are split *)
  let rule_cost = (
    IMap.fold (fun r_id v acc ->
      if (Rule.is_split r_id (Some v)) then
        acc + (Rule.get_cost (Chunk_group.get_rule_kind chunk_group r_id))
      else
        acc
    ) rvm 0
  ) in

  let cost = span_cost + rule_cost in
  { chunk_group; rvm; cost; overflow; nesting_set; }

let is_rule_bound t rule_id =
  IMap.mem rule_id t.rvm

let get_candidate_rules t =
  let { Chunk_group.chunks; block_indentation; _ } = t.chunk_group in
  let candidate_rules, _, _ = List.fold chunks ~init:(ISet.empty, 0, false) ~f:(
    fun (rules, len, found_overflow) c ->
      if found_overflow
      then rules, len, true
      else if has_split_before_chunk c t.rvm &&
        get_overflow len > 0 &&
        ISet.diff rules (ISet.of_list @@ IMap.keys t.rvm) <> ISet.empty
      then rules, len, true
      else
        let rules, len = if has_split_before_chunk c t.rvm then
          let len = Nesting.get_indent c.Chunk.nesting t.nesting_set in
          ISet.empty, len + block_indentation
        else
          rules, if c.Chunk.space_if_not_split then len + 1 else len
        in
        let len = len + (String.length c.Chunk.text) in
        let len = if has_comma_after_chunk c t.rvm
          then len + 1 else len in
        let rules = ISet.add c.Chunk.rule rules in
        rules, len, false
  ) in

  (* Also add parent rules *)
  ISet.elements @@ ISet.fold (fun id acc ->
    let acc = ISet.add id acc in
    let rules = try
      IMap.find_unsafe id t.chunk_group.Chunk_group.rule_dependency_map
      with Not_found -> [] in
    List.fold rules ~init:acc ~f:(fun acc id -> ISet.add id acc)
  ) candidate_rules ISet.empty

let compare_rule_sets s1 s2 =
  let bound_rule_ids = List.sort_uniq ~cmp:Pervasives.compare @@
    IMap.keys s1.rvm @ IMap.keys s2.rvm in
  let is_split rule_id state = Rule.is_split () @@ IMap.get rule_id state.rvm in
  let rec aux = function
    | [] -> 0
    | rule_id :: ids ->
      let diff = compare (is_split rule_id s1) (is_split rule_id s2) in
      if diff <> 0 then diff else aux ids
  in
  aux bound_rule_ids

let compare s1 s2 =
  if s1.cost <> s2.cost
  then s1.cost - s2.cost
  else if s1.overflow <> s2.overflow
    then s1.overflow - s2.overflow
    else compare_rule_sets s1 s2

let pick_best_state s1 s2 =
  if s1.overflow <> s2.overflow then begin
    if s1.overflow < s2.overflow then s1 else s2
  end else if compare s1 s2 < 0 then s1 else s2

let __debug t =
  (* TODO: make a new rule strings string *)
  let rule_strings = List.map (IMap.bindings t.rvm) (fun (k, v) ->
    string_of_int k ^ ": " ^ string_of_int v
  ) in
  let rule_count = string_of_int (Chunk_group.get_rule_count t.chunk_group) in
  let rule_str = rule_count ^ " [" ^ (String.concat "," rule_strings) ^ "]" in
  (string_of_int t.overflow) ^ "," ^ (string_of_int t.cost) ^ " " ^ rule_str
