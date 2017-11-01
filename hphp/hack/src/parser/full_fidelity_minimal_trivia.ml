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
 * Minimal trivia
 *
 * A minimal trivia knows its kind and width. It does not know its text.
 *
 *)

open Full_fidelity_trivia_kind

type t = {
  kind: Full_fidelity_trivia_kind.t;
  width: int
}

let make_whitespace _text _offset width =
  { kind = WhiteSpace; width }

let make_eol _text _offset width =
  { kind = EndOfLine; width }

let make_single_line_comment _text _offset width =
  { kind = SingleLineComment; width }

let make_fallthrough _text _offset width =
  { kind = FallThrough; width }

let make_unsafe _text _offset width =
  { kind = Unsafe; width }

let make_unsafe_expression _text _offset width =
  { kind = UnsafeExpression; width }

let make_fix_me _text _offset width =
  { kind = FixMe; width}

let make_ignore_error _text _offset width =
  { kind = IgnoreError; width }

let make_delimited_comment _text _offset width =
  { kind = DelimitedComment; width }

let make_extra_token_error _text _offset width =
  { kind = ExtraTokenError; width }

let width trivia =
  trivia.width

let kind trivia =
  trivia.kind

let to_json trivia =
  let open Hh_json in
  JSON_Object [
    ("kind", JSON_String (to_string trivia.kind));
    ("width", int_ trivia.width); ]
