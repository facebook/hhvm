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
 * Minimal token
 *
 * A minimal token knows its kind, its width (without trivia), and its
 * associated trivia. It does not know its text.
 *)

module TokenKind = Full_fidelity_token_kind
module MinimalTrivia = Full_fidelity_minimal_trivia
module TriviaKind = Full_fidelity_trivia_kind

type t = {
  kind: TokenKind.t;
  width: int;
  leading: MinimalTrivia.t list;
  trailing: MinimalTrivia.t list
}

let make kind width leading trailing =
  { kind; width; leading; trailing }

let leading_width token =
  let folder sum t = sum + (MinimalTrivia.width t) in
  List.fold_left folder 0 token.leading

let trailing_width token =
  let folder sum t = sum + (MinimalTrivia.width t) in
  List.fold_left folder 0 token.trailing

let width token =
  token.width

let full_width token =
  (leading_width token) + (width token) + (trailing_width token)

let kind token =
  token.kind

let with_kind token kind =
  { token with kind }

let leading token =
  token.leading

let trailing token =
  token.trailing

let has_leading_end_of_line token =
  Core.List.exists token.leading
    ~f:(fun trivia ->  MinimalTrivia.kind trivia = TriviaKind.EndOfLine)

let with_leading leading token =
  { token with leading }

let to_json token =
  let open Hh_json in
  JSON_Object [
    ("kind", JSON_String (TokenKind.to_string token.kind));
    ("width", int_ token.width);
    ("leading", JSON_Array (List.map MinimalTrivia.to_json token.leading));
    ("trailing", JSON_Array (List.map MinimalTrivia.to_json token.trailing)) ]
