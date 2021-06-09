(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module EditableSyntax = Full_fidelity_editable_syntax
module SourceText = Full_fidelity_source_text
module Env = Format_env
open Hh_prelude
open Noformat
open Boundaries

let env_from_config config =
  let env = Option.value config ~default:Env.default in
  if env.Env.indent_width < 0 then invalid_arg "Invalid indent width";
  if env.Env.line_width < 0 then invalid_arg "Invalid line width";
  env

(** Format a single node.
 *
 * A trailing newline will be included (and leading indentation, if
 * requested). *)
let format_node ?config ?(indent = 0) node =
  let source_text = EditableSyntax.text node in
  let env = env_from_config config in
  let rec nest times doc =
    if times <= 0 then
      doc
    else
      nest (times - 1) (Doc.BlockNest [doc])
  in
  node
  |> Hack_format.transform env
  |> nest indent
  |> Chunk_builder.build
  |> Line_splitter.solve env ~source_text

let text_with_formatted_ranges
    ?(range : Interval.t option)
    (text : string)
    (formatted_ranges : (Interval.t * string) list) : Buffer.t =
  let (start_offset, end_offset) =
    match range with
    | Some (start_offset, end_offset) -> (start_offset, end_offset)
    | None -> (0, String.length text)
  in
  let buf = Buffer.create (end_offset - start_offset + 256) in
  let bytes_seen = ref start_offset in
  List.iter formatted_ranges ~f:(fun ((st, ed), formatted) ->
      for i = !bytes_seen to st - 1 do
        Buffer.add_char buf text.[i]
      done;
      Buffer.add_string buf formatted;
      bytes_seen := ed);
  for i = !bytes_seen to end_offset - 1 do
    Buffer.add_char buf text.[i]
  done;
  buf

(** Format an entire file. *)
let format_tree ?config tree =
  let source_text = SyntaxTree.text tree in
  let text = SourceText.text source_text in
  let env = env_from_config config in
  let chunk_groups =
    tree
    |> SyntaxTransforms.editable_from_positioned
    |> Hack_format.transform env
    |> Chunk_builder.build
  in
  let line_boundaries = get_line_boundaries text in
  let noformat_ranges =
    get_suppressed_formatting_ranges env line_boundaries tree
  in
  let whole_file = [(0, String.length text)] in
  let ranges = Interval.diff_sorted_lists whole_file noformat_ranges in
  let formatted_ranges =
    List.map ranges ~f:(fun range ->
        (range, Line_splitter.solve env ~range ~source_text:text chunk_groups))
  in
  let buf = text_with_formatted_ranges text formatted_ranges in
  Buffer.contents buf

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
  let source_text = SyntaxTree.text tree in
  let text = SourceText.text source_text in
  let env = env_from_config config in
  let chunk_groups =
    tree
    |> SyntaxTransforms.editable_from_positioned
    |> Hack_format.transform env
    |> Chunk_builder.build
  in
  let line_boundaries = get_line_boundaries text in
  let noformat_ranges =
    get_suppressed_formatting_ranges env line_boundaries tree
  in
  let ranges = Interval.diff_sorted_lists [range] noformat_ranges in
  let formatted_ranges =
    List.map ranges ~f:(fun range ->
        (range, Line_splitter.solve env ~range ~source_text:text chunk_groups))
  in
  let buf = text_with_formatted_ranges ~range text formatted_ranges in
  Buffer.contents buf

(** Return the source of the entire file with the given intervals formatted.
 *
 * The intervals are a list of half-open intervals of 1-based line numbers.
 * They are permitted to overlap. *)
let format_intervals ?config intervals tree =
  let source_text = SyntaxTree.text tree in
  let text = SourceText.text source_text in
  let env = env_from_config config in
  let chunk_groups =
    tree
    |> SyntaxTransforms.editable_from_positioned
    |> Hack_format.transform env
    |> Chunk_builder.build
  in
  let line_boundaries = get_line_boundaries text in
  let atom_boundaries = get_atom_boundaries chunk_groups in
  let ranges =
    intervals
    |> List.map ~f:(line_interval_to_offset_range line_boundaries)
    (* If we get a range which starts or ends exactly at an atom boundary,
       expand it to the NEXT atom boundary so that we get the whitespace
       surrounding the formatted range. We want the whitespace surrounding the
       tokens in the range, but we want to leave alone all other whitespace
       outside of the range. *)
    |> List.map ~f:(fun (st, ed) -> (st - 1, ed + 1))
    |> List.map ~f:(expand_to_atom_boundaries atom_boundaries)
    |> List.sort ~compare:Interval.compare
    |> Interval.union_consecutive_overlapping
  in
  let noformat_ranges =
    get_suppressed_formatting_ranges env line_boundaries tree
  in
  let ranges = Interval.diff_sorted_lists ranges noformat_ranges in
  let formatted_ranges =
    List.map ranges ~f:(fun range ->
        ( range,
          Line_splitter.solve
            env
            ~range
            ~include_leading_whitespace:
              (not
                 (List.exists atom_boundaries ~f:(fun (st, _) -> fst range = st)))
            ~include_trailing_whitespace:
              (not
                 (List.exists atom_boundaries ~f:(fun (_, ed) -> snd range = ed)))
            ~source_text:text
            chunk_groups ))
  in
  let buf = text_with_formatted_ranges text formatted_ranges in
  (* Dirty hack: Since we don't print the whitespace surrounding formatted
     ranges, we don't print the trailing newline at the end of the file if the
     last line in the file was modified. Add it here manually. *)
  if
    Buffer.length buf > 0
    && Char.(Buffer.nth buf (Buffer.length buf - 1) <> '\n')
  then
    Buffer.add_char buf '\n';
  Buffer.contents buf

(** Format a node at the given offset.
 *
 * Finds the node which is the direct parent of the token at the given byte
 * offset and formats a range containing that node which ends at the given
 * offset. The range that was formatted is returned (as a pair of 0-based byte
 * offsets in the original file) along with the formatted text.
 *
 * The provided offset must point to the last byte in a token. If not, an
 * invalid_arg exception will be raised.
 *
 * Designed to be suitable for as-you-type-formatting. *)
let format_at_offset ?config (tree : SyntaxTree.t) offset =
  let source_text = SyntaxTree.text tree in
  let env = env_from_config config in
  let chunk_groups =
    tree
    |> SyntaxTransforms.editable_from_positioned
    |> Hack_format.transform env
    |> Chunk_builder.build
  in
  let module PS = Full_fidelity_positioned_syntax in
  (* Grab the node which is the direct parent of the token at offset. If the
   * direct parent is a CompoundStatement, skip it and get the grandparent
   * (so we format entire IfStatements or MethodishDeclarations when formatting
   * at the closing brace). *)
  let is_sk_compound_statement = function
    | Full_fidelity_syntax_kind.CompoundStatement -> true
    | _ -> false
  in
  let is_sk_syntax_list = function
    | Full_fidelity_syntax_kind.SyntaxList -> true
    | _ -> false
  in
  let (token, node) =
    let module SK = Full_fidelity_syntax_kind in
    match PS.parentage (SyntaxTree.root tree) offset with
    | token :: parent :: grandparent :: _
      when is_sk_compound_statement (PS.kind parent)
           && not (is_sk_syntax_list (PS.kind grandparent)) ->
      (token, grandparent)
    | token :: parent :: _ -> (token, parent)
    | _ -> invalid_arg "Invalid offset"
  in
  (* Only format at the end of a token. *)
  if offset <> PS.end_offset token then
    invalid_arg "Offset must point to the last character of a token";

  (* Our ranges are half-open, so the range end is the token end offset + 1. *)
  let ed = offset + 1 in
  (* Take a half-open range which starts at the beginning of the parent node *)
  let range = (PS.start_offset node, ed) in
  (* Expand the start offset to the nearest line boundary in the original
   * source, since we want to add a leading newline before the node we're
   * formatting if one should be there and isn't already present. *)
  let range = (fst (expand_to_line_boundaries source_text range), ed) in
  (* Expand the start offset to the nearest atom boundary in the original
   * source, so that all whitespace preceding the formatted atoms is included in
   * the range. *)
  let atom_boundaries = get_atom_boundaries chunk_groups in
  let range = (fst (expand_to_atom_boundaries atom_boundaries range), ed) in
  (* Produce the formatted text *)
  let formatted =
    Line_splitter.solve
      env
      ~range
      ~include_leading_whitespace:false
      ~include_trailing_whitespace:false
      ~source_text:(SourceText.text source_text)
      chunk_groups
  in
  (range, formatted)

let format_doc (env : Env.t) (doc : Doc.t) =
  doc |> Chunk_builder.build |> Line_splitter.solve env

let format_doc_unbroken (env : Env.t) (doc : Doc.t) =
  doc
  |> Chunk_builder.build
  |> Line_splitter.unbroken env
  |> Line_splitter.print env
