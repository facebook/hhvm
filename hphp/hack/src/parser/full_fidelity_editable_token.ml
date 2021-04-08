(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * An editable token contains the text of the token; these tokens are not
 * backed by a source text, like the positioned tokens are.
 *)

module Trivia = Full_fidelity_editable_trivia
module SourceText = Full_fidelity_source_text
module TokenKind = Full_fidelity_token_kind

type t = {
  kind: TokenKind.t;
  text: string;
  leading: Trivia.t list;
  trailing: Trivia.t list;
}
[@@deriving show, eq]

let create kind text leading trailing = { kind; text; leading; trailing }

let make kind source_text offset width leading trailing =
  let leading_width =
    List.fold_left (fun sum t -> sum + Trivia.width t) 0 leading
  in
  let text = SourceText.sub source_text (offset + leading_width) width in
  { kind; text; leading; trailing }

let leading_width token =
  let folder sum t = sum + Trivia.width t in
  List.fold_left folder 0 token.leading

let trailing_width token =
  let folder sum t = sum + Trivia.width t in
  List.fold_left folder 0 token.trailing

let width token = String.length token.text

let full_width token = leading_width token + width token + trailing_width token

let kind token = token.kind

let with_kind token kind = { token with kind }

let leading token = token.leading

let with_leading leading token = { token with leading }

let trailing token = token.trailing

let with_trailing trailing token = { token with trailing }

let filter_leading_trivia_by_kind token kind =
  token.leading |> List.filter (fun trivia -> trivia.Trivia.kind = kind)

let has_trivia_kind token kind =
  [token.leading; token.trailing]
  |> List.exists (List.exists (fun trivia -> trivia.Trivia.kind = kind))

let text token = token.text

let leading_text token = Trivia.text_from_trivia_list (leading token)

let trailing_text token = Trivia.text_from_trivia_list (trailing token)

let full_text token = leading_text token ^ text token ^ trailing_text token

let to_json token =
  Hh_json.(
    JSON_Object
      [
        ("kind", JSON_String (TokenKind.to_string token.kind));
        ("text", JSON_String token.text);
        ("leading", JSON_Array (List.map Trivia.to_json token.leading));
        ("trailing", JSON_Array (List.map Trivia.to_json token.trailing));
      ])

let source_text _ =
  raise (Invalid_argument "Editable token doesn't support source_text")

let leading_start_offset _ =
  raise (Invalid_argument "Editable token doesn't support leading_start_offset")
