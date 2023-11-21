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

type t = {
  chunk_group: Chunk_group.t;
  lines: (int * ISet.t) list;
  (* Rule bindings map.
   * Rules in this map are bound to be broken on or not broken on in this solve
   * state. Rules not in the map are not yet bound. A rule that is bound to be
   * broken on will have all its splits broken in the solution. *)
  rbm: bool IMap.t;
  nesting_set: ISet.t;
  cost: int;
  overflow: int;
  candidate_rules: ISet.t;
  (* Expensive calculation cache *)
  unprocessed_overflow: int Lazy.t;
  rules_on_partially_bound_lines: ISet.t Lazy.t;
}

let chunks t = t.chunk_group.Chunk_group.chunks

let rbm_has_split_before_chunk c rbm =
  let rule_id = c.Chunk.rule in
  IMap.find_opt rule_id rbm |> Option.value ~default:false

let rbm_has_comma_after_chunk c rbm =
  Option.value_map c.Chunk.comma ~default:false ~f:(fun (rule_id, _) ->
      IMap.find_opt rule_id rbm |> Option.value ~default:false)

let has_split_before_chunk t ~chunk = rbm_has_split_before_chunk chunk t.rbm

let has_comma_after_chunk t ~chunk = rbm_has_comma_after_chunk chunk t.rbm

let get_bound_ruleset rbm = ISet.of_list @@ IMap.keys rbm

let get_overflow env len = max (len - env.Env.line_width) 0

let compute_indent_level chunk_group nesting_set chunk =
  let block_indentation = chunk_group.Chunk_group.block_indentation in
  block_indentation + Nesting.get_indent_level chunk.Chunk.nesting nesting_set

let get_indent_level t chunk =
  compute_indent_level t.chunk_group t.nesting_set chunk

let get_indent_columns env chunk_group nesting_set chunk =
  env.Env.indent_width * compute_indent_level chunk_group nesting_set chunk

(**
 * Create a list of lines
 *
 * Each element of lines a tuple of (int, ISet.t)
 * Which correspond to the overflow and set of rules that correspond to
 * a particular line of output for a given Solve_state
 *)
let build_lines env chunk_group rbm nesting_set =
  let { Chunk_group.chunks; _ } = chunk_group in
  let get_text_length chunk ~has_comma =
    let comma_len =
      if has_comma then
        1
      else
        0
    in
    comma_len + chunk.Chunk.length
  in
  let get_prefix_whitespace_length env chunk ~is_split =
    if is_split && chunk.Chunk.indentable then
      get_indent_columns env chunk_group nesting_set chunk
    else if chunk.Chunk.space_if_not_split then
      1
    else
      0
  in
  let rec aux remaining_chunks acc =
    let (acc_len, acc_rules) = acc in
    match remaining_chunks with
    | [] -> [(get_overflow env acc_len, acc_rules)]
    | hd :: tl ->
      (* TODO: consider adding parent rules *)
      let rule = hd.Chunk.rule in
      let is_split = rbm_has_split_before_chunk hd rbm in
      let has_comma = rbm_has_comma_after_chunk hd rbm in
      let chunk_len =
        get_text_length hd ~has_comma
        + get_prefix_whitespace_length env hd ~is_split
      in
      if is_split then
        (get_overflow env acc_len, acc_rules)
        :: aux tl (chunk_len, ISet.add rule ISet.empty)
      else
        aux tl (chunk_len + acc_len, ISet.add rule acc_rules)
  in
  aux chunks (0, ISet.empty)

