(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
  val parse_script : t -> t * SCI.r
  val parse_header_only : Env.t -> Full_fidelity_source_text.t -> SCI.r option
end = struct
module SCWithToken = SmartConstructorsWrappers.SyntaxKind(SCI)

module Lexer = Full_fidelity_lexer.WithToken(Syntax.Token)
module SourceText = Full_fidelity_source_text
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
  TypeParserSyntax.WithExpressionAndDeclParser (ExpressionParser) (DeclParser)

type t = {
  lexer : Lexer.t;
  errors : SyntaxError.t list;
  context: Context.t;
  env: Env.t;
  sc_state : SCWithToken.t;
}

let make (env:Env.t) text =
  let lexer = Lexer.make
    ~is_experimental_mode:(Env.is_experimental_mode env)
    ~force_hh:(Env.force_hh env)
    ~enable_xhp:(Env.enable_xhp env)
    ~disable_unsafe_expr:(Env.disable_unsafe_expr env)
    ~disable_unsafe_block:(Env.disable_unsafe_block env)
    text in
  { lexer
  ; errors = []
  ; context = Context.empty
  ; env
  ; sc_state = SCWithToken.initial_state env
  }

let errors parser =
  parser.errors @ (Lexer.errors parser.lexer)

let env parser = parser.env

let sc_state parser = parser.sc_state

let parse_header_only env text =
  let { env; lexer; errors; context; sc_state } = make env text in
  let decl_parser = DeclParser.make env lexer errors context sc_state in
  let _, result = DeclParser.parse_leading_markup_section decl_parser in
  match result with
  | Some result -> Some (SCWithToken.extract result)
  | None -> None

let parse_script parser =
  let decl_parser =
    DeclParser.make
      parser.env
      parser.lexer
      parser.errors
      parser.context
      parser.sc_state
  in
  let (decl_parser, node) = DeclParser.parse_script decl_parser in
  let lexer = DeclParser.lexer decl_parser in
  let errors = DeclParser.errors decl_parser in
  let context = DeclParser.context decl_parser in
  let env = DeclParser.env decl_parser in
  let sc_state = DeclParser.sc_state decl_parser in
  { lexer; errors; context; env; sc_state }, SCWithToken.extract node

end (* WithSmartConstructors *)

module SC = SyntaxSmartConstructors.WithSyntax(Syntax)
include WithSmartConstructors(SC)

end (* WithSyntax *)

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_minimal_syntax
module Parser = WithSyntax(Syntax)
open Syntax

let parse_mode text =
  let suffix = Relative_path.suffix (SourceText.file_path text) in
  let is_hhi = String_utils.string_ends_with suffix ".hhi" in
  let header = Parser.parse_header_only (Env.make ()) text in
  match header with
  | None -> Some FileInfo.Mstrict (* no header - assume .hack file *)
  | Some header ->
    begin match syntax header with
    | MarkupSection
      { markup_prefix = pfx
      ; markup_text = txt
      ; markup_suffix =
        { syntax = MarkupSuffix
          { markup_suffix_less_than_question = ltq
          ; markup_suffix_name = name
          }
        ; _
        }
      ; _
      } ->
        begin match syntax name with
        | Missing -> Some FileInfo.Mphp
        | Token t when Token.kind t = Full_fidelity_token_kind.Equal -> Some FileInfo.Mphp
        | _ ->
          let skip_length =
            full_width pfx +
            full_width txt +
            full_width ltq +
            leading_width name
          in
          let language = width name
            |> SourceText.sub text skip_length
            |> String.lowercase_ascii
          in
          if language = "php" then Some FileInfo.Mphp else
          if is_hhi then Some FileInfo.Mdecl else
          let skip_length = skip_length + width name in
          let s = SourceText.sub text skip_length (trailing_width name) in
          let s = String.trim s in
          let l = String.length s in
          let mode =
            if l < 2 || s.[0] <> '/' || s.[1] <> '/' then "" else
              String.trim (String.sub s 2 (l - 2))
          in
          let mode =
            try List.hd (Str.split (Str.regexp " +") mode)
            with _ -> ""
          in
          FileInfo.parse_mode mode
        end
    | _ -> Some FileInfo.Mstrict
  end
