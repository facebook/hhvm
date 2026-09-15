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
  rule_map: Rule.t I_map.t;
  dependency_map: int list I_map.t;
  next_id: int;
}

let make () =
  { rule_map = I_map.empty; dependency_map = I_map.empty; next_id = 0 }

let make_rule t rule_kind =
  let rule = { Rule.id = t.next_id; kind = rule_kind } in
  let t =
    {
      t with
      rule_map = I_map.add rule.Rule.id rule t.rule_map;
      next_id = t.next_id + 1;
    }
  in
  (t, rule)

(* TODO: figure out how to share this logic with chunk_group.ml *)
let get_rule_kind t id =
  let r = I_map.find id t.rule_map in
  r.Rule.kind

let mark_dependencies t lazy_rules active_rule_ids child_id =
  let lazy_rule_list = ISet.elements lazy_rules in
  let rule_ids = lazy_rule_list @ active_rule_ids in
  let new_dep_map =
    List.fold_left rule_ids ~init:t.dependency_map ~f:(fun dep_map id ->
        let rule = I_map.find id t.rule_map in
        if Rule.cares_about_children rule.Rule.kind then
          let dependency_list = I_map.find_opt child_id dep_map in
          I_map.add
            child_id
            (match dependency_list with
            | Some l -> id :: l
            | None -> [id])
            dep_map
        else
          dep_map)
  in
  { t with dependency_map = new_dep_map }
