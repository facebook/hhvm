(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module JSON = Hh_json
module Hashtbl = Stdlib.Hashtbl

let of_pos pos =
  let (line, scol, ecol) = Pos.info_pos pos in
  JSON.JSON_Object
    [
      ("path", JSON.string_ (Pos.to_absolute pos |> Pos.filename));
      ("line", JSON.int_ line);
      ("start", JSON.int_ scol);
      ("end", JSON.int_ ecol);
    ]

let of_marker env pos fields kind =
  let shape_ty =
    Typing_defs.(
      mk (Typing_reason.Rnone, Tshape (Missing_origin, Closed_shape, fields)))
  in
  JSON.JSON_Object
    [
      ("pos", of_pos pos);
      ("kind", JSON.string_ (Codemod.show_kind kind));
      ("type", JSON.string_ (Typing_print.full env shape_ty));
    ]

let codemod_kind_of_marker_kind = function
  | Allocation -> Some Codemod.Allocation
  | Parameter
  | Return
  | Constant ->
    Some Codemod.Hint
  | Debug -> None

let group_of_results ~error_count env results =
  let directives =
    List.filter_map
      ~f:(function
        | Shape_like_dict (pos, kind, fields) ->
          Option.map
            ~f:(of_marker env pos fields)
            (codemod_kind_of_marker_kind kind)
        | Dynamically_accessed_dict _ -> None)
      results
    |> JSON.array_ (fun x -> x)
  in
  JSON.JSON_Object
    [("directives", directives); ("error_count", JSON.int_ error_count)]

let to_singletons = List.map ~f:(fun l -> [l])

let codemods_of_entries env ~(solve : solve_entries) ~atomic constraint_entries
    : Hh_json.json list =
  let errors = Hashtbl.create (List.length constraint_entries) in
  let () =
    let add_errors ConstraintEntry.{ id; error_count; _ } =
      let _ = Hashtbl.add errors id error_count in
      ()
    in
    constraint_entries |> List.iter ~f:add_errors
  in
  let add_errors_back id shape_result =
    let error_count = Option.value ~default:0 @@ Hashtbl.find_opt errors id in
    (error_count, shape_result)
  in
  let groups_of_results (error_count, shape_result) =
    let shape_results =
      if not atomic then
        to_singletons shape_result
      else
        [shape_result]
    in
    List.map shape_results ~f:(group_of_results ~error_count env)
  in
  solve env constraint_entries
  |> SMap.mapi add_errors_back
  |> SMap.values
  |> List.bind ~f:groups_of_results
