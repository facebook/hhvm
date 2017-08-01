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
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Operator = Full_fidelity_operator
module PrecedenceParser = Full_fidelity_precedence_parser

open TokenKind
open Full_fidelity_minimal_syntax

module WithStatementAndDeclAndTypeParser
  (StatementParser : Full_fidelity_statement_parser_type.StatementParserType)
  (DeclParser : Full_fidelity_declaration_parser_type.DeclarationParserType)
  (TypeParser : Full_fidelity_type_parser_type.TypeParserType) :
  Full_fidelity_expression_parser_type.ExpressionParserType = struct

  include PrecedenceParser
  include Full_fidelity_parser_helpers.WithParser(PrecedenceParser)

  exception Cancel_attempt

  type binary_expression_prefix_kind =
    | Prefix_byref_assignment | Prefix_assignment | Prefix_none

  let parse_type_specifier parser =
    let type_parser = TypeParser.make parser.lexer
      parser.errors parser.context in
    let (type_parser, node) = TypeParser.parse_type_specifier type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_generic_type_arguments_opt parser =
    let type_parser = TypeParser.make parser.lexer
      parser.errors parser.context in
    let (type_parser, node) =
      TypeParser.parse_generic_type_argument_list_opt type_parser
    in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_return_type parser =
    let type_parser = TypeParser.make parser.lexer
      parser.errors parser.context in
    let (type_parser, node) = TypeParser.parse_return_type type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let parse_parameter_list_opt parser =
    let decl_parser = DeclParser.make parser.lexer parser.errors
      parser.context in
    let (decl_parser, right, params, left ) =
      DeclParser.parse_parameter_list_opt decl_parser in
    let lexer = DeclParser.lexer decl_parser in
    let errors = DeclParser.errors decl_parser in
    let parser = { parser with lexer; errors } in
    (parser, right, params, left)

  let parse_compound_statement parser =
    let statement_parser = StatementParser.make parser.lexer
      parser.errors parser.context in
    let (statement_parser, node) =
      StatementParser.parse_compound_statement statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

  let rec parse_expression parser =
    let (parser, term) = parse_term parser in
    if kind term = SyntaxKind.QualifiedNameExpression
    then parse_remaining_expression_or_specified_function_call parser term
    else parse_remaining_expression parser term

  and parse_expression_with_reset_precedence parser =
    with_reset_precedence parser parse_expression

  and parse_expression_with_operator_precedence parser operator =
    with_operator_precedence parser operator parse_expression

  (* try to parse an expression. If parser cannot make progress, return None *)
  (* TODO: Not currently used; can we delete this? *)
  and _parse_expression_optional parser ~reset_prec =
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

  and parse_as_name_or_error parser =
    (* TODO: Are there "reserved" keywords that absolutely cannot start
       an expression? If so, list them above and make them produce an
       error. *)
    let (parser1, token) = next_token_as_name parser in
    match (Token.kind token) with
    | Name -> parse_name_or_collection_literal_expression parser1 token
    | kind when PrecedenceParser.expects_here parser kind ->
      (* ERROR RECOVERY: If we're encountering a token that matches a kind in
       * the previous scope of the expected stack, don't eat it--just mark the
       * name missing and continue parsing, starting from the offending token. *)
      let parser = with_error parser SyntaxError.error1015 in
      (parser, make_missing())
    | _ ->
      (* ERROR RECOVERY: If we're encountering anything other than a Name
       * or the next expected kind, eat the offending token.
       * TODO: Increase the coverage of PrecedenceParser.expects_next, so that
       * we wind up eating fewer of the tokens that'll be needed by the outer
       * statement / declaration parsers. *)
      let parser = with_error parser1 SyntaxError.error1015 in
      (parser, make_token token)

  and parse_term parser =
    let (parser1, token) = next_xhp_class_name_or_other parser in
    match (Token.kind token) with
    | ExecutionString -> (* TODO: Make these an error in Hack *)
       (parser1, make_literal_expression (make_token token))
    | DecimalLiteral
    | OctalLiteral
    | HexadecimalLiteral
    | BinaryLiteral
    | FloatingLiteral
    | SingleQuotedStringLiteral
    | NowdocStringLiteral
    | DoubleQuotedStringLiteral
    | BooleanLiteral
    | NullLiteral -> (parser1, make_literal_expression (make_token token))
    | HeredocStringLiteral ->
      (* We have a heredoc string literal but it might contain embedded
         expressions. Start over. *)
      let (parser, token, name) = next_docstring_header parser in
      parse_heredoc_string parser (make_token token) name
    | HeredocStringLiteralHead
    | DoubleQuotedStringLiteralHead ->
      parse_double_quoted_string parser1 (make_token token)
    | Variable -> parse_variable_or_lambda parser
    | XHPClassName
    | Name
    | QualifiedName -> parse_name_or_collection_literal_expression parser1 token
    | Self
    | Parent -> parse_scope_resolution_or_name parser
    | Static ->
      parse_anon_or_awaitable_or_scope_resolution_or_name parser
    | Yield -> parse_yield_expression parser
    | Dollar -> parse_dollar_expression parser
    | Suspend
      (* TODO: The operand to a suspend is required to be a call to a
      coroutine. Give an error in a later pass if this isn't the case. *)
    | Exclamation
    | PlusPlus
    | MinusMinus
    | Tilde
    | Minus
    | Plus
    | Ampersand
    | Await
    | Clone
    | Print
    | At -> parse_prefix_unary_expression parser
    | LeftParen -> parse_cast_or_parenthesized_or_lambda_expression parser
    | LessThan -> parse_possible_xhp_expression parser
    | List  -> parse_list_expression parser
    | New -> parse_object_creation_expression parser
    | Array -> parse_array_intrinsic_expression parser
    | Varray -> parse_varray_intrinsic_expression parser
    | Vec -> parse_vector_intrinsic_expression parser
    | Darray -> parse_darray_intrinsic_expression parser
    | Dict -> parse_dictionary_intrinsic_expression parser
    | Keyset -> parse_keyset_intrinsic_expression parser
    | LeftBracket -> parse_array_creation_expression parser
    | Tuple -> parse_tuple_expression parser
    | Shape -> parse_shape_expression parser
    | Function -> parse_anon parser
    | DollarDollar ->
      (parser1, make_pipe_variable_expression (make_token token))
    | Async
    | Coroutine -> parse_anon_or_lambda_or_awaitable parser
    | Include
    | Include_once
    | Require
    | Require_once -> parse_inclusion_expression parser
    | Empty -> parse_empty_expression parser
    | Isset -> parse_isset_expression parser
    | Define -> parse_define_expression parser
    | Eval -> parse_eval_expression parser
    | TokenKind.EndOfFile
    | _ -> parse_as_name_or_error parser

  and parse_empty_expression parser =
    (* TODO: This is a PHP-ism. Open questions:
      * Should we allow a trailing comma? it is not a function call and
        never has more than one argument. See D4273242 for discussion.
      * Is there any restriction on the kind of expression this can be?
      * Should this be an error in strict mode?
      * Should this be in the specification?
      * Empty is case-insensitive; should use of non-lowercase be an error?
    *)
    (* TODO: The original Hack and HHVM parsers accept "empty" as an
    identifier, so we do too; consider whether it should be reserved. *)
    let (parser1, keyword) = assert_token parser Empty in
    if peek_token_kind parser1 = LeftParen then
      let (parser, left) = assert_token parser1 LeftParen in
      let (parser, arg) = parse_expression_with_reset_precedence parser in
      let (parser, right) = require_right_paren parser in
      let result = make_empty_expression keyword left arg right in
      (parser, result)
    else
      parse_as_name_or_error parser

  and parse_eval_expression parser =
    (* TODO: This is a PHP-ism. Open questions:
      * Should we allow a trailing comma? it is not a function call and
        never has more than one argument. See D4273242 for discussion.
      * Is there any restriction on the kind of expression this can be?
      * Should this be an error in strict mode?
      * Should this be in the specification?
      * Eval is case-insensitive. Should use of non-lowercase be an error?
    *)
    (* TODO: The original Hack and HHVM parsers accept "eval" as an
    identifier, so we do too; consider whether it should be reserved. *)
    let (parser1, keyword) = assert_token parser Eval in
    if peek_token_kind parser1 = LeftParen then
      let (parser, left) = assert_token parser1 LeftParen in
      let (parser, arg) = parse_expression_with_reset_precedence parser in
      let (parser, right) = require_right_paren parser in
      let result = make_eval_expression keyword left arg right in
      (parser, result)
    else
      parse_as_name_or_error parser

  and parse_isset_expression parser =
    (* TODO: This is a PHP-ism. Open questions:
      * Should we allow a trailing comma? See D4273242 for discussion.
      * Is there any restriction on the kind of expression the arguments can be?
      * Should this be an error in strict mode?
      * Should this be in the specification?
      * PHP requires that there be at least one argument; should we require
        that? if so, should we give the error in the parser or a later pass?
      * Isset is case-insensitive. Should use of non-lowercase be an error?
    *)
    (* TODO: The original Hack and HHVM parsers accept "isset" as an
    identifier, so we do too; consider whether it should be reserved. *)

    let (parser1, keyword) = assert_token parser Isset in
    if peek_token_kind parser1 = LeftParen then
      let (parser, left, args, right) = parse_expression_list_opt parser1 in
      let result = make_isset_expression keyword left args right in
      (parser, result)
    else
      parse_as_name_or_error parser

  and parse_define_expression parser =
    (* TODO: This is a PHP-ism. Open questions:
      * Should we allow a trailing comma? See D4273242 for discussion.
      * Is there any restriction on the kind of expression the arguments can be?
        They must be string, value, bool, but do they have to be compile-time
        constants, for instance?
      * Should this be an error in strict mode? You should use const instead.
      * Should this be in the specification?
      * PHP requires that there be at least two arguments; should we require
        that? if so, should we give the error in the parser or a later pass?
      * is define case-insensitive?
    *)
    (* TODO: The original Hack and HHVM parsers accept "define" as an
    identifier, so we do too; consider whether it should be reserved. *)
    let (parser1, keyword) = assert_token parser Define in
    if peek_token_kind parser1 = LeftParen then
      let (parser, left, args, right) = parse_expression_list_opt parser1 in
      let result = make_define_expression keyword left args right in
      (parser, result)
    else
      parse_as_name_or_error parser

  and parse_double_quoted_string parser head =
    parse_string_literal parser head ""

  and parse_heredoc_string parser head name =
    parse_string_literal parser head name

  and parse_braced_expression_in_string parser =
    (*
    We are parsing something like "abc{$x}def" or "abc${x}def", and we
    are at the left brace.

    We know that the left brace will not be preceded by trivia. However in the
    second of the two cases mentioned above it is legal for there to be trivia
    following the left brace.  If we are in the first case, we've already
    verified that there is no trailing trivia after the left brace.

    The expression may be followed by arbitrary trivia, including
    newlines and comments. That means that the closing brace may have
    leading trivia. But under no circumstances does the closing brace have
    trailing trivia.

    It's an error for the closing brace to be missing.

    Therefore we lex the left brace normally, parse the expression normally,
    but require that there be a right brace. We do not lex the trailing trivia
    on the right brace.

    ERROR RECOVERY: If the right brace is missing, treat the remainder as
    string text. *)

    let (parser, left_brace) = assert_token parser LeftBrace in
    let (parser, expr) = parse_expression_with_reset_precedence parser in
    let (parser1, token) = next_token_no_trailing parser in
    let (parser, right_brace) =
      if (Token.kind token) = RightBrace then
        (parser1, make_token token)
      else
        (with_error parser SyntaxError.error1006, (make_missing())) in
    let node = make_embedded_braced_expression left_brace expr right_brace in
    (parser, node)

  and parse_string_literal parser head name =
    (* SPEC

    Double-quoted string literals and heredoc string literals use basically
    the same rules; here we have just the grammar for double-quoted string
    literals.

    string-variable::
      variable-name   offset-or-property-opt

    offset-or-property::
      offset-in-string
      property-in-string

    offset-in-string::
      [   name   ]
      [   variable-name   ]
      [   integer-literal   ]

    property-in-string::
      ->   name

    TODO: What about ?->

    The actual situation is considerably more complex than indicated
    in the specification.

    TODO: Consider updating the specification.

    * The tokens in the grammar above have no leading or trailing trivia.

    * An embedded variable expression may also be enclosed in curly braces;
      however, the $ of the variable expression must follow immediately after
      the left brace.

    * An embedded variable expression inside braces allows trivia between
      the tokens and before the right brace.

    * An embedded variable expression inside braces can be a much more complex
      expression than indicated by the grammar above.  For example,
      {$c->x->y[0]} is good, and {$c[$x instanceof foo ? 0 : 1]} is good,
      but {$c instanceof foo ? $x : $y} is not.  It is not clear to me what
      the legal grammar here is; it seems best in this situation to simply
      parse any expression and do an error pass later.

    * Note that the braced expressions can include double-quoted strings.
      {$c["abc"]} is good, for instance.

    * ${ is illegal in strict mode. In non-strict mode, ${varname is treated
      the same as {$varname, and may be an arbitrary expression.

    * TODO: We need to produce errors if there are unbalanced brackets,
      example: "$x[0" is illegal.

    * TODO: Similarly for any non-valid thing following the left bracket,
      including trivia. example: "$x[  0]" is illegal.

    *)

    let merge token head =
      (* TODO: Assert that new head has no leading trivia, old head has no
      trailing trivia. *)
      (* Invariant: A token inside a list of string fragments is always a head,
      body or tail. *)
      (* TODO: Is this invariant what we want? We could preserve the parse of
         the string. That is, something like "a${b}c${d}e" is at present
         represented as head, expr, body, expr, tail.  It could be instead
         head, dollar, left brace, expr, right brace, body, dollar, left
         brace, expr, right brace, tail. Is that better?

         TODO: Similarly we might want to preserve the structure of
         heredoc strings in the parse: that there is a header consisting of
         an identifier, and so on, and then body text, etc. *)
      let k = match (Token.kind head, Token.kind token) with
      | (DoubleQuotedStringLiteralHead, DoubleQuotedStringLiteralTail) ->
        DoubleQuotedStringLiteral
      | (HeredocStringLiteralHead, HeredocStringLiteralTail) ->
        HeredocStringLiteral
      | (DoubleQuotedStringLiteralHead, _) -> DoubleQuotedStringLiteralHead
      | (HeredocStringLiteralHead, _) -> HeredocStringLiteralHead
      | (_, DoubleQuotedStringLiteralTail) -> DoubleQuotedStringLiteralTail
      | (_, HeredocStringLiteralTail) -> HeredocStringLiteralTail
      | _ -> StringLiteralBody in
      let w = (Token.width head) + (Token.width token) in
      let l = Token.leading head in
      let t = Token.trailing token in
      let result = Token.make k w l t in
      make_token result in

    let merge_head token acc =
      match acc with
      | h :: t ->
        begin
        match MinimalSyntax.get_token h with
        | None ->
          let k = Token.kind token in
          let token = match k with
            | StringLiteralBody
            | HeredocStringLiteralTail
            | DoubleQuotedStringLiteralTail -> token
            | _ -> Token.with_kind token StringLiteralBody in
          (make_token token) :: acc
        | Some head -> (merge token head) :: t
        end
      | _ -> (make_token token) :: acc in

    let parse_embedded_expression parser token =
      let var_expr = make_variable_expression (make_token token) in
      let (parser1, token1) = next_token_in_string parser name in
      let (parser2, token2) = next_token_in_string parser1 name in
      let (parser3, token3) = next_token_in_string parser2 name in
      match (Token.kind token1, Token.kind token2, Token.kind token3) with
      | (MinusGreaterThan, Name, _) ->
        let expr = make_embedded_member_selection_expression var_expr
          (make_token token1) (make_token token2) in
        (parser2, expr)
      | (LeftBracket, Name, RightBracket) ->
        let expr = make_embedded_subscript_expression var_expr
          (make_token token1)
          (make_qualified_name_expression (make_token token2))
          (make_token token3) in
        (parser3, expr)
      | (LeftBracket, Variable, RightBracket) ->
        let expr = make_embedded_subscript_expression var_expr
          (make_token token1) (make_variable_expression (make_token token2))
          (make_token token3) in
        (parser3, expr)
      | (LeftBracket, DecimalLiteral, RightBracket)
      | (LeftBracket, OctalLiteral, RightBracket)
      | (LeftBracket, HexadecimalLiteral, RightBracket)
      | (LeftBracket, BinaryLiteral, RightBracket) ->
        let expr = make_embedded_subscript_expression var_expr
          (make_token token1) (make_literal_expression (make_token token2))
          (make_token token3) in
        (parser3, expr)
      | _ -> (parser, var_expr) in

    let rec handle_left_brace parser acc =
      (* Note that here we use next_token_in_string because we need to know
      whether there is trivia between the left brace and the $x which follows.*)
      let (parser1, left_brace) = next_token_in_string parser name in
      let (_, token) = next_token_in_string parser1 name in
      (* TODO: What about "{$$}" ? *)
      match Token.kind token with
      | Dollar ->
        (* We do not support {$ inside a string unless the $ begins a
        variable name. Append the { and start again on the $. *)
        (* TODO: Is this right? Suppose we have "{${x}".  Is that the same
        as "{"."${x}" ? Double check this. *)
        (* TODO: Give an error. *)
        aux parser1 (merge_head left_brace acc)
      | Variable ->
        (* Parse any expression followed by a close brace.
           TODO: We do not actually support all possible expressions;
                 see above. Do we want to (1) catch this at parse time,
                 (2) catch it in a later pass, or (3) just allow any
                 expression here? *)
        let (parser, expr) = parse_braced_expression_in_string parser in
        aux parser (expr :: acc)
      | _ ->
        (* We got a { not followed by a $. Ignore it. *)
        (* TODO: Give a warning? *)
        aux parser1 (merge_head left_brace acc)

    and handle_dollar parser dollar acc =
      (* We need to parse ${x} as though it was {$x} *)
      (* TODO: This should be an error in strict mode. *)
      (* We must not have trivia between the $ and the {, but we can have
      trivia after the {. That's why we use next_token_in_string here. *)
      let (_, token) = next_token_in_string parser name in
      match Token.kind token with
      | LeftBrace ->
        (* The thing in the braces has to be an expression that begins
        with a variable, and the variable does *not* begin with a $. It's
        just the word.

        Unlike the {$var} case, there *can* be trivia before the expression,
        which means that trivia is likely the trailing trivia of the brace,
        not leading trivia of the expression. *)
        (* TODO: Enforce these rules by producing an error if they are
        violated. *)
        (* TODO: Make the parse tree for the leading word in the expression
        a variable expression, not a qualified name expression. *)

        let (parser, expr) = parse_braced_expression_in_string parser in
        aux parser (expr :: (make_token dollar) :: acc)

      | _ ->
        (* We got a $ not followed by a { or variable name. Ignore it. *)
        (* TODO: Give a warning? *)
        aux parser (merge_head dollar acc)

    and aux parser acc =
      let (parser1, token) = next_token_in_string parser name in
      match Token.kind token with
      | HeredocStringLiteralTail
      | DoubleQuotedStringLiteralTail -> (parser1, (merge_head token acc))
      | LeftBrace -> handle_left_brace parser acc
      | Variable ->
        let (parser, expr) = parse_embedded_expression parser1 token in
        aux parser (expr :: acc)
      | Dollar ->  handle_dollar parser1 token acc
      | _ -> aux parser1 (merge_head token acc) in

    let (parser, results) = aux parser [head] in
    (* If we've ended up with a single string literal with no internal
    structure, do not represent that as a list with one item. *)
    let results = match results with
    | h :: [] -> h
    | _ -> make_list (List.rev results) in
    let result = make_literal_expression results in
    (parser, result)

  and parse_inclusion_expression parser =
  (* SPEC:
    inclusion-directive:
      require-multiple-directive
      require-once-directive

    require-multiple-directive:
      require  include-filename  ;

    include-filename:
      expression

    require-once-directive:
      require_once  include-filename  ;

    In non-strict mode we allow an inclusion directive (without semi) to be
    used as an expression. It is therefore easier to actually parse this as:

    inclusion-directive:
      inclusion-expression  ;

    inclusion-expression:
      require include-filename
      require_once include-filename

    TODO: We allow "include" and "include_once" as well, which are PHP-isms
    specified as not supported in Hack. Do we need to produce an error in
    strict mode?

    TODO: Produce an error if this is used in an expression context
    in strict mode.
    *)

    let (parser, require) = next_token parser in
    let operator = Operator.prefix_unary_from_token (Token.kind require) in
    let require = make_token require in
    let (parser, filename) = parse_expression_with_operator_precedence
      parser operator in
    let result = make_inclusion_expression require filename in
    (parser, result)

  and peek_next_kind_if_operator parser =
    let kind = peek_token_kind parser in
    if Operator.is_trailing_operator_token kind then
      Some kind
    else
      None

  and operator_has_lower_precedence operator_kind parser =
    let operator = Operator.trailing_from_token operator_kind in
    (Operator.precedence operator) < parser.precedence

  and next_is_lower_precedence parser =
    match peek_next_kind_if_operator parser with
    | None -> true
    | Some kind -> operator_has_lower_precedence kind parser

  and parse_remaining_expression_or_specified_function_call parser term =
    try begin
      let parser, type_arguments = parse_generic_type_arguments_opt parser in
      if kind type_arguments <> SyntaxKind.TypeArguments
      || List.exists is_missing @@ children type_arguments
      then raise Cancel_attempt;
      let term = make_generic_type_specifier term type_arguments in
      let parser, function_call = parse_function_call parser term in
      if kind function_call <> SyntaxKind.FunctionCallExpression
      then raise Cancel_attempt;
      parser, function_call
    end with Cancel_attempt -> parse_remaining_expression parser term

  (* checks if t is a prefix unary expression where operator has expected kind
     and and operand matched predicate *)
  and check_prefix_unary_expression t expected_kind operand_predicate =
    match syntax t with
    | PrefixUnaryExpression {
        prefix_unary_operator = {
          syntax = Token t
        ; _ };
        prefix_unary_operand
      } when Token.kind t = expected_kind ->
        operand_predicate prefix_unary_operand
    | _ -> false

  (* Checks if given expression is a PHP variable.
  per PHP grammar:
  https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#grammar-variable
   A variable is an expression that can in principle be used as an lvalue *)
  and can_be_used_as_lvalue t =
    is_variable_expression t ||
    is_subscript_expression t ||
    is_member_selection_expression t ||
    is_scope_resolution_expression t ||
    check_prefix_unary_expression t Dollar can_be_used_as_lvalue

  (* checks if expression is a valid right hand side in by-ref assignment
   which is '&'PHP variable *)
  and is_byref_assignment_source t =
    check_prefix_unary_expression t Ampersand can_be_used_as_lvalue

  (*detects if left_term and operator can be treated as a beginning of
   assignment (respecting the precedence of operator on the left of
   left term). Returns
   - Prefix_none - either operator is not one of assignment operators or
   precedence of the operator on the left is higher than precedence of
   assignment.
   - Prefix_assignment - left_term  and operator can be interpreted as a
   prefix of assignment
   - Prefix_byref_assignment - left_term and operator can be interpreted as a
   prefix of byref assignment.*)
  and check_if_parsable_as_assignment left_term operator left_precedence =
    (* in PHP precedence of assignment in expression is bumped up to
       recognize cases like !$x = ... or $a == $b || $c = ...
       which should be parsed as !($x = ...) and $a == $b || ($c = ...)
    *)
    if left_precedence >= Operator.precedence_for_assignment_in_expressions then
      Prefix_none
    else match operator with
    | Equal when can_be_used_as_lvalue left_term -> Prefix_byref_assignment
    | Equal when is_list_expression left_term -> Prefix_assignment
    | PlusEqual | MinusEqual | StarEqual | SlashEqual |
      StarStarEqual | DotEqual | PercentEqual | AmpersandEqual |
      BarEqual | CaratEqual | LessThanLessThanEqual |
      GreaterThanGreaterThanEqual
      when can_be_used_as_lvalue left_term ->
      Prefix_assignment
    | _ -> Prefix_none

  and parse_remaining_expression parser term =
    match peek_next_kind_if_operator parser with
    | None -> (parser, term)
    | Some token ->
    let assignment_prefix_kind =
      check_if_parsable_as_assignment term token parser.precedence
    in
    (* stop parsing expression if:
    - precedence of the operator is less than precedence of the operator
      on the left
    AND
    - <term> <operator> does not look like a prefix of
      some assignment expression*)
    if operator_has_lower_precedence token parser &&
       assignment_prefix_kind = Prefix_none then (parser, term)
    else match token with
    (* Binary operators *)
    (* TODO Add an error if PHP and / or / xor are used in Hack.  *)
    (* TODO Add an error if PHP style <> is used in Hack. *)
    | And
    | Or
    | Xor
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
    | LessThanGreaterThan
    | LessThan
    | ExclamationEqualEqual
    | LessThanEqual
    | LessThanEqualGreaterThan
    | GreaterThanEqual
    | Ampersand
    | Bar
    | LessThanLessThan
    | GreaterThanGreaterThan
    | Carat
    | BarGreaterThan
    | QuestionQuestion ->
      parse_remaining_binary_expression parser term assignment_prefix_kind
    | Instanceof ->
      parse_instanceof_expression parser term
    | QuestionMinusGreaterThan
    | MinusGreaterThan ->
      let (parser, result) = parse_member_selection_expression parser term in
      parse_remaining_expression parser result
    | ColonColon ->
      let (parser, result) = parse_scope_resolution_expression parser term in
      parse_remaining_expression parser result
    | PlusPlus
    | MinusMinus -> parse_postfix_unary parser term
    | LeftParen -> parse_function_call parser term
    | LeftBracket
    | LeftBrace -> parse_subscript parser term
    | Question ->
      let (parser, token) = assert_token parser Question in
      let (parser, result) = parse_conditional_expression parser term token in
      parse_remaining_expression parser result
    | _ -> (parser, term)

  and parse_member_selection_expression parser term =
    (* SPEC:
    member-selection-expression:
      postfix-expression  ->  name
      postfix-expression  ->  variable-name
      postfix-expression  ->  xhp-class-name (DRAFT XHP SPEC)

    null-safe-member-selection-expression:
      postfix-expression  ?->  name
      postfix-expression  ?->  variable-name
      postfix-expression  ?->  xhp-class-name (DRAFT XHP SPEC)

    PHP allows $a->{$b}; to be more compatible with PHP, and give
    good errors, we allow that here as well.

    TODO: Produce an error if the braced syntax is used in Hack.

    *)
    let (parser, token) = next_token parser in
    let op = make_token token in
    (* TODO: We are putting the name / variable into the tree as a token
    leaf, rather than as a name or variable expression. Is that right? *)
    let (parser, name) = if peek_token_kind parser = LeftBrace then
      parse_braced_expression parser
    else
      require_xhp_class_name_or_name_or_variable parser in
    let result = if (Token.kind token) = MinusGreaterThan then
      make_member_selection_expression term op name
    else
      make_safe_member_selection_expression term op name in
    (parser, result)

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
      | LeftBracket -> require_right_bracket parser
      | _ -> require_right_brace parser in
      let left = make_token left in
      let result = make_subscript_expression term left index right in
      parse_remaining_expression parser result
    end

  and parse_expression_list_opt parser =
    (* SPEC

      TODO: This business of allowing ... does not appear in the spec. Add it.

      ERROR RECOVERY: A ... expression can only appear at the end of a
      formal parameter list. However, we parse it everywhere without error,
      and detect the error in a later pass.

      Note that it *is* legal for a ... expression be followed by a trailing
      comma, even though it is not legal for such in a formal parameter list.

      TODO: Can *any* expression appear after the ... ?

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

  and parse_designator parser =
    (* SPEC:
        class-type-designator:
          parent
          self
          static
          member-selection-expression
          null-safe-member-selection-expression
          qualified-name
          scope-resolution-expression
          subscript-expression
          variable-name

TODO: Update the spec to allow qualified-name < type arguments >
TODO: This will need to be fixed to allow situations where the qualified name
      is also a non-reserved token.
    *)
    let (parser1, token) = next_token parser in
    let kind = peek_token_kind parser1 in
    match Token.kind token with
    | Parent
    | Self
    | Static when kind = LeftParen ->
      (parser1, make_token token)
    | Name
    | QualifiedName when kind = LeftParen || kind = LessThan ->
      (* We want to parse new C() and new C<int>() as types, but
      new C::$x() as an expression. *)
      parse_type_specifier parser
    | _ ->
        parse_expression_with_operator_precedence parser Operator.NewOperator
        (* TODO: We need to verify in a later pass that the expression is a
        scope resolution (that does not end in class!), a member selection,
        a name, a variable, a property, or an array subscript expression. *)

  and parse_object_creation_expression parser =
    (* SPEC
      object-creation-expression:
        new  class-type-designator  (  argument-expression-list-opt  )
    *)
    (* PHP allows the entire expression list to be omitted. *)
    (* TODO: SPEC ERROR: PHP allows the entire expression list to be omitted,
     * but Hack disallows this behavior. (See SyntaxError.error2038.) However,
     * the Hack spec still states that the argument expression list is optional.
     * Update the spec to say that the argument expression list is required. *)
    let (parser, new_token) = assert_token parser New in
    let (parser, designator) = parse_designator parser in
    let (parser, left, args, right) =
    if peek_token_kind parser = LeftParen then
      parse_expression_list_opt parser
    else
      (parser, make_missing(), make_missing(), make_missing()) in
    let result =
      make_object_creation_expression new_token designator left args right in
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
      TODO: Hack allows "yield break".
      TODO: Should this be its own production, or can it be a yield expression?
      TODO: Is this an expression or a statement?
      TODO: Add it to the specification.
    *)
    let parser, yield_kw = assert_token parser Yield in
    match peek_token_kind parser with
    | From ->
      let parser, from_kw = assert_token parser From in
      let parser, operand = parse_expression parser in
      parser, make_yield_from_expression yield_kw from_kw operand
    | Break ->
      let parser, break_kw = assert_token parser Break in
      parser, make_yield_expression yield_kw break_kw
    | _ ->
      let parser, operand = parse_array_element_init parser in
      parser, make_yield_expression yield_kw operand

  and parse_cast_or_parenthesized_or_lambda_expression parser =
  (* We need to disambiguate between casts, lambdas and ordinary
    parenthesized expressions. *)
    if is_cast_expression parser then
      parse_cast_expression parser
    else if is_lambda_expression parser then
      parse_lambda_expression parser
    else
      parse_parenthesized_expression parser

  and is_easy_cast_type kind =
    (* See comments below. *)
    match kind with
    | Array | Bool | Double | Float | Int | Object | String -> true
    | _ -> false

  and token_implies_cast kind =
    (* See comments below. *)
    match kind with
    (* Keywords that imply cast *)
    | Abstract
    | Array
    | Arraykey
    | Async
    | TokenKind.Attribute
    | Await
    | Bool
    | Break
    | Case
    | Catch
    | Category
    | Children
    | Class
    | Classname
    | Clone
    | Const
    | Construct
    | Continue
    | Coroutine
    | Darray
    | Dict
    | Default
    | Define
    | Destruct
    | Do
    | Double
    | Echo
    | Else
    | Elseif
    | Empty
    | Enum
    | Eval
    | Extends
    | Fallthrough
    | Float
    | Final
    | Finally
    | For
    | Foreach
    | From
    | Function
    | Global
    | Goto
    | If
    | Implements
    | Include
    | Include_once
    | Insteadof
    | Int
    | Interface
    | Isset
    | Keyset
    | List
    | Mixed
    | Namespace
    | New
    | Newtype
    | Noreturn
    | Num
    | Object
    | Parent
    | Print
    | Private
    | Protected
    | Public
    | Require
    | Require_once
    | Required
    | Resource
    | Return
    | Self
    | Shape
    | Static
    | String
    | Super
    | Suspend
    | Switch
    | This
    | Throw
    | Trait
    | Try
    | Tuple
    | Type
    | Unset
    | Use
    | Var
    | Varray
    | Vec
    | Void
    | Where
    | While
    | Yield -> true
    (* Names that imply cast *)
    | Name
    | NamespacePrefix
    | QualifiedName
    | Variable -> true
    (* Symbols that imply cast *)
    | At
    | DollarDollar
    | Exclamation
    | LeftParen
    | Minus
    | MinusMinus
    | Dollar
    | Plus
    | PlusPlus
    | Tilde -> true
    (* Literals that imply cast *)
    | BinaryLiteral
    | BooleanLiteral
    | DecimalLiteral
    | DoubleQuotedStringLiteral
    | DoubleQuotedStringLiteralHead
    | StringLiteralBody
    | DoubleQuotedStringLiteralTail
    | ExecutionString
    | FloatingLiteral
    | HeredocStringLiteral
    | HeredocStringLiteralHead
    | HeredocStringLiteralTail
    | HexadecimalLiteral
    | NowdocStringLiteral
    | NullLiteral
    | OctalLiteral
    | SingleQuotedStringLiteral -> true
    (* Keywords that imply parenthesized expression *)
    | And
    | As
    | Instanceof
    | Or
    | Xor -> false
    (* Symbols that imply parenthesized expression *)
    | Ampersand
    | AmpersandAmpersand
    | AmpersandEqual
    | Bar
    | BarBar
    | BarEqual
    | BarGreaterThan
    | Carat
    | CaratEqual
    | Colon
    | ColonColon
    | Comma
    | Dot
    | DotEqual
    | DotDotDot
    | Equal
    | EqualEqual
    | EqualEqualEqual
    | EqualEqualGreaterThan
    | EqualGreaterThan
    | ExclamationEqual
    | LessThanGreaterThan
    | ExclamationEqualEqual
    | GreaterThan
    | GreaterThanEqual
    | GreaterThanGreaterThan
    | GreaterThanGreaterThanEqual
    | LessThanLessThanEqual
    | MinusEqual
    | MinusGreaterThan
    | Question
    | QuestionMinusGreaterThan
    | QuestionQuestion
    | RightBrace
    | RightBracket
    | RightParen
    | LeftBrace
    | LeftBracket
    | LessThan
    | LessThanEqual
    | LessThanEqualGreaterThan
    | LessThanLessThan
    | Percent
    | PercentEqual
    | PlusEqual
    | Semicolon
    | Slash
    | SlashEqual
    | SlashGreaterThan
    | Star
    | StarEqual
    | StarStar
    | StarStarEqual -> false
    (* Misc *)
    | Markup
    | LessThanQuestion
    | QuestionGreaterThan
    | ErrorToken
    | TokenKind.EndOfFile -> false
    (* TODO: Sort out rules for interactions between casts and XHP. *)
    | LessThanSlash
    | XHPCategoryName
    | XHPElementName
    | XHPClassName
    | XHPStringLiteral
    | XHPBody
    | XHPComment -> false

  and is_cast_expression parser =
    (* SPEC:
    cast-expression:
      (  cast-type  ) unary-expression
    cast-type:
      array, bool, double, float, int, object, string, or a name

    TODO: This implies that a cast "(name)" can only be a simple name, but
    I would expect that (\Foo\Bar), (:foo), (array<int>), and the like
    should also be legal casts. If we implement that then we will need
    a sophisticated heuristic to determine whether this is a cast or a
    parenthesized expression.

    The cast expression introduces an ambiguity: (x)-y could be a
    subtraction or a cast on top of a unary minus. We resolve this
    ambiguity as follows:

    * If the thing in parens is one of the keywords mentioned above, then
      it's a cast.
    * If the token which follows (x) is as or instanceof then
      it's a parenthesized expression.
    * PHP-ism extension: if the token is and, or or xor, then it's a
      parenthesized expression.
    * Otherwise, if the token which follows (x) is $$, @, ~, !, (, +, -,
      any name, qualified name, variable name, literal, or keyword then
      it's a cast.
    * Otherwise, it's a parenthesized expression. *)

    let (parser, _) = assert_token parser LeftParen in
    let (parser, type_token) = next_token parser in
    let type_token = Token.kind type_token in
    let (parser, right_paren) = next_token parser in
    let right_paren = Token.kind right_paren in
    let following_token = peek_token_kind parser in
    if right_paren != RightParen then
      false
    else if is_easy_cast_type type_token then
      true
    else if type_token != Name then
      false
    else
      token_implies_cast following_token

  and parse_cast_expression parser =
    (* We don't get here unless we have a legal cast, thanks to the
       previous call to is_cast_expression. See notes above. *)
    let (parser, left) = assert_token parser LeftParen in
    let (parser, cast_type) = next_token parser in
    let cast_type = make_token cast_type in
    let (parser, right) = assert_token parser RightParen in
    let (parser, operand) = parse_expression_with_operator_precedence
      parser Operator.CastOperator in
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
       a more sophisticated recovery strategy.  For example, if we have
       (x)==> then odds are pretty good that a lambda was intended and the
       error should say that ($x)==> was expected.
    *)
    let sig_and_arrow parser =
      let (parser, signature) = parse_lambda_signature parser in
      require_lambda_arrow parser in
    parses_without_error parser sig_and_arrow

  and parse_lambda_expression parser =
    (* SPEC
      lambda-expression:
        async-opt  lambda-function-signature  ==>  lambda-body
    *)
    let (parser, async) = optional_token parser Async in
    let (parser, coroutine) = optional_token parser Coroutine in
    let (parser, signature) = parse_lambda_signature parser in
    let (parser, arrow) = require_lambda_arrow parser in
    let (parser, body) = parse_lambda_body parser in
    let result = make_lambda_expression async coroutine signature arrow body in
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
    let (parser, right_paren) = require_right_paren parser in
    let syntax =
      make_parenthesized_expression left_paren expression right_paren in
    (parser, syntax)

  and parse_postfix_unary parser term =
    let (parser, token) = next_token parser in
    let term = make_postfix_unary_expression term (make_token token) in
    parse_remaining_expression parser term

  and parse_prefix_unary_expression parser =
    (* TODO: Operand to ++ and -- must be an lvalue. *)
    let (parser, token) = next_token parser in
    let operator = Operator.prefix_unary_from_token (Token.kind token) in
    let token = make_token token in
    let (parser, operand) = parse_expression_with_operator_precedence
      parser operator in
    let result = make_prefix_unary_expression token operand in
    (parser, result)

  and parse_simple_variable parser =
    match peek_token_kind parser with
    | Variable ->
      let (parser1, variable) = next_token parser in
      (parser1, make_token variable)
    | Dollar -> parse_dollar_expression parser
    | _ -> require_variable parser

  and parse_dollar_expression parser =
    let (parser, dollar) = assert_token parser Dollar in
    let (parser, operand) =
      if peek_token_kind parser = TokenKind.LeftBrace
      then parse_braced_expression parser
      else
        parse_expression_with_operator_precedence parser
          (Operator.prefix_unary_from_token TokenKind.Dollar)
    in
    let result = make_prefix_unary_expression dollar operand in
    (parser, result)

  and parse_instanceof_expression parser left =
    (* SPEC:
    instanceof-expression:
      instanceof-subject  instanceof   instanceof-type-designator

    instanceof-subject:
      expression

    instanceof-type-designator:
      qualified-name
      variable-name

    TODO: The spec is plainly wrong here. This is a bit of a mess and there
    are a number of issues.

    The issues arise from the fact that the thing on the right can be either
    a type, or an expression that evaluates to a string that names the type.

    The grammar in the spec, above, says that the only things that can be
    here are a qualified name -- in which case it names the type directly --
    or a variable of classname type, which names the type.  But this is
    not the grammar that is accepted by Hack / HHVM.  The accepted grammar
    treats "instanceof" as a binary operator which takes expressions on
    each side, and is of lower precedence than ->.  Thus

    $x instanceof $y -> z

    must be parsed as ($x instanceof ($y -> z)), and not, as the grammar
    implies, (($x instanceof $y) -> z).

    But wait, it gets worse.

    The less-than operator is of lower precedence than instanceof, so
    "$x instanceof foo < 10" should be parsed as (($x instanceof foo) < 10).
    But it seems plausible that we might want to parse
    "$x instanceof foo<int>" someday, in which case now we have an ambiguity.
    How do we know when we see the < whether we are attempting to parse a type?

    Moreover: we need to be able to parse XHP class names on the right hand
    side of the operator.  That is, we need to be able to say

    $x instanceof :foo

    However, we cannot simply say that the grammar is

    instanceof-type-designator:
      xhp-class-name
      expression

    Why not?   Because that then gives the wrong parse for:

    class :foo { static $bar = "abc" }
    class abc { }
    ...
    $x instanceof :foo :: $bar

    We need to parse that as $x instanceof (:foo :: $bar).

    The solution to all this is as follows.

    First, an XHP class name must be a legal expression. I had thought that
    it might be possible to say that an XHP class name is a legal type, or
    legal in an expression context when immediately followed by ::, but
    that's not the case. We need to be able to parse both

    $x instanceof :foo :: $bar

    and

    $x instanceof :foo

    so the most expedient way to do that is to parse any expression on the
    right, and to make XHP class names into legal expressions.

    So, with all this in mind, the grammar we will actually parse here is:

    instanceof-type-designator:
      expression

    This has the unfortunate property that the common case, say,

    $x instanceof C

    creates a parse node for C as a name token, not as a name token wrapped
    up as a simple type.

    Should we ever need to parse both arbitrary expressions and arbitrary
    types here, we'll have some tricky problems to solve.

    *)
    let (parser, op) = assert_token parser Instanceof in
    let precedence = Operator.precedence Operator.InstanceofOperator in
    let (parser, right_term) = parse_term parser in
    let (parser, right) = parse_remaining_binary_expression_helper
      parser right_term precedence in
    let result = make_instanceof_expression left op right in
    parse_remaining_expression parser result

  and parse_remaining_binary_expression
    parser left_term assignment_prefix_kind =
    (* We have a left term. If we get here then we know that
     * we have a binary operator to its right, and that furthermore,
     * the binary operator is of equal or higher precedence than the
     * whatever is going on in the left term.
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
     * We have the term A in hand; the precedence is low.
     * We see that x follows A.
     * We obtain the precedence of x. It is higher than the precedence of A,
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
      let is_rhs_of_assignment = assignment_prefix_kind <> Prefix_none in
      assert (not (next_is_lower_precedence parser) || is_rhs_of_assignment);

      let (parser1, token) = next_token parser in
      let operator = Operator.trailing_from_token (Token.kind token) in
      let default () =
        let precedence = Operator.precedence operator in
        let (parser2, right_term) =
          if is_rhs_of_assignment then
            (* reset the current precedence to make sure that expression on
              the right hand side of the assignment is fully consumed *)
            with_reset_precedence parser1 parse_term
          else
            parse_term parser1 in
        let (parser2, right_term) = parse_remaining_binary_expression_helper
          parser2 right_term precedence in
        let term = make_binary_expression
          left_term (make_token token) right_term in
        parse_remaining_expression parser2 term
      in
      (*if we are on the right hand side of the assignment - peek if next
      token is '&'. If it is - then parse next term. If overall next term is
      '&'PHP variable then the overall expression should be parsed as
      ... (left_term = & right_term) ...
      *)
      if assignment_prefix_kind = Prefix_byref_assignment &&
         Token.kind (peek_token parser1) = Ampersand then
        let (parser2, right_term) =
          parse_term @@ with_precedence
            parser1
            Operator.precedence_for_assignment_in_expressions in
        if is_byref_assignment_source right_term then
          let left_term = make_binary_expression
            left_term (make_token token) right_term
          in
          let (parser2, left_term) = parse_remaining_binary_expression_helper
            parser2 left_term parser.precedence
          in
          parse_remaining_expression parser2 left_term
        else
          default ()
      else
        default ()

  and parse_remaining_binary_expression_helper
      parser right_term left_precedence =
    (* This gathers up terms to the right of an operator that are
       operands of operators of higher precedence than the
       operator to the left. For instance, if we have
       A + B * C / D + E and we just parsed A +, then we want to
       gather up B * C / D into the right side of the +.
       In this case "right term" would be B and "left precedence"
       would be the precedence of +.
       See comments above for more details. *)
    let kind = Token.kind (peek_token parser) in
    if Operator.is_trailing_operator_token kind then
      let right_operator = Operator.trailing_from_token kind in
      let right_precedence = Operator.precedence right_operator in
      let associativity = Operator.associativity right_operator in
      let is_parsable_as_assignment =
        (* check if this is the case ... $a = ...
           where
             'left_precedence' - precedence of the operation on the left of $a
             'rigft_term' - $a
             'kind' - operator that follows right_term

          in case if right_term is valid left hand side for the assignment
          and token is assignment operator and left_precedence is less than
          bumped priority fort the assignment we reset precedence before parsing
          right hand side of the assignment to make sure it is consumed.
          *)
        check_if_parsable_as_assignment
          right_term kind left_precedence <> Prefix_none
      in
      if right_precedence > left_precedence ||
        (associativity = Operator.RightAssociative &&
         right_precedence = left_precedence ) ||
         is_parsable_as_assignment then
        let (parser2, right_term) =
          let precedence =
            if is_parsable_as_assignment then
              (* if expression can be parsed as an assignment, keep track of
                 the precedence on the left of the assignment (it is ok since
                 we'll internally boost the precedence when parsing rhs of the
                 assignment)
                 This is necessary for cases like:
                 ... + $a = &$b * $c + ...
                       ^             ^
                       #             $
                 it should be parsed as
                 (... + ($a = &$b) * $c) + ...
                 when we are at position (#)
                 - we will first consume byref assignment as a e1
                 - check that precedence of '*' is greater than precedence of
                   the '+' (left_precedence) and consume e1 * $c as $e2
                 - check that precedence of '+' is less or equal than precedence
                   of the '+' (left_precedence) and stop so the final result
                   before we get to the point ($) will be
                   (... + $e2)
                 *)
              left_precedence
            else
              right_precedence
          in
          let parser1 = with_precedence parser precedence in
          parse_remaining_expression parser1 right_term
        in
        let parser3 = with_precedence parser2 parser.precedence in
        parse_remaining_binary_expression_helper
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
    let kind = peek_token_kind parser in
    (* e1 ?: e2 -- where there is no consequence -- is legal.
       However this introduces an ambiguity:
       x ? :y::m : z
       is that
       x   ?:   y::m   :   z
       or
       x   ?   :y::m   :   z

       We assume the latter.
       TODO: Review this decision.
       TODO: Add this to the XHP draft specification.
       *)
    let missing_consequence =
      kind = Colon && not (is_next_xhp_class_name parser) in
    let (parser, consequence) = if missing_consequence then
      (parser, make_missing())
    else
      with_reset_precedence parser parse_expression in
    let (parser, colon) = require_colon parser in
    let (parser, term) = parse_term parser in
    let precedence = Operator.precedence Operator.ConditionalQuestionOperator in
    let (parser, alternative) = parse_remaining_binary_expression_helper
      parser term precedence in
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
    let parser, right_brace = require_right_brace parser in
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
    let parser, arrow = require_arrow parser in
    let parser, expr2 = parse_expression_with_reset_precedence parser in
    let syntax = make_element_initializer expr1 arrow expr2 in
    (parser, syntax)

  and parse_list_expression parser =
    (* SPEC:
      list-intrinsic:
        list  (  expression-list-opt  )
      expression-list:
        expression-opt
        expression-list , expression-opt

      See https://github.com/hhvm/hack-langspec/issues/82

      list-intrinsic must be used as the left-hand operand in a
      simple-assignment-expression of which the right-hand operand
      must be an expression that designates a vector-like array or
      an instance of the class types Vector, ImmVector, or Pair
      (the "source").

      TODO: Produce an error later if the expressions in the list destructuring
      are not lvalues.
      *)
    let (parser, keyword) = assert_token parser List in
    let (parser, left, items, right) =
      parse_parenthesized_comma_list_opt_items_opt
        parser parse_expression_with_reset_precedence in
    let result = make_list_expression keyword left items right in
    (parser, result)

  (* grammar:
   * array_intrinsic := array ( array-initializer-opt )
   *)
  and parse_array_intrinsic_expression parser =
    let (parser, array_keyword) = assert_token parser Array in
    let (parser, left_paren, members, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing
        parser parse_array_element_init in
    let syntax = make_array_intrinsic_expression array_keyword left_paren
      members right_paren in
    (parser, syntax)

  and parse_bracketed_collection_intrinsic_expression
      parser
      keyword_token
      parse_element_function
      make_intrinsinc_function =
    let (parser1, keyword) = assert_token parser keyword_token in
    let (parser1, left_bracket) = optional_token parser1 LeftBracket in
    if is_missing left_bracket then
      (* Fall back to dict being an ordinary name. Perhaps we're calling a
         function whose name is indicated by the keyword_token, for example. *)
      parse_as_name_or_error parser
    else
      let (parser, members) =
        parse_comma_list_opt_allow_trailing
          parser1
          RightBracket
          SyntaxError.error1015
          parse_element_function in
      let (parser, right_bracket) = require_right_bracket parser in
      let result =
        make_intrinsinc_function keyword left_bracket members right_bracket in
      (parser, result)


  and parse_darray_intrinsic_expression parser =
    (* TODO: Create the grammar and add it to the spec. *)
    parse_bracketed_collection_intrinsic_expression
      parser
      Darray
      parse_keyed_element_initializer
      make_darray_intrinsic_expression

  and parse_dictionary_intrinsic_expression parser =
    (* TODO: Create the grammar and add it to the spec. *)
    (* TODO: Can the list have a trailing comma? *)
    parse_bracketed_collection_intrinsic_expression
      parser
      Dict
      parse_keyed_element_initializer
      make_dictionary_intrinsic_expression

  and parse_keyset_intrinsic_expression parser =
    parse_bracketed_collection_intrinsic_expression
      parser
      Keyset
      parse_expression_with_reset_precedence
      make_keyset_intrinsic_expression

  and parse_varray_intrinsic_expression parser =
    (* TODO: Create the grammar and add it to the spec. *)
    parse_bracketed_collection_intrinsic_expression
      parser
      Varray
      parse_expression_with_reset_precedence
      make_varray_intrinsic_expression

  and parse_vector_intrinsic_expression parser =
    (* TODO: Create the grammar and add it to the spec. *)
    (* TODO: Can the list have a trailing comma? *)
    parse_bracketed_collection_intrinsic_expression
      parser
      Vec
      parse_expression_with_reset_precedence
      make_vector_intrinsic_expression

  (* array_creation_expression :=
       [ array-initializer-opt ]
     array-initializer :=
       array-initializer-list ,-opt
     array-initializer-list :=
        array-element-initializer
        array-element-initializer , array-initializer-list
  *)
  and parse_array_creation_expression parser =
    let (parser, left_bracket, members, right_bracket) =
      parse_bracketted_comma_list_opt_allow_trailing
      parser parse_array_element_init in
    let syntax = make_array_creation_expression left_bracket
      members right_bracket in
    (parser, syntax)

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
        scope-resolution-expression  =>  expression
        *)

    (* ERROR RECOVERY: We allow any expression as the left hand side.
       TODO: Make a later error pass that detects when it is not a
       literal or name. *)
    let (parser, name) = with_reset_precedence parser parse_expression in
    let (parser, arrow) = require_arrow parser in
    let (parser, value) = with_reset_precedence parser parse_expression in
    let result = make_field_initializer name arrow value in
    (parser, result)

  and parse_shape_expression parser =
    (* SPEC
      shape-literal:
        shape  (  field-initializer-list-opt  )

      field-initializer-list:
        field-initializers  ,-op

      field-initializers:
        field-initializer
        field-initializers  ,  field-initializer
    *)
    let (parser, shape) = assert_token parser Shape in
    let (parser, left_paren, fields, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing
        parser parse_field_initializer in
    let result = make_shape_expression shape left_paren fields right_paren in
    (parser, result)

  and parse_tuple_expression parser =
    (* SPEC
    tuple-literal:
      tuple  (  expression-list-one-or-more  )

    expression-list-one-or-more:
      expression
      expression-list-one-or-more  ,  expression

    TODO: Can the list be comma-terminated? If so, update the spec.
    TODO: We need to produce an error in a later pass if the list is empty.
    *)
    let (parser, keyword) = assert_token parser Tuple in
    let (parser, left_paren, items, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing
        parser parse_expression_with_reset_precedence in
    let result = make_tuple_expression keyword left_paren items right_paren in
    (parser, result)

  and parse_use_variable parser =
    (* TODO: Is it better that this returns the variable as a *token*, or
    as an *expression* that consists of the token? We do the former. *)
    let (parser, ampersand) = optional_token parser Ampersand in
    let (parser, variable) = require_variable parser in
    if is_missing ampersand then
      (parser, variable)
    else
      let result = make_prefix_unary_expression ampersand variable in
      (parser, result)

  and parse_anon_or_lambda_or_awaitable parser =
    (* TODO: The original Hack parser accepts "async" as an identifier, and
    so we do too. We might consider making it reserved. *)
    (* Skip any async or coroutine declarations that may be present. When we
       feed the original parser into the syntax parsers. they will take care of
       them as appropriate. *)
    let (parser1, _) = optional_token parser Static in
    let (parser1, _) = optional_token parser1 Async in
    let (parser1, _) = optional_token parser1 Coroutine in
    match peek_token_kind parser1 with
    | Function -> parse_anon parser
    | LeftBrace -> parse_async_block parser
    | Variable
    | LeftParen -> parse_lambda_expression parser
    | _ -> parse_as_name_or_error parser

  and parse_async_block parser =
    (*
     * grammar:
     *  awaitable-creation-expression :
     *    async-opt  coroutine-opt  compound-statement
     * TODO awaitable-creation-expression must not be used as the
     *      anonymous-function-body in a lambda-expression
     *)
    let parser, async = optional_token parser Async in
    let parser, coroutine = optional_token parser Coroutine in
    let parser, stmt = parse_compound_statement parser in
    parser, make_awaitable_creation_expression async coroutine stmt

  and parse_anon_use_opt parser =
    (* SPEC:
      anonymous-function-use-clause:
        use  (  use-variable-name-list  ,-opt  )

      use-variable-name-list:
        variable-name
        use-variable-name-list  ,  variable-name

      TODO: Strict mode requires that it be a list of variables; in
      non-strict mode we allow variables to be decorated with a leading
      & to indicate they are captured by reference. We need to give an
      error in a later pass for this.
    *)
    let (parser, use_token) = optional_token parser Use in
    if is_missing use_token then
      (parser, (make_missing()))
    else
      let (parser, left, vars, right) =
        parse_parenthesized_comma_list_opt_allow_trailing
          parser parse_use_variable in
      let result = make_anonymous_function_use_clause use_token
        left vars right in
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
        static-opt async-opt coroutine-opt  function
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
    let (parser, static) = optional_token parser Static in
    let (parser, async) = optional_token parser Async in
    let (parser, coroutine) = optional_token parser Coroutine in
    let (parser, fn) = assert_token parser Function in
    let (parser, left_paren, params, right_paren) =
      parse_parameter_list_opt parser in
    let (parser, colon, return_type) = parse_optional_return parser in
    let (parser, use_clause) = parse_anon_use_opt parser in
    let (parser, body) = parse_compound_statement parser in
    let result =
      make_anonymous_function
        static
        async
        coroutine
        fn
        left_paren
        params
        right_paren
        colon
        return_type
        use_clause
        body in
    (parser, result)

  and parse_braced_expression parser =
    let (parser, left_brace) = assert_token parser LeftBrace in
    let (parser, expression) = parse_expression_with_reset_precedence parser in
    let (parser, right_brace) = require_right_brace parser in
    let node = make_braced_expression left_brace expression right_brace in
    (parser, node)

  and require_right_brace_xhp parser =
    let (parser1, token) = next_xhp_body_token parser in
    if (Token.kind token) = TokenKind.RightBrace then
      (parser1, make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (with_error parser SyntaxError.error1006, (make_missing()))

  and parse_xhp_body_braced_expression parser =
    (* The difference between a regular braced expression and an
       XHP body braced expression is:
       <foo bar={$x}/*this_is_a_comment*/>{$y}/*this_is_body_text!*/</foo>
    *)
    let (parser, left_brace) = assert_token parser LeftBrace in
    let (parser, expression) = parse_expression_with_reset_precedence parser in
    let (parser, right_brace) = require_right_brace_xhp parser in
    let node = make_braced_expression left_brace expression right_brace in
    (parser, node)

  and parse_xhp_attribute parser =
    let (parser1, token, _) = next_xhp_element_token parser in
    if (Token.kind token) != XHPElementName then
      (parser, None)
    else
      let name = make_token token in
      let (parser, token, _) = next_xhp_element_token parser1 in
      if (Token.kind token) != Equal then
        let node = make_xhp_attribute name (make_missing()) (make_missing()) in
        let parser = with_error parser SyntaxError.error1016 in
        (* ERROR RECOVERY: The = is missing; assume that the name belongs
           to the attribute, but that the remainder is missing, and start
           looking for the next attribute. *)
        (parser, Some node)
      else
        let equal = make_token token in
        let (parser2, token, text) = next_xhp_element_token parser in
        match (Token.kind token) with
        | XHPStringLiteral ->
          let node = make_xhp_attribute name equal (make_token token) in
          (parser2, Some node)
        | LeftBrace ->
          let (parser, expr) = parse_braced_expression parser in
          let node = make_xhp_attribute name equal expr in
          (parser, Some node)
        | _ ->
        (* ERROR RECOVERY: The expression is missing; assume that the "name ="
           belongs to the attribute and start looking for the next attribute. *)
          let node = make_xhp_attribute name equal (make_missing()) in
          let parser = with_error parser SyntaxError.error1017 in
          (parser, Some node)

  and parse_xhp_body_element parser =
    let (parser1, token) = next_xhp_body_token parser in
    match Token.kind token with
    | XHPComment
    | XHPBody -> (parser1, Some (make_token token))
    | LeftBrace ->
      let (parser, expr) = parse_xhp_body_braced_expression parser in
      (parser, Some expr)
    | RightBrace ->
      (* If we find a free-floating right-brace in the middle of an XHP body
      that's just fine. It's part of the text. However, it is also likely
      to be a mis-edit, so we'll keep it as a right-brace token so that
      tooling can flag it as suspicious. *)
      (parser1, Some (make_token token))
    | LessThan ->
      let (parser, expr) = parse_possible_xhp_expression parser in
      (parser, Some expr)
    | _ -> (parser, None)

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

  and parse_xhp_expression parser left_angle name name_text =
    let (parser, attrs) = parse_list_until_none parser parse_xhp_attribute in
    let (parser1, token, _) = next_xhp_element_token ~no_trailing:true parser in
    match (Token.kind token) with
    | SlashGreaterThan ->
      let xhp_open = make_xhp_open left_angle name attrs (make_token token) in
      let xhp = make_xhp_expression
        xhp_open (make_missing()) (make_missing()) in
      (parser1, xhp)
    | GreaterThan ->
      let xhp_open = make_xhp_open left_angle name attrs (make_token token) in
      let (parser, xhp_body) =
        parse_list_until_none parser1 parse_xhp_body_element in
      let (parser, xhp_close) = parse_xhp_close parser name_text in
      let xhp = make_xhp_expression xhp_open xhp_body xhp_close in
      (parser, xhp)
    | _ ->
      (* ERROR RECOVERY: Assume the unexpected token belongs to whatever
         comes next. *)
      let xhp_open = make_xhp_open left_angle name attrs (make_missing()) in
      let xhp = make_xhp_expression
        xhp_open (make_missing()) (make_missing()) in
      let parser = with_error parser SyntaxError.error1013 in
      (parser, xhp)

  and parse_possible_xhp_expression parser =
    (* We got a < token where an expression was expected. *)
    let (parser, less_than) = assert_token parser LessThan in
    let (parser1, name, text) = next_xhp_element_token parser in
    if (Token.kind name) = XHPElementName then
      parse_xhp_expression parser1 less_than (make_token name) text
    else
      (* ERROR RECOVERY
      Hard to say what to do here. We are expecting an expression;
      we could simply produce an error for the < and call that the
      expression. Or we could assume the the left side of an inequality is
      missing, give a missing node for the left side, and parse the
      remainder as the right side. We'll go for the former for now. *)
      (with_error parser SyntaxError.error1015, less_than)

  and parse_anon_or_awaitable_or_scope_resolution_or_name parser =
    (* static is a legal identifier, if next token is scope resolution operatpr
      - parse expresson as scope resolution operator, otherwise try to interpret
      it as anonymous function (will fallback to name in case of failure) *)
    if peek_token_kind ~lookahead:1 parser = ColonColon then
      parse_scope_resolution_or_name parser
    else
      parse_anon_or_lambda_or_awaitable parser

  and parse_scope_resolution_or_name parser =
    (* parent, self and static are legal identifiers.  If the next
    thing that follows is a scope resolution operator, parse them as
    ordinary tokens, and then we'll pick them up as the operand to the
    scope resolution operator when we call parse_remaining_expression.
    Otherwise, parse them as ordinary names.  *)
    let (parser1, qualifier) = next_token parser in
    if peek_token_kind parser1 = ColonColon then
      (parser1, (make_token qualifier))
    else
      parse_as_name_or_error parser

  and parse_scope_resolution_expression parser qualifier =
    (* SPEC
      scope-resolution-expression:
        scope-resolution-qualifier  ::  name
        scope-resolution-qualifier  ::  class

      scope-resolution-qualifier:
        qualified-name
        variable-name
        self
        parent
        static
    *)
    (* TODO: The left hand side can in fact be any expression in this parser;
    we need to add a later error pass to detect that the left hand side is
    a valid qualifier. *)
    (* TODO: The right hand side, if a name or a variable, is treated as a
    name or variable *token* and not a name or variable *expression*. Is
    that the desired tree topology? Give this more thought; it might impact
    rename refactoring semantics. *)
    let (parser, op) = require_coloncolon parser in
    let (parser, name) =
      let parser1, token = next_token parser in
      match Token.kind token with
      | Class -> parser1, make_token token
      | Dollar -> parse_dollar_expression parser
      | _ -> require_name_or_variable_or_error parser SyntaxError.error1048
    in
    let result = make_scope_resolution_expression qualifier op name in
    (parser, result)
end
