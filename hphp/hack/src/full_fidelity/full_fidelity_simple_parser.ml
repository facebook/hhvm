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

module WithLexer(Lexer : Full_fidelity_lexer_sig.Lexer_S) = struct
  module Lexer = Lexer

  type t = {
    lexer : Lexer.t;
    errors : SyntaxError.t list
  }

  let make lexer errors =
    { lexer; errors }

  let errors parser =
    parser.errors @ (Lexer.errors parser.lexer)

  let with_errors parser errors =
    { parser with errors }

  let lexer parser =
    parser.lexer

  let with_lexer parser lexer =
    { parser with lexer }

end