let build_candidate_rules_and_update_rbm rbm lines rule_dependency_map =
  let bound_rules = get_bound_ruleset rbm in
  let rec get_candidate_and_dead_rules lines dead_rules =
    match lines with
    | [] -> (ISet.empty, dead_rules)
    | hd :: tl ->
      let (overflow, rules) = hd in
      let unbound_rules = ISet.diff rules bound_rules in
      if overflow = 0 || ISet.is_empty unbound_rules then
        get_candidate_and_dead_rules tl @@ ISet.union dead_rules unbound_rules
      else
        (rules, dead_rules)
  in
  let (base_candidate_rules, dead_rules) =
    get_candidate_and_dead_rules lines ISet.empty
  in
  (* Also add parent rules *)
  let deps = rule_dependency_map in
  let candidate_rules =
    ISet.fold
      (fun id acc ->
        let rules = Option.value ~default:[] (IMap.find_opt id deps) in
        ISet.union acc @@ ISet.of_list rules)
      base_candidate_rules
      base_candidate_rules
  in
  let dead_rules = ISet.diff dead_rules candidate_rules in
  let rbm =
    ISet.fold
      (fun r acc ->
        if not (IMap.mem r rbm) then
          IMap.add r false acc
        else
          acc)
      dead_rules
      rbm
  in
  (candidate_rules, rbm)

let calculate_unprocessed_overflow lines bound_ruleset =
  List.fold lines ~init:0 ~f:(fun acc (overflow, rules) ->
      if ISet.is_empty @@ ISet.diff rules bound_ruleset then
        acc
      else
        acc + overflow)

let calculate_rules_on_partially_bound_lines lines bound_ruleset =
  let rules_per_line = List.map lines ~f:snd in
  List.fold rules_per_line ~init:ISet.empty ~f:(fun acc set ->
      let diff = ISet.diff set bound_ruleset in
      if
        ISet.cardinal diff <> 0
        (* Fully bound line *)
        (* Add rules for partially bound lines *)
      then
        ISet.union acc @@ ISet.inter set bound_ruleset
      else
        acc)

let make env chunk_group rbm =
  let { Chunk_group.chunks; rule_dependency_map; _ } = chunk_group in
  let (nesting_set, _) =
    List.fold_left
      chunks
      ~init:(ISet.empty, ISet.empty)
        (* We only care about the first occurance of each nesting id *)
      ~f:(fun (nset, idset) c ->
        let nid = Chunk.get_nesting_id c in
        if ISet.mem nid idset then
          (nset, idset)
        else if rbm_has_split_before_chunk c rbm then
          (ISet.add nid nset, ISet.add nid idset)
        else
          (nset, ISet.add nid idset))
  in
  let lines = build_lines env chunk_group rbm nesting_set in
  (* calculate the overflow of the last chunk *)
  let overflow = List.fold ~init:0 ~f:( + ) @@ List.map ~f:fst lines in
  (* add to cost the number of spans that are split
   * (implicitly giving each span a cost of 1) *)
  let broken_spans =
    List.fold chunks ~init:ISet.empty ~f:(fun acc c ->
        if rbm_has_split_before_chunk c rbm then
          c.Chunk.spans
          |> List.map ~f:Span.id
          |> List.fold_right ~init:acc ~f:ISet.add
        else
          acc)
  in
  let span_cost = ISet.cardinal broken_spans in
  (* add to cost the cost of all rules that are split *)
  let rule_cost =
    IMap.fold
      (fun r_id v acc ->
        if v then
          acc + Rule.get_cost (Chunk_group.get_rule_kind chunk_group r_id)
        else
          acc)
      rbm
      0
  in
  let cost = span_cost + rule_cost in
  (* Precompute candidate_rules and update the rbm by binding unbound rules on
   * lines with 0 overflow to false *)
  let (candidate_rules, rbm) =
    build_candidate_rules_and_update_rbm rbm lines rule_dependency_map
  in
  let bound_ruleset = get_bound_ruleset rbm in
  let unprocessed_overflow =
    lazy (calculate_unprocessed_overflow lines bound_ruleset)
  in
  let rules_on_partially_bound_lines =
    lazy (calculate_rules_on_partially_bound_lines lines bound_ruleset)
  in
  {
    chunk_group;
    lines;
    rbm;
    cost;
    overflow;
    nesting_set;
    candidate_rules;
    unprocessed_overflow;
    rules_on_partially_bound_lines;
  }

