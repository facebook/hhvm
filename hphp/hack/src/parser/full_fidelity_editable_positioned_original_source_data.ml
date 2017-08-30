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
 * SourceData represents information with relation to the original SourceText.
 *)

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module Token = Full_fidelity_positioned_token
module Trivia = Full_fidelity_positioned_trivia

(**
 * Data about the token with respect to the original source text.
 *)
type t = {
  source_text: SourceText.t;
  offset: int; (* Beginning of first trivia *)
  leading_width: int;
  width: int; (* Width of actual token, not counting trivia *)
  trailing_width: int;
  leading: Trivia.t list;
  trailing: Trivia.t list;
}

let empty =
  {
    source_text = SourceText.empty;
    offset = 0;
    leading_width = 0;
    width = 0;
    trailing_width = 0;
    leading = [];
    trailing = [];
  }

let from_positioned_token positioned_token =
  {
    source_text = Token.source_text positioned_token;
    offset = Token.leading_start_offset positioned_token;
    leading_width = Token.leading_width positioned_token;
    width = Token.width positioned_token;
    trailing_width = Token.trailing_width positioned_token;
    leading = Token.leading positioned_token;
    trailing = Token.trailing positioned_token;
  }

let from_positioned_syntax syntax =
  {
    source_text = Syntax.source_text syntax;
    offset = Syntax.leading_start_offset syntax;
    leading_width = Syntax.leading_width syntax;
    width = Syntax.width syntax;
    trailing_width = Syntax.trailing_width syntax;
    leading = [];
    trailing = [];
  }

let source_text data =
  data.source_text

let leading_width data =
  data.leading_width

let width data =
  data.width

let trailing_width data =
  data.trailing_width

let leading_start_offset data =
  data.offset

let leading data =
  data.leading

let trailing data =
  data.trailing

let start_offset data =
  leading_start_offset data + leading_width data

let end_offset data =
  start_offset data + width data

let full_width data =
  leading_width data + width data + trailing_width data

let text data =
  SourceText.sub (source_text data) (start_offset data) (width data)

(**
 * Merges two SourceDatas by computing the span between them.
 *
 * The resulting SourceData will have:
 *   These properties of the beginning SourceData:
 *     - SourceText
 *     - offset
 *     - leading_width
 *     - leading
 *   These properties of the ending SourceData:
 *     - trailing_width
 *     - trailing
 *
 * The width will be computed as the distance from the beginning SourceData's
 * start_offset to the ending SourceData's end_offset.
 *)
let spanning_between b e =
  {
    source_text = b.source_text;
    offset = b.offset;
    leading_width = b.leading_width;
    width = end_offset e - start_offset b;
    trailing_width = e.trailing_width;
    leading = b.leading;
    trailing = e.trailing;
  }

let to_json data =
  let open Hh_json in
  JSON_Object [
    ("offset", int_ data.offset);
    ("leading_width", int_ data.leading_width);
    ("width", int_ data.width);
    ("trailing_width", int_ data.trailing_width);
  ]
