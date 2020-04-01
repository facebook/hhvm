(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text
module TriviaKind = Full_fidelity_trivia_kind

module type LexableTrivia_S = sig
  type t [@@deriving show]

  val make_whitespace : SourceText.t -> int -> int -> t

  val make_eol : SourceText.t -> int -> int -> t

  val make_single_line_comment : SourceText.t -> int -> int -> t

  val make_fallthrough : SourceText.t -> int -> int -> t

  val make_fix_me : SourceText.t -> int -> int -> t

  val make_ignore_error : SourceText.t -> int -> int -> t

  val make_extra_token_error : SourceText.t -> int -> int -> t

  val make_delimited_comment : SourceText.t -> int -> int -> t

  val kind : t -> TriviaKind.t

  val width : t -> int
end