let add_breaks_from_source rbm source_text chunk_group =
  let (rbm, _) =
    List.fold
      chunk_group.Chunk_group.chunks
      ~init:(rbm, 0)
      ~f:(fun (rbm, prev_chunk_end) chunk ->
        let (chunk_start, chunk_end) = Chunk.get_range chunk in
        let rbm =
          let rec aux i =
            i < chunk_start && (Char.equal source_text.[i] '\n' || aux (i + 1))
          in
          if aux prev_chunk_end then
            IMap.add chunk.Chunk.rule true rbm
          else
            rbm
        in
        (rbm, chunk_end))
  in
  rbm

let rbm_from_source source_text chunk_group =
  let rbm = IMap.empty in
  add_breaks_from_source rbm source_text chunk_group

(** When we are unable to find a good solution, this function produces a
    "best-effort" solution based on the original source text. We break the rules
    configured to always break, then break any rules which appear to be broken
    in the original source text, then propagate breakage (breaking any parental
    rules containing those rules we already bound to be broken on). *)
let from_source env source_text chunk_group =
  let rbm = Chunk_group.get_always_rule_bindings chunk_group in
  let rbm = add_breaks_from_source rbm source_text chunk_group in
  let rbm = Chunk_group.propagate_breakage chunk_group rbm in
  make env chunk_group rbm

(** Every rule is broken. Everything is going hog wild. *)
let rbm_broken_everywhere chunk_group =
  Chunk_group.get_rules chunk_group
  |> List.fold ~init:IMap.empty ~f:(fun acc k -> IMap.add k true acc)

let from_rbm env rbm chunk_group = make env chunk_group rbm

let is_rule_bound t rule_id = IMap.mem rule_id t.rbm

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
  get_unprocessed_overflow s1 = get_unprocessed_overflow s2
  &&
  let s1_rules = get_rules_on_partially_bound_lines s1 in
  let s2_rules = get_rules_on_partially_bound_lines s2 in
  ISet.cardinal s1_rules = ISet.cardinal s2_rules
  && ISet.for_all
       (fun s1_key ->
         Option.equal
           Bool.equal
           (IMap.find_opt s1_key s1.rbm)
           (IMap.find_opt s1_key s2.rbm))
       s1_rules

let compare_rule_sets s1 s2 =
  let bound_rule_ids =
    Stdlib.List.sort_uniq Int.compare @@ IMap.keys s1.rbm @ IMap.keys s2.rbm
  in
  let is_split rule_id state =
    IMap.find_opt rule_id state.rbm |> Option.value ~default:false
  in
  let rec aux = function
    | [] -> 0
    | rule_id :: ids ->
      let diff = Bool.compare (is_split rule_id s1) (is_split rule_id s2) in
      if diff <> 0 then
        diff
      else
        aux ids
  in
  aux bound_rule_ids

let compare s1 s2 =
  if s1.cost <> s2.cost then
    s1.cost - s2.cost
  else if s1.overflow <> s2.overflow then
    s1.overflow - s2.overflow
  else
    compare_rule_sets s1 s2

let pick_best_state s1 s2 =
  if s1.overflow <> s2.overflow then
    if s1.overflow < s2.overflow then
      s1
    else
      s2
  else if compare s1 s2 < 0 then
    s1
  else
    s2

let compare_overlap s1 s2 =
  if not (is_overlapping s1 s2) then
    None
  else
    Some (pick_best_state s1 s2)

let __debug t =
  (* TODO: make a new rule strings string *)
  let rule_strings =
    List.map (IMap.bindings t.rbm) ~f:(fun (k, v) ->
        string_of_int k ^ ": " ^ string_of_bool v)
  in
  let rule_count = string_of_int (Chunk_group.get_rule_count t.chunk_group) in
  let rule_str =
    rule_count ^ " [" ^ String.concat ~sep:"," rule_strings ^ "]"
  in
  string_of_int t.overflow ^ "," ^ string_of_int t.cost ^ " " ^ rule_str
