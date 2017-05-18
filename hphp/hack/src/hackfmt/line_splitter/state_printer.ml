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
  List.iteri chunks ~f:(fun i c ->
    if Solve_state.has_split_before_chunk c rvm then begin
      Buffer.add_string b "\n";
      let indent = Nesting.get_indent c.Chunk.nesting nesting_set in
      let indent = indent + block_indentation in
      if c.Chunk.text <> "" && c.Chunk.indentable then
        Buffer.add_string b (String.make indent ' ');
    end else if c.Chunk.space_if_not_split then Buffer.add_string b " ";
    Buffer.add_string b c.Chunk.text;
    if Solve_state.has_comma_after_chunk c rvm then Buffer.add_string b ",";
  );
  Buffer.contents b
