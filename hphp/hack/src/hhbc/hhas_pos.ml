(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Span, emitted as prefix to classes and functions *)
type span = int * int

let pos_to_span pos =
  let (line_begin, line_end, _col_begin, _col_end) =
    Pos.info_pos_extended pos
  in
  (line_begin, line_end)
