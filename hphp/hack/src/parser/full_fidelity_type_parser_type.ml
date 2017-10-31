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
    module type TypeParser_S = sig
      type t
      val make : Full_fidelity_parser_env.t
        -> Lexer.t
        -> Full_fidelity_syntax_error.t list
        -> Full_fidelity_parser_context.WithToken(Syntax.Token).t
        -> t
      val lexer : t -> Lexer.t
      val errors : t -> Full_fidelity_syntax_error.t list
      val parse_type_specifier : ?allow_var:bool -> t ->
        t * Syntax.t
      val parse_return_type : t -> t * Syntax.t
      val parse_possible_generic_specifier : t -> t * Syntax.t
      val parse_type_constraint_opt : t -> t * Syntax.t
      val parse_generic_type_parameter_list: t -> t * Syntax.t
      val parse_generic_parameter_list_opt: t -> t * Syntax.t
      val parse_generic_type_argument_list_opt: t ->
        t * Syntax.t
    end (* TypeParser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
