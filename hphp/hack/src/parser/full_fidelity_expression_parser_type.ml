(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S
  module WithLexer(Lexer : Lexer_S) = struct
    module type ExpressionParser_S = sig
      type t
      val make : Full_fidelity_parser_env.t
        -> Lexer.t
        -> Full_fidelity_syntax_error.t list
        -> Full_fidelity_parser_context.WithToken(Syntax.Token).t
        -> t
      val lexer : t -> Lexer.t
      val errors : t -> Full_fidelity_syntax_error.t list
      val parse_expression : t -> t * Syntax.t
      val parse_simple_variable: t -> t * Syntax.t
    end (* ExpressionParser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
