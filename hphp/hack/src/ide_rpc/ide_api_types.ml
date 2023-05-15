(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* 1-based position is used here *)
type position = {
  line: int;
  column: int;
}
[@@deriving show]

type range = {
  st: position;
  ed: position;
}
[@@deriving show]

type file_position = {
  filename: string;
  position: position;
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

let ide_pos_to_fc (x : position) : File_content.position =
  let (line, column) = (x.line, x.column) in
  { File_content.line; column }

let ide_range_to_fc (x : range) : File_content.range =
  let (st, ed) = (x.st |> ide_pos_to_fc, x.ed |> ide_pos_to_fc) in
  { File_content.st; ed }

let ide_text_edit_to_fc (x : text_edit) : File_content.text_edit =
  let (text, range) = (x.text, x.range |> Option.map ~f:ide_range_to_fc) in
  { File_content.text; range }

let ide_pos_from_fc (x : File_content.position) : position =
  let (line, column) = (x.File_content.line, x.File_content.column) in
  { line; column }

let ide_range_from_fc (x : File_content.range) : range =
  let (st, ed) =
    (x.File_content.st |> ide_pos_from_fc, x.File_content.ed |> ide_pos_from_fc)
  in
  { st; ed }

let pos_to_range x =
  let (st_line, st_column, ed_line, ed_column) = Pos.destruct_range x in
  {
    st = { line = st_line; column = st_column };
    ed = { line = ed_line; column = ed_column };
  }

let pos_to_file_range x =
  { range_filename = Pos.filename x; file_range = pos_to_range x }

let range_to_string_single_line x =
  Printf.sprintf
    "line %d, characters %d-%d"
    x.st.line
    x.st.column
    (x.ed.column - 1)
