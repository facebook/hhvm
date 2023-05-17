(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Sexplib.Std

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

[@@@warning "-32"] (* unused variables *)

module OffsetMap = struct
  (* placeholder definitions to placate open source build.
   * These definitions will be mercilessly shadowed if deriving show fires
   * like it's supposed to *)
  let pp _ _ = ()

  let show _ = ""

  include Line_break_map
end

[@@@warning "+32"]

type t = {
  file_path: Relative_path.t;
  length: int;
  text: string; [@opaque]
  offset_map: OffsetMap.t;
}
[@@deriving show, eq, sexp_of]

type pos = t * int

let make file_path content =
  {
    file_path;
    text = content;
    offset_map = OffsetMap.make content;
    length = String.length content;
  }

let empty = make Relative_path.default ""

let from_file file =
  let content = Sys_utils.cat (Relative_path.to_absolute file) in
  make file content

let file_path source_text = source_text.file_path

let length source_text = source_text.length

let text t = t.text

let get source_text index =
  if index < source_text.length then
    String.unsafe_get source_text.text index
  else
    '\x00'

let sub source_text start length =
  let len = source_text.length in
  if start >= len then
    ""
  else if start + length > len then
    String.sub source_text.text start (len - start)
  else
    String.sub source_text.text start length

(* Fetch the contents of just one line of text *)
let line_text (source_text : t) (line_number : int) : string =
  try
    let offset_start =
      OffsetMap.position_to_offset source_text.offset_map (line_number, 1)
    in
    let offset_end =
      OffsetMap.position_to_offset source_text.offset_map (line_number + 1, 1)
    in
    (* Strip off the newline if one exists. If we're getting the very last line of the file, and
     * the very last line doesn't end in a newline, this unsafe_get won't return a \n char. *)
    let offset_end =
      if String.unsafe_get source_text.text (offset_end - 1) = '\n' then
        offset_end - 1
      else
        offset_end
    in
    sub source_text offset_start (offset_end - offset_start)
    (* If the line was outside the boundaries of the file, just return a blank string *)
  with
  | _ -> ""

(* Take a zero-based offset, produce a one-based (line, char) pair. *)
let offset_to_position source_text offset =
  OffsetMap.offset_to_position source_text.offset_map offset

(* Take a one-based (line, char) pair, produce a zero-based offset *)
let position_to_offset source_text position =
  OffsetMap.position_to_offset source_text.offset_map position

(* Create a Pos.t from two offsets in a source_text (given a path) *)
let relative_pos pos_file source_text start_offset end_offset =
  let offset_to_lnum_bol_offset offset =
    OffsetMap.offset_to_file_pos_triple source_text.offset_map offset
  in
  let pos_start = offset_to_lnum_bol_offset start_offset in
  let pos_end = offset_to_lnum_bol_offset end_offset in
  Pos.make_from_lnum_bol_offset ~pos_file ~pos_start ~pos_end

let sub_of_pos ?length source_text pos =
  let offset =
    let (first_line, first_col) = Pos.line_column pos in
    position_to_offset source_text (first_line, first_col + 1)
  in
  let length =
    match length with
    | Some n -> n
    | None -> Pos.length pos
  in
  sub source_text offset length
