(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *)
module SourceText = Full_fidelity_source_text

module type PositionedSyntax_S = sig
  module Token : Lexable_positioned_token_sig.LexablePositionedToken_S

  include Syntax_sig.Syntax_S with module Token := Token

  val text : t -> string

  val full_text : t -> string

  val leading_text : t -> string

  val source_text : t -> SourceText.t

  val start_offset : t -> int

  val end_offset : t -> int

  val is_synthetic : t -> bool

  (*
   * Similar to position except that the end_offset does not include
   * the last character. (the end offset is one larger than given by position)
   *)
  val position_exclusive : Relative_path.t -> t -> Pos.t option
end
