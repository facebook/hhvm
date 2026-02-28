(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type range = {
  st: File_content.Position.t;
  ed: File_content.Position.t;
}
[@@deriving show]

type file_position = {
  filename: string;
  position: File_content.Position.t;
}

type file_range = {
  range_filename: string;
  file_range: range;
}

type text_edit = {
  range: range option;
  text: string;
}
[@@deriving show]

let ide_range_from_fc { File_content.st; ed } = { st; ed }

let ide_range_to_fc { st; ed } = { File_content.st; ed }

let pos_to_range x =
  let (st_line, st_column, ed_line, ed_column) =
    Pos.destruct_range_one_based x
  in
  {
    st = File_content.Position.from_one_based st_line st_column;
    ed = File_content.Position.from_one_based ed_line ed_column;
  }

let pos_to_file_range x =
  { range_filename = Pos.filename x; file_range = pos_to_range x }

let range_to_string_single_line { st; ed } =
  let (line, start_c) = File_content.Position.line_column_one_based st in
  let (_, end_c) = File_content.Position.line_column_one_based ed in
  Printf.sprintf "line %d, characters %d-%d" line start_c end_c
