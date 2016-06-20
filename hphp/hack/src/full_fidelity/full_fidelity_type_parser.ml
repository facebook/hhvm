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
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Lexer = Full_fidelity_lexer

open TokenKind
open Syntax

type t = {
  lexer : Lexer.t;
  errors : SyntaxError.t list
}

let make lexer errors =
  { lexer; errors }

let errors parser =
  parser.errors

let lexer parser =
  parser.lexer

let with_error parser message =
  (* TODO: Should be able to express errors on whole syntax node. *)
  (* TODO: Is this even right? Won't this put the error on the trivia? *)
  let start_offset = Lexer.start_offset parser.lexer in
  let end_offset = Lexer.end_offset parser.lexer in
  let error = SyntaxError.make start_offset end_offset message in
  { parser with errors = error :: parser.errors }

let next_token parser =
  let (lexer, token) = Lexer.next_token_in_type parser.lexer in
  let parser = { parser with lexer } in
  (parser, token)

let next_token_as_name parser =
  (* TODO: This isn't right.  Pass flags to the lexer. *)
  let (lexer, token) = Lexer.next_token_as_name parser.lexer in
  let parser = { parser with lexer } in
  (parser, token)

let skip_token parser =
  let (lexer, _) = Lexer.next_token_in_type parser.lexer in
  let parser = { parser with lexer } in
  parser

let peek_token parser =
  let (_, token) = Lexer.next_token_in_type parser.lexer in
  token

(* TODO: What about something like for::for? Is that a legal
  type constant?  *)

let rec parse_type_specifier parser =
  let (parser1, token) = next_token parser in
  match Token.kind token with
  | Bool
  | Int
  | Float
  | Num
  | String
  | Arraykey
  | Void
  | Resource
  | Mixed -> (parser1, make_simple_type_specifier (make_token token))
  | This -> parse_simple_type_or_type_constant parser
  | Name -> parse_simple_type_or_type_constant_or_generic parser
  | Self -> parse_remaining_type_constant parser1 (make_token token)
  | QualifiedName -> parse_possible_generic_specifier parser
  | Array -> parse_array_type_specifier parser
  | LeftParen -> parse_tuple_or_closure_type_specifier parser
  | Shape -> parse_shape_specifier parser
  | Question -> parse_nullable_type_specifier parser
  | Classname -> parse_classname_type_specifier parser
  | _ ->
    let parser = with_error parser1 SyntaxError.error1007 in
    (parser, make_error [(make_token token)])

(* SPEC
  type-constant-type-name:
    name  ::  name
    self  ::  name
    this  ::  name
    type-constant-type-name  ::  name
*)

and parse_remaining_type_constant parser left =
  let (parser, separator) = next_token parser in
  let (parser1, right) = next_token_as_name parser in
  if (Token.kind right) = Name then
    begin
      let syntax =
        make_type_constant left (make_token separator) (make_token right) in
      let token = peek_token parser in
      if (Token.kind token) = ColonColon then
        parse_remaining_type_constant parser1 syntax
      else
        (parser1, syntax)
    end
  else
    (* ERROR RECOVERY: Assume that the thing following the ::
       that is not a name belongs to the next thing to be
       parsed; treat the name as missing. *)
    let parser = with_error parser1 SyntaxError.error1004 in
    let syntax = make_type_constant
      left (make_token separator) (make_missing()) in
    (parser, syntax)

and parse_simple_type_or_type_constant parser =
  let (parser, name) = next_token parser in
  let token = peek_token parser in
  match Token.kind token with
  | ColonColon -> parse_remaining_type_constant parser (make_token name)
  | _ -> (parser, make_simple_type_specifier (make_token name))

and parse_simple_type_or_type_constant_or_generic parser =
  let parser0 = skip_token parser in
  let token = peek_token parser0 in
  match Token.kind token with
  | LessThan -> parse_possible_generic_specifier parser
  | _ -> parse_simple_type_or_type_constant parser

(* SPEC
  class-interface-trait-specifier:
    qualified-name generic-type-argument-listopt
*)

and parse_possible_generic_specifier parser =
  let (parser, name) = next_token parser in
  let (parser, arguments) = parse_generic_type_argument_list_opt parser in
  if (kind arguments) = SyntaxKind.Missing then
    (parser, make_simple_type_specifier (make_token name))
  else
    (parser, make_generic_type_specifier (make_token name) arguments)

and parse_generic_type_argument_list_opt parser =
  let token = peek_token parser in
  if (Token.kind token) = LessThan then
    parse_generic_type_argument_list parser
  else
    (parser, make_missing())

and parse_type_list parser close_kind =
  let rec aux parser acc =
    let (parser1, token) = next_token parser in
    let kind = Token.kind token in
    if kind = close_kind || kind = EndOfFile then
      (* ERROR RECOVERY: If we're here and we got a close brace then
         the list is empty; we expect at least one type. If we're here
         at the end of the file, then we were expecting one more type. *)
      let parser = with_error parser SyntaxError.error1007 in
      (parser, ((make_missing()) :: acc))
    else if kind = Comma then

      (* ERROR RECOVERY: We're expecting a type but we got a comma.
         Assume the type was missing, eat the comma, and move on.
         TODO: This could be poor recovery. For example:

              function bar (Foo< , int blah)

        Plainly the type is missing, but the comma is not associated with
        the type, it's associated with the formal parameter list.  *)

      let parser = with_error parser1 SyntaxError.error1007 in
      aux parser ((make_token token) :: (make_missing()) :: acc)
    else
      let (parser, ty) = parse_type_specifier parser in
      let (parser1, token) = next_token parser in
      let kind = Token.kind token in
      if kind = close_kind then
        (parser, (ty :: acc))
      else if kind = Comma then
        aux parser1 ((make_token token) :: ty :: acc)
      else
        (* ERROR RECOVERY: We were expecting a close brace or comma, but
           got neither. Bail out. Caller will give an error. *)
        (parser, (ty :: acc)) in
  let (parser, types) = aux parser [] in
  (parser, make_list (List.rev types))

and parse_generic_type_argument_list parser =
  let (parser, open_angle) = next_token parser in
  let open_angle = make_token open_angle in
  let (parser, args) = parse_type_list parser GreaterThan in
  let (parser1, close_angle) = next_token parser in
  if (Token.kind close_angle) = GreaterThan then
    let result = make_type_arguments open_angle args (make_token close_angle) in
    (parser1, result)
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing > or ,.  Assume that it is the > that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1014 in
    let result = make_type_arguments open_angle args (make_missing()) in
    (parser, result)

and parse_array_type_specifier parser =
  let (parser, token) = next_token parser in
    (* TODO *)
    (parser, make_error [make_token token])

and parse_tuple_or_closure_type_specifier parser =
  let (parser1, _) = next_token parser in
  let token = peek_token parser1 in
  if (Token.kind token) = Function then
    parse_closure_type_specifier parser
  else
    parse_tuple_type_specifier parser

and parse_closure_type_specifier parser =
  let (parser, token) = next_token parser in
    (* TODO *)
    (parser, make_error [make_token token])

and parse_tuple_type_specifier parser =
  let (parser, left_paren) = next_token parser in
  let left_paren = make_token left_paren in
  let (parser, args) = parse_type_list parser RightParen in
  let (parser1, right_paren) = next_token parser in
  if (Token.kind right_paren) = RightParen then
    let result = make_tuple_type_specifier left_paren args
      (make_token right_paren) in
    (parser1, result)
  else
    (* ERROR RECOVERY: Don't eat the token that is in the place of the
       missing ) or ,.  Assume that it is the ) that is missing and
       try to parse whatever is coming after the type.  *)
    let parser = with_error parser SyntaxError.error1022 in
    let result = make_tuple_type_specifier left_paren args (make_missing()) in
    (parser, result)

and parse_nullable_type_specifier parser =
  let (parser, token) = next_token parser in
    (* TODO *)
    (parser, make_error [make_token token])

and parse_classname_type_specifier parser =
  let (parser, token) = next_token parser in
    (* TODO *)
    (parser, make_error [make_token token])

and parse_shape_specifier parser =
  let (parser, token) = next_token parser in
    (* TODO *)
    (parser, make_error [make_token token])
