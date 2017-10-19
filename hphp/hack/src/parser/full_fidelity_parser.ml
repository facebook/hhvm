(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Lexer = Full_fidelity_minimal_lexer
module SyntaxError = Full_fidelity_syntax_error
module Context = Full_fidelity_parser_context
module rec ExpressionParser :
  Full_fidelity_expression_parser_type.ExpressionParserType =
  Full_fidelity_expression_parser.WithStatementAndDeclAndTypeParser
    (StatementParser) (DeclParser) (TypeParser)
and StatementParser :
  Full_fidelity_statement_parser_type.StatementParserType =
  Full_fidelity_statement_parser.WithExpressionAndDeclAndTypeParser
    (ExpressionParser) (DeclParser) (TypeParser)
and DeclParser :
  Full_fidelity_declaration_parser_type.DeclarationParserType =
  Full_fidelity_declaration_parser.WithExpressionAndStatementAndTypeParser
    (ExpressionParser) (StatementParser) (TypeParser)
and TypeParser :
  Full_fidelity_type_parser_type.TypeParserType =
  Full_fidelity_type_parser.WithExpressionParser(ExpressionParser)

type t = {
  lexer : Lexer.t;
  errors : SyntaxError.t list;
  context: Context.t;
  hhvm_compat_mode: bool;
}

let make ?(hhvm_compat_mode = false) text =
  { lexer = Lexer.make text
  ; errors = []
  ; context = Context.empty
  ; hhvm_compat_mode
  }

let errors parser =
  parser.errors @ (Lexer.errors parser.lexer)

let hhvm_compat_mode parser =
  parser.hhvm_compat_mode

let parse_script parser =
  let decl_parser = DeclParser.make parser.lexer
    parser.errors parser.context in
  let (decl_parser, node) = DeclParser.parse_script decl_parser in
  let lexer = DeclParser.lexer decl_parser in
  let errors = DeclParser.errors decl_parser in
  let parser = { parser with lexer; errors } in
  (parser, node)
