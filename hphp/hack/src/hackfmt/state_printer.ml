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
  let chunks = Solve_state.chunks state in

  let chunks = match range with
    | None -> chunks
    | Some range ->
      List.filter chunks ~f:(fun c ->
        Interval.intervals_overlap range (c.Chunk.start_char, c.Chunk.end_char)
      )
  in
  List.iter chunks ~f:begin fun chunk ->
    let range_starts_in_chunk, range_ends_in_chunk =
      match range, chunk.Chunk.tokens with
      | Some (range_start, range_end), tokens
        when tokens <> [] ->
        chunk.Chunk.start_char < range_start, range_end < chunk.Chunk.end_char
      | _ -> false, false
    in

    if not range_starts_in_chunk then begin
      if Solve_state.has_split_before_chunk state ~chunk then begin
        Buffer.add_string b "\n";
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
  Buffer.contents b
