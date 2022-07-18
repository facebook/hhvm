(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module JSON = Hh_json
open Hh_prelude
open Shape_analysis_types

let of_pos pos =
  let (line, scol, ecol) = Pos.info_pos pos in
  JSON.JSON_Object
    [
      ("path", JSON.string_ (Pos.to_absolute pos |> Pos.filename));
      ("line", JSON.int_ line);
      ("start", JSON.int_ scol);
      ("end", JSON.int_ ecol);
    ]

let of_marker env pos kind fields =
  let shape_ty =
    Typing_defs.(mk (Typing_reason.Rnone, Tshape (Closed_shape, fields)))
  in
  JSON.JSON_Object
    [
      ("pos", of_pos pos);
      ("kind", JSON.string_ (show_marker_kind kind));
      ("type", JSON.string_ (Typing_print.full env shape_ty));
    ]

let of_results env results =
  List.filter_map
    ~f:(function
      | Shape_like_dict (pos, kind, fields) ->
        begin
          match kind with
          | Allocation
          | Parameter
          | Return ->
            Some (of_marker env pos kind fields)
          | Argument -> None
        end
      | Dynamically_accessed_dict _ -> None)
    results
  |> JSON.array_ (fun x -> x)
