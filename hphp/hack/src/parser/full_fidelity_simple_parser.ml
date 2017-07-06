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
module Context = Full_fidelity_parser_context

module WithLexer(Lexer : Full_fidelity_lexer_sig.Lexer_S) = struct
  module Lexer = Lexer

  type t = {
    lexer : Lexer.t;
    errors : SyntaxError.t list;
    context : Context.t
  }

  let make lexer errors context =
    { lexer; errors; context }

  let errors parser =
    parser.errors @ (Lexer.errors parser.lexer)

  let with_errors parser errors =
    { parser with errors }

  let lexer parser =
    parser.lexer

  let with_lexer parser lexer =
    { parser with lexer }

  let context parser =
    parser.context

  let with_context parser context =
    { parser with context }

  (* Add a new expected token to 'parser''s context. *)
  let expect parser token_kind =
    let new_context = Context.expect parser.context token_kind in
    with_context parser new_context

  (* Check if token_kind is in the parser's list of expected token kinds. *)
  let expects parser token_kind =
    Context.expects parser.context token_kind

end
