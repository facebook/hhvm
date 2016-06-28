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
module Operator = Full_fidelity_operator
module PrecedenceParser = Full_fidelity_precedence_parser

open TokenKind
open Syntax

module WithStatementAndDeclParser
  (StatementParser : Full_fidelity_statement_parser_type.StatementParserType)
  (DeclParser : Full_fidelity_declaration_parser_type.DeclarationParserType) :
  Full_fidelity_expression_parser_type.ExpressionParserType = struct

  include PrecedenceParser
  include Full_fidelity_parser_helpers.WithParser(PrecedenceParser)

  let rec parse_expression parser =
    let (parser, term) = parse_term parser in
    parse_remaining_expression parser term

  (* try to parse an expression. If parser cannot make progress, return None *)
  and parse_expression_optional parser =
    let module Lexer = PrecedenceParser.Lexer in
    let offset = Lexer.start_offset (lexer parser) in
    let (parser, expr) = parse_expression parser in
    let offset1 = Lexer.start_offset (lexer parser) in
    if offset1 = offset then None else Some (parser, expr)

  and parse_term parser =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | DecimalLiteral
    | OctalLiteral
    | HexadecimalLiteral
    | BinaryLiteral
    | FloatingLiteral
    | SingleQuotedStringLiteral
    | HeredocStringLiteral (*TODO: Special? *)
    | NowdocStringLiteral (* TODO: Special? *)
    | BooleanLiteral -> (parser1, make_literal_expression (make_token token))
    | NullLiteral ->
      (* TODO: Something special about null *)
      (parser1, make_literal_expression (make_token token))

    | DoubleQuotedStringLiteral
      (* TODO: Parse interior *)
      -> (parser1, make_literal_expression (make_token token))

    | Variable ->
        (parser1, make_variable_expression (make_token token))

    | Name
    | QualifiedName ->
        (parser1, make_qualified_name_expression (make_token token))

    | Exclamation
    | PlusPlus
    | MinusMinus
    | Tilde
    | Minus
    | Plus
    | Ampersand
    | Await
    | Yield
    | Clone
    | At ->
      parse_prefix_unary_expression parser

    | LeftParen ->
      parse_parenthesized_or_lambda_expression parser

    | LessThan -> (* TODO: XHP *)
      (parser1, make_token token)
    | Class -> (* TODO When is this legal? *)
      (parser1, make_token token)

    | List  -> parse_list_expression parser
    | New -> parse_object_creation_expression parser
    | Shape (* TODO: Parse shape *)
    | Async (* TODO: Parse lambda *)
    | Function  (* TODO: Parse local function *)


    | LeftBracket (* TODO: ? *)
    | Dollar (* TODO: ? *)
    | DollarDollar (* TODO: ? *)


    (* TODO: Array *)
    (* TODO: Collections *)
    (* TODO: List *)
    (* TODO: imports *)
    (* TODO: non-lowercased true false null array *)
    (* TODO: Unsafe *)
    (* TODO: What keywords are legal as names? *)

    | Final | Abstract | Interface | Trait ->
      (* TODO: Error *)
     (parser1, make_token token)
    | EndOfFile
    | _ ->
      (* TODO: Error, expected expression *)
      (parser1, make_token token)

  and parse_remaining_expression parser term =
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    (* Binary operators *)
    | Plus
    | Minus
    | Star
    | Slash
    | StarStar
    | Equal
    | BarEqual
    | PlusEqual
    | StarEqual
    | StarStarEqual
    | SlashEqual
    | DotEqual
    | MinusEqual
    | PercentEqual
    | CaratEqual
    | AmpersandEqual
    | LessThanLessThanEqual
    | GreaterThanGreaterThanEqual
    | EqualEqualEqual
    | GreaterThan
    | Percent
    | Dot
    | EqualEqual
    | AmpersandAmpersand
    | BarBar
    | ExclamationEqual
    | LessThan
    | ExclamationEqualEqual
    | LessThanEqual
    | GreaterThanEqual
    | Ampersand
    | Bar
    | LessThanLessThan
    | GreaterThanGreaterThan
    | Carat
    | BarGreaterThan
    | MinusGreaterThan
    | QuestionMinusGreaterThan
    | ColonColon
    | QuestionQuestion
    | Instanceof ->
    (* TODO: "and" "or" "xor" *)
      parse_remaining_binary_operator parser term

    | EqualEqualGreaterThan ->
      (* TODO parse lambda *)
      (parser1, make_token token)
    | PlusPlus
    | MinusMinus -> parse_postfix_unary parser term
    | LeftParen -> parse_function_call parser term
    | LeftBracket
    | LeftBrace -> (* TODO indexers *) (* TODO: Produce an error for brace *)
      (parser1, make_token token)
    | Question -> parse_conditional_expression parser1 term (make_token token)
    | _ -> (parser, term)

  and parse_designator parser =
    (* SPEC
      class-type-designator:
        static
        qualified-name
        variable-name
    *)

    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Name
    | QualifiedName
    | Variable
    | Static -> parser1, (make_token token)
    | LeftParen ->
      (* ERROR RECOVERY: We have "new (", so the type is almost certainly
         missing. Mark the type as missing, don't eat the token, and we'll
         try to parse the parameter list. *)
      (with_error parser SyntaxError.error1027, (make_missing()))
    | _ ->
      (* ERROR RECOVERY: We have "new " followed by something not recognizable
         as a type name. Take the token as the name and hope that what follows
         is the parameter list. *)
      (with_error parser1 SyntaxError.error1027, (make_token token))

  and parse_expression_list parser =
    (* SPEC
      argument-expression-list:
        expression
        argument-expression-list  ,  expression
    *)

    let rec aux parser exprs =
      let (parser, expr) = with_reset_precedence parser parse_expression in
      let exprs = expr :: exprs in
      let (parser1, token) = next_token parser in
      match (Token.kind token) with
      | Comma ->
        aux parser1 ((make_token token) :: exprs )
      | RightParen ->
        (parser, exprs)
      | EndOfFile
      | _ ->
        (* ERROR RECOVERY TODO: How to recover? *)
        let parser = with_error parser SyntaxError.error1009 in
        (parser, exprs) in
    let (parser, exprs) = aux parser [] in
    (parser, make_list (List.rev exprs))

  and parse_expression_list_opt parser =
    let token = peek_token parser in
    if (Token.kind token) = RightParen then (parser, make_missing())
    else parse_expression_list parser

  and parse_object_creation_expression parser =
    (* SPEC
      object-creation-expression:
        new  class-type-designator  (  argument-expression-listopt  )
    *)
    let (parser, new_token) = next_token parser in
    let (parser, designator) = parse_designator parser in
    let (parser, left_paren) =
     expect_token parser LeftParen SyntaxError.error1019 in
    let (parser, args) = parse_expression_list_opt parser in
    let (parser, right_paren) =
      expect_token parser RightParen SyntaxError.error1011 in
    let result = make_object_creation_expression (make_token new_token)
      designator left_paren args right_paren in
    (parser, result)

  and parse_function_call parser receiver =
    (* SPEC
      function-call-expression:
        postfix-expression  (  argument-expression-listopt  )
    *)
    let (parser, left_paren) = next_token parser in
    let (parser, args) = parse_expression_list_opt parser in
    let (parser, right_paren) =
      expect_token parser RightParen SyntaxError.error1011 in
    let result = make_function_call_expression receiver (make_token left_paren)
      args right_paren in
    (parser, result)

  and parse_parenthesized_or_lambda_expression parser =
    (*TODO: Does not yet deal with lambdas *)
    let (parser, left_paren) = next_token parser in
    let (parser, expression) = with_reset_precedence parser parse_expression in
    let (parser, right_paren) =
      expect_token parser RightParen SyntaxError.error1011 in
    let syntax =
      make_parenthesized_expression
        (make_token left_paren) expression right_paren in
    (parser, syntax)

  and parse_postfix_unary parser term =
    let (parser, token) = next_token parser in
    let term = make_postfix_unary_operator (make_token token) term in
    parse_remaining_expression parser term

  and parse_prefix_unary_expression parser =
    (* TODO: Operand to ++ and -- must be an lvalue. *)
    let (parser1, token) = next_token parser in
    let operator = Operator.prefix_unary_from_token (Token.kind token) in
    let precedence = Operator.precedence operator in
    let parser2 = with_precedence parser1 precedence in
    let (parser3, expression) = parse_expression parser2 in
    let syntax = make_prefix_unary_operator (make_token token) expression in
    let parser4 = with_precedence parser3 parser.precedence in
    (parser4, syntax)

  and parse_remaining_binary_operator parser left_term =
    (* We have a left term. If we get here then we know that
     * we have a binary operator to its right.
     *
     * Here's how this works.  Suppose we have something like
     *
     *     A x B y C
     *
     * where A, B and C are terms, and x and y are operators.
     * We must determine whether this parses as
     *
     *     (A x B) y C
     *
     * or
     *
     *     A x (B y C)
     *
     * We have the former if either x is higher precedence than y,
     * or x and y are the same precedence and x is left associative.
     * Otherwise, if x is lower precedence than y, or x is right
     * associative, then we have the latter.
     *
     * How are we going to figure this out?
     *
     * We have the term A in hand; the precedence is zero.
     * We see that x follows A.
     * We obtain the precedence of x. It is greater than zero,
     * so we obtain B, and then we call a helper method that
     * collects together everything to the right of B that is
     * of higher precedence than x. (Or equal, and right-associative.)
     *
     * So, if x is of lower precedence than y (or equal and right-assoc)
     * then the helper will construct (B y C) as the right term, and then
     * we'll make A x (B y C), and we're done.  Otherwise, the helper
     * will simply return B, we'll construct (A x B) and recurse with that
     * as the left term.
     *)

      let (parser1, token) = next_token parser in
      let operator = Operator.trailing_from_token (Token.kind token) in
      let precedence = Operator.precedence operator in
      if precedence < parser.precedence then
        (parser, left_term)
      else
        let (parser2, right_term) = parse_term parser1 in
        let (parser2, right_term) = parse_remaining_binary_operator_helper
          parser2 right_term precedence in
        let term =
          make_binary_operator left_term (make_token token) right_term in
        parse_remaining_expression parser2 term

  and parse_remaining_binary_operator_helper
      parser right_term left_precedence =
    (* This gathers up terms to the right of an operator that are
       operands of operators of higher precedence than the
       operator to the left. For instance, if we have
       A + B * C / D + E and we just parsed A +, then we want to
       gather up B * C / D into the right side of the +.
       In this case "right term" would be B and "left precedence"
       would be the precedence of +.
       See comments above for more details. *)
    let token = peek_token parser in
    if Operator.is_trailing_operator_token (Token.kind token) then
      let right_operator = Operator.trailing_from_token (Token.kind token) in
      let right_precedence = Operator.precedence right_operator in
      let associativity = Operator.associativity right_operator in
      if right_precedence > left_precedence ||
        (associativity = Operator.RightAssociative &&
          right_precedence = left_precedence ) then
        let parser1 = with_precedence parser right_precedence in
        let (parser2, right_term) =
          parse_remaining_expression parser1 right_term in
        let parser3 = with_precedence parser2 parser.precedence in
        parse_remaining_binary_operator_helper
          parser3 right_term left_precedence
      else
        (parser, right_term)
    else
      (parser, right_term)

  and parse_conditional_expression parser test question =
    (* POSSIBLE SPEC PROBLEM
       We allow any expression, including assignment expressions, to be in
       the consequence and alternative of a conditional expression, even
       though assignment is lower precedence than ?:.  This is legal:
       $a ? $b = $c : $d = $e
       Interestingly, this is illegal in C and Java, which require parens,
       but legal in C#.
    *)
    let token = peek_token parser in
    (* e1 ?: e2 -- where there is no consequence -- is legal *)
    let (parser, consequence) = if (Token.kind token) = Colon then
      (parser, make_missing())
    else
      with_reset_precedence parser parse_expression in
    let (parser, colon) =
      expect_token parser Colon SyntaxError.error1020 in
    let (parser, alternative) = with_reset_precedence parser parse_expression in
    let result = make_conditional_expression
      test question consequence colon alternative in
    (parser, result)

  and parse_list_expression parser =
    let parser, keyword_token = next_token parser in
    let parser, left_paren =
      expect_token parser LeftParen SyntaxError.error1019 in
    let parser, members = parse_list_expression_list parser in
    let parser, right_paren =
      expect_token parser RightParen SyntaxError.error1011 in
    let syntax = make_listlike_expression
      (make_token keyword_token) left_paren members right_paren in
    (parser, syntax)
  and parse_list_expression_list parser =
    let rec aux parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | Comma ->
        aux parser1 ((make_token token) :: acc)
      | RightParen
      | Equal -> (* list intrinsic appears only on LHS of equal sign *)
        (parser, make_list (List.rev acc))
      | _ -> begin
        (* ERROR RECOVERY if parser makes no progress, make an error *)
        match parse_expression_optional parser with
        | None ->
          let parser = with_error parser SyntaxError.error1015 in
          (parser, make_missing ())
        | Some (parser, expr) -> aux parser (expr :: acc)
        end
    in
    aux parser []
end
