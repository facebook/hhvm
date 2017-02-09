(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* 1-based position is used here *)
type position = {
  line : int;
  column : int;
}

type range = {
  st : position;
  ed : position;
}

type file_position = {
  filename : string;
  position : position;
}

type file_range = {
  range_filename : string;
  file_range : range;
}

type text_edit = {
  range : range option;
  text : string;
}

type coverage_level =
  | Unchecked (* Completely unchecked code, i.e. Tanys *)
  | Partial   (* Partially checked code, e.g. array, Awaitable<_> with no
                 concrete type parameters *)
  | Checked   (* Completely checked code *)

let pos_to_range x =
  let st_line, st_column, ed_line, ed_column = Pos.destruct_range x in
  {
    st = {
      line = st_line;
      column = st_column;
    };
    ed = {
      line = ed_line;
      column = ed_column;
    }
  }

let pos_to_ide_pos x =
  {
    line = Pos.line x;
    column = Pos.start_cnum x;
  }

let pos_to_file_range x =
  {
    range_filename = Pos.filename x;
    file_range = pos_to_range x;
  }

let range_to_string_single_line x =
  Printf.sprintf ("line %d, characters %d-%d")
    x.st.line x.st.column (x.ed.column - 1)
