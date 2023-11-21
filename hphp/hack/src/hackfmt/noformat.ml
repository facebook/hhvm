(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Token = Full_fidelity_positioned_token
module Trivia = Full_fidelity_positioned_trivia
open Hh_prelude
open Boundaries
open Format_env

let generated_tag = "@" ^ "generated"

let partially_generated_tag = "@" ^ "partially-generated"

let begin_manual_section_regexp =
  Str.regexp "/\\* BEGIN MANUAL SECTION [^*]*\\*/"

let end_manual_section_tag = "/* END MANUAL SECTION */"

let lint_ignore_tag = "@" ^ "lint-ignore"

let is_generated_file text = String.is_substring text ~substring:generated_tag

let is_partially_generated_file text =
  String.is_substring text ~substring:partially_generated_tag

let is_begin_manual_section_tag text =
  try
    let (_ : int) = Str.search_forward begin_manual_section_regexp text 0 in
    true
  with
  | Stdlib.Not_found -> false

let is_end_manual_section_tag text = String.equal text end_manual_section_tag

let is_lint_ignore text = String.is_substring text ~substring:lint_ignore_tag

let add_fixme_ranges fixmes trivia =
  let trivium_range trivium =
    (Trivia.start_offset trivium, Trivia.end_offset trivium)
  in
  let add_fixme_range fixmes trivium =
    match Trivia.kind trivium with
    | Full_fidelity_trivia_kind.(FixMe | IgnoreError) ->
      trivium_range trivium :: fixmes
    | Full_fidelity_trivia_kind.(DelimitedComment | SingleLineComment)
      when is_lint_ignore (Trivia.text trivium) ->
      trivium_range trivium :: fixmes
    | _ -> fixmes
  in
  List.fold trivia ~init:fixmes ~f:add_fixme_range

let add_manual_tags manual_tags trivia =
  let add_manual_tag manual_tags trivium =
    match Trivia.kind trivium with
    | Full_fidelity_trivia_kind.DelimitedComment
      when is_begin_manual_section_tag (Trivia.text trivium) ->
      `Begin (Trivia.end_offset trivium) :: manual_tags
    | Full_fidelity_trivia_kind.DelimitedComment
      when is_end_manual_section_tag (Trivia.text trivium) ->
      `End (Trivia.start_offset trivium) :: manual_tags
    | _ -> manual_tags
  in
  List.fold trivia ~init:manual_tags ~f:add_manual_tag

(** Return the start and end offsets of every fixme comment and every manual
    section (of partially-generated files), sorted by order of appearance in the
    file. *)
let get_fixme_ranges_and_manual_sections tree :
    Interval.t list * Interval.t list =
  let rec aux acc node =
    match Syntax.get_token node with
    | None -> List.fold (List.rev (Syntax.children node)) ~init:acc ~f:aux
    | Some t ->
      let (fixmes, manual_tags) = acc in
      let trailing = List.rev (Token.trailing t) in
      let leading = List.rev (Token.leading t) in
      let fixmes = add_fixme_ranges fixmes trailing in
      let fixmes = add_fixme_ranges fixmes leading in
      let manual_tags = add_manual_tags manual_tags trailing in
      let manual_tags = add_manual_tags manual_tags leading in
      (fixmes, manual_tags)
  in
  let (fixmes, manual_tags) = aux ([], []) (SyntaxTree.root tree) in
  let (manual_sections, _) =
    List.fold
      manual_tags
      ~init:([], None)
      ~f:(fun (manual_sections, section_start) tag ->
        match (section_start, tag) with
        | (None, `Begin start_offset) -> (manual_sections, Some start_offset)
        | (Some start_offset, `End end_offset) ->
          ((start_offset, end_offset) :: manual_sections, None)
        (* Malformed: multiple BEGIN tags in a row. Ignore the earlier. *)
        | (Some _, `Begin start_offset) -> (manual_sections, Some start_offset)
        (* Malformed: an END tag with no BEGIN. Ignore it. *)
        | (None, `End _) -> (manual_sections, section_start))
  in
  (fixmes, List.rev manual_sections)

(** Return the start and end offsets of every range in which formatting is
    suppressed, sorted by order of appearance in the file. Formatting is
    suppressed around HH_FIXME/HH_IGNORE_ERROR comments, in generated
    sections of partially-generated files, and in generated files. *)
let get_suppressed_formatting_ranges env line_boundaries tree =
  let source_text = SyntaxTree.text tree in
  let (fixme_ranges, manual_sections) =
    get_fixme_ranges_and_manual_sections tree
  in
  let expand_to_line_boundaries =
    expand_to_line_boundaries ~ranges:line_boundaries source_text
  in
  (* Expand each range to contain the entire line the fixme is on, then add one
     to the range end and expand again to include the next line, since the fixme
     suppresses errors on both the line in which it appears and the line
     following it. *)
  let fixme_ranges =
    fixme_ranges
    |> List.map ~f:expand_to_line_boundaries
    |> List.map ~f:(fun (st, ed) -> (st, ed + 1))
    |> List.map ~f:expand_to_line_boundaries
  in
  if env.format_generated_code then
    Interval.union_consecutive_overlapping fixme_ranges
  else
    let generated_sections =
      let text = SourceText.text source_text in
      let whole_file = [(0, String.length text)] in
      if is_generated_file text then
        whole_file
      else if is_partially_generated_file text then
        Interval.diff_sorted_lists whole_file manual_sections
        |> List.map ~f:expand_to_line_boundaries
      else
        []
    in
    List.merge fixme_ranges generated_sections ~compare:Interval.compare
    |> Interval.union_consecutive_overlapping
