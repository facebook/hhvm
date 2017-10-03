(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * An EditablePositionedToken represents a token that comes from a positioned
 * source but may have been modified. The token may have had its text or kind
 * changed, or may have been moved to a new location in the AST. An
 * EditablePositionedToken falls into one of these categories:
 *
 *   - Original.  The token was positioned in the original source text and not
 *     modified.
 *   - SynthesizedFromOriginal.  The token was positioned in the original
 *     SourceText and modified. It may have had its text or kind changed, and it
 *     may now be positioned in a location that does not logically correspond to
 *     its location in the original SourceText.
 *   - Synthetic.  The token never existed in the original SourceText. It was
 *     synthesized during the AST computation process (colloquially,
 *     "lowering").
 *)

module PositionedToken = Full_fidelity_positioned_token
module SourceData = Full_fidelity_editable_positioned_original_source_data
module TokenKind = Full_fidelity_token_kind

(**
 * Data about the token with respect to the original source text.
 *)
type synthetic_token_data = {
  text: string;
}

type token_data =
  | Original of SourceData.t
  | SynthesizedFromOriginal of synthetic_token_data * SourceData.t
  | Synthetic of synthetic_token_data

(**
 * Data common to all EditablePositionedTokens.
 *)
type t = {
  kind: TokenKind.t;
  leading_text: string;
  trailing_text: string;
  token_data: token_data;
}

let from_positioned_token positioned_token =
  {
    kind = PositionedToken.kind positioned_token;
    leading_text = PositionedToken.leading_text positioned_token;
    trailing_text = PositionedToken.trailing_text positioned_token;
    token_data = Original (SourceData.from_positioned_token positioned_token);
  }

(**
 * Retains the original_source_data and trivia from the existing
 * EditablePositionedToken if present.
 *)
let synthesize_from editable_positioned_token kind text =
  let synthetic_token_data = { text; } in
  let token_data =
    match editable_positioned_token.token_data with
    | Original original_source_data
    | SynthesizedFromOriginal (_, original_source_data) ->
        SynthesizedFromOriginal (synthetic_token_data, original_source_data)
    | Synthetic _ ->
        Synthetic synthetic_token_data in
  { editable_positioned_token with kind; token_data; }

let synthesize_new kind text leading_text trailing_text =
  {
    kind;
    leading_text;
    trailing_text;
    token_data = Synthetic { text; };
  }

let text token =
  match token.token_data with
  | Original original_source_data ->
      SourceData.text original_source_data
  | SynthesizedFromOriginal ({ text; _; }, _)
  | Synthetic { text; _; } ->
      text

let leading_text token =
  token.leading_text

let trailing_text token =
  token.trailing_text

let filter_leading_trivia_by_kind token kind =
  match token.token_data with
  | Original orig_token
  | SynthesizedFromOriginal (_, orig_token)
    ->
      List.filter (fun t -> Full_fidelity_positioned_trivia.kind t = kind)
        (SourceData.leading orig_token)
  | _ -> []

let has_trivia_kind token kind =
  match token.token_data with
  | Original orig_token
  | SynthesizedFromOriginal (_, orig_token)
    ->
      let kind_of = Full_fidelity_positioned_trivia.kind in
      List.exists (fun t -> kind_of t = kind) (SourceData.leading orig_token) ||
      List.exists (fun t -> kind_of t = kind) (SourceData.trailing orig_token)
  | _ -> false (* Assume we don't introduce well-formed trivia *)

let full_text token =
  leading_text token ^ text token ^ trailing_text token

let original_source_data_or_default token =
  match token.token_data with
  | Original original_source_data
  | SynthesizedFromOriginal (_, original_source_data) ->
      original_source_data
  | Synthetic _ ->
      SourceData.empty

let kind token =
  token.kind

let source_text token =
  SourceData.source_text (original_source_data_or_default token)

let leading_width token =
  SourceData.leading_width (original_source_data_or_default token)

let width token =
  SourceData.width (original_source_data_or_default token)

let trailing_width token =
  SourceData.trailing_width (original_source_data_or_default token)

let leading_start_offset token =
  SourceData.leading_start_offset (original_source_data_or_default token)

let start_offset token =
  SourceData.start_offset (original_source_data_or_default token)

let leading token =
  SourceData.leading (original_source_data_or_default token)

let trailing token =
  SourceData.trailing (original_source_data_or_default token)

let with_updated_original_source_data token update_original_source_data =
  let token_data =
    match token.token_data with
    | Original original_source_data ->
        Original (update_original_source_data original_source_data)
    | SynthesizedFromOriginal (synthetic_token_data, original_source_data) ->
        SynthesizedFromOriginal (
          synthetic_token_data,
          update_original_source_data original_source_data
        )
    | Synthetic _ ->
        token.token_data in
  { token with token_data }

let concatenate b e =
  let token_data =
    match b.token_data, e.token_data with
    | Original b_source_data, Original e_source_data ->
        Original (SourceData.spanning_between b_source_data e_source_data)
    | (SynthesizedFromOriginal (_, b_source_data) | Original b_source_data),
      (SynthesizedFromOriginal (_, e_source_data) | Original e_source_data) ->
        SynthesizedFromOriginal (
          { text = text b ^ text e },
          (SourceData.spanning_between b_source_data e_source_data)
        )
    | Synthetic _, _
    | _, Synthetic _ ->
        Synthetic { text = text b ^ text e } in
  {
    kind = kind b;
    leading_text = leading_text b;
    trailing_text = trailing_text e;
    token_data;
  }

let to_json token =
  let original_source_data = original_source_data_or_default token in
  let open Hh_json in
  JSON_Object [
    ("kind", JSON_String (TokenKind.to_string token.kind));
    ("leading_text", JSON_String token.leading_text);
    ("trailing_text", JSON_String token.trailing_text);
    ("original_source_data", SourceData.to_json original_source_data);
    ("text", JSON_String (text token));
  ]
