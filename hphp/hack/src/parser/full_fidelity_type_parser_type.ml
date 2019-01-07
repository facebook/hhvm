(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module Context = Full_fidelity_parser_context.WithToken(Syntax.Token)
  module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S
  module WithLexer(Lexer : Lexer_S) = struct
    module type TypeParser_S = sig
      module SC : SmartConstructors.SmartConstructors_S
      type t
      val make : Full_fidelity_parser_env.t
        -> Lexer.t
        -> Full_fidelity_syntax_error.t list
        -> Full_fidelity_parser_context.WithToken(Syntax.Token).t
        -> SC.t
        -> t
      val sc_call : t -> (SC.t -> SC.t * SC.r) -> t * SC.r
      val lexer : t -> Lexer.t
      val errors : t -> Full_fidelity_syntax_error.t list
      val context : t -> Context.t
      val env : t -> Full_fidelity_parser_env.t
      val sc_state : t -> SC.t
      val parse_type_specifier : ?allow_var:bool -> t -> t * SC.r
      val parse_simple_type_or_type_constant : t -> t * SC.r
      val parse_return_type : t -> t * SC.r
      val parse_possible_generic_specifier : t -> t * SC.r
      val parse_type_constraint_opt : t -> t * SC.r
      val parse_generic_type_parameter_list: t -> t * SC.r
      val parse_generic_type_argument_list: t -> t * SC.r * bool
      val parse_remaining_type_specifier: SC.r -> t -> t * SC.r
    end (* TypeParser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
