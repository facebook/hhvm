(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * An editable token contains the text of the token; these tokens are not
 * backed by a source text, like the positioned tokens are.
 *)

module PositionedToken = Full_fidelity_positioned_token
module Trivia = Full_fidelity_editable_trivia
module SourceText = Full_fidelity_source_text
module TokenKind = Full_fidelity_token_kind

type t = {
  kind: TokenKind.t;
  text: string;
  leading: Trivia.t list;
  trailing: Trivia.t list
}

let make kind text leading trailing =
  { kind; text; leading; trailing }

let leading_width token =
  let folder sum t = sum + (Trivia.width t) in
  List.fold_left folder 0 token.leading

let trailing_width token =
  let folder sum t = sum + (Trivia.width t) in
  List.fold_left folder 0 token.trailing

let width token =
  String.length token.text

let full_width token =
  (leading_width token) + (width token) + (trailing_width token)

let kind token =
  token.kind

let leading token =
  token.leading

let trailing token =
  token.trailing

let text token =
  token.text

let leading_text token =
  Trivia.text_from_trivia_list (leading token)

let trailing_text token =
  Trivia.text_from_trivia_list (trailing token)

let full_text token =
  (leading_text token) ^ (text token) ^ (trailing_text token)

let from_positioned source_text positioned_token offset =
  let lw = PositionedToken.leading_width positioned_token in
  let w = PositionedToken.width positioned_token in
  let leading = Trivia.from_positioned_list source_text
    (PositionedToken.leading positioned_token) offset in
  let text = SourceText.sub source_text (offset + lw) w in
  let trailing = Trivia.from_positioned_list source_text
    (PositionedToken.trailing positioned_token) (offset + lw + w) in
  make (PositionedToken.kind positioned_token) text leading trailing

let to_json token =
  let open Hh_json in
  JSON_Object [
    ("kind", JSON_String (TokenKind.to_string token.kind));
    ("text", JSON_String token.text);
    ("leading", JSON_Array (List.map Trivia.to_json token.leading));
    ("trailing", JSON_Array (List.map Trivia.to_json token.trailing)) ]
