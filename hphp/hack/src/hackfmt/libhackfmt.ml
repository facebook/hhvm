(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxTree = Full_fidelity_syntax_tree
module EditableSyntax = Full_fidelity_editable_syntax
module SourceText = Full_fidelity_source_text
module Env = Format_env

open Core
open Printf

let get_line_boundaries lines =
  let bytes_seen = ref 0 in
  Array.of_list @@ List.map lines (fun line -> begin
    let line_start = !bytes_seen in
    let line_end = line_start + String.length line + 1 in
    bytes_seen := line_end;
    line_start, line_end
  end)

let expand_to_line_boundaries ?ranges source_text range =
  let start_offset, end_offset = range in
  (* Ensure that 0 <= start_offset <= end_offset <= source_text length *)
  let start_offset = max start_offset 0 in
  let end_offset = max start_offset end_offset in
  let end_offset = min end_offset (SourceText.length source_text) in
  let start_offset = min start_offset end_offset in
  (* Ensure that start_offset and end_offset fall on line boundaries *)
  let ranges = match ranges with
    | Some ranges -> ranges
    | None -> get_line_boundaries
      (SourceText.text source_text |> String_utils.split_on_newlines)
  in
  Array.fold_left (fun (st, ed) (line_start, line_end) ->
    let st = if st > line_start && st < line_end then line_start else st in
    let ed = if ed > line_start && ed < line_end then line_end else ed in
    st, ed
  ) (start_offset, end_offset) ranges

let line_interval_to_offset_range line_boundaries (start_line, end_line) =
  if start_line > end_line then
    invalid_arg @@
      sprintf "Illegal line interval: %d,%d" start_line end_line;
  if start_line < 1 || start_line > Array.length line_boundaries
  || end_line < 1 || end_line > Array.length line_boundaries
  then
    invalid_arg @@
      sprintf "Can't format line interval %d,%d in file with %d lines"
        start_line end_line (Array.length line_boundaries);
  let start_offset, _ = line_boundaries.(start_line - 1) in
  let _, end_offset = line_boundaries.(end_line - 1) in
  start_offset, end_offset

let get_split_boundaries solve_states =
  List.fold solve_states
    ~init:[]
    ~f:begin fun boundaries solve_state ->
      let chunks = Solve_state.chunks solve_state in
      match chunks with
      | [] -> boundaries
      | hd :: tl ->
        let boundaries, last_range = List.fold tl
          ~init:(boundaries, Chunk.get_range hd)
          ~f:begin fun (boundaries, curr_range) chunk ->
            let chunk_range = Chunk.get_range chunk in
            if Solve_state.has_split_before_chunk solve_state ~chunk
              then curr_range :: boundaries, chunk_range
              else boundaries, (fst curr_range, snd chunk_range)
          end
        in
        last_range :: boundaries
    end
  |> List.rev

let expand_to_split_boundaries boundaries range =
  List.fold boundaries ~init:range ~f:(fun (st, ed) (b_st, b_ed) ->
    let st = if st > b_st && st < b_ed then b_st else st in
    let ed = if ed > b_st && ed < b_ed then b_ed else ed in
    st, ed
  )

let env_from_config config =
  let env = Option.value config ~default:Env.default in
  if Env.indent_width env < 0 then invalid_arg "Invalid indent width";
  if Env.line_width env < 0 then invalid_arg "Invalid line width";
  env

let env_from_tree config tree =
  let env = env_from_config config in
  if SyntaxTree.is_php tree
  then {env with Env.add_trailing_commas = false}
  else env

(** Format an entire file. *)
let format_tree ?config tree =
  let source_text = SourceText.text (SyntaxTree.text tree) in
  let env = env_from_tree config tree in
  tree
  |> EditableSyntax.from_tree
  |> Hack_format.transform env
  |> Chunk_builder.build
  |> Line_splitter.solve env source_text

(** Format a given range in a file.
 *
 * The range is a half-open interval of byte offsets into the file.
 *
 * If the range boundaries would bisect a token, the entire token will appear in
 * the formatted output.
 *
 * If the first token in the range would have indentation preceding it in the
 * full formatted file, the leading indentation will be included in the output.
 *
 * If the last token in the range would have a trailing newline in the full
 * formatted file, the trailing newline will be included in the output.
 *
 * Non-indentation space characters are not included at the beginning or end of
 * the formatted output (unless they are in a comment or string literal). *)
let format_range ?config range tree =
  let source_text = SourceText.text (SyntaxTree.text tree) in
  let env = env_from_tree config tree in
  tree
  |> EditableSyntax.from_tree
  |> Hack_format.transform env
  |> Chunk_builder.build
  |> Line_splitter.solve env ~range source_text

(** Return the source of the entire file with the given intervals formatted.
 *
 * The intervals are a list of half-open intervals of 1-based line numbers.
 * They are permitted to overlap. *)
let format_intervals ?config intervals tree =
  let source_text = SyntaxTree.text tree in
  let text = SourceText.text source_text in
  let lines = String_utils.split_on_newlines text in
  let env = env_from_tree config tree in
  let chunk_groups =
    tree
    |> EditableSyntax.from_tree
    |> Hack_format.transform env
    |> Chunk_builder.build
  in
  let solve_states = Line_splitter.find_solve_states env
    (SourceText.text source_text) chunk_groups in
  let line_boundaries = get_line_boundaries lines in
  let split_boundaries = get_split_boundaries solve_states in
  let ranges = intervals
    |> List.map ~f:(line_interval_to_offset_range line_boundaries)
    |> List.map ~f:(expand_to_split_boundaries split_boundaries)
    |> Interval.union_list
    |> List.sort ~cmp:Interval.comparator
  in
  let formatted_intervals = List.map ranges (fun range ->
    range, Line_splitter.print env ~range solve_states
  ) in
  let length = SourceText.length source_text in
  let buf = Buffer.create (length + 256) in
  let bytes_seen = ref 0 in
  List.iter formatted_intervals (fun ((st, ed), formatted) ->
    for i = !bytes_seen to st - 1 do
      Buffer.add_char buf text.[i];
    done;
    Buffer.add_string buf formatted;
    bytes_seen := ed;
  );
  for i = !bytes_seen to length - 1 do
    Buffer.add_char buf text.[i];
  done;
  Buffer.contents buf

(** Format a node at the given offset.
 *
 * Finds the node which is the direct parent of the token at the given byte
 * offset and formats a range containing that node. The range that was formatted
 * is returned along with the formatted text of the node.
 *
 * Designed to be suitable for as-you-type-formatting. *)
let format_at_offset ?config tree offset =
  let source_text = SyntaxTree.text tree in
  let env = env_from_tree config tree in
  let chunk_groups =
    tree
    |> EditableSyntax.from_tree
    |> Hack_format.transform env
    |> Chunk_builder.build in
  let module PS = Full_fidelity_positioned_syntax in
  (* Grab the node which is the direct parent of the token at offset *)
  let token, node = match PS.parentage (PS.from_tree tree) offset with
    | token :: node :: _ -> token, node
    | _ -> invalid_arg "Invalid offset" in
  (* Format up to the end of the token at the given offset. *)
  let offset = PS.end_offset token in
  (* Our ranges are half-open, so the range end is the token end offset + 1. *)
  let ed = offset + 1 in
  (* Take a half-open range which starts at the beginning of the parent node *)
  let range = (PS.start_offset node, ed) in
  (* Expand the start offset to the nearest line boundary in the original
   * source, since we want to add a leading newline before the node we're
   * formatting if one should be there and isn't already present. *)
  let range = (fst (expand_to_line_boundaries source_text range), ed) in
  (* Find a solution for that range, then expand the range start to a split
   * boundary (a line boundary in the formatted source). *)
  let solve_states = Line_splitter.find_solve_states env
    ~range (SourceText.text source_text) chunk_groups in
  let split_boundaries = get_split_boundaries solve_states in
  let range = (fst (expand_to_split_boundaries split_boundaries range), ed) in
  (* Produce the formatted text *)
  let formatted = Line_splitter.print env solve_states ~range in
  (* We don't want a trailing newline here (in as-you-type-formatting, it would
   * place the cursor on the next line), but the Line_splitter always prints one
   * at the end of a formatted range when the last token ought to have a
   * trailing newline. So, we remove it. *)
  range, String_utils.rstrip formatted "\n"
