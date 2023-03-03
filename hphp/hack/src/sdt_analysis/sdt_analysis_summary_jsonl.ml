(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types
module J = Hh_json

let stats_json_of_summary
    Summary.
      { id_cnt; syntactically_nadable_cnt; nadable_cnt; nadable_groups = _ } =
  J.JSON_Object
    [
      ("entry_kind", J.string_ "stats");
      ("id_cnt", J.int_ id_cnt);
      ("syntactically_nadable_cnt", J.int_ syntactically_nadable_cnt);
      ("nadable_cnt", J.int_ nadable_cnt);
    ]

let stats_json_of_group nadables =
  let of_nadable Summary.{ id; kind } =
    let kind_str =
      match kind with
      | Summary.Function -> "function"
      | Summary.ClassLike classish_kind -> show_classish_kind classish_kind
    in
    J.JSON_Object
      [("sid", J.string_ @@ H.Id.sid_of_t id); ("kind", J.string_ kind_str)]
  in
  let nadables_arr = J.array_ of_nadable nadables in
  J.JSON_Object
    [
      ("entry_kind", J.string_ "add_no_auto_dynamic_attr");
      ("items", nadables_arr);
    ]

let of_summary summary : string Sequence.t =
  let to_string = J.json_to_string ~sort_keys:true ~pretty:false in
  let stats =
    stats_json_of_summary summary |> to_string |> Sequence.singleton
  in
  let groups =
    summary.Summary.nadable_groups
    |> Sequence.map ~f:stats_json_of_group
    |> Sequence.map ~f:to_string
  in
  Sequence.append stats groups
