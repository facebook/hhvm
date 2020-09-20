(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Minimal token
 *
 * A minimal token knows its kind, its width (without trivia), and its
 * associated trivia. It does not know its text.
 *)

module TokenKind = Full_fidelity_token_kind
module Trivia = Full_fidelity_minimal_trivia

type t = {
  kind: TokenKind.t;
  width: int;
  leading: Trivia.t list;
  trailing: Trivia.t list;
}
[@@deriving show, eq]

let make kind _source _offset width leading trailing =
  { kind; width; leading; trailing }

let leading_width token =
  let folder sum t = sum + Trivia.width t in
  List.fold_left folder 0 token.leading

let trailing_width token =
  let folder sum t = sum + Trivia.width t in
  List.fold_left folder 0 token.trailing

let width token = token.width

let full_width token = leading_width token + width token + trailing_width token

let kind token = token.kind

let with_kind token kind = { token with kind }

let leading token = token.leading

let trailing token = token.trailing

let with_leading leading token = { token with leading }

let with_trailing trailing token = { token with trailing }

let filter_leading_trivia_by_kind token kind =
  List.filter (fun t -> Trivia.kind t = kind) token.leading

let has_trivia_kind token kind =
  List.exists (fun t -> Trivia.kind t = kind) token.leading
  || List.exists (fun t -> Trivia.kind t = kind) token.trailing

let leading_start_offset _ = (* Not available *) 0

let text _ = (* Not available *) ""

let source_text _ = (* Not available *) Full_fidelity_source_text.empty

let to_json token =
  Hh_json.(
    JSON_Object
      [
        ("kind", JSON_String (TokenKind.to_string token.kind));
        ("width", int_ token.width);
        ("leading", JSON_Array (List.map Trivia.to_json token.leading));
        ("trailing", JSON_Array (List.map Trivia.to_json token.trailing));
      ])
