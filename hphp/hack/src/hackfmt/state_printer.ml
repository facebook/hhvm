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

type subchunk =
  | Atom of {text: string; range: Interval.t}
  | Comma
  | Space
  | Newline
  | Indent of int

let string_of_subchunks env subchunks =
  let buf = Buffer.create 200 in
  List.iter subchunks ~f:begin function
    | Atom {text; _} -> Buffer.add_string buf text
    | Comma -> Buffer.add_char buf ','
    | Space -> Buffer.add_char buf ' '
    | Newline -> Buffer.add_char buf '\n'
    | Indent indent ->
      Buffer.add_string buf @@
        if Env.indent_with_tabs env
        then String.make indent '\t'
        else String.make (indent * Env.indent_width env) ' '
  end;
  Buffer.contents buf

let subchunks_in_range subchunks (range_start, range_end) =
  (* Filter to the atoms which overlap with the range, including the leading and
   * trailing non-Atom subchunks. *)
  let subchunks = List.take_while subchunks ~f:begin function
    | Atom {range = st,_; _} -> st < range_end
    | _ -> true
  end in
  let subchunks = List.rev subchunks in
  let subchunks = List.take_while subchunks ~f:begin function
    | Atom {range = st,ed; _} -> ed > range_start ||
                                 st = range_start && ed = range_start
    | _ -> true
  end in
  (* Drop trailing spaces and indentation *)
  let subchunks = List.drop_while subchunks ~f:begin function
    | Space | Indent _ -> true
    | _ -> false
  end in
  let subchunks = List.rev subchunks in
  subchunks

let print_state env ?range state =
  let subchunks = ref [] in
  let add_subchunk sc = subchunks := sc :: !subchunks in
  List.iter (Solve_state.chunks state) ~f:begin fun chunk ->
    if Solve_state.has_split_before_chunk state ~chunk
    then begin
      add_subchunk Newline;
      if not (Chunk.is_empty chunk) && chunk.Chunk.indentable then
        add_subchunk (Indent (Solve_state.get_indent_level state chunk));
    end
    else if chunk.Chunk.space_if_not_split then add_subchunk Space;

    List.iter chunk.Chunk.atoms ~f:begin fun atom ->
      let {Chunk.leading_space; text; range} = atom in
      if leading_space then add_subchunk Space;
      add_subchunk (Atom {text; range});
    end;

    if Solve_state.has_comma_after_chunk state ~chunk then add_subchunk Comma;
  end;
  (* Every chunk group has a newline trailing it, but chunks are associated with
   * the split preceding them. In order to ensure that we print trailing
   * newlines and not leading newlines, we always add a newline here, and strip
   * leading newlines below. *)
  add_subchunk Newline;
  let subchunks = List.rev !subchunks in
  let subchunks =
    match range with
    | None -> subchunks
    | Some range -> subchunks_in_range subchunks range
  in
  (* Drop leading whitespace *)
  let subchunks = List.drop_while subchunks ~f:begin function
    | Newline | Space -> true
    | _ -> false
  end in
  string_of_subchunks env subchunks
