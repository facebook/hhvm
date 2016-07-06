(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module Token = Full_fidelity_minimal_token
module Syntax = Full_fidelity_minimal_syntax
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SyntaxError = Full_fidelity_syntax_error

module type ParserType = sig
  module Lexer : Full_fidelity_lexer_sig.Lexer_S
  type t
  val errors : t -> SyntaxError.t list
  val with_errors : t -> SyntaxError.t list -> t
  val lexer : t -> Lexer.t
  val with_lexer : t -> Lexer.t -> t
end

module WithParser(Parser : ParserType) = struct

  let next_token parser =
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let peek_token parser =
    let lexer = Parser.lexer parser in
    let (_, token) = Parser.Lexer.next_token lexer in
    token

  let skip_token parser =
    let (parser, _) = next_token parser in
    parser

  let with_error parser message =
    (* TODO: Should be able to express errors on whole syntax node. *)
    (* TODO: Is this even right? Won't this put the error on the trivia? *)
    let lexer = Parser.lexer parser in
    let start_offset = Parser.Lexer.start_offset lexer in
    let end_offset = Parser.Lexer.end_offset lexer in
    let error = SyntaxError.make start_offset end_offset message in
    let errors = Parser.errors parser in
    Parser.with_errors parser (error :: errors)

  let expect_token parser kind error =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, Syntax.make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (with_error parser error, (Syntax.make_missing()))

  let optional_token parser kind =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, Syntax.make_token token)
    else
      (parser, Syntax.make_missing())

  let assert_token parser kind =
    let (parser, token) = next_token parser in
    assert ((Token.kind token) = kind);
    (parser, Syntax.make_token token)

  let next_token_as_name parser =
    (* TODO: This isn't right.  Pass flags to the lexer. *)
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token_as_name lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)


  (* This parses a comma-separated list of items that must contain at least
     one item.  The list is terminated by a close_kind token. The item is
     parsed by the given function. *)
  let parse_comma_list parser close_kind error parse_item =
    let rec aux parser acc =
      let (parser1, token) = next_token parser in
      let kind = Token.kind token in
      if kind = close_kind || kind = TokenKind.EndOfFile then
        (* ERROR RECOVERY: If we're here and we got a close token then
           the list is empty; we expect at least one type. If we're here
           at the end of the file, then we were expecting one more type. *)
        let parser = with_error parser error in
        (parser, ((Syntax.make_missing()) :: acc))
      else if kind = TokenKind.Comma then

        (* ERROR RECOVERY: We're expecting a type but we got a comma.
           Assume the type was missing, eat the comma, and move on.
           TODO: This could be poor recovery. For example:

                function bar (Foo< , int blah)

          Plainly the type is missing, but the comma is not associated with
          the type, it's associated with the formal parameter list.  *)

        let parser = with_error parser1 error in
        let missing = Syntax.make_missing() in
        let token = Syntax.make_token token in
        let list_item = Syntax.make_list_item missing token in
        aux parser (list_item :: acc)
      else
        let (parser, item) = parse_item parser in
        let (parser1, token) = next_token parser in
        let kind = Token.kind token in
        if kind = close_kind then
          (parser, (item :: acc))
        else if kind = TokenKind.Comma then
          let token = Syntax.make_token token in
          let list_item = Syntax.make_list_item item token in
          aux parser1 (list_item :: acc)
        else
          (* ERROR RECOVERY: We were expecting a close token or comma, but
             got neither. Bail out. Caller will give an error. *)
          (parser, (item :: acc)) in
    let (parser, types) = aux parser [] in
    (parser, Syntax.make_list (List.rev types))

end
