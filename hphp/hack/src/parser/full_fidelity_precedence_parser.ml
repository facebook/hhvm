(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S
module Context = Full_fidelity_parser_context.WithToken(Syntax.Token)
module TokenKind = Full_fidelity_token_kind
module type SCWithToken_S = SmartConstructorsWrappers.SyntaxKind_S

module WithLexer(Lexer : Lexer_S) = struct
  module Lexer = Lexer

  module WithSmartConstructors
  (SC : SCWithToken_S with module Token = Syntax.Token) = struct
    module SC = SC

    (* [Trick] Hack to keep track of prefix unary expressions created and change
    parser behavior based on this knowledge *)
    type prefix_unary_expression_type = {
      node : SC.r;
      operator_kind : TokenKind.t;
      operand : SC.r;
    } [@@deriving show]

type context_type = Context.t
let show_context_type _x = "<Full_fidelity_parser_context.WithToken(Syntax.Token).t>"
let pp_context_type _fmt _x = Printf.printf "%s\n" "<Full_fidelity_parser_context.WithToken(Syntax.Token).t>"

type t = {
  lexer : Lexer.t;
  errors : Full_fidelity_syntax_error.t list;
  context: context_type;
  precedence : int;
  allow_as_expressions: bool;
  env : Full_fidelity_parser_env.t;
  sc_state : SC.t;
  prefix_unary_expression_stack : prefix_unary_expression_type list;
} [@@deriving show]

    let pos parser = (Lexer.source parser.lexer, Lexer.end_offset parser.lexer)

    let sc_call parser f =
      let (sc_state, result) = f parser.sc_state in
      {parser with sc_state}, result

    let sc_state parser =
      parser.sc_state


let make env lexer errors context sc_state =
  { lexer
  ; errors
  ; context
  ; precedence = 0
  ; env
  ; sc_state
  ; prefix_unary_expression_stack = []
  ; allow_as_expressions = true
  }

let errors parser =
  parser.errors @ (Lexer.errors parser.lexer)

let env parser =
  parser.env

let allow_as_expressions parser =
  parser.allow_as_expressions

let with_as_expressions parser ~enabled f =
  let old_enabled = allow_as_expressions parser in
  let parser =
    if old_enabled <> enabled
    then { parser with allow_as_expressions = enabled }
    else parser in
  let parser, r = f parser in
  let parser =
    if old_enabled <> allow_as_expressions parser
    then { parser with allow_as_expressions = old_enabled }
    else parser in
  parser, r

let with_errors parser errors =
  { parser with errors }

let with_lexer parser lexer =
  { parser with lexer }

let lexer parser =
  parser.lexer

let context parser =
  parser.context

let with_context parser context =
  { parser with context }

let skipped_tokens parser =
  Context.skipped_tokens parser.context

let with_skipped_tokens parser skipped_tokens =
  let new_context = Context.with_skipped_tokens
    parser.context skipped_tokens in
  with_context parser new_context

let clear_skipped_tokens parser =
  with_skipped_tokens parser []

(** Wrapper functions for interfacing with parser context **)

let expect parser token_kind_list =
  let new_context = Context.expect parser.context token_kind_list in
  with_context parser new_context

let expects parser token_kind =
  Context.expects parser.context token_kind

let expects_here parser token_kind =
  Context.expects_here parser.context token_kind


(** Precedence functions **)

let with_precedence parser precedence =
  { parser with precedence }

let with_numeric_precedence parser new_precedence parse_function =
  let old_precedence = parser.precedence in
  let parser = with_precedence parser new_precedence in
  let (parser, result) = parse_function parser in
  let parser = with_precedence parser old_precedence in
  (parser, result)

let with_operator_precedence parser operator parse_function =
  let new_precedence = Full_fidelity_operator.precedence parser.env operator in
  with_numeric_precedence parser new_precedence parse_function

let with_reset_precedence parser parse_function =
  with_numeric_precedence parser 0 parse_function

let next_xhp_element_token ?(no_trailing=false) parser =
  let (lexer, token, text) =
    Lexer.next_xhp_element_token ~no_trailing parser.lexer in
  let parser = { parser with lexer } in
  (parser, token, text)

let next_xhp_body_token parser =
  let (lexer, token) = Lexer.next_xhp_body_token parser.lexer in
  let parser = { parser with lexer } in
  (parser, token)

  include SmartConstructors.ParserWrapper(struct
    type parser_type = t
    module SCI = SC
    let call = sc_call
  end)
end (* WithSmartConstructors *)
end (* WithLexer *)
end (* WithSyntax *)
