(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Source location, emitted in .srcloc *)
type srcloc = (int*int) * (int*int)

(* Span, emitted as prefix to classes and functions *)
type span = int*int

let pos_to_srcloc pos =
  let line_begin, line_end, col_begin, col_end = Pos.info_pos_extended pos in
  ((line_begin, col_begin), (line_end, col_end))

let pos_to_span pos =
  let line_begin, line_end, _col_begin, _col_end = Pos.info_pos_extended pos in
  (line_begin, line_end)
