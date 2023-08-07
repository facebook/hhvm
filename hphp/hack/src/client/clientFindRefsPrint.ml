(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let print_json results ~half_open_interval =
  let entries =
    List.map results ~f:(fun (name, pos) ->
        let filename = Pos.filename pos in
        let (st_line, st_column, _ed_line, ed_column) =
          if half_open_interval then
            Pos.destruct_range pos
          else
            let (line, start, end_) = Pos.info_pos pos in
            (line, start, line, end_)
        in
        Hh_json.JSON_Object
          [
            ("name", Hh_json.JSON_String name);
            ("filename", Hh_json.JSON_String filename);
            ("line", Hh_json.int_ st_line);
            ("char_start", Hh_json.int_ st_column);
            ("char_end", Hh_json.int_ ed_column);
          ])
  in
  Hh_json.JSON_Array entries |> Hh_json.json_to_string |> print_endline

let print_closed_interval
    (results : ServerCommandTypes.Find_refs.result) ~(json : bool) =
  if json then
    print_json results ~half_open_interval:false
  else begin
    List.iter results ~f:(fun (name, pos) ->
        Printf.printf "%s %s\n" (Pos.string pos) name);
    Printf.printf "%d total results\n" (List.length results)
  end
