(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


module SourceText = Full_fidelity_source_text
module MinimalTrivia = Full_fidelity_minimal_trivia

type lexer_opts = (bool * bool * bool * bool)

external scan_leading_xhp_trivia:
  SourceText.t -> lexer_opts -> int -> MinimalTrivia.t list = "scan_leading_xhp_trivia"

external scan_trailing_xhp_trivia:
  SourceText.t -> lexer_opts -> int -> MinimalTrivia.t list = "scan_trailing_xhp_trivia"

external scan_leading_php_trivia:
  SourceText.t -> lexer_opts -> int -> MinimalTrivia.t list = "scan_leading_php_trivia"

external scan_trailing_php_trivia:
  SourceText.t -> lexer_opts -> int -> MinimalTrivia.t list = "scan_trailing_php_trivia"
