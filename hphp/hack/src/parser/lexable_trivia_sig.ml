(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module type LexableTrivia_S = sig
  type t
  val make_whitespace: int -> t
  val make_eol: int -> t
  val make_single_line_comment: int -> t
  val make_fallthrough: int -> t
  val make_unsafe: int -> t
  val make_unsafe_expression: int -> t
  val make_fix_me: int -> t
  val make_ignore_error: int -> t
  val make_extra_token_error: int -> t
  val make_delimited_comment: int -> t
  val kind : t -> Full_fidelity_trivia_kind.t
end
