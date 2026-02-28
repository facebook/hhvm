(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text
open Hh_prelude
open Printf

let get_line_boundaries text =
  let lines = String_utils.split_on_newlines text in
  let bytes_seen = ref 0 in
  Array.of_list
  @@ List.map lines ~f:(fun line ->
         let line_start = !bytes_seen in
         let line_end = line_start + String.length line + 1 in
         bytes_seen := line_end;
         (line_start, line_end))

let expand_to_line_boundaries ?ranges source_text range =
  let (start_offset, end_offset) = range in
  (* Ensure that 0 <= start_offset <= end_offset <= source_text length *)
  let start_offset = max start_offset 0 in
  let end_offset = max start_offset end_offset in
  let end_offset = min end_offset (SourceText.length source_text) in
  let start_offset = min start_offset end_offset in
  (* Ensure that start_offset and end_offset fall on line boundaries *)
  let ranges =
    match ranges with
    | Some ranges -> ranges
    | None -> get_line_boundaries (SourceText.text source_text)
  in
  Stdlib.Array.fold_left
    (fun (st, ed) (line_start, line_end) ->
      let st =
        if st > line_start && st < line_end then
          line_start
        else
          st
      in
      let ed =
        if ed > line_start && ed < line_end then
          line_end
        else
          ed
      in
      (st, ed))
    (start_offset, end_offset)
    ranges

let line_interval_to_offset_range line_boundaries (start_line, end_line) =
  if start_line > end_line then
    invalid_arg @@ sprintf "Illegal line interval: %d,%d" start_line end_line;
  if
    start_line < 1
    || start_line > Array.length line_boundaries
    || end_line < 1
    || end_line > Array.length line_boundaries
  then
    invalid_arg
    @@ sprintf
         "Can't format line interval %d,%d in file with %d lines"
         start_line
         end_line
         (Array.length line_boundaries);
  let (start_offset, _) = line_boundaries.(start_line - 1) in
  let (_, end_offset) = line_boundaries.(end_line - 1) in
  (start_offset, end_offset)

let get_atom_boundaries chunk_groups =
  chunk_groups
  |> List.concat_map ~f:(fun chunk_group -> chunk_group.Chunk_group.chunks)
  |> List.concat_map ~f:(fun chunk -> chunk.Chunk.atoms)
  |> List.map ~f:(fun atom -> atom.Chunk.range)

let expand_to_atom_boundaries boundaries (r_st, r_ed) =
  let rev_bounds = List.rev boundaries in
  let st =
    match List.find rev_bounds ~f:(fun (b_st, _) -> b_st <= r_st) with
    | Some bound -> fst bound
    | None -> r_st
  in
  let ed =
    match List.find boundaries ~f:(fun (_, b_ed) -> b_ed >= r_ed) with
    | Some bound -> snd bound
    | None -> r_ed
  in
  (st, ed)
