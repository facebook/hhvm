(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  bolmap: int array;
  mutable last_offset: int;
  mutable curr_index: int;
} [@@deriving show, eq, sexp_of]

let make text =
  (* Clever Tricks Warning
   * ---------------------
   * We prepend 0, so as to make the invariant hold that there is always a
   * perceived line break at the start of the file. We use this in translating
   * offsets to line numbers.
   *
   * Similarly, whether there's a line break at the end or not, the line break
   * map will always end with the length of the original string. This solves
   * off-by-one issues.
   *)
  let len = String.length text in
  let newline_list =
    let result = ref [] in
    for i = 1 to len do
      let prev = text.[i - 1] in
      if
        Char.(
          (prev = '\r' && (Int.(i = len) || not (text.[i] = '\n')))
          || prev = '\n')
      then
        result := i :: !result
    done;
    (match !result with
     | r :: _ as rs when r <> len -> result := len :: rs
     | _ -> ());
    0 :: List.rev !result
  in
  {
    bolmap = Array.of_list newline_list;
    last_offset = 0;
    curr_index = 0;
  }

let offset_to_file_pos_triple bolmap offset =
  let len = Array.length bolmap.bolmap in
  if bolmap.curr_index >= len then bolmap.curr_index <- len - 1;
  let rec forward_search i =
    let offset_at_i = Array.unsafe_get bolmap.bolmap i in
    if offset < offset_at_i then
      i - 1
    else if i + 1 >= len then
      len - 1
    else
      forward_search (i + 1)
  in
  let rec backward_search i =
    let offset_at_i = Array.unsafe_get bolmap.bolmap i in
    if offset >= offset_at_i then
      i
    else if i = 0 then
      0
    else
      backward_search (i - 1)
  in
  let index =
    if bolmap.last_offset < offset && bolmap.curr_index <> len - 1 then
      forward_search (bolmap.curr_index + 1)
    else if bolmap.last_offset > offset then
      backward_search bolmap.curr_index
    else
      bolmap.curr_index
  in
  let line_start = bolmap.bolmap.(index) in
  bolmap.curr_index <- index;
  bolmap.last_offset <- offset;
  (index + 1, line_start, offset)

let offset_to_position bolmap offset =
  let (index, line_start, offset) = offset_to_file_pos_triple bolmap offset in
  (index, offset - line_start + 1)

(* TODO: add more informative data to the exception *)
exception Position_not_found

let position_to_offset ?(existing = false) bolmap (line, column) =
  let len = Array.length bolmap.bolmap in
  let file_line = line in
  (* Treat all file_line errors the same: Not_found *)
  let line_start =
    try bolmap.bolmap.(file_line - 1) with
    | _ -> raise Position_not_found
  in
  let offset = line_start + column - 1 in
  if
    (not existing) || (offset >= 0 && offset <= bolmap.bolmap.(min (len - 1) file_line))
  then
    offset
  else
    raise Position_not_found

let offset_to_line_start_offset bolmap offset =
  offset - snd (offset_to_position bolmap offset) + 1

let offsets_to_line_lengths bolmap =
  let (lengths, _) =
    Array.fold_right
      ~f:(fun offset (acc, mnext) ->
        match mnext with
        | None -> (acc, Some offset)
        | Some next -> ((next - offset) :: acc, Some offset))
      bolmap.bolmap
      ~init:([], None)
  in
  lengths
