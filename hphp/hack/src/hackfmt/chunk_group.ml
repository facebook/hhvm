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

type t = {
  chunks: Chunk.t list;
  rule_map: Rule.t IMap.t;
  rule_dependency_map: (int list) IMap.t;
  block_indentation: int;
}

let get_rule_count t =
  IMap.cardinal t.rule_map

let get_rules t =
  (* TODO verify or log if there are unused rules *)
  List.map (IMap.bindings t.rule_map) ~f:fst

let get_rule_kind t id =
  let r = IMap.find_unsafe id t.rule_map in
  r.Rule.kind

let get_char_range t =
  t.chunks |> List.fold ~init:(max_int,0)
    ~f:(fun (start_char, end_char) chunk ->
      min start_char chunk.Chunk.start_char,
      max end_char chunk.Chunk.end_char
    )

let constrain_rules t rbm rule_list =
  let aux rule_id = Rule.cares_about_children (get_rule_kind t rule_id) in
  let rules_that_care = List.filter rule_list ~f:aux in
  List.fold rules_that_care ~init:rbm ~f:(fun acc k -> IMap.add k true acc)

let get_initial_rule_bindings t =
  let is_always_rule _k v = v.Rule.kind = Rule.Always in
  let always_rules = IMap.filter is_always_rule t.rule_map in
  let get_dependencies rule_id =
    try IMap.find_unsafe rule_id t.rule_dependency_map with Not_found -> [] in
  let constrain k _v acc = constrain_rules t acc (get_dependencies k) in
  let init_map = IMap.map (fun _ -> true) always_rules in
  IMap.fold constrain always_rules init_map

(* When a child rule is broken on, all its parent rules must break too. *)
let is_dependency_satisfied parent_kind parent_val child_val =
  (* If the parent rule doesn't care about whether its child rules break, then
   * the dependency doesn't matter. *)
  if not (Rule.cares_about_children parent_kind) then true
  else
    (* Otherwise, the dependency is only unsatisfied when the parent rule is
     * bound not to break and the child rule is bound to break. *)
    match parent_val, child_val with
    | Some false, true -> false
    | _ -> true

let are_rule_bindings_valid t rbm =
  let valid_map = IMap.mapi (fun rule_id v ->
    let parent_list = try IMap.find_unsafe rule_id t.rule_dependency_map
      with Not_found -> []
    in
    List.for_all parent_list ~f:(fun parent_id ->
      let parent_rule = IMap.find_unsafe parent_id t.rule_map in
      let parent_value = IMap.get parent_id rbm in
      is_dependency_satisfied parent_rule.Rule.kind parent_value v
    )
  ) rbm in
  List.for_all ~f:(fun x -> x) @@ List.map ~f:snd @@ IMap.bindings valid_map

let dependency_map_to_string t =
  let get_map_values map = List.map ~f:snd @@ IMap.bindings @@ map in
  let str_list = get_map_values @@ IMap.mapi (fun k v_list ->
    let values = List.map v_list ~f:string_of_int in
    string_of_int k ^ ": [" ^ String.concat ", " values ^ "]"
  ) t.rule_dependency_map in
  "{" ^ String.concat ", " str_list ^ "}"
