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
  lines: (int * ISet.t) list;
  rvm: int IMap.t;
  nesting_set: ISet.t;
  cost: int;
  overflow: int;
  candidate_rules: ISet.t;

  (* Expensive calculation cache *)
  unprocessed_overflow: int Lazy.t;
  rules_on_partially_bound_lines: ISet.t Lazy.t;
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

let get_bound_ruleset rvm = ISet.of_list @@ IMap.keys rvm

let get_overflow len = max (len - _LINE_WIDTH) 0

(**
 * Create a list of lines
 *
 * Each element of lines a tuple of (int, ISet.t)
 * Which correspond to the overflow and set of rules that correspond to
 * a particular line of output for a given Solve_state
 *)
let build_lines chunk_group rvm nesting_set =
  let { Chunk_group.chunks; block_indentation; _ } = chunk_group in

  let get_text_length chunk ~has_comma =
    let comma_len = if has_comma then 1 else 0 in
    comma_len + String.length chunk.Chunk.text
  in
  let get_prefix_whitespace_length chunk ~is_split =
    if is_split && chunk.Chunk.indentable
    then block_indentation + Nesting.get_indent chunk.Chunk.nesting nesting_set
    else if chunk.Chunk.space_if_not_split then 1 else 0
  in

  let rec aux remaining_chunks acc =
    let (acc_len, acc_rules) = acc in
    match remaining_chunks with
      | [] -> [(get_overflow acc_len, acc_rules)]
      | hd :: tl ->
        (* TODO: consider adding parent rules *)
        let rule = hd.Chunk.rule in
        let is_split = has_split_before_chunk hd rvm in
        let has_comma = has_comma_after_chunk hd rvm in
        let chunk_len = get_text_length hd ~has_comma +
          get_prefix_whitespace_length hd ~is_split in

        if is_split
        then (get_overflow acc_len, acc_rules) ::
          aux tl (chunk_len, ISet.add rule ISet.empty)
        else aux tl (chunk_len + acc_len, ISet.add rule acc_rules)
  in
  aux chunks (0, ISet.empty)

let build_candidate_rules_and_update_rvm rvm lines rule_dependency_map =
  let bound_rules = get_bound_ruleset rvm in
  let rec get_candidate_and_dead_rules lines dead_rules =
    match lines with
      | [] -> ISet.empty, dead_rules
      | hd :: tl ->
          let (overflow, rules) = hd in
          let unbound_rules = ISet.diff rules bound_rules in
          if overflow = 0 || ISet.is_empty unbound_rules
          then get_candidate_and_dead_rules tl @@
            ISet.union dead_rules unbound_rules
          else rules, dead_rules
  in
  let base_candidate_rules, dead_rules =
    get_candidate_and_dead_rules lines ISet.empty in

  (* Also add parent rules *)
  let deps = rule_dependency_map in
  let candidate_rules = ISet.fold (fun id acc ->
    ISet.union acc @@
      try ISet.of_list @@ IMap.find_unsafe id deps with Not_found -> ISet.empty
  ) base_candidate_rules base_candidate_rules in

  let dead_rules = ISet.diff dead_rules candidate_rules in
  let rvm = ISet.fold (fun r acc ->
    if not (IMap.mem r rvm)
    then IMap.add r 0 acc
    else acc
  ) dead_rules rvm in
  candidate_rules, rvm

let calculate_unprocessed_overflow lines bound_ruleset =
  List.fold lines ~init:0 ~f:(fun acc (overflow, rules) ->
    if ISet.is_empty @@ ISet.diff rules bound_ruleset
    then acc
    else acc + overflow
  )

let calculate_rules_on_partially_bound_lines lines bound_ruleset =
  let rules_per_line = List.map lines ~f:snd in
  List.fold rules_per_line ~init:ISet.empty ~f:(
    fun acc set ->
      let diff = ISet.diff set bound_ruleset in
      if ISet.cardinal diff <> 0 (* Fully bound line *)
      (* Add rules for partially bound lines *)
      then ISet.union acc @@ ISet.inter set bound_ruleset
      else acc
  )


let make chunk_group rvm =
  let { Chunk_group.chunks; block_indentation; rule_dependency_map; _ } =
    chunk_group in

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

  let lines = build_lines chunk_group rvm nesting_set in

  (* calculate the overflow of the last chunk *)
  let overflow = List.fold ~init:0 ~f:(+) @@ List.map ~f:fst lines in

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

  (* Precompute candidate_rules and update the rvm by binding unbound rules on
    lines with 0 overflow to 0 **)
  let candidate_rules, rvm =
    build_candidate_rules_and_update_rvm rvm lines rule_dependency_map in

  let bound_ruleset = get_bound_ruleset rvm in
  let unprocessed_overflow =
    lazy (calculate_unprocessed_overflow lines bound_ruleset) in
  let rules_on_partially_bound_lines =
    lazy (calculate_rules_on_partially_bound_lines lines bound_ruleset) in

  { chunk_group; lines; rvm; cost; overflow; nesting_set; candidate_rules;
    unprocessed_overflow; rules_on_partially_bound_lines; }

let is_rule_bound t rule_id =
  IMap.mem rule_id t.rvm

let get_candidate_rules t = t.candidate_rules

let get_unprocessed_overflow t = Lazy.force t.unprocessed_overflow

let get_rules_on_partially_bound_lines t =
  Lazy.force t.rules_on_partially_bound_lines

(**
 * The idea behind overlapping states, is that sometimes we have two states in
 * the queue where all expansions of one will be strictly worse than all
 * expansions of the other. In this case we want to only keep one of the states
 * in order to reduce branching.
 *
 * The implementation here is meant to target list-like structures where
 * we come up with different solutions for a particular list item.
 *)
let is_overlapping s1 s2 =
  get_unprocessed_overflow s1 = get_unprocessed_overflow s2 &&
    let s1_rules = get_rules_on_partially_bound_lines s1 in
    let s2_rules = get_rules_on_partially_bound_lines s2 in
    ISet.cardinal s1_rules = ISet.cardinal s2_rules &&
    ISet.for_all (fun s1_key ->
      IMap.find_unsafe s1_key s1.rvm =
        try IMap.find_unsafe s1_key s2.rvm with Not_found -> -1) s1_rules

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

let compare_overlap s1 s2 =
  if not (is_overlapping s1 s2)
  then None
  else Some (pick_best_state s1 s2)

let __debug t =
  (* TODO: make a new rule strings string *)
  let rule_strings = List.map (IMap.bindings t.rvm) (fun (k, v) ->
    string_of_int k ^ ": " ^ string_of_int v
  ) in
  let rule_count = string_of_int (Chunk_group.get_rule_count t.chunk_group) in
  let rule_str = rule_count ^ " [" ^ (String.concat "," rule_strings) ^ "]" in
  (string_of_int t.overflow) ^ "," ^ (string_of_int t.cost) ^ " " ^ rule_str
