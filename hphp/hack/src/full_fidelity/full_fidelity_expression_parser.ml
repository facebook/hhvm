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
module TypeParser = Full_fidelity_type_parser

open TokenKind
open Syntax

module WithStatementAndDeclParser
  (StatementParser : Full_fidelity_statement_parser_type.StatementParserType)
  (DeclParser : Full_fidelity_declaration_parser_type.DeclarationParserType) :
  Full_fidelity_expression_parser_type.ExpressionParserType = struct

  include PrecedenceParser
  include Full_fidelity_parser_helpers.WithParser(PrecedenceParser)

  let parse_return_type parser =
    let type_parser = TypeParser.make parser.lexer parser.errors in
    let (type_parser, node) = TypeParser.parse_return_type type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_parameter_list_opt parser =
    let decl_parser = DeclParser.make parser.lexer parser.errors in
    let (decl_parser, right, params, left ) =
      DeclParser.parse_parameter_list_opt decl_parser in
    let lexer = DeclParser.lexer decl_parser in
    let errors = DeclParser.errors decl_parser in
    let parser = { parser with lexer; errors } in
    (parser, right, params, left)

  let parse_compound_statement parser =
    let statement_parser = StatementParser.make parser.lexer parser.errors in
    let (statement_parser, node) =
      StatementParser.parse_compound_statement statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let rec parse_expression parser =
    let (parser, term) = parse_term parser in
    parse_remaining_expression parser term

  and parse_expression_with_reset_precedence parser =
    with_reset_precedence parser parse_expression

  (* try to parse an expression. If parser cannot make progress, return None *)
  and parse_expression_optional parser ~reset_prec =
    let module Lexer = PrecedenceParser.Lexer in
    let offset = Lexer.start_offset (lexer parser) in
    let (parser, expr) =
      if reset_prec then with_reset_precedence parser parse_expression
      else parse_expression parser in
    let offset1 = Lexer.start_offset (lexer parser) in
    if offset1 = offset then None else Some (parser, expr)

  and parses_without_error parser f =
    let old_errors = List.length (errors parser) in
    let (parser, result) = f parser in
    let new_errors = List.length(errors parser) in
    old_errors = new_errors

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

    | Variable -> parse_variable_or_lambda parser

    | Name
    | QualifiedName ->
        parse_name_or_collection_literal_expression parser1 token
    | Yield -> parse_yield_expression parser
    | Print -> parse_print_expression parser
    | Exclamation
    | PlusPlus
    | MinusMinus
    | Tilde
    | Minus
    | Plus
    | Ampersand
    | Await
    | Clone
    | At ->
      parse_prefix_unary_expression parser

    | LeftParen ->
      parse_cast_or_parenthesized_or_lambda_expression parser

    | LessThan ->
        parse_possible_xhp_expression parser

    | Class -> (* TODO When is this legal? *)
      (parser1, make_token token)

    | List  -> parse_list_expression parser
    | New -> parse_object_creation_expression parser
    | Array -> parse_array_intrinsic_expression parser
    | LeftBracket -> parse_array_creation_expression parser
    | Shape -> parse_shape_expression parser
    | Function -> parse_anon parser
    | DollarDollar ->
      (parser1, make_pipe_variable_expression (make_token token))
    | Async ->
      parse_anon_or_lambda_or_awaitable parser

    | Dollar (* TODO: ? *)

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

    | PlusPlus
    | MinusMinus -> parse_postfix_unary parser term
    | LeftParen -> parse_function_call parser term
    | LeftBracket
    | LeftBrace -> parse_subscript parser term
    | Question -> parse_conditional_expression parser1 term (make_token token)
    | _ -> (parser, term)

  and parse_subscript parser term =
    (* SPEC
      subscript-expression:
        postfix-expression  [  expression-opt  ]
        postfix-expression  {  expression-opt  }   [Deprecated form]
    *)
    (* TODO: Produce an error for brace case in a later pass *)
    let (parser, left) = next_token parser in
    let (parser1, right) = next_token parser in
    match (Token.kind left, Token.kind right) with
    | (LeftBracket, RightBracket)
    | (LeftBrace, RightBrace) ->
      let left = make_token left in
      let index = make_missing() in
      let right = make_token right in
      let result = make_subscript_expression term left index right in
      parse_remaining_expression parser1 result
    | _ ->
    begin
      let (parser, index) = with_reset_precedence parser parse_expression in
      let (parser, right) = match Token.kind left with
      | LeftBracket -> expect_right_bracket parser
      | _ -> expect_right_brace parser in
      let left = make_token left in
      let result = make_subscript_expression term left index right in
      parse_remaining_expression parser result
    end

  and parse_expression_list_opt parser =
    (* SPEC
      argument-expression-list:
        argument-expressions   ,-opt
      argument-expressions:
        expression
        ... expression
        argument-expressions  ,  expression
    *)
    (* This function parses the parens as well. *)
    let f parser =
      with_reset_precedence parser parse_decorated_expression_opt in
    parse_parenthesized_comma_list_opt_allow_trailing parser f

  and parse_decorated_expression_opt parser =
    match peek_token_kind parser with
    | DotDotDot ->
      let (parser, dots) = next_token parser in
      let (parser, expr) = parse_expression parser in
      let dots = make_token dots in
      parser, make_decorated_expression dots expr
    | _ -> parse_expression parser

  and parse_object_creation_expression parser =
    (* SPEC
      object-creation-expression:
        new  class-type-designator  (  argument-expression-listopt  )

      class-type-designator:
        static
        qualified-name
        variable-name
    *)
    let (parser, new_token) = next_token parser in
    let (parser1, token) = next_token parser in
    (* TODO: handle error reporting:
      not all types of expressions are allowed
      dynamic calls are not allowed in strict mode
    *)
    let (parser, designator, left, args, right) = match Token.kind token with
    | Static ->
        let (parser, designator) = parser1, (make_token token) in
        let (parser, left, args, right) = parse_expression_list_opt parser in
        (parser, designator, left, args, right)
    | _ ->
      let (parser, expr) = parse_expression parser in
      match syntax expr with
        | FunctionCallExpression fce -> (
            parser,
            fce.function_call_receiver,
            fce.function_call_lparen,
            fce.function_call_arguments,
            fce.function_call_rparen
          )
        | _ ->
          (parser, expr, make_missing(), make_missing(), make_missing())
    in

    let result = make_object_creation_expression (make_token new_token)
      designator left args right in
    (parser, result)

  and parse_function_call parser receiver =
    (* SPEC
      function-call-expression:
        postfix-expression  (  argument-expression-list-opt  )
    *)
    let (parser, left, args, right) = parse_expression_list_opt parser in
    let result = make_function_call_expression receiver left args right in
    parse_remaining_expression parser result

  and parse_variable_or_lambda parser =
    let (parser1, variable) = assert_token parser Variable in
    if peek_token_kind parser1 = EqualEqualGreaterThan then
      parse_lambda_expression parser
    else
      (parser1, make_variable_expression variable)

  and parse_yield_expression parser =
    (* SPEC:
      yield  array-element-initializer
    *)
    let (parser, token) = assert_token parser Yield in
    let (parser, operand) = parse_array_element_init parser in
    let result = make_yield_expression token operand in
    (parser, result)

  and parse_print_expression parser =
    (* SPEC:
      TODO: this is the php spec and the hhvm yac grammar,
      update the github spec

      print expr
    *)
    let (parser, token) = assert_token parser Print in
    let (parser, expr) = parse_expression parser in
    let syntax = make_print_expression token expr in
    (parser, syntax)

  and parse_cast_or_parenthesized_or_lambda_expression parser =
  (* We need to disambiguate between casts, lambdas and ordinary
    parenthesized expressions. *)
    if is_cast_expression parser then
      parse_cast_expression parser
    else if is_lambda_expression parser then
      parse_lambda_expression parser
    else
      parse_parenthesized_expression parser

  and is_cast_expression parser =
    (* We have a left paren in hand and wish to know whether we are parsing a
       cast, lambda or parenthesized expression. There are only eight possible
       cast prefixes so those are the easiest to detect. *)
    let (parser, left_paren) = assert_token parser LeftParen in
    let (parser, type_token) = next_token parser in
    match Token.kind type_token with
    | Array
    | Object
    | Unset
    | Double
    | Bool
    | Int
    | Float
    | String ->
      peek_token_kind parser = RightParen
    | _ -> false

  and parse_cast_expression parser =
    (* SPEC:
      cast-expression:
        (  cast-type  ) unary-expression
      cast-type: one of
        bool  int  float  string

      TODO: The specification needs to be updated; double, object, array
      and unset are also legal cast types.  Note also that the spec does not
      define "double", "unset" or "object" as keywords.
    *)
    (* We don't get here unless we have a legal cast, thanks to the
       previous call to is_cast_expression. *)
    let (parser, left) = assert_token parser LeftParen in
    let (parser, cast_type) = next_token parser in
    let cast_type = make_token cast_type in
    let (parser, right) = assert_token parser RightParen in
    let (parser, operand) = parse_expression parser in
    let result = make_cast_expression left cast_type right operand in
    (parser, result)

  and is_lambda_expression parser =
    (* We have a left paren in hand and we already know we're not in a cast.
       We need to know whether this is a parenthesized expression or the
       signature of a lambda.

       There are a number of difficulties. For example, we cannot simply
       check to see if a colon follows the expression:

       $a = $b ? ($x) : ($y)              ($x) is parenthesized expression
       $a = $b ? ($x) : int ==> 1 : ($y)  ($x) is lambda signature

       ERROR RECOVERY:

       What we'll do here is simply attempt to parse a lambda formal parameter
       list. If we manage to do so *without error*, and the thing which follows
       is ==>, then this is definitely a lambda. If those conditions are not
       met then we assume we have a parenthesized expression in hand.

       TODO: There could be situations where we have good evidence that a
       lambda is intended but these conditions are not met. Consider
       a more sophisticated recovery strategy.
    *)
    let sig_and_arrow parser =
      let (parser, signature) = parse_lambda_signature parser in
      expect_lambda_arrow parser in
    parses_without_error parser sig_and_arrow

  and parse_lambda_expression parser =
    (* SPEC
      lambda-expression:
        async-opt  lambda-function-signature  ==>  lambda-body
    *)
    let (parser, async) = optional_token parser Async in
    let (parser, signature) = parse_lambda_signature parser in
    let (parser, arrow) = expect_lambda_arrow parser in
    let (parser, body) = parse_lambda_body parser in
    let result = make_lambda_expression async signature arrow body in
    (parser, result)

  and parse_lambda_signature parser =
    (* SPEC:
      lambda-function-signature:
        variable-name
        (  anonymous-function-parameter-declaration-list-opt  ) /
          anonymous-function-return-opt
    *)
    let (parser1, token) = next_token parser in
    if Token.kind token = Variable then
      (parser1, make_token token)
    else
      let (parser, left, params, right) = parse_parameter_list_opt parser in
      let (parser, colon, return_type) = parse_optional_return parser in
      let result = make_lambda_signature left params right colon return_type in
      (parser, result)

  and parse_lambda_body parser =
    (* SPEC:
      lambda-body:
        expression
        compound-statement
    *)
    if peek_token_kind parser = LeftBrace then
      parse_compound_statement parser
    else
      with_reset_precedence parser parse_expression

  and parse_parenthesized_expression parser =
    let (parser, left_paren) = assert_token parser LeftParen in
    let (parser, expression) = with_reset_precedence parser parse_expression in
    let (parser, right_paren) = expect_right_paren parser in
    let syntax =
      make_parenthesized_expression left_paren expression right_paren in
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
    let (parser, colon) = expect_colon parser in
    let (parser, alternative) = with_reset_precedence parser parse_expression in
    let result = make_conditional_expression
      test question consequence colon alternative in
    (parser, result)

  and parse_name_or_collection_literal_expression parser token =
    let name = make_token token in
    match peek_token_kind parser with
    | LeftBrace ->
      parse_collection_literal_expression parser name
    | _ ->
      (parser, make_qualified_name_expression name)

  and parse_collection_literal_expression parser name =
    let parser, left_brace = assert_token parser LeftBrace in
    let parser, initialization_list = parse_optional_initialization parser in
    let parser, right_brace = expect_right_brace parser in
    (* Validating the name is a collection type happens in a later phase *)
    let syntax = make_collection_literal_expression
      name left_brace initialization_list right_brace
    in
    (parser, syntax)

  and parse_optional_initialization parser =
    if peek_token_kind parser = RightBrace then
      parser, make_missing ()
    else
    let parser1, expr = parse_expression parser in
    match peek_token_kind parser1 with
    | EqualGreaterThan ->
      parse_comma_list_allow_trailing
        parser RightBrace SyntaxError.error1015 parse_keyed_element_initializer
    | _ ->
      parse_comma_list_allow_trailing
        parser RightBrace SyntaxError.error1015
        parse_expression_with_reset_precedence

  and parse_keyed_element_initializer parser =
    let parser, expr1 = parse_expression_with_reset_precedence parser in
    let parser, arrow = expect_arrow parser in
    let parser, expr2 = parse_expression_with_reset_precedence parser in
    let syntax = make_element_initializer expr1 arrow expr2 in
    (parser, syntax)

  and parse_list_expression parser =
    let parser, keyword_token = next_token parser in
    let parser, left_paren = expect_left_paren parser in
    let parser, members =
      with_reset_precedence parser parse_list_expression_list in
    let parser, right_paren = expect_right_paren parser in
    let syntax = make_listlike_expression
      (make_token keyword_token) left_paren members right_paren in
    (parser, syntax)

  and parse_list_expression_list parser =
    let rec aux parser acc =
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | Comma ->
        let missing_expr = make_missing () in
        let item = make_list_item missing_expr (make_token token) in
        aux parser1 (item :: acc)
      | RightParen
      | Equal -> (* list intrinsic appears only on LHS of equal sign *)
        (parser, make_list (List.rev acc))
      | _ -> begin
        match parse_expression_optional parser ~reset_prec:false with
        | None ->
          (* ERROR RECOVERY if parser makes no progress, make an error *)
          let parser = with_error parser SyntaxError.error1015 in
          (parser, make_missing () :: acc |> List.rev |> make_list)
        | Some (parser, expr) ->
          let (parser1, token) = next_token parser in
          let parser, trailing = begin
            match Token.kind token with
            | Comma -> parser1, make_list_item expr (make_token token)
            | RightParen -> parser, make_list_item expr (make_missing ())
            | _ ->
              (* ERROR RECOVERY: require a comma or right paren *)
              let parser = with_error parser SyntaxError.error1009 in
              parser, make_list_item expr (make_missing ())
          end in
          aux parser (trailing :: acc)
    end in
    aux parser []
  (* grammar:
   * array_intrinsic := array ( array-initializer-opt )
   *)
  and parse_array_intrinsic_expression parser =
    let (parser, array_keyword) = assert_token parser Array in
    let (parser, left_paren) = expect_left_paren parser in
    let (parser, members) = parse_array_initializer_opt parser true in
    let (parser, right_paren) = expect_right_paren parser in
    let syntax = make_array_intrinsic_expression array_keyword left_paren
      members right_paren in
    (parser, syntax)

  (* array_creation_expression := [ array-initializer-opt ] *)
  and parse_array_creation_expression parser =
    let (parser, left_bracket) = expect_left_bracket parser in
    let (parser, members) = parse_array_initializer_opt parser false in
    let (parser, right_bracket) = expect_right_bracket parser in
    let syntax = make_array_creation_expression left_bracket
      members right_bracket in
    (parser, syntax)
  (* array-initializer := array-initializer-list ,-opt *)
  and parse_array_initializer_opt parser is_intrinsic =
    let token = peek_token parser in
    match Token.kind token with
    | RightParen when is_intrinsic -> (parser, make_missing ())
    | RightBracket when not is_intrinsic -> (parser, make_missing ())
    | _ ->  parse_array_init_list parser is_intrinsic
  (* array-initializer-list :=
   * array-element-initializer
   * array-element-initializer , array-initializer-list *)
  and parse_array_init_list parser is_intrinsic =
    let rec aux parser acc =
      let parser, element = parse_array_element_init parser in
      let parser1, token = next_token parser in
      match Token.kind token with
      | Comma ->
        let item = make_list_item element (make_token token) in
        let acc = item :: acc in
        let next_token = peek_token parser1 in
        begin
          match Token.kind next_token with
          (* special case when comma finishes the array definition *)
          | RightParen when is_intrinsic ->
            (parser1, make_list (List.rev acc))
          | RightBracket when not is_intrinsic ->
            (parser1, make_list (List.rev acc))
          | _ -> aux parser1 acc
        end
      (* end of array definition *)
      | RightParen when is_intrinsic ->
        let item = make_list_item element (make_missing ()) in
        (parser, make_list (List.rev (item :: acc)))
      | RightBracket when not is_intrinsic ->
        let item = make_list_item element (make_missing ()) in
        (parser, make_list (List.rev (item :: acc)))
      | _ -> (* ERROR RECOVERY *)
        let error = if is_intrinsic then SyntaxError.error1009
                    else SyntaxError.error1031 in
        let parser = with_error parser error in
        (* do not eat token here *)
        (parser, element :: acc |> List.rev |> make_list)
    in
    aux parser []

  (* array-element-initializer :=
   * expression
   * expression => expression
   *)
  and parse_array_element_init parser =
    let parser, expr1 =
      with_reset_precedence parser parse_expression in
    let parser1, token = next_token parser in
    match Token.kind token with
    | EqualGreaterThan ->
      let parser, expr2 = with_reset_precedence parser1 parse_expression in
      let arrow = make_token token in
      let result = make_element_initializer expr1 arrow expr2 in
      (parser, result)
    | _ -> (parser, expr1)

  and parse_field_initializer parser =
    (* SPEC
      field-initializer:
        single-quoted-string-literal  =>  expression
        integer-literal  =>  expression
        qualified-name  =>  expression
        *)
    let (parser1, token) = next_token parser in
    let (parser, name) = match Token.kind token with
    | SingleQuotedStringLiteral
    | DecimalLiteral
    | OctalLiteral
    | HexadecimalLiteral
    | BinaryLiteral
    | Name
    | QualifiedName -> (parser1, make_token token)
    | EqualGreaterThan ->
      (* ERROR RECOVERY: We're missing the name. *)
      (with_error parser SyntaxError.error1025, make_missing())
    | _ ->
      (* ERROR RECOVERY: We're missing the name and we have no arrow either.
         Just eat the token and hope we get an arrow next. *)
      (with_error parser1 SyntaxError.error1025, make_missing()) in
    let (parser, arrow) = expect_arrow parser in
    let (parser, value) = with_reset_precedence parser parse_expression in
    let result = make_field_initializer name arrow value in
    (parser, result)

  and parse_field_initializer_list_opt parser =
    (* SPEC
      field-initializer-list:
        field-initializer
        field-initializer-list    ,  field-initializer
    *)
    parse_comma_list_opt
      parser RightParen SyntaxError.error1025 parse_field_initializer

  and parse_shape_expression parser =
    (* SPEC
      shape-literal:
        shape  (  field-initializer-list-opt  )
    *)
    let (parser, shape) = assert_token parser Shape in
    let (parser, left_paren) = expect_left_paren parser in
    let (parser, fields) = parse_field_initializer_list_opt parser in
    let (parser, right_paren) = expect_right_paren parser in
    let result = make_shape_expression shape left_paren fields right_paren in
    (parser, result)

  and parse_use_variable parser =
    (* TODO: Is it better that this returns the variable as a *token*, or
    as an *expression* that consists of the token? We do the former. *)
    let (parser, ampersand) = optional_token parser Ampersand in
    let (parser, variable) = expect_variable parser in
    if is_missing ampersand then
      (parser, variable)
    else
      let result = make_prefix_unary_operator ampersand variable in
      (parser, result)

  and parse_variable_list parser =
    (* SPEC:
      use-variable-name-list:
        variable-name
        use-variable-name-list  ,  variable-name
    *)
    (* TODO: Strict mode requires that it be a list of variables; in
       non-strict mode we allow variables to be decorated with a leading
       & to indicate they are captured by reference. We need to give an
       error in a later pass for this. *)
    parse_comma_list_opt
      parser RightParen SyntaxError.error1025 parse_use_variable

  and parse_anon_or_lambda_or_awaitable parser =
    let (parser1, _) = assert_token parser Async in
    if peek_token_kind parser1 = Function then
      parse_anon parser
    else if peek_token_kind parser1 = LeftBrace then
      parse_async_block parser
    else
      parse_lambda_expression parser

  and parse_async_block parser =
    (*
     * grammar:
     *  awaitable-creation-expression :
     *    async compound-statement
     * TODO awaitable-creation-expression must not be used as the
     *      anonymous-function-body in a lambda-expression
     *)
    let parser, async = assert_token parser Async in
    let parser, stmt = parse_compound_statement parser in
    parser, make_awaitable_creation_expression async stmt

  and parse_anon_use_opt parser =
    (* SPEC:
      anonymous-function-use-clause:
        use  (  use-variable-name-list  )
    *)
    let (parser, use_token) = optional_token parser Use in
    if is_missing use_token then
      (parser, use_token)
    else
      let (parser, left_paren) = expect_left_paren parser in
      let (parser, variables) = parse_variable_list parser in
      let (parser, right_paren) = expect_right_paren parser in
      let result = make_anonymous_function_use_clause use_token
        left_paren variables right_paren in
      (parser, result)

  and parse_optional_return parser =
    (* Parse an optional "colon-folowed-by-return-type" *)
    let (parser, colon) = optional_token parser Colon in
    let (parser, return_type) =
      if is_missing colon then
        (parser, (make_missing()))
      else
        parse_return_type parser in
    (parser, colon, return_type)

  and parse_anon parser =
    (* SPEC
      anonymous-function-creation-expression:
        async-opt  function
        ( anonymous-function-parameter-list-opt  )
        anonymous-function-return-opt
        anonymous-function-use-clauseopt
        compound-statement
    *)
    (* An anonymous function's formal parameter list is the same as a named
       function's formal parameter list except that types are optional.
       The "..." syntax and trailing commas are supported. We'll simply
       parse an optional parameter list; it already takes care of making the
       type annotations optional. *)
    let (parser, async) = optional_token parser Async in
    let (parser, fn) = assert_token parser Function in
    let (parser, left_paren, params, right_paren) =
      parse_parameter_list_opt parser in
    let (parser, colon, return_type) = parse_optional_return parser in
    let (parser, use_clause) = parse_anon_use_opt parser in
    let (parser, body) = parse_compound_statement parser in
    let result = make_anonymous_function async fn left_paren params
      right_paren colon return_type use_clause body in
    (parser, result)

  and parse_braced_expression parser =
    let (parser, left_brace) = next_token parser in
    let precedence = parser.precedence in
    let parser = with_precedence parser 0 in
    let (parser, expression) = parse_expression parser in
    let (parser, right_brace) = expect_right_brace parser in
    let parser = with_precedence parser precedence in
    let node =
      make_braced_expression (make_token left_brace) expression right_brace in
    (parser, node)

  and parse_xhp_attribute parser name =
    let (parser1, token, _) = next_xhp_element_token parser in
    if (Token.kind token) != Equal then
      let node = make_xhp_attr name (make_missing()) (make_missing()) in
      let parser = with_error parser SyntaxError.error1016 in
      (* ERROR RECOVERY: The = is missing; assume that the name belongs
         to the attribute, but that the remainder is missing, and start
         looking for the next attribute. *)
      (parser, node)
    else
      let equal = make_token token in
      let (parser2, token, text) = next_xhp_element_token parser1 in
      match (Token.kind token) with
      | XHPStringLiteral ->
        let node = make_xhp_attr name equal (make_token token) in
        (parser2, node)
      | LeftBrace ->
        let (parser, expr) = parse_braced_expression parser1 in
        let node = make_xhp_attr name equal expr in
        (parser, node)
      | _ ->
        (* ERROR RECOVERY: The expression is missing; assume that the "name ="
           belongs to the attribute and start looking for the next attribute. *)
        let node = make_xhp_attr name equal (make_missing()) in
        let parser = with_error parser1 SyntaxError.error1017 in
        (parser, node)

  and parse_xhp_attributes parser =
    let rec aux parser acc =
      let (parser1, token, _) = next_xhp_element_token parser in
      if (Token.kind token) = XHPElementName then
        let (parser, attr) = parse_xhp_attribute parser1 (make_token token) in
        aux parser (attr :: acc)
      else
        (parser, acc) in
    let (parser, attrs) = aux parser [] in
    (parser, make_list (List.rev attrs))

  and parse_xhp_body parser =
    let rec aux acc parser =
      let (parser1, token) = next_xhp_body_token parser in
      match Token.kind token with
      | XHPComment
      | XHPBody -> aux ((make_token token) :: acc) parser1
      | LeftBrace ->
        let (parser, expr) = parse_braced_expression parser in
        aux (expr :: acc) parser
      | XHPElementName ->
        let (parser, expr) = parse_possible_xhp_expression parser in
        aux (expr :: acc) parser
      | _ -> (parser, acc) in
    let (parser, body_elements) = aux [] parser in
    (parser, make_list (List.rev body_elements))

  and parse_xhp_close parser _ =
    let (parser1, less_than_slash, _) = next_xhp_element_token parser in
    if (Token.kind less_than_slash) = LessThanSlash then
      let (parser2, name, name_text) = next_xhp_element_token parser1 in
      if (Token.kind name) = XHPElementName then
        (* TODO: Check that the given and name_text are the same. *)
        let (parser3, greater_than, _) = next_xhp_element_token parser2 in
        if (Token.kind greater_than) = GreaterThan then
          (parser3, make_xhp_close (make_token less_than_slash)
            (make_token name) (make_token greater_than))
        else
          (* ERROR RECOVERY: *)
          let parser = with_error parser2 SyntaxError.error1039 in
          (parser, make_xhp_close
            (make_token less_than_slash) (make_token name) (make_missing()))
      else
        (* ERROR RECOVERY: *)
        let parser = with_error parser1 SyntaxError.error1039 in
        (parser, make_xhp_close
          (make_token less_than_slash) (make_missing()) (make_missing()))
    else
      (* ERROR RECOVERY: We probably got a < without a following / or name.
         TODO: For now we'll just bail out. We could use a more
         sophisticated strategy here. *)
      let parser = with_error parser1 SyntaxError.error1026 in
      (parser, make_xhp_close
        (make_token less_than_slash) (make_missing()) (make_missing()))

  and parse_xhp_expression parser name name_text =
    let (parser, attrs) = parse_xhp_attributes parser in
    let (parser1, token, _) = next_xhp_element_token parser in
    match (Token.kind token) with
    | SlashGreaterThan ->
      let xhp_open = make_xhp_open name attrs (make_token token) in
      let xhp = make_xhp xhp_open (make_missing()) (make_missing()) in
      (parser1, xhp)
    | GreaterThan ->
      let xhp_open = make_xhp_open name attrs (make_token token) in
      let (parser, xhp_body) = parse_xhp_body parser1 in
      let (parser, xhp_close) = parse_xhp_close parser name_text in
      let xhp = make_xhp xhp_open xhp_body xhp_close in
      (parser, xhp)
    | _ ->
      (* ERROR RECOVERY: Assume the unexpected token belongs to whatever
         comes next. *)
      let xhp_open = make_xhp_open name attrs (make_missing()) in
      let xhp = make_xhp xhp_open (make_missing()) (make_missing()) in
      let parser = with_error parser SyntaxError.error1013 in
      (parser, xhp)

  and parse_possible_xhp_expression parser =
    (* We got a < token where an expression was expected. *)
    let (parser, token, text) = next_xhp_element_token parser in
    if (Token.kind token) = XHPElementName then
      parse_xhp_expression parser (make_token token) text
    else
      (* ERROR RECOVERY
      Hard to say what to do here. We are expecting an expression;
      we could simply produce an error for the < and call that the
      expression. Or we could assume the the left side of an inequality is
      missing, give a missing node for the left side, and parse the
      remainder as the right side. We'll go for the former for now. *)
      (with_error parser SyntaxError.error1015, (make_token token))
end
