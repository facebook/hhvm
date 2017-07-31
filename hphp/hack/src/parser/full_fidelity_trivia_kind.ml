(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 * This module contains the type describing the structure of a syntax tree.
 *
 **
 *
 *)

type t =
  | WhiteSpace
  | EndOfLine
  | DelimitedComment
  | SingleLineComment
  | Unsafe
  | UnsafeExpression
  | FixMe
  | IgnoreError
  | FallThrough
  | ExtraTokenError

let to_string kind =
  match kind with
  | WhiteSpace        -> "whitespace"
  | EndOfLine         -> "end_of_line"
  | DelimitedComment  -> "delimited_comment"
  | SingleLineComment -> "single_line_comment"
  | Unsafe            -> "unsafe"
  | UnsafeExpression  -> "unsafe_expression"
  | FixMe             -> "fix_me"
  | IgnoreError       -> "ignore_error"
  | FallThrough       -> "fall_through"
  | ExtraTokenError   -> "extra_token_error"
