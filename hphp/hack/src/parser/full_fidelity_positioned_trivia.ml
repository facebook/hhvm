(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Positioned trivia
 *
 * A positioned trivia stores the original source text, the offset of the
 * start of the trivia, and its width.  From all this information we can
 * rapidly compute the absolute position of the trivia, or obtain its text.
 *
 *)

module TriviaKind = Full_fidelity_trivia_kind
module SourceText = Full_fidelity_source_text
module MinimalTrivia = Full_fidelity_minimal_trivia

type t = {
  kind: TriviaKind.t;
  source_text: SourceText.t;
  offset: int;
  width: int;
}
[@@deriving show, eq]

let make_ignore_error source_text offset width =
  { kind = TriviaKind.IgnoreError; source_text; offset; width }

let make_extra_token_error source_text offset width =
  { kind = TriviaKind.ExtraTokenError; source_text; offset; width }

let make_fallthrough source_text offset width =
  { kind = TriviaKind.FallThrough; source_text; offset; width }

let make_fix_me source_text offset width =
  { kind = TriviaKind.FixMe; source_text; offset; width }

let make_whitespace source_text offset width =
  { kind = TriviaKind.WhiteSpace; source_text; offset; width }

let make_eol source_text offset width =
  { kind = TriviaKind.EndOfLine; source_text; offset; width }

let make_single_line_comment source_text offset width =
  { kind = TriviaKind.SingleLineComment; source_text; offset; width }

let make_delimited_comment source_text offset width =
  { kind = TriviaKind.DelimitedComment; source_text; offset; width }

let width trivia = trivia.width

let kind trivia = trivia.kind

let start_offset trivia = trivia.offset

let end_offset trivia = trivia.offset + trivia.width - 1

let source_text trivia = trivia.source_text

let text trivia =
  SourceText.sub (source_text trivia) (start_offset trivia) (width trivia)

let from_minimal source_text minimal_trivia offset =
  let kind = MinimalTrivia.kind minimal_trivia in
  let width = MinimalTrivia.width minimal_trivia in
  { kind; source_text; offset; width }

let from_minimal_list source_text ts offset =
  let rec aux acc ts offset =
    match ts with
    | [] -> acc
    | h :: t ->
      let et = from_minimal source_text h offset in
      aux (et :: acc) t (offset + MinimalTrivia.width h)
  in
  List.rev (aux [] ts offset)

let to_json trivia =
  Hh_json.(
    JSON_Object
      [
        ("kind", JSON_String (TriviaKind.to_string trivia.kind));
        ("text", JSON_String (text trivia));
        ("offset", int_ trivia.offset);
        ("width", int_ trivia.width);
      ])
