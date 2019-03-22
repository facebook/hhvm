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
    module type DeclarationParser_S = sig
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
      val parse_script : t -> t * SC.r
      val parse_leading_markup_section : t -> t * SC.r option
      val parse_function_declaration : t -> SC.r -> t * SC.r
      val parse_parameter_list_opt : t -> t * SC.r * SC.r * SC.r
      val parse_classish_declaration : t -> SC.r -> t * SC.r
      val parse_classish_extends_opt : t -> t * SC.r * SC.r
      val parse_classish_implements_opt : t -> t * SC.r * SC.r
      val parse_classish_body : t -> t * SC.r
      val parse_attribute_specification_opt : t -> t * SC.r
    end (* DeclarationParser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
