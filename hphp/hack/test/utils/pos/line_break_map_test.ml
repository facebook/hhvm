(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Line_break_map

(* Test the conversions of offsets to line/column numbers and back & the
 * resolution of the offset of the start of a line from an arbitrary offset.
 *)
let () =
  let str = "Hello\nWorld!\n\rHow have you been?\r\nWell, thanks" in
  let breaks = 4 in
  (* Better to count this in the string *)
  let len = String.length str in
  let m = make str in
  let rec range n k =
    if k > n then
      []
    else
      k :: range n (k + 1)
  in
  List.iter
    (fun offset ->
      let ch = (try str.[offset] with _ -> '_') in
      let (line, column) = offset_to_position m offset in
      let line_start = offset_to_line_start_offset m offset in
      let regen_offset = position_to_offset m (line, column) in
      Printf.eprintf
        " >> %2d -> (%2d,%2d) `%d -> %2d  %C\n"
        offset
        line
        column
        line_start
        regen_offset
        ch;
      assert (offset < 0 || offset = regen_offset);
      assert (column = offset - line_start + 1);
      assert ((regen_offset >= 0 && regen_offset < len) || line = breaks + 2))
    (range (String.length str) 0)
