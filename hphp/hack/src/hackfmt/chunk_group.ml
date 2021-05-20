(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  chunks: Chunk.t list;
  rule_map: Rule.t IMap.t;
  rule_dependency_map: int list IMap.t;
  block_indentation: int;
}

let get_rule_count t = IMap.cardinal t.rule_map

let get_rules t =
  (* TODO verify or log if there are unused rules *)
  List.map (IMap.bindings t.rule_map) ~f:fst

let get_rule_kind t id =
  let r = IMap.find id t.rule_map in
  r.Rule.kind

let get_char_range t =
  t.chunks
  |> List.fold ~init:(Int.max_value, 0) ~f:(fun (start_char, end_char) chunk ->
         let (chunk_start, chunk_end) = Chunk.get_range chunk in
         (min start_char chunk_start, max end_char chunk_end))

let propagate_breakage t initial_bindings =
  initial_bindings
  |> IMap.filter (fun _ is_broken -> is_broken)
  |> IMap.keys
  |> List.fold ~init:initial_bindings ~f:(fun acc rule_id ->
         let dependencies =
           Option.value
             ~default:[]
             (IMap.find_opt rule_id t.rule_dependency_map)
         in
         dependencies
         |> List.filter ~f:(fun id ->
                Rule.cares_about_children (get_rule_kind t id))
         |> List.fold ~init:acc ~f:(fun acc id -> IMap.add id true acc))

let get_always_rules t =
  t.rule_map |> IMap.filter (fun _ v -> Rule.is_always v.Rule.kind) |> IMap.keys

let get_always_rule_bindings t =
  get_always_rules t
  |> List.fold ~init:IMap.empty ~f:(fun acc id -> IMap.add id true acc)

let get_initial_rule_bindings t =
  propagate_breakage t (get_always_rule_bindings t)

(* When a child rule is broken on, all its parent rules must break too. *)
let is_dependency_satisfied parent_kind parent_val child_val =
  (* If the parent rule doesn't care about whether its child rules break, then
   * the dependency doesn't matter. *)
  if not (Rule.cares_about_children parent_kind) then
    true
  else
    (* Otherwise, the dependency is only unsatisfied when the parent rule is
     * bound not to break and the child rule is bound to break. *)
    match (parent_val, child_val) with
    | (Some false, true) -> false
    | _ -> true

let are_rule_bindings_valid t rbm =
  let valid_map =
    IMap.mapi
      (fun rule_id v ->
        let parent_list =
          Option.value ~default:[] (IMap.find_opt rule_id t.rule_dependency_map)
        in
        List.for_all parent_list ~f:(fun parent_id ->
            let parent_rule = IMap.find parent_id t.rule_map in
            let parent_value = IMap.find_opt parent_id rbm in
            is_dependency_satisfied parent_rule.Rule.kind parent_value v))
      rbm
  in
  List.for_all ~f:(fun x -> x) @@ List.map ~f:snd @@ IMap.bindings valid_map

let dependency_map_to_string t =
  let get_map_values map = List.map ~f:snd @@ IMap.bindings @@ map in
  let str_list =
    get_map_values
    @@ IMap.mapi
         (fun k v_list ->
           let values = List.map v_list ~f:string_of_int in
           string_of_int k ^ ": [" ^ String.concat ~sep:", " values ^ "]")
         t.rule_dependency_map
  in
  "{" ^ String.concat ~sep:", " str_list ^ "}"
