(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module Lexer = Full_fidelity_lexer

type t = {
  lexer : Lexer.t;
  errors : SyntaxError.t list;
  precedence : int
}

let make lexer errors =
  { lexer; errors; precedence = 0 }

let errors parser =
  parser.errors

let with_errors parser errors =
  { parser with errors }

let with_lexer parser lexer =
  { parser with lexer }

let lexer parser =
  parser.lexer

let with_precedence parser precedence =
  { parser with precedence }

let with_reset_precedence parser parse_function =
  let old_precedence = parser.precedence in
  let parser = with_precedence parser 0 in
  let (parser, result) = parse_function parser in
  let parser = with_precedence parser old_precedence in
  (parser, result)
