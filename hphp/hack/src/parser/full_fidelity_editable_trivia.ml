(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * An editable trivia contains the text of the trivia; these trivia are not
 * backed by a source text, like the positioned trivia are.
 *)

module TriviaKind = Full_fidelity_trivia_kind
module PositionedTrivia = Full_fidelity_positioned_trivia
module SourceText = Full_fidelity_source_text

type t = {
  kind: TriviaKind.t;
  text: string;
}
[@@deriving show]

let make_ignore_error source_text offset width =
  {
    kind = TriviaKind.IgnoreError;
    text = SourceText.sub source_text offset width;
  }

let make_extra_token_error source_text offset width =
  {
    kind = TriviaKind.ExtraTokenError;
    text = SourceText.sub source_text offset width;
  }

let make_fallthrough source_text offset width =
  {
    kind = TriviaKind.FallThrough;
    text = SourceText.sub source_text offset width;
  }

let make_fix_me source_text offset width =
  { kind = TriviaKind.FixMe; text = SourceText.sub source_text offset width }

let make_whitespace source_text offset width =
  {
    kind = TriviaKind.WhiteSpace;
    text = SourceText.sub source_text offset width;
  }

let make_eol source_text offset width =
  {
    kind = TriviaKind.EndOfLine;
    text = SourceText.sub source_text offset width;
  }

let make_single_line_comment source_text offset width =
  {
    kind = TriviaKind.SingleLineComment;
    text = SourceText.sub source_text offset width;
  }

let make_delimited_comment source_text offset width =
  {
    kind = TriviaKind.DelimitedComment;
    text = SourceText.sub source_text offset width;
  }

(* HackFormat is using this to create trivia. It's deeply manipulating strings
 * so it was easier to create this helper function to avoid ad-hoc
 * SourceText construction.*)
let create_delimited_comment text = { kind = TriviaKind.DelimitedComment; text }

let width trivia = String.length trivia.text

let kind trivia = trivia.kind

let text trivia = trivia.text

let text_from_trivia_list trivia_list =
  (* TODO: Better way to accumulate a string? *)
  let folder str trivia = str ^ text trivia in
  List.fold_left folder "" trivia_list

let from_positioned source_text positioned_trivia offset =
  let kind = PositionedTrivia.kind positioned_trivia in
  let width = PositionedTrivia.width positioned_trivia in
  let text = SourceText.sub source_text offset width in
  { kind; text }

let from_positioned_list source_text ts offset =
  let rec aux acc ts offset =
    match ts with
    | [] -> acc
    | h :: t ->
      let et = from_positioned source_text h offset in
      aux (et :: acc) t (offset + PositionedTrivia.width h)
  in
  List.rev (aux [] ts offset)

let to_json trivia =
  Hh_json.(
    JSON_Object
      [
        ("kind", JSON_String (TriviaKind.to_string trivia.kind));
        ("text", JSON_String trivia.text);
      ])
