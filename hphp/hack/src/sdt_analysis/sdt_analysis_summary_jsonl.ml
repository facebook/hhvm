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

let entry_kind_label = "entry_kind"

let stats_json_of_summary
    Summary.
      { id_cnt; syntactically_nadable_cnt; nadable_cnt; nadable_groups = _ } =
  J.JSON_Object
    [
      (entry_kind_label, J.string_ "stats");
      ("id_cnt", J.int_ id_cnt);
      ("syntactically_nadable_cnt", J.int_ syntactically_nadable_cnt);
      ("nadable_cnt", J.int_ nadable_cnt);
    ]

module Nad : sig
  val json_of_nadables : Summary.nadable list -> J.json
end = struct
  module Names = struct
    let add_no_auto_dynamic_attr = "add_no_auto_dynamic_attr"

    let items = "items"

    let sid = "sid"

    let kind = "kind"

    let function_ = "function"

    let classish_kind_unknown = "classish_kind_unknown"
  end

  let json_obj_of_nadable Summary.{ id; kind } =
    let kind_str =
      match kind with
      | Summary.Function -> Names.function_
      | Summary.ClassLike classish_kind_opt ->
        Option.(
          classish_kind_opt
          >>| show_classish_kind
          |> value ~default:Names.classish_kind_unknown)
    in
    J.JSON_Object
      [
        (Names.sid, J.string_ @@ H.Id.sid_of_t id);
        (Names.kind, J.string_ kind_str);
      ]

  let json_of_nadables nadables =
    let nadables_arr = J.array_ json_obj_of_nadable nadables in
    J.JSON_Object
      [
        (entry_kind_label, J.string_ Names.add_no_auto_dynamic_attr);
        (Names.items, nadables_arr);
      ]
end

let of_summary summary : string Sequence.t =
  let to_string = J.json_to_string ~sort_keys:true ~pretty:false in
  let stats =
    stats_json_of_summary summary |> to_string |> Sequence.singleton
  in
  let groups =
    summary.Summary.nadable_groups
    |> Sequence.map ~f:Nad.json_of_nadables
    |> Sequence.map ~f:to_string
  in
  Sequence.append stats groups
