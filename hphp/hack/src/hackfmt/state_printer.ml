(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Env = Format_env

open Core
open Utils

let print_state env ?range state =
  let b = Buffer.create 200 in
  let state_chunks = Solve_state.chunks state in

  (* When printing a chunk group, we want to print a newline after and not
   * before. This corresponds with the way trailing trivia works (the final
   * atom in the chunk is associated with the trailing newline) and the
   * expectations of range-formatting at line boundaries.
   *
   * However, chunks are associated with the split preceding them, so if we
   * print a newline character before every chunk whose associated split is
   * broken on, we will print the chunk group with a leading newline and no
   * trailing newline. Instead, we don't print a newline before the first chunk
   * we print, and do print a trailing newline if the last chunk in the range is
   * bound to have a newline after it. *)
  let chunk_printed = ref false in

  let chunks = match range with
    | None -> state_chunks
    | Some range ->
      List.filter state_chunks ~f:(fun c ->
        Interval.intervals_overlap range (c.Chunk.start_char, c.Chunk.end_char)
      )
  in
  List.iter chunks ~f:begin fun chunk ->
    let range_starts_in_chunk, range_ends_in_chunk =
      match range, chunk.Chunk.atoms with
      | Some (range_start, range_end), atoms
        when atoms <> [] ->
        let first_atom = List.hd_exn chunk.Chunk.atoms in
        let last_atom = List.last_exn chunk.Chunk.atoms in
        let atom_start = first_atom.Chunk.source_offset in
        let atom_end =
          last_atom.Chunk.source_offset + last_atom.Chunk.width
        in
        atom_start < range_start, range_end < atom_end
      | _ -> false, false
    in

    if not range_starts_in_chunk then begin
      if Solve_state.has_split_before_chunk state ~chunk then begin
        if !chunk_printed
        then Buffer.add_string b "\n"
        else chunk_printed := true;
        let indent = Solve_state.get_indent state env ~chunk in
        if chunk.Chunk.text <> "" && chunk.Chunk.indentable then
          Buffer.add_string b @@
            if Env.indent_with_tabs env
            then String.make (indent / (Env.indent_width env)) '\t'
            else String.make indent ' ';
      end else if chunk.Chunk.space_if_not_split then Buffer.add_string b " ";
    end;

    let text =
      if range_starts_in_chunk || range_ends_in_chunk
      then Chunk.print_range chunk (unsafe_opt range)
      else chunk.Chunk.text
    in
    Buffer.add_string b text;

    if Solve_state.has_comma_after_chunk state ~chunk && not range_ends_in_chunk
      then Buffer.add_string b ",";
  end;

  (* We want a line break after this chunk group when any of the following
   * conditions holds:
   *
   * - The range ended in this chunk group, and the first chunk after the end
   *   of the range has a line break before it in this solve state.
   * - The range included the final chunk in this chunk group. *)
  let group_range = Chunk_group.get_char_range state.Solve_state.chunk_group in
  let range_includes_trailing_newline =
    match range with
    | None -> true
    | Some range when not (Interval.intervals_overlap range group_range) -> false
    | Some range ->
      let range_end = snd range in
      try
        let chunk =
          List.find_exn state_chunks
            ~f:(fun c -> range_end >= c.Chunk.end_char)
        in
        Solve_state.has_split_before_chunk state ~chunk
      with Not_found ->
        range_end >= snd group_range
  in
  if range_includes_trailing_newline then
    Buffer.add_string b "\n";

  Buffer.contents b
