(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Utils

let print_state ?range state =
  let b = Buffer.create 200 in
  let {Solve_state.rvm; nesting_set; chunk_group; _} = state in
  let {Chunk_group.chunks; block_indentation; _} = chunk_group in

  let chunks = match range with
    | None -> chunks
    | Some range ->
      List.filter chunks ~f:(fun c ->
        Interval.intervals_overlap range (c.Chunk.start_char, c.Chunk.end_char)
      )
  in
  List.iter chunks ~f:begin fun c ->
    let range_starts_in_chunk, range_ends_in_chunk =
      match range, c.Chunk.tokens with
      | Some (range_start, range_end), tokens
        when tokens <> [] ->
        c.Chunk.start_char < range_start, range_end < c.Chunk.end_char
      | _ -> false, false
    in

    if not range_starts_in_chunk then begin
      if Solve_state.has_split_before_chunk c rvm then begin
        Buffer.add_string b "\n";
        let indent = Nesting.get_indent c.Chunk.nesting nesting_set in
        let indent = indent + block_indentation in
        if c.Chunk.text <> "" && c.Chunk.indentable then
          Buffer.add_string b (String.make indent ' ');
      end else if c.Chunk.space_if_not_split then Buffer.add_string b " ";
    end;

    let text =
      if range_starts_in_chunk || range_ends_in_chunk
      then Chunk.print_range c (unsafe_opt range)
      else c.Chunk.text
    in
    Buffer.add_string b text;

    if Solve_state.has_comma_after_chunk c rvm && not range_ends_in_chunk then
      Buffer.add_string b ",";
  end;
  Buffer.contents b
