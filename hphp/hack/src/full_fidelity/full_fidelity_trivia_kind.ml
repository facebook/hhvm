(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(* @generated *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
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

let to_string kind =
  match kind with
  | WhiteSpace -> "whitespace"
  | EndOfLine -> "end_of_line"
  | DelimitedComment -> "delimited_comment"
  | SingleLineComment -> "single_line_comment"
  | Unsafe -> "unsafe"
  | UnsafeExpression -> "unsafe_expression"
  | FixMe -> "fix_me"
  | IgnoreError -> "ignore_error"
  | FallThrough -> "fall_through"
