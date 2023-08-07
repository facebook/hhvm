(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let print_readable res =
  List.iter res ~f:(fun (name, pos) ->
      let pos_str = Pos.string pos in
      print_endline (pos_str ^ " " ^ name));
  print_endline (string_of_int (List.length res) ^ " total results")

let print_ide_readable results =
  List.iter (List.rev results) ~f:(fun (_name, p) ->
      Printf.printf "%s\n" (Pos.string p));
  print_endline (string_of_int (List.length results) ^ " total results")

let print_ide_json results =
  let entries =
    List.map results ~f:(fun (symbol_name, pos) ->
        let range_filename = Pos.filename pos in
        (* "This returns a half-open interval" *)
        let (st_line, st_column, _ed_line, ed_column) =
          Pos.destruct_range pos
        in
        Hh_json.JSON_Object
          [
            ("name", Hh_json.JSON_String symbol_name);
            ("filename", Hh_json.JSON_String range_filename);
            ("line", Hh_json.int_ st_line);
            ("char_start", Hh_json.int_ st_column);
            ("char_end", Hh_json.int_ ed_column);
          ])
  in
  Hh_json.JSON_Array entries |> Hh_json.json_to_string |> print_endline

let print_json results =
  let entries =
    List.map results ~f:(fun (name, pos) ->
        let filename = Pos.filename pos in
        (* "This returns a closed interval that's incorrect for multi-line spans" *)
        let (line, start, end_) = Pos.info_pos pos in
        Hh_json.JSON_Object
          [
            ("name", Hh_json.JSON_String name);
            ("filename", Hh_json.JSON_String filename);
            ("line", Hh_json.int_ line);
            ("char_start", Hh_json.int_ start);
            ("char_end", Hh_json.int_ end_);
          ])
  in
  Hh_json.JSON_Array entries |> Hh_json.json_to_string |> print_endline

let print
    (res : ServerCommandTypes.Find_refs.result) ~(ide : bool) ~(json : bool) =
  match (json, ide) with
  | (true, false) -> print_json res
  | (true, true) -> print_ide_json res
  | (false, false) -> print_readable res
  | (false, true) -> print_ide_readable res
