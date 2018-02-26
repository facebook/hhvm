(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Env = Full_fidelity_parser_env
module type SC_S = SmartConstructors.SmartConstructors_S
module type SCWithToken_S = SmartConstructorsWrappers.SyntaxKind_S

[@@@ocaml.warning "-60"] (* https://caml.inria.fr/mantis/view.php?id=7522 *)
module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module WithSmartConstructors (SCI : SC_S with module Token = Syntax.Token) :
sig
  type t
  val make : Env.t -> Full_fidelity_source_text.t -> t
  val errors : t -> Full_fidelity_syntax_error.t list
  val env : t -> Env.t
  val sc_state : t -> SCI.t
  val parse_script : t -> t * Syntax.t
end = struct
module SCWithToken = SmartConstructorsWrappers.SyntaxKind(SCI)

module Lexer = Full_fidelity_lexer.WithToken(Syntax.Token)
module SyntaxError = Full_fidelity_syntax_error
module Context =
  Full_fidelity_parser_context.WithToken(Syntax.Token)

module type ExpressionParser_S = Full_fidelity_expression_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .ExpressionParser_S

module ExpressionParserSyntax_ = Full_fidelity_expression_parser
  .WithSyntax(Syntax)
module ExpressionParserSyntax = ExpressionParserSyntax_
  .WithSmartConstructors(SCWithToken)

module type StatementParser_S = Full_fidelity_statement_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .StatementParser_S

module StatementParserSyntax_ = Full_fidelity_statement_parser
  .WithSyntax(Syntax)
module StatementParserSyntax = StatementParserSyntax_
  .WithSmartConstructors(SCWithToken)

module type DeclarationParser_S = Full_fidelity_declaration_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .DeclarationParser_S

module DeclarationParserSyntax_ = Full_fidelity_declaration_parser
  .WithSyntax(Syntax)
module DeclarationParserSyntax = DeclarationParserSyntax_
  .WithSmartConstructors(SCWithToken)

module type TypeParser_S = Full_fidelity_type_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .TypeParser_S

module TypeParserSyntax_ = Full_fidelity_type_parser
  .WithSyntax(Syntax)
module TypeParserSyntax = TypeParserSyntax_
  .WithSmartConstructors(SCWithToken)

module rec ExpressionParser : (ExpressionParser_S with module SC = SCWithToken) =
  ExpressionParserSyntax.WithStatementAndDeclAndTypeParser
    (StatementParser) (DeclParser) (TypeParser)
and StatementParser : (StatementParser_S with module SC = SCWithToken) =
  StatementParserSyntax.WithExpressionAndDeclAndTypeParser
    (ExpressionParser) (DeclParser) (TypeParser)
and DeclParser : (DeclarationParser_S with module SC = SCWithToken) =
  DeclarationParserSyntax.WithExpressionAndStatementAndTypeParser
    (ExpressionParser) (StatementParser) (TypeParser)
and TypeParser : (TypeParser_S with module SC = SCWithToken) =
  TypeParserSyntax.WithExpressionParser(ExpressionParser)

type t = {
  lexer : Lexer.t;
  errors : SyntaxError.t list;
  context: Context.t;
  env: Env.t;
  sc_state : SCWithToken.t;
}

let make env text =
  { lexer = Lexer.make text
  ; errors = []
  ; context = Context.empty
  ; env
  ; sc_state = SCWithToken.initial_state ()
  }

let errors parser =
  parser.errors @ (Lexer.errors parser.lexer)

let env parser = parser.env

let sc_state parser = parser.sc_state

let parse_script parser =
  let decl_parser = DeclParser.make parser.env
    parser.lexer parser.errors parser.context parser.sc_state in
  let (decl_parser, node) = DeclParser.parse_script decl_parser in
  let lexer = DeclParser.lexer decl_parser in
  let errors = DeclParser.errors decl_parser in
  let parser = { parser with lexer; errors } in
  (parser, node)

end (* WithSmartConstructors *)

module SC = Full_fidelity_syntax_smart_constructors.WithSyntax(Syntax)
include WithSmartConstructors(SC)

end (* WithSyntax *)
