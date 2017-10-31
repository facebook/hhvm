(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * This is an abstraction over source files used by the full-fidelity lexer.
 * Right now it is just a thin wrapper over a string, but in the future I'd
 * like to enhance this to efficiently support operations such as:
 *
 * - representing many different portions of a file without duplicating text
 * - lazily loading content from files
 * - caching file contents intelligently
 * - reporting when files are updated perhaps?
 *
 * We might not need all of these things, but it will be nice to have an
 * abstraction when we need some of them.
 *)

module OffsetMap = Line_break_map

type t = {
  file_path : Relative_path.t;
  text : string;
  offset_map : OffsetMap.t
}

let make file_path content =
  { file_path; text = content; offset_map = OffsetMap.make content }

let empty =
  make Relative_path.default ""

let from_file file =
  let content =
    try Sys_utils.cat (Relative_path.to_absolute file) with _ -> "" in
  make file content

let text source_text =
  source_text.text

let file_path source_text =
  source_text.file_path

let length source_text =
  String.length source_text.text

let get_text t =
  t.text

let get source_text index =
  String.get source_text.text index

let sub source_text start length =
  let len = String.length source_text.text in
  if start >= len then
    ""
  else if start + length > len then
    String.sub source_text.text start (len - start)
  else
    String.sub source_text.text start length

(* Take a zero-based offset, produce a one-based (line, char) pair. *)
let offset_to_position source_text offset =
  OffsetMap.offset_to_position source_text.offset_map offset

(* Take a one-based (line, char) pair, produce a zero-based offset *)
let position_to_offset source_text position =
  OffsetMap.position_to_offset source_text.offset_map position

(* Create a Pos.t from two offsets in a source_text (given a path) *)
let relative_pos pos_file source_text start_offset end_offset =
  let offset_to_file_pos offset =
    try
      let pos_lnum, pos_bol, pos_cnum =
        OffsetMap.offset_to_file_pos_triple source_text.offset_map offset
      in
      File_pos.of_lnum_bol_cnum ~pos_lnum ~pos_bol ~pos_cnum
    with Invalid_argument _ -> File_pos.dummy
  in
  let pos_start = offset_to_file_pos start_offset in
  let pos_end   = offset_to_file_pos end_offset in
  Pos.make_from_file_pos ~pos_file ~pos_start ~pos_end

let is_newline source_text offset =
  let c = get source_text offset in
  c = '\r' || c = '\n'

(* Takes an offset, returns the offset of the first character in this line. *)
let start_of_line source_text offset =
  OffsetMap.offset_to_line_start_offset source_text.offset_map offset

(* Takes an offset, returns the offset of the last non-newline character
   in this line, if there is one, or the first newline character in a blank
   line. Note that if we are on a newline, the last non-newline character in
   this file could be before the given offset. *)
let end_of_line source_text offset =
  let len = length source_text in
  if len = 0 then
    0
  else
    let sol = start_of_line source_text offset in
    if is_newline source_text sol then
      sol
    else
      let rec aux i =
        if i >= len then
          len - 1
        else if is_newline source_text i then
          i - 1
        else
          aux i + 1 in
      aux sol + 1
