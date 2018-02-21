(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct

module Token = Syntax.Token
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Operator = Full_fidelity_operator
module Lexer = Full_fidelity_lexer.WithToken(Syntax.Token)
module Env = Full_fidelity_parser_env
module PrecedenceSyntax = Full_fidelity_precedence_parser
  .WithSyntax(Syntax)
module PrecedenceParser = PrecedenceSyntax
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S

module type StatementParser_S = Full_fidelity_statement_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .StatementParser_S

module type DeclarationParser_S = Full_fidelity_declaration_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .DeclarationParser_S

module type TypeParser_S = Full_fidelity_type_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .TypeParser_S

module type ExpressionParser_S = Full_fidelity_expression_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .ExpressionParser_S

module ParserHelperSyntax = Full_fidelity_parser_helpers.WithSyntax(Syntax)
module ParserHelper =
  ParserHelperSyntax.WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))

module WithSmartConstructors (SCI : SCWithKind_S with module Token = Syntax.Token)
= struct

module WithStatementAndDeclAndTypeParser
  (StatementParser : StatementParser_S with module SC = SCI)
  (DeclParser : DeclarationParser_S with module SC = SCI)
  (TypeParser : TypeParser_S with module SC = SCI)
  : (ExpressionParser_S with module SC = SCI)
  = struct

  open TokenKind
  open Syntax

  module Parser = PrecedenceParser.WithSmartConstructors(SCI)
  include Parser
  include ParserHelper.WithParser(Parser)

  type binary_expression_prefix_kind =
    | Prefix_byref_assignment | Prefix_assignment | Prefix_none

  let make_and_track_prefix_unary_expression parser operator kind operand =
    let node = make_prefix_unary_expression operator operand in
    let prefix_unary_expression_stack =
      {node; operator_kind = kind; operand} ::
        parser.prefix_unary_expression_stack
    in
    {parser with prefix_unary_expression_stack}, node

  let find_in_prefix_unary_expression_stack parser node =
    List.find_opt (fun {node = n; _} -> n == node)
      parser.prefix_unary_expression_stack

  (* [Trick] *)

  let with_type_parser : 'a . t -> (TypeParser.t -> TypeParser.t * 'a) -> t * 'a
  = fun parser f ->
    let type_parser =
      TypeParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state
    in
    let (type_parser, node) = f type_parser in
    let env = TypeParser.env type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let context = TypeParser.context type_parser in
    let sc_state = TypeParser.sc_state type_parser in
    let parser = { parser with env; lexer; errors; context; sc_state } in
    (parser, node)

  let parse_generic_type_arguments_opt parser =
    with_type_parser parser
      (fun p ->
        let (p, items, no_arg_is_missing) =
          TypeParser.parse_generic_type_argument_list_opt p
        in
        (p, (items, no_arg_is_missing))
      )

  let with_decl_parser : 'a . t -> (DeclParser.t -> DeclParser.t * 'a) -> t * 'a
  = fun parser f ->
    let decl_parser =
      DeclParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state
    in
    let (decl_parser, node) = f decl_parser in
    let env = DeclParser.env decl_parser in
    let lexer = DeclParser.lexer decl_parser in
    let errors = DeclParser.errors decl_parser in
    let context = DeclParser.context decl_parser in
    let sc_state = DeclParser.sc_state decl_parser in
    let parser = { parser with env; lexer; errors; context; sc_state } in
    (parser, node)

  let parse_compound_statement parser =
    let statement_parser =
      StatementParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state
    in
    let (statement_parser, statement) =
      StatementParser.parse_compound_statement statement_parser in
    let env = StatementParser.env statement_parser in
    let lexer = StatementParser.lexer statement_parser in
    let errors = StatementParser.errors statement_parser in
    let context = StatementParser.context statement_parser in
    let sc_state = StatementParser.sc_state statement_parser in
    let parser = { parser with env; lexer; errors; context; sc_state } in
    (parser, statement)

  let parse_parameter_list_opt parser =
    let (parser, (left, token, right)) = with_decl_parser parser
      (fun decl_parser ->
        let (parser, left, token, right) =
          DeclParser.parse_parameter_list_opt decl_parser
        in
        parser, (left, token, right)
      )
    in
    (parser, left, token, right)

  let rec parse_expression parser =
    let (parser, term) = parse_term parser in
    parse_remaining_expression parser term

  and parse_expression_with_reset_precedence parser =
    with_reset_precedence parser parse_expression

  and parse_expression_with_operator_precedence parser operator =
    with_operator_precedence parser operator parse_expression

  and parse_if_no_error parser f =
    let old_errors = List.length (errors parser) in
    try
      let (parser, result) = f parser in
      let new_errors = List.length(errors parser) in
      Option.some_if (old_errors = new_errors) (parser, result)
    with Failure _ -> None

  and parse_as_name_or_error parser =
    (* TODO: Are there "reserved" keywords that absolutely cannot start
       an expression? If so, list them above and make them produce an
       error. *)
    let (parser1, token) = next_token_as_name parser in
    match (Token.kind token) with
    | Name ->
      let (parser1, name) =
        scan_remaining_qualified_name parser1 (make_token token) in
      parse_name_or_collection_literal_expression parser1 name
    | kind when Parser.expects_here parser kind ->
      (* ERROR RECOVERY: If we're encountering a token that matches a kind in
       * the previous scope of the expected stack, don't eat it--just mark the
       * name missing and continue parsing, starting from the offending token. *)
      let missing = make_missing parser in
      let parser = with_error parser SyntaxError.error1015 in
      (parser, missing)
    | _ ->
      (* ERROR RECOVERY: If we're encountering anything other than a Name
       * or the next expected kind, eat the offending token.
       * TODO: Increase the coverage of PrecedenceParser.expects_next, so that
       * we wind up eating fewer of the tokens that'll be needed by the outer
       * statement / declaration parsers. *)
      let parser = with_error parser1 SyntaxError.error1015 in
      (parser, make_token token)

  and parse_term parser =
    let (parser1, token) = next_xhp_class_name_or_other_token parser in
    match (Token.kind token) with
    (* TODO: Make these an error in Hack *)
    | ExecutionStringLiteral
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
      parse_heredoc_string parser token name
    | HeredocStringLiteralHead
    | DoubleQuotedStringLiteralHead ->
      parse_double_quoted_like_string
        parser1 token Lexer.Literal_double_quoted
    | ExecutionStringLiteralHead ->
      parse_double_quoted_like_string
        parser1 token Lexer.Literal_execution_string
    | Variable -> parse_variable_or_lambda parser
    | XHPClassName ->
      parse_name_or_collection_literal_expression parser1 (make_token token)
    | Name ->
      let (parser1, qualified_name) =
        scan_remaining_qualified_name parser1 (make_token token) in
      parse_name_or_collection_literal_expression parser1 qualified_name
    | Backslash ->
      let (parser1, qualified_name) =
        scan_qualified_name parser1 (make_token token) in
      parse_name_or_collection_literal_expression parser1 qualified_name
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
    | LessThan -> parse_possible_xhp_expression ~consume_trailing_trivia:true parser
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
    | HaltCompiler -> parse_halt_compiler_expression parser
    | Eval -> parse_eval_expression parser
    | kind when Parser.expects parser kind ->
      (* ERROR RECOVERY: if we've prematurely found a token we're expecting
       * later, mark the expression missing, throw an error, and do not advance
       * the parser. *)
      let missing = make_missing parser in
      let parser = with_error parser SyntaxError.error1015 in
      (parser, missing)
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

  and parse_halt_compiler_expression parser =
    let (parser1, keyword) = assert_token parser HaltCompiler in
    if peek_token_kind parser1 = LeftParen then
      let (parser, left, args, right) = parse_expression_list_opt parser1 in
      parser, make_halt_compiler_expression keyword left args right
    else
      let parser = with_error parser SyntaxError.error1019 in
      parse_as_name_or_error parser

  and parse_double_quoted_like_string parser head literal_kind =
    parse_string_literal parser head literal_kind

  and parse_heredoc_string parser head name =
    parse_string_literal parser head (Lexer.Literal_heredoc name)

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
    let (parser1, name_or_keyword_as_name) = next_token_as_name parser in
    let (parser1, right_brace) = next_token_no_trailing parser1 in
    let (parser, expr, right_brace) =
      match Token.kind name_or_keyword_as_name, Token.kind right_brace with
      | Name, RightBrace ->
        let expr = make_token name_or_keyword_as_name in
        let right_brace = make_token right_brace in
        parser1, expr, right_brace
      | _, _ ->
        let (parser, expr) = parse_expression_with_reset_precedence parser in
        let (parser1, token) = next_token_no_trailing parser in
        let (parser, right_brace) =
          if (Token.kind token) = RightBrace then
            (parser1, make_token token)
          else
            let missing = make_missing parser in
            let parser = with_error parser SyntaxError.error1006 in
            (parser, missing)
        in
        parser, expr, right_brace
    in
    let node = make_embedded_braced_expression left_brace expr right_brace in
    (parser, node)

  and parse_string_literal parser head literal_kind =
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

    let merge token = function
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
    | Some head ->
      let k = match (Token.kind head, Token.kind token) with
      | (DoubleQuotedStringLiteralHead, DoubleQuotedStringLiteralTail) ->
        DoubleQuotedStringLiteral
      | (ExecutionStringLiteralHead, ExecutionStringLiteralTail) ->
        ExecutionStringLiteral
      | (HeredocStringLiteralHead, HeredocStringLiteralTail) ->
        HeredocStringLiteral
      | (DoubleQuotedStringLiteralHead, _) ->
        DoubleQuotedStringLiteralHead
      | (ExecutionStringLiteralHead, _) ->
        ExecutionStringLiteralHead
      | (HeredocStringLiteralHead, _) ->
        HeredocStringLiteralHead
      | (_, DoubleQuotedStringLiteralTail) ->
        DoubleQuotedStringLiteralTail
      | (_, HeredocStringLiteralTail) ->
        HeredocStringLiteralTail
      | (_, ExecutionStringLiteralTail) ->
        ExecutionStringLiteralTail
      | _ ->
        StringLiteralBody
      in
      let s = Token.source_text head in
      let o = Token.leading_start_offset head in
      let w = (Token.width head) + (Token.width token) in
      let l = Token.leading head in
      let t = Token.trailing token in
      (* TODO: Make a "position" type that is a tuple of source and offset. *)
      Some (Token.make k s o w l t)
    | None ->
      let token = match Token.kind token with
      | StringLiteralBody
      | HeredocStringLiteralTail
      | DoubleQuotedStringLiteralTail
      | ExecutionStringLiteralTail ->
        token
      | _ ->
        Token.with_kind token StringLiteralBody
      in
      Some token
    in

    let put_opt head acc =
      Option.value_map ~default:acc ~f:(fun h -> make_token h :: acc) head
    in

    let parse_embedded_expression parser token =
      let var_expr = make_variable_expression (make_token token) in
      let (parser1, token1) = next_token_in_string parser literal_kind in
      let (parser2, token2) = next_token_in_string parser1 literal_kind in
      let (parser3, token3) = next_token_in_string parser2 literal_kind in
      match (Token.kind token1, Token.kind token2, Token.kind token3) with
      | (MinusGreaterThan, Name, _) ->
        let expr = make_embedded_member_selection_expression var_expr
          (make_token token1) (make_token token2) in
        (parser2, expr)
      | (LeftBracket, Name, RightBracket) ->
        let expr = make_embedded_subscript_expression var_expr
          (make_token token1)
          (make_token token2)
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
      | _ -> (parser, var_expr)
    in

    let rec handle_left_brace parser head acc =
      (* Note that here we use next_token_in_string because we need to know
      whether there is trivia between the left brace and the $x which follows.*)
      let (parser1, left_brace) = next_token_in_string parser literal_kind in
      let (_, token) = next_token_in_string parser1 literal_kind in
      (* TODO: What about "{$$}" ? *)
      match Token.kind token with
      | Variable ->
        (* Parse any expression followed by a close brace.
           TODO: We do not actually support all possible expressions;
                 see above. Do we want to (1) catch this at parse time,
                 (2) catch it in a later pass, or (3) just allow any
                 expression here? *)
        let (parser, expr) = parse_braced_expression_in_string parser in
        aux parser None (expr :: (put_opt head acc))
      | _ ->
        (* We do not support {$ inside a string unless the $ begins a
        variable name. Append the { and start again on the $. *)
        (* TODO: Is this right? Suppose we have "{${x}".  Is that the same
        as "{"."${x}" ? Double check this. *)
        (* TODO: Give an error. *)
        (* We got a { not followed by a $. Ignore it. *)
        (* TODO: Give a warning? *)
        aux parser1 (merge left_brace head) acc

    and handle_dollar parser dollar head acc =
      (* We need to parse ${x} as though it was {$x} *)
      (* TODO: This should be an error in strict mode. *)
      (* We must not have trivia between the $ and the {, but we can have
      trivia after the {. That's why we use next_token_in_string here. *)
      let (_, token) = next_token_in_string parser literal_kind in
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
        aux parser None (expr :: (make_token dollar) :: (put_opt head acc))
      | _ ->
        (* We got a $ not followed by a { or variable name. Ignore it. *)
        (* TODO: Give a warning? *)
        aux parser (merge dollar head) acc

    and aux parser head acc =
      let (parser1, token) = next_token_in_string parser literal_kind in
      match Token.kind token with
      | HeredocStringLiteralTail
      | DoubleQuotedStringLiteralTail
      | ExecutionStringLiteralTail ->
        parser1, (put_opt (merge token head) acc)
      | LeftBrace ->
        handle_left_brace parser head acc
      | Variable ->
        let (parser, expr) = parse_embedded_expression parser1 token in
        aux parser None (expr :: (put_opt head acc))
      | Dollar ->
        handle_dollar parser1 token head acc
      | _ ->
        aux parser1 (merge token head) acc
    in

    let (parser, results) = aux parser (Some head) [] in
    (* If we've ended up with a single string literal with no internal
    structure, do not represent that as a list with one item. *)
    let results = match results with
    | h :: [] -> h
    | _ -> make_list parser (List.rev results) in
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

  and parse_remaining_expression_or_specified_function_call parser term
      prefix_kind =
    let (parser1, (type_arguments, no_arg_is_missing)) =
      parse_generic_type_arguments_opt parser
    in
    if no_arg_is_missing
    && is_type_arguments type_arguments
    && parser.errors = parser1.errors
    then
      let parser, result =
        begin match peek_token_kind parser1 with
        | ColonColon ->
          (* handle a<type-args>::... case *)
          let type_specifier =
            make_generic_type_specifier term type_arguments in
          parse_scope_resolution_expression parser1 type_specifier
        | _ ->
          let (parser, left, args, right) = parse_expression_list_opt parser1 in
          parser, make_function_call_with_type_arguments_expression
            term type_arguments left args right
        end in
      parse_remaining_expression parser result
    else
      parse_remaining_binary_expression parser term prefix_kind

  (* Checks if given expression is a PHP variable.
  per PHP grammar:
  https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#grammar-variable
   A variable is an expression that can in principle be used as an lvalue *)
  and can_be_used_as_lvalue parser t =
    if is_variable_expression t
    || is_subscript_expression t
    || is_member_selection_expression t
    || is_scope_resolution_expression t
    then true
    else prefix_unary_expression_checker_helper parser t Dollar

  (* Checks if given node is prefix unary expression and verifies operator kind.
  Recursively run can_be_used_as_lvalue *)
  and prefix_unary_expression_checker_helper parser t kind =
    match find_in_prefix_unary_expression_stack parser t with
    | Some { operator_kind; operand; _ } ->
      if operator_kind = kind then
        can_be_used_as_lvalue parser operand
      else
        false
    | None -> false

  (* checks if expression is a valid right hand side in by-ref assignment
   which is '&'PHP variable *)
  and is_byref_assignment_source parser t =
    prefix_unary_expression_checker_helper parser t Ampersand

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
  and check_if_parsable_as_assignment parser left_term operator left_precedence
  =
    (* in PHP precedence of assignment in expression is bumped up to
       recognize cases like !$x = ... or $a == $b || $c = ...
       which should be parsed as !($x = ...) and $a == $b || ($c = ...)
    *)
    if left_precedence >= Operator.precedence_for_assignment_in_expressions then
      Prefix_none
    else match operator with
    | Equal when can_be_used_as_lvalue parser left_term ->
      Prefix_byref_assignment
    | Equal when is_list_expression left_term -> Prefix_assignment
    | PlusEqual | MinusEqual | StarEqual | SlashEqual |
      StarStarEqual | DotEqual | PercentEqual | AmpersandEqual |
      BarEqual | CaratEqual | LessThanLessThanEqual |
      GreaterThanGreaterThanEqual
      when can_be_used_as_lvalue parser left_term ->
      Prefix_assignment
    | _ -> Prefix_none

  and can_term_take_type_args term =
    is_name term
    || is_qualified_name term
    || is_member_selection_expression term
    || is_safe_member_selection_expression term
    || is_scope_resolution_expression term

  and parse_remaining_expression parser term =
    match peek_next_kind_if_operator parser with
    | None -> (parser, term)
    | Some token ->
    let assignment_prefix_kind =
      check_if_parsable_as_assignment parser term token parser.precedence
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
    | LessThan when can_term_take_type_args term ->
      parse_remaining_expression_or_specified_function_call parser term
        assignment_prefix_kind
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
    | LessThan
    | GreaterThan
    | Percent
    | Dot
    | EqualEqual
    | AmpersandAmpersand
    | BarBar
    | ExclamationEqual
    | LessThanGreaterThan
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
    | Is ->
      parse_is_expression parser term
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
    | QuestionColon ->
      parse_remaining_binary_expression parser term assignment_prefix_kind
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
    let (parser, name) =
      match peek_token_kind parser with
      | LeftBrace ->
        parse_braced_expression parser
      | Variable when Env.php5_compat_mode (env parser) ->
        parse_variable_in_php5_compat_mode parser
      | Dollar ->
        parse_dollar_expression parser
      | _ ->
        require_xhp_class_name_or_name_or_variable parser in
    let result = if (Token.kind token) = MinusGreaterThan then
      make_member_selection_expression term op name
    else
      make_safe_member_selection_expression term op name in
    (parser, result)

  and parse_variable_in_php5_compat_mode parser =
    (* PHP7 had a breaking change in parsing variables:
       (https://wiki.php.net/rfc/uniform_variable_syntax).
       Hack parser by default uses PHP7 compatible more which interprets
       variables accesses left-to-right. It usually matches PHP5 behavior
       except for cases with '$' operator, member accesses and scope resolution
       operators:
       $$a[1][2] -> ($$a)[1][2]
       $a->$b[c] -> ($a->$b)[c]
       X::$a[b]() -> (X::$a)[b]()

       In order to preserve backward compatibility we can parse
       variable/subscript expressions and treat them as if
       braced expressions to enfore PHP5 semantics
       $$a[1][2] -> ${$a[1][2]}
       $a->$b[c] -> $a->{$b[c]}
       X::$a[b]() -> X::{$a[b]}()
       *)
    let parser1, e =
      let precedence = Operator.precedence Operator.IndexingOperator in
      parse_expression (with_precedence parser precedence) in
    let parser1 = with_precedence parser1 parser.precedence in
    parser1, e

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
      let index = make_missing parser in
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

      TODO: Add call-convention-opt to the specification.
      (This work is tracked by task T22582676.)

      TODO: Update grammar for inout parameters.
      (This work is tracked by task T22582715.)

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
        call-convention-opt  expression
        argument-expressions  ,  expression
    *)
    (* This function parses the parens as well. *)
    let f parser =
      with_reset_precedence parser parse_decorated_expression_opt in
    parse_parenthesized_comma_list_opt_allow_trailing parser f

  and parse_decorated_expression_opt parser =
    match peek_token_kind parser with
    | DotDotDot
    | Inout ->
      let (parser, decorator) = next_token parser in
      let (parser, expr) = parse_expression parser in
      let decorator = make_token decorator in
      parser, make_decorated_expression decorator expr
    | _ -> parse_expression parser

  and parse_start_of_type_specifier parser start_token =
    let (parser, name) =
      if Token.kind start_token = Backslash
      then scan_qualified_name parser (make_token start_token)
      else scan_remaining_qualified_name parser (make_token start_token) in
    match peek_token_kind parser with
    | LeftParen | LessThan -> Some (parser, name)
    | _ -> None

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
    let default parser =
      parse_expression_with_operator_precedence parser Operator.NewOperator in
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Parent
    | Self ->
      begin match peek_token_kind parser1 with
      | LeftParen -> (parser1, make_token token)
      | LessThan ->
        let (parser1, (type_arguments, no_arg_is_missing)) =
          parse_generic_type_arguments_opt parser1
        in
        if no_arg_is_missing
        && is_type_arguments type_arguments
        && parser.errors = parser1.errors
        then
          let type_specifier =
            make_generic_type_specifier (make_token token) type_arguments in
          parser1, type_specifier
        else
          default parser
      | _ ->
        default parser
      end
    | Static when peek_token_kind parser1 = LeftParen ->
      (parser1, make_token token)
    | Name
    | Backslash ->
      begin match parse_start_of_type_specifier parser1 token with
      | Some (parser, name) ->
        (* We want to parse new C() and new C<int>() as types, but
        new C::$x() as an expression. *)
        with_type_parser parser (TypeParser.parse_remaining_type_specifier name)
      | None ->
        default parser
      end
    | _ ->
      default parser
      (* TODO: We need to verify in a later pass that the expression is a
      scope resolution (that does not end in class!), a member selection,
      a name, a variable, a property, or an array subscript expression. *)

  and parse_object_creation_expression parser =
    (* SPEC
      object-creation-expression:
        new object-creation-what
    *)
    let (parser, new_token) = assert_token parser New in
    let (parser, new_what) =
      let (parser1, token) = next_token parser in
      begin match Token.kind token with
      | Class -> parse_anonymous_class token parser1
      | _ -> parse_constructor_call parser
      end in
    let result = make_object_creation_expression new_token new_what in
    (parser, result)

  and parse_anonymous_class class_token parser =
    let class_token = make_token class_token in
    let (parser, left, args, right) =
      if peek_token_kind parser = LeftParen
      then parse_expression_list_opt parser
      else
        let missing1 = make_missing parser in
        let missing2 = make_missing parser in
        let missing3 = make_missing parser in
        (parser, missing1, missing2, missing3)
    in
    let parser
        , ( classish_extends
          , classish_extends_list
          , classish_implements
          , classish_implements_list
          , body
          )
    = with_decl_parser parser
      (fun decl_parser ->
        let (decl_parser, classish_extends, classish_extends_list) =
          DeclParser.parse_classish_extends_opt decl_parser in
        let (decl_parser, classish_implements, classish_implements_list) =
          DeclParser.parse_classish_implements_opt decl_parser in
        let (decl_parser, body) = DeclParser.parse_classish_body decl_parser in
        decl_parser
        , ( classish_extends
          , classish_extends_list
          , classish_implements
          , classish_implements_list
          , body
          )
      )
    in
    let result = make_anonymous_class class_token left args right
      classish_extends classish_extends_list classish_implements
      classish_implements_list body in
    (parser, result)

  and parse_constructor_call parser =
    (* SPEC
      constructor-call:
        class-type-designator  (  argument-expression-list-opt  )
    *)
    (* PHP allows the entire expression list to be omitted. *)
    (* TODO: SPEC ERROR: PHP allows the entire expression list to be omitted,
     * but Hack disallows this behavior. (See SyntaxError.error2038.) However,
     * the Hack spec still states that the argument expression list is optional.
     * Update the spec to say that the argument expression list is required. *)
    let (parser, designator) = parse_designator parser in
    let (parser, left, args, right) =
      if peek_token_kind parser = LeftParen then
        parse_expression_list_opt parser
      else
        let missing1 = make_missing parser in
        let missing2 = make_missing parser in
        let missing3 = make_missing parser in
        (parser, missing1, missing2, missing3)
    in
    let result =
      make_constructor_call designator left args right in
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
    | Semicolon ->
      let missing = make_missing parser in
      let yield_expr = make_yield_expression yield_kw missing in
      parser, yield_expr
    | _ ->
      let parser, operand = parse_array_element_init parser in
      parser, make_yield_expression yield_kw operand

  and parse_cast_or_parenthesized_or_lambda_expression parser =
    (* We need to disambiguate between casts, lambdas and ordinary
    parenthesized expressions. *)
    match possible_cast_expression parser with
    | Some (parser, left, cast_type, right) ->
      let (parser, operand) = parse_expression_with_operator_precedence
        parser Operator.CastOperator in
      let result = make_cast_expression left cast_type right operand in
      (parser, result)
    | _ -> begin
      match possible_lambda_expression parser with
      | Some (parser, signature) ->
        parse_lambda_expression_after_signature parser signature
      | None ->
        parse_parenthesized_expression parser
      end

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
    | HaltCompiler
    | Declare
    | Destruct
    | Do
    | Double
    | Echo
    | Else
    | Elseif
    | Empty
    | Endfor
    | Endforeach
    | Enddeclare
    | Endif
    | Endswitch
    | Endwhile
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
    | Inout
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
    | Using
    | Var
    | Varray
    | Vec
    | Void
    | Where
    | While
    | Yield -> true
    (* Names that imply cast *)
    | Name
    | Backslash
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
    | ExecutionStringLiteral
    | ExecutionStringLiteralHead
    | ExecutionStringLiteralTail
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
    | Is
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
    | QuestionColon
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

  and possible_cast_expression parser =
    (* SPEC:
    cast-expression:
      (  cast-type  ) unary-expression
    cast-type:
      array, bool, double, float, int, object, string, unset or a name

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
    * If the token which follows (x) is "as" or "instanceof" then
      it's a parenthesized expression.
    * PHP-ism extension: if the token is "and", "or" or "xor", then it's a
      parenthesized expression.
    * Otherwise, if the token which follows (x) is $$, @, ~, !, (, +, -,
      any name, qualified name, variable name, literal, or keyword then
      it's a cast.
    * Otherwise, it's a parenthesized expression. *)

    let (parser, left_paren) = assert_token parser LeftParen in
    let (parser, type_token) = next_token parser in
    let type_token_kind = Token.kind type_token in
    let (parser, right_paren) = next_token parser in
    let is_easy_cast_type_or_at_least_name =
      match type_token_kind with
      | Array | Bool | Double | Float | Int | Object | String | Unset -> Some true
      | Name -> Some false
      | _ -> None in
    let is_cast = Token.kind right_paren = RightParen &&
      Option.value_map ~default:false is_easy_cast_type_or_at_least_name
        ~f:(fun b -> b || token_implies_cast (peek_token_kind parser)) in
    if is_cast then
      Some (parser, left_paren, make_token type_token, make_token right_paren)
    else
      None

  and possible_lambda_expression parser =
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
    let signature_result = parse_if_no_error parser parse_lambda_signature in
    match signature_result with
    | Some (parser, _) when (peek_token_kind parser) = EqualEqualGreaterThan ->
      signature_result
    | _ -> None

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

  and parse_lambda_expression_after_signature parser signature =
    (* We had a signature with no async or coroutine, and we disambiguated it
    from a cast. *)
    let async = make_missing parser in
    let coroutine = make_missing parser in
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
    let kind = Token.kind token in
    let operator = Operator.prefix_unary_from_token kind in
    let token = make_token token in
    let (parser, operand) = parse_expression_with_operator_precedence
      parser operator in
    make_and_track_prefix_unary_expression parser token kind operand

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
      match peek_token_kind parser with
      | LeftBrace ->
        parse_braced_expression parser
      | Variable when Env.php5_compat_mode (env parser) ->
        parse_variable_in_php5_compat_mode parser
      | _ ->
        parse_expression_with_operator_precedence parser
          (Operator.prefix_unary_from_token Dollar) in
    make_and_track_prefix_unary_expression parser dollar Dollar operand

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

  and parse_is_expression parser left =
    (* SPEC:
    is-expression:
      is-subject  is  type-specifier

    is-subject:
      expression
    *)
    let (parser, op) = assert_token parser Is in
    let (parser, right) =
      with_type_parser parser TypeParser.parse_type_specifier
    in
    let result = make_is_expression left op right in
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
        if is_byref_assignment_source parser2 right_term then
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
          parser
          right_term
          kind
          left_precedence <> Prefix_none
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
    let (parser, consequence) =
      if missing_consequence then
        let missing = make_missing parser in
        (parser, missing)
      else
        with_reset_precedence parser parse_expression
    in
    let (parser, colon) = require_colon parser in
    let (parser, term) = parse_term parser in
    let precedence = Operator.precedence Operator.ConditionalQuestionOperator in
    let (parser, alternative) = parse_remaining_binary_expression_helper
      parser term precedence in
    let result = make_conditional_expression
      test question consequence colon alternative in
    (parser, result)

  and parse_name_or_collection_literal_expression parser name =
    match peek_token_kind parser with
    | LeftBrace ->
      let name = make_simple_type_specifier name in
      parse_collection_literal_expression parser name
    | LessThan ->
      let (parser1, (type_arguments, no_arg_is_missing)) =
        parse_generic_type_arguments_opt parser
      in
      if no_arg_is_missing
      && is_type_arguments type_arguments
      && parser.errors = parser1.errors
      && peek_token_kind parser1 = LeftBrace
      then
        let name = make_generic_type_specifier name type_arguments in
        parse_collection_literal_expression parser1 name
      else
        (parser, name)
    | _ ->
      (parser, name)

  and parse_collection_literal_expression parser name =

    (* SPEC
    collection-literal:
      key-collection-class-type  {  cl-initializer-list-with-keys-opt  }
      non-key-collection-class-type  {  cl-initializer-list-without-keys-opt  }
      pair-type  {  cl-element-value  ,  cl-element-value  }

      The types are grammatically qualified names; however the specification
      states that they must be as follows:
      * keyed collection type can be Map or ImmMap
      * non-keyed collection type can be Vector, ImmVector, Set or ImmSet
      * pair type can be Pair

      We will not attempt to determine if the names give the name of an
      appropriate type here. That's for the type checker.

      The argumment lists are:

      * for keyed, an optional comma-separated list of
        expression => expression pairs
      * for non-keyed, an optional comma-separated list of expressions
      * for pairs, a comma-separated list of exactly two expressions

      In all three cases, the lists may be comma-terminated.
      TODO: This fact is not represented in the specification; it should be.
      This work item is tracked by spec issue #109.
    *)

    let (parser, left_brace, initialization_list, right_brace) =
      parse_braced_comma_list_opt_allow_trailing parser parse_init_expression in
    (* Validating the name is a collection type happens in a later phase *)
    let syntax = make_collection_literal_expression
      name left_brace initialization_list right_brace in
    (parser, syntax)

  and parse_init_expression parser =
    (* ERROR RECOVERY
       We expect either a list of expr, expr, expr, ... or
       expr => expr, expr => expr, expr => expr, ...
       Rather than require at parse time that the list be all one or the other,
       we allow both, and give an error in the type checker.
    *)
    let parser, expr1 = parse_expression_with_reset_precedence parser in
    let parser, arrow = optional_token parser TokenKind.EqualGreaterThan in
    if is_missing arrow then
      (parser, expr1)
    else
      let parser, expr2 = parse_expression_with_reset_precedence parser in
      let syntax = make_element_initializer expr1 arrow expr2 in
      (parser, syntax)

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
        double_quoted_string_literal  =>  expression
        qualified-name  =>  expression
        scope-resolution-expression  =>  expression
        *)

    (* Specification is wrong, and fixing it is being tracked by
     * https://github.com/hhvm/hack-langspec/issues/108
     *)

    (* ERROR RECOVERY: We allow any expression on the left-hand side,
     * even though only some expressions are legal;
     * we will give an error in a later pass
     *)
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
      make_and_track_prefix_unary_expression parser ampersand Ampersand variable

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
      let missing = make_missing parser in
      (parser, missing)
    else
      let (parser, left, vars, right) =
        parse_parenthesized_comma_list_opt_allow_trailing
          parser parse_use_variable in
      let result = make_anonymous_function_use_clause use_token
        left vars right
      in
      (parser, result)

  and parse_optional_return parser =
    (* Parse an optional "colon-folowed-by-return-type" *)
    let (parser, colon) = optional_token parser Colon in
    let (parser, return_type) =
      if is_missing colon then
        let missing = make_missing parser in
        (parser, missing)
      else
        with_type_parser parser TypeParser.parse_return_type
    in
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
    let (parser, colon, return_type, use_clause, is_php7) =
      let (parser, use_clause) = parse_anon_use_opt parser in
      if is_missing use_clause then begin
        let (parser, colon, return_type) = parse_optional_return parser in
        let (parser, use_clause) = parse_anon_use_opt parser in
        (parser, colon, return_type, use_clause, false)
        end
      else begin
        (* might be PHP7 style lambda where return type follows use clause *)
        let (parser, colon, return_type) = parse_optional_return parser in
        (parser, colon, return_type, use_clause, not (is_missing colon))
        end in
    let (parser, body) = parse_compound_statement parser in
    let result =
      if is_php7
      then
        make_php7_anonymous_function
          static
          async
          coroutine
          fn
          left_paren
          params
          right_paren
          use_clause
          colon
          return_type
          body
      else
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
      let missing = make_missing parser in
      let parser = with_error parser SyntaxError.error1006 in
      (parser, missing)

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
    let (parser', token, _) = next_xhp_element_token parser in
    match (Token.kind token) with
    | LeftBrace -> parse_xhp_spread_attribute parser
    | XHPElementName -> parse_xhp_simple_attribute parser' (make_token token)
    | _ -> (parser, None)

  and parse_xhp_spread_attribute parser =
    let (parser, left_brace, _) = next_xhp_element_token parser in
    let (parser, ellipsis) = assert_token parser DotDotDot in
    let (parser, expression) = parse_expression_with_reset_precedence parser in
    let (parser, right_brace) = require_right_brace parser in
    let node = make_xhp_spread_attribute (make_token left_brace) ellipsis expression right_brace in
    (parser, Some node)

  and parse_xhp_simple_attribute parser name =
    (* Parse the attribute name and then defensively check for well-formed
     * attribute assignment *)
    let (parser', token, _) = next_xhp_element_token parser in
    if (Token.kind token) != Equal then
      let value = make_missing parser in
      let node = make_xhp_simple_attribute name (make_missing parser') value in
      let parser = with_error parser SyntaxError.error1016 in
      (* ERROR RECOVERY: The = is missing; assume that the name belongs
         to the attribute, but that the remainder is missing, and start
         looking for the next attribute. *)
      (parser, Some node)
    else
      let equal = make_token token in
      let (parser'', token, text) = next_xhp_element_token parser' in
      match (Token.kind token) with
      | XHPStringLiteral ->
        let node = make_xhp_simple_attribute name equal (make_token token) in
        (parser'', Some node)
      | LeftBrace ->
        let (parser, expr) = parse_braced_expression parser' in
        let node = make_xhp_simple_attribute name equal expr in
        (parser, Some node)
      | _ ->
      (* ERROR RECOVERY: The expression is missing; assume that the "name ="
         belongs to the attribute and start looking for the next attribute. *)
        let node = make_xhp_simple_attribute name equal (make_missing parser'') in
        let parser = with_error parser' SyntaxError.error1017 in
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
      let (parser, expr) =
        parse_possible_xhp_expression ~consume_trailing_trivia:false  parser in
      (parser, Some expr)
    | _ -> (parser, None)

  and parse_xhp_close ~consume_trailing_trivia parser _ =
    let (parser1, less_than_slash, _) = next_xhp_element_token parser in
    if (Token.kind less_than_slash) = LessThanSlash then
      let (parser2, name, name_text) = next_xhp_element_token parser1 in
      if (Token.kind name) = XHPElementName then
        (* TODO: Check that the given and name_text are the same. *)
        let (parser3, greater_than, _) =
          next_xhp_element_token ~no_trailing:(not consume_trailing_trivia) parser2 in
        if (Token.kind greater_than) = GreaterThan then
          (parser3, make_xhp_close (make_token less_than_slash)
            (make_token name) (make_token greater_than))
        else
          (* ERROR RECOVERY: *)
          let parser = with_error parser2 SyntaxError.error1039 in
          let less_than_slash_token = make_token less_than_slash in
          let name_token = make_token name in
          let missing = make_missing parser in
          (parser, make_xhp_close less_than_slash_token name_token missing)
      else
        (* ERROR RECOVERY: *)
        let parser = with_error parser1 SyntaxError.error1039 in
        let less_than_slash_token = make_token less_than_slash in
        let missing1 = make_missing parser in
        let missing2 = make_missing parser in
        (parser, make_xhp_close less_than_slash_token missing1 missing2)
    else
      (* ERROR RECOVERY: We probably got a < without a following / or name.
         TODO: For now we'll just bail out. We could use a more
         sophisticated strategy here. *)
      let parser = with_error parser1 SyntaxError.error1026 in
      let less_than_slash_token = make_token less_than_slash in
      let missing1 = make_missing parser in
      let missing2 = make_missing parser in
      (parser, make_xhp_close less_than_slash_token missing1 missing2)

  and parse_xhp_expression ~consume_trailing_trivia parser left_angle name name_text =
    let (parser, attrs) = parse_list_until_none parser parse_xhp_attribute in
    let (parser1, token, _) = next_xhp_element_token ~no_trailing:true parser in
    match (Token.kind token) with
    | SlashGreaterThan ->
      let xhp_open = make_xhp_open left_angle name attrs (make_token token) in
      let missing1 = make_missing parser in
      let missing2 = make_missing parser in
      let xhp = make_xhp_expression xhp_open missing1 missing2 in
      (parser1, xhp)
    | GreaterThan ->
      let xhp_open = make_xhp_open left_angle name attrs (make_token token) in
      let (parser, xhp_body) =
        parse_list_until_none parser1 parse_xhp_body_element in
      let (parser, xhp_close) = parse_xhp_close ~consume_trailing_trivia parser name_text in
      let xhp = make_xhp_expression xhp_open xhp_body xhp_close in
      (parser, xhp)
    | _ ->
      (* ERROR RECOVERY: Assume the unexpected token belongs to whatever
         comes next. *)
      let missing = make_missing parser in
      let xhp_open = make_xhp_open left_angle name attrs missing in
      let missing1 = make_missing parser in
      let missing2 = make_missing parser in
      let xhp = make_xhp_expression xhp_open missing1 missing2 in
      let parser = with_error parser SyntaxError.error1013 in
      (parser, xhp)

  and parse_possible_xhp_expression ~consume_trailing_trivia parser =
    (* We got a < token where an expression was expected. *)
    let (parser, less_than) = assert_token parser LessThan in
    let (parser1, name, text) = next_xhp_element_token parser in
    if (Token.kind name) = XHPElementName then
      parse_xhp_expression
        ~consume_trailing_trivia parser1 less_than (make_token name) text
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
      | LeftBrace -> parse_braced_expression parser
      | Variable when Env.php5_compat_mode (env parser) ->
        let parser1, e = parse_variable_in_php5_compat_mode parser in
        (* for :: only do PHP5 transform for call expressions
           in other cases fall back to the regular parsing logic *)
        if peek_token_kind parser1 = LeftParen &&
          (* make sure the left parenthesis means a call
             for the expression we are currently parsing, and
             are not for example for a constructor call whose
             name would be the result of this expression. *)
          not @@ operator_has_lower_precedence LeftParen parser
        then parser1, e
        else require_name_or_variable_or_error parser SyntaxError.error1048
      | _ ->
        require_name_or_variable_or_error parser SyntaxError.error1048
    in
    let result = make_scope_resolution_expression qualifier op name in
    (parser, result)
end
end (* WithSmartConstructors *)
end (* WithSyntax *)
