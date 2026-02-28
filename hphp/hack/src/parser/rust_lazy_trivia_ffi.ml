(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SourceText = Full_fidelity_source_text

module RustPositionedTrivium = struct
  type t = {
    kind: Full_fidelity_trivia_kind.t;
    offset: int;
    width: int;
  }
  [@@deriving show, eq]
end

external scan_leading_xhp_trivia :
  SourceText.t -> int -> int -> RustPositionedTrivium.t list
  = "scan_leading_xhp_trivia"

external scan_trailing_xhp_trivia :
  SourceText.t -> int -> int -> RustPositionedTrivium.t list
  = "scan_trailing_xhp_trivia"

external scan_leading_php_trivia :
  SourceText.t -> int -> int -> RustPositionedTrivium.t list
  = "scan_leading_php_trivia"

external scan_trailing_php_trivia :
  SourceText.t -> int -> int -> RustPositionedTrivium.t list
  = "scan_trailing_php_trivia"
