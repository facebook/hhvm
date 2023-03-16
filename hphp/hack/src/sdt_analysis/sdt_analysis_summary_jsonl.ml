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

  val nadables_of_json_exn : J.json -> Summary.nadable list option
end = struct
  module Names = struct
    let add_no_auto_dynamic_attr = "add_no_auto_dynamic_attr"

    let items = "items"

    let sid = "sid"

    let kind = "kind"

    let function_ = "function"
  end

  let extract_exn access_result =
    access_result |> Result.ok |> Option.value_exn |> fst

  let json_obj_of_nadable Summary.{ id; kind } =
    let kind_str =
      match kind with
      | Summary.Function -> Names.function_
      | Summary.ClassLike classish_kind_opt ->
        classish_kind_opt
        |> Option.sexp_of_t sexp_of_classish_kind
        |> Sexp.to_string
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

  let nadable_of_json_obj_exn obj =
    let sid = J.Access.get_string Names.sid (obj, []) |> extract_exn in
    let kind_string = J.Access.get_string Names.kind (obj, []) |> extract_exn in
    if String.equal kind_string Names.function_ then
      let id = H.Id.Function sid in
      let kind = Summary.Function in
      Summary.{ id; kind }
    else
      let classish_kind_opt =
        kind_string |> Sexp.of_string |> Option.t_of_sexp classish_kind_of_sexp
      in
      let id = H.Id.ClassLike sid in
      let kind = Summary.ClassLike classish_kind_opt in
      Summary.{ id; kind }

  let nadables_of_json_exn obj =
    match J.Access.get_string entry_kind_label (obj, []) |> extract_exn with
    | key when String.equal key Names.add_no_auto_dynamic_attr ->
      J.Access.get_array Names.items (obj, [])
      |> extract_exn
      |> List.map ~f:nadable_of_json_obj_exn
      |> Option.some
    | _ -> None
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

let nadables_of_line_exn s =
  let obj = J.json_of_string ~strict:true s in
  Nad.nadables_of_json_exn obj
