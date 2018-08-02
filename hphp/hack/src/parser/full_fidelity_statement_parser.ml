(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module WithSyntax(Syntax: Syntax_sig.Syntax_S) = struct
module Token = Syntax.Token
module Trivia = Token.Trivia
module TriviaKind = Full_fidelity_trivia_kind
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Env = Full_fidelity_parser_env
module SimpleParserSyntax =
  Full_fidelity_simple_parser.WithSyntax(Syntax)
module SimpleParser = SimpleParserSyntax.WithLexer(
  Full_fidelity_lexer.WithToken(Syntax.Token))

module ParserHelperSyntax = Full_fidelity_parser_helpers.WithSyntax(Syntax)
module ParserHelper = ParserHelperSyntax
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S

module type ExpressionParser_S = Full_fidelity_expression_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .ExpressionParser_S

module type DeclarationParser_S = Full_fidelity_declaration_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .DeclarationParser_S

module type TypeParser_S = Full_fidelity_type_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .TypeParser_S

module type StatementParser_S = Full_fidelity_statement_parser_type
  .WithSyntax(Syntax)
  .WithLexer(Full_fidelity_lexer.WithToken(Syntax.Token))
  .StatementParser_S

open TokenKind
open Syntax

module WithSmartConstructors (SCI : SCWithKind_S with module Token = Syntax.Token)
= struct

module WithExpressionAndDeclAndTypeParser
  (ExpressionParser : ExpressionParser_S with module SC = SCI)
  (DeclParser : DeclarationParser_S with module SC = SCI)
  (TypeParser : TypeParser_S with module SC = SCI) :
  (StatementParser_S with module SC = SCI) = struct

  module Parser = SimpleParser.WithSmartConstructors (SCI)
  include Parser
  include ParserHelper.WithParser(Parser)

  let parse_type_specifier parser =
    let type_parser =
      TypeParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state in
    let (type_parser, spec) = TypeParser.parse_type_specifier type_parser in
    let env = TypeParser.env type_parser in
    let lexer = TypeParser.lexer type_parser in
    let errors = TypeParser.errors type_parser in
    let context = TypeParser.context type_parser in
    let sc_state = TypeParser.sc_state type_parser in
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, spec)

  let with_expr_parser
  : 'a . t -> (ExpressionParser.t -> ExpressionParser.t * 'a) -> t * 'a
  = fun parser f ->
    let expr_parser =
      ExpressionParser.make
        parser.env
        parser.lexer
        parser.errors
        parser.context
        parser.sc_state in
    let (expr_parser, node) = f expr_parser in
    let env = ExpressionParser.env expr_parser in
    let lexer = ExpressionParser.lexer expr_parser in
    let errors = ExpressionParser.errors expr_parser in
    let context = ExpressionParser.context expr_parser in
    let sc_state = ExpressionParser.sc_state expr_parser in
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, node)

  let parse_expression parser =
    with_expr_parser parser ExpressionParser.parse_expression

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
    let parser = { env; lexer; errors; context; sc_state } in
    (parser, node)

  let rec parse_statement parser =
    match peek_token_kind parser with
    | Async
    | Coroutine
    | Function -> parse_possible_php_function parser
    | Abstract
    | Final
    | Interface
    | Trait
    | Class -> parse_php_class parser
    | Fallthrough -> parse_possible_erroneous_fallthrough parser
    | For -> parse_for_statement parser
    | Foreach -> parse_foreach_statement parser
    | Do -> parse_do_statement parser
    | While -> parse_while_statement parser
    | Declare -> parse_declare_statement parser
    | Let when Env.is_experimental_mode (env parser) -> parse_let_statement parser
    | Using ->
      let (parser, missing) = Make.missing parser (pos parser) in
      parse_using_statement parser missing
    | Await when peek_token_kind ~lookahead:1 parser = Using ->
      let parser, await_kw = assert_token parser Await in
      parse_using_statement parser await_kw
    | If -> parse_if_statement parser
    | Switch -> parse_switch_statement parser
    | Try -> parse_try_statement parser
    | Break -> parse_break_statement parser
    | Continue -> parse_continue_statement parser
    | Return -> parse_return_statement parser
    | Throw -> parse_throw_statement parser
    | LeftBrace -> parse_compound_statement parser
    | Static ->
      parse_function_static_declaration_or_expression_statement parser
    | Echo -> parse_echo_statement parser
    | Global -> parse_global_statement_or_expression_statement parser
    | Unset -> parse_unset_statement parser
    | Case ->
      let (parser, result) = parse_case_label parser in
      (* TODO: This puts the error in the wrong place. We should highlight
      the entire label, not the trailing colon. *)
      let parser = with_error parser SyntaxError.error2003 in
      (parser, result)
    | Default ->
      let (parser, result) = parse_default_label parser in
      (* TODO: This puts the error in the wrong place. We should highlight
      the entire label, not the trailing colon. *)
      let parser = with_error parser SyntaxError.error2004 in
      (parser, result)
    | Name when peek_token_kind ~lookahead:1 parser = Colon ->
      parse_goto_label parser
    | Goto -> parse_goto_statement parser
    | QuestionGreaterThan ->
      let (p, s, _) = parse_markup_section parser ~is_leading_section:false in
      (p, s)
    | Semicolon -> parse_expression_statement parser
    (* ERROR RECOVERY: when encountering a token that's invalid now but the
     * context says is expected later, make the whole statement missing
     * and continue on, starting at the unexpected token. *)
    (* TODO T20390825: Make sure this this won't cause premature recovery. *)
    | kind when Parser.expects parser kind ->
      Make.missing parser (pos parser)
    | _ -> parse_expression_statement parser

  and parse_markup_section parser ~is_leading_section =
    let parser, prefix =
      (* for markup section at the beginning of the file
         treat ?> as a part of markup text *)
      (* The closing ?> tag is not legal hack, but accept it here and give an
         error in a later pass *)
      if not is_leading_section
        && peek_token_kind parser = TokenKind.QuestionGreaterThan then
        fetch_token parser
      else
        Make.missing parser (pos parser)
    in
    let parser, markup, suffix_opt = scan_markup parser ~is_leading_section in
    let (parser, markup) = Make.token parser markup in
    let (parser, suffix, is_echo_tag, has_suffix) =
      match suffix_opt with
      | Some (less_than_question, language_opt) ->
        let (parser, less_than_question_token) =
          Make.token parser less_than_question
        in
        (* if markup section ends with <?= tag
           then script section embedded between tags should be treated as if it
           will be an argument to 'echo'. Technically it should be restricted to
           expression but since it permits trailing semicolons we parse it as
           expression statement.
           TODO: consider making it even more loose and parse it as declaration
           for better error recovery in cases when user
           accidentally type '<?=' instead of '<?php' so declaration in script
           section won't throw parser off the rails. *)
        let (parser, language, is_echo_tag) =
          match language_opt with
          | Some language ->
            let (parser, token) = Make.token parser language in
            (parser, token, (Token.kind language = TokenKind.Equal))
          | None ->
            let (parser, missing) = Make.missing parser (pos parser) in
            (parser, missing, false)
        in
        let (parser, suffix) =
          Make.markup_suffix parser less_than_question_token language
        in
        ( parser
        , suffix
        , is_echo_tag
        , true
        )
      | None ->
        let (parser, missing) = Make.missing parser (pos parser) in
        (parser, missing, false, false)
    in
    let parser, expression =
      if is_echo_tag then parse_statement parser
      else Make.missing parser (pos parser)
    in
    let (parser, s) =
      Make.markup_section parser prefix markup suffix expression
    in
    (parser, s, has_suffix)

  and parse_possible_php_function parser =
    (* ERROR RECOVERY: PHP supports nested named functions, but Hack does not.
    (Hack only supports anonymous nested functions as expressions.)

    If we have a statement beginning with function left-paren, then parse it
    as a statement expression beginning with an anonymous function; it will
    then have to end with a semicolon.

    If it starts with something else, parse it as a function.

    TODO: Give an error for nested nominal functions in a later pass.

    *)
    let kind0 = peek_token_kind ~lookahead:0 parser in
    let kind1 = peek_token_kind ~lookahead:1 parser in
    match kind0, kind1 with
    | (Async | Coroutine), Function
      when peek_token_kind ~lookahead:2 parser = LeftParen ->
      parse_expression_statement parser
    | Function, LeftParen (* Verbose-style lambda *)
    | (Async | Coroutine), LeftParen (* Async / coroutine, compact-style lambda *)
    | Async, LeftBrace (* Async block *)
      -> parse_expression_statement parser
    | _ ->
      let (parser, missing) = Make.missing parser (pos parser) in
      with_decl_parser
        parser
        (fun p ->
          DeclParser.parse_function_declaration p missing)

  and parse_php_class parser =
    (* PHP allows classes nested inside of functions, but hack does not *)
    (* TODO check for hack error: no classish declarations inside functions *)
    let (parser, missing) = Make.missing parser (pos parser) in
    with_decl_parser parser
      (fun parser -> DeclParser.parse_classish_declaration parser missing)

  (* Helper: parses ( expr ) *)
  and parse_paren_expr parser =
    let (parser, left_paren) = require_left_paren parser in
    let (parser, expr_syntax) = parse_expression parser in
    let (parser, right_paren) = require_right_paren parser in
    (parser, left_paren, expr_syntax, right_paren)

  and parse_for_statement parser =
    (* SPEC
    for-statement:
      for   (   for-initializer-opt   ;   for-control-opt   ;    \
        for-end-of-loop-opt   )   statement

    Each clause is an optional, comma-separated list of expressions.
    Note that unlike most such lists in Hack, it may *not* have a trailing
    comma.
    TODO: There is no compelling reason to not allow a trailing comma
    from the grammatical point of view. Each clause unambiguously ends in
    either a semi or a paren, so we can allow a trailing comma without
    difficulty.

    *)
    let parser, for_keyword_token = assert_token parser For in
    let parser, for_left_paren = require_left_paren parser in
    let parser, for_initializer_expr = parse_comma_list_opt
      parser Semicolon SyntaxError.error1015 parse_expression in
    let parser, for_first_semicolon = require_semicolon parser in
    let parser, for_control_expr = parse_comma_list_opt
      parser Semicolon SyntaxError.error1015 parse_expression in
    let parser, for_second_semicolon = require_semicolon parser in
    let parser, for_end_of_loop_expr = parse_comma_list_opt
      parser RightParen SyntaxError.error1015 parse_expression in
    let parser, for_right_paren = require_right_paren parser in
    let parser, for_statement =
      let _, open_token = next_token parser in
      match Token.kind open_token with
      | Colon -> parse_alternate_loop_statement parser ~terminator:Endfor
      | _ -> parse_statement parser
    in
    Make.for_statement
      parser
      for_keyword_token
      for_left_paren
      for_initializer_expr
      for_first_semicolon
      for_control_expr
      for_second_semicolon
      for_end_of_loop_expr
      for_right_paren
      for_statement

  and parse_foreach_statement parser =
    let parser, foreach_keyword_token = assert_token parser Foreach in
    let parser, foreach_left_paren = require_left_paren parser in
    let parser = Parser.expect_in_new_scope parser [ RightParen ] in
    let parser, foreach_collection_name =
      with_expr_parser parser (fun p ->
        ExpressionParser.with_as_expresssions p ~enabled:false ExpressionParser.parse_expression
      ) in
    let parser, await_token = optional_token parser Await in
    let parser, as_token = require_as parser in
    let (parser1, after_as) = parse_expression parser in
    let (parser, foreach_key, foreach_arrow, foreach_value) =
      match Token.kind (peek_token parser1) with
      | RightParen ->
        let (parser, missing1) = Make.missing parser (pos parser) in
        let (parser, missing2) = Make.missing parser (pos parser) in
        let (parser, value) = parse_expression parser in
        (parser, missing1, missing2, value)
      | EqualGreaterThan ->
        let parser, arrow = assert_token parser1 EqualGreaterThan in
        let parser, value = parse_expression parser in
        (parser, after_as, arrow, value)
      | _ ->
        let parser = with_error parser1 SyntaxError.invalid_foreach_element in
        let (parser, token) = fetch_token parser in
        let (parser, error) = Make.error parser token in
        let (parser, foreach_value) = parse_expression parser in
        (parser, after_as, error, foreach_value)
    in
    let parser, right_paren_token = require_right_paren parser in
    let parser = Parser.pop_scope parser [ RightParen ] in
    let parser, foreach_statement =
      match peek_token_kind parser with
      | Colon -> parse_alternate_loop_statement parser ~terminator:Endforeach
      | _ -> parse_statement parser
    in
    Make.foreach_statement
      parser
      foreach_keyword_token
      foreach_left_paren
      foreach_collection_name
      await_token
      as_token
      foreach_key
      foreach_arrow
      foreach_value
      right_paren_token
      foreach_statement

  and parse_do_statement parser =
    let (parser, do_keyword_token) =
      assert_token parser Do in
    let (parser, statement_node) =
      parse_statement parser in
    let (parser, do_while_keyword_token) = require_while parser in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, do_semicolon_token) = require_semicolon parser in
    Make.do_statement
      parser
      do_keyword_token
      statement_node
      do_while_keyword_token
      left_paren_token
      expr_node
      right_paren_token
      do_semicolon_token

  and parse_while_statement parser =
    let (parser, while_keyword_token) = assert_token parser While in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser
    in
    let (parser, statement_node) =
      match peek_token_kind parser with
      | Colon -> parse_alternate_loop_statement parser ~terminator:Endwhile
      | _ -> parse_statement parser
    in
    Make.while_statement
      parser
      while_keyword_token
      left_paren_token
      expr_node
      right_paren_token
      statement_node

  (* SPEC:
    let-statement:
      let   name   =   expression   ;
      let   name   :   type   =   expression   ;
  *)
  and parse_let_statement parser =
    let (parser, let_keyword_token) = assert_token parser Let in
    let (parser, name_token) = require_name parser in
    let (parser, colon_token, type_token) =
      match peek_token_kind parser with
      | Colon ->
        let (parser, colon_token) = assert_token parser Colon in
        let (parser, type_token) = parse_type_specifier parser in
        parser, colon_token, type_token
      | _ ->
        let (parser, missing_colon) = Make.missing parser (pos parser) in
        let (parser, missing_type) = Make.missing parser (pos parser) in
        parser, missing_colon, missing_type
    in
    let (parser, equal_token) = require_equal parser in
    let (parser, expr_node) = parse_expression parser in
    let (parser, init_node) =
      Make.simple_initializer parser equal_token expr_node
    in
    let (parser, semi_token) = require_semicolon parser in
    Make.let_statement
      parser
      let_keyword_token
      name_token
      colon_token
      type_token
      init_node
      semi_token

  (* SPEC:
    declare-statement:
      declare   (   expression   )   ;
      declare   (   expression   )   compound-statement

      declare   (   expression   ):
            compound-statement enddeclare;
    TODO: Update the specification of the grammar
   *)
  and parse_declare_statement parser =
    let (parser, declare_keyword_token) =
      assert_token parser Declare in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    match peek_token_kind parser with
    | Semicolon ->
      let (parser, semi) = assert_token parser Semicolon in
      Make.declare_directive_statement
        parser
        declare_keyword_token
        left_paren_token
        expr_node
        right_paren_token
        semi
    | Colon ->
      let (parser, statement_node) =
        parse_alternate_loop_statement parser ~terminator:Enddeclare
      in
      Make.declare_block_statement
        parser
        declare_keyword_token
        left_paren_token
        expr_node
        right_paren_token
        statement_node
    | _ ->
      let (parser, statement_node) = parse_statement parser in
      Make.declare_block_statement
        parser
        declare_keyword_token
        left_paren_token
        expr_node
        right_paren_token
        statement_node

  (* SPEC:
    using-statement:
      await-opt   using   expression   ;
      await-opt   using   (   expression-list   )   compound-statement

    TODO: Update the specification of the grammar
   *)
  and parse_using_statement parser await_kw =
    let (parser, using_kw) = assert_token parser Using in
    (* Decision point - Are we at a function scope or a body scope *)
    let token_kind = peek_token_kind parser in
    (* if next token is left paren it can be either
    - parenthesized expression followed by semicolon for function scoped using
    - comma separated list of expressions wrapped in parens for blocks.
      To distinguish between then try parse parenthesized expression and then
      check next token. NOTE: we should not use 'parse_expression' here
      since it might parse (expr) { smth() } as subscript expression $expr{$index}
    *)
    let (parser1, expr) =
      if token_kind = LeftParen then
        with_expr_parser parser
          ExpressionParser.parse_cast_or_parenthesized_or_lambda_expression
      else parse_expression parser in
    let (parser1, token) = next_token parser1 in
    match Token.kind token with
    | Semicolon ->
      let (parser, semi) = Make.token parser1 token in
      Make.using_statement_function_scoped parser await_kw using_kw expr semi
    | _ ->
      let (parser, left_paren) = require_left_paren parser in
      let (parser, expressions) = parse_comma_list
        parser RightParen SyntaxError.error1015 parse_expression
      in
      let (parser, right_paren) = require_right_paren parser in
      let (parser, statements) = parse_statement parser in
      Make.using_statement_block_scoped parser await_kw using_kw left_paren
        expressions right_paren statements

  and parse_unset_statement parser =
    (*
    TODO: This is listed as unsupported in Hack in the spec; is that true?
    TODO: If it is formally supported in Hack then update the spec; if not
    TODO: then should we make it illegal in strict mode?
    TODO: Can the list be comma-terminated?
    TODO: Can the list be empty?
    TODO: The list has to be expressions which evaluate as variables;
          add an error checking pass.
    TODO: Unset is case-insentive. Should non-lowercase be an error?
    *)
    let (parser, keyword) = assert_token parser Unset in
    let (parser, left_paren, variables, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing parser parse_expression
    in
    let (parser, semi) = require_semicolon parser in
    Make.unset_statement parser keyword left_paren variables right_paren semi

  and parse_if_statement parser =
    (* SPEC:
  if-statement:
    if   (   expression   )   statement   elseif-clauses-opt    else-clause-opt
    if   (   expression   ):  statement   alt-elif-clauses-opt  alt-else-clause-opt endif;

  elseif-clauses:
    elseif-clause
    elseif-clauses   elseif-clause

  alt-elif-clauses:
    alt-elif-clause
    alt-elif-clauses   alt-elif-clause

  elseif-clause:
    elseif   (   expression   )   statement

  alt-elif-clause:
    elseif   (   expression   ):  statement

  else-clause:
    else   statement

  alt-else-clause:
    else:  statement

    *)

    (* parses the "( expr ) statement" segment of If, Elseif or Else clauses.
     * Return a tuple of 5 elements, the first one being the resultant parser
     *)
    let parse_if_body_helper parser_body =
      let (parser_body, left_paren_token, expr_node, right_paren_token) =
        parse_paren_expr parser_body in
      let parser1, opening_token = next_token parser_body in
      let (parser1, opening_token_syntax) = Make.token parser1 opening_token in
      let (parser_body, statement_node) = match Token.kind opening_token with
      | Colon -> parse_alternate_if_block parser1 parse_statement
      | _ -> parse_statement parser_body in
      ( parser_body
      , left_paren_token
      , expr_node
      , right_paren_token
      , opening_token
      , opening_token_syntax
      , statement_node
      )
    in
    let parse_elseif_opt parser_elseif =
      if peek_token_kind parser_elseif = Elseif then
        let (parser_elseif, elseif_token) = assert_token parser_elseif Elseif in
        let ( parser_elseif
            , elseif_left_paren
            , elseif_condition_expr
            , elseif_right_paren
            , elseif_opening_token
            , elseif_opening_token_syntax
            , elseif_statement
            ) =
          parse_if_body_helper parser_elseif in
        let (parser_elseif, elseif_syntax) = match Token.kind elseif_opening_token with
        | Colon ->
          Make.alternate_elseif_clause
            parser_elseif
            elseif_token
            elseif_left_paren
            elseif_condition_expr
            elseif_right_paren
            elseif_opening_token_syntax
            elseif_statement
        | _ ->
          Make.elseif_clause
            parser_elseif
            elseif_token
            elseif_left_paren
            elseif_condition_expr
            elseif_right_paren
            elseif_statement
        in
        (parser_elseif, Some elseif_syntax)
      else
        (parser_elseif, None)
    in
    (* do not eat token and return Missing if first token is not Else *)
    let parse_else_opt parser_else =
      let (parser_else, else_token) = optional_token parser_else Else in
      if SC.is_missing else_token then
        (parser_else, else_token)
      else
        let parser1, opening_token = next_token parser_else in
        match Token.kind opening_token with
        | Colon ->
          let (_parser, opening_token_syntax) =
            Make.token parser opening_token
          in
          let (parser_else, else_consequence) =
            parse_alternate_if_block parser1 parse_statement
          in
          Make.alternate_else_clause
            parser_else
            else_token
            opening_token_syntax
            else_consequence
        | _ ->
          let (parser_else, else_consequence) = parse_statement parser_else in
          Make.else_clause parser_else else_token else_consequence
    in
    let (parser, if_keyword_token) = assert_token parser If in
    let ( parser
        , if_left_paren
        , if_expr
        , if_right_paren
        , if_opening_token
        , if_opening_token_syntax
        , if_consequence
        ) =
      parse_if_body_helper parser in
    let (parser, elseif_syntax) =
      parse_list_until_none parser parse_elseif_opt in
    let (parser, else_syntax) = parse_else_opt parser in
    match Token.kind if_opening_token with
    | Colon ->
      let (parser, closing_token) =
        require_token parser Endif (SyntaxError.error1059 Endif)
      in
      let (parser, semicolon_token) = require_semicolon parser in
      Make.alternate_if_statement
        parser
        if_keyword_token
        if_left_paren
        if_expr
        if_right_paren
        if_opening_token_syntax
        if_consequence
        elseif_syntax
        else_syntax
        closing_token
        semicolon_token
    | _ ->
      Make.if_statement
        parser
        if_keyword_token
        if_left_paren
        if_expr
        if_right_paren
        if_consequence
        elseif_syntax
        else_syntax

  and parse_switch_statement parser =
    (* SPEC:

    The spec for switches is very simple:

    switch-statement:
      switch  (  expression  )  compound-statement
    labeled-statement:
      case-label
      default-label
    case-label:
      case   expression  :  statement
    default-label:
      default  :  statement

    where the compound statement, if not empty, must consist of only labeled
    statements.

    These rules give a nice simple parse but it has some unfortunate properties.
    Consider:

    switch (foo)
    {
      case 1:
      case 2:
        break;
      default:
        break;
    }

    What's the parse of the compound statement contents based on that grammar?

    case 1:
        case 2:
            break;
    default:
        break;

    That is, the second case is a child of the first. That makes it harder
    to write analyzers, it makes it harder to write pretty printers, and so on.

    What do we really want here? We want a switch to be a collection of
    *sections* where each section has one or more *labels* and zero or more
    *statements*.

    switch-statement:
      switch  (  expression  )  { switch-sections-opt }

    switch-sections:
      switch-section
      switch-sections switch-section

    switch-section:
      section-labels
      section-statements-opt
      section-fallthrough-opt

    section-fallthrough:
      fallthrough  ;

    section-labels:
      section-label
      section-labels section-label

    section-statements:
      statement
      section-statements statement

    The parsing of course has to be greedy; we never want to say that there
    are zero statements *between* two sections.

    TODO: Update the specification with these rules.

    *)

    let (parser, switch_keyword_token) = assert_token parser Switch in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (_, opening_token) = next_token parser in
    let ((parser, opening_token_syntax), closing_token_kind) =
    match Token.kind opening_token with
    | Colon -> assert_token parser Colon, Endswitch
    | _ -> require_left_brace parser, RightBrace in
    let (parser, section_list) =
      let (parser1, token) = next_token parser in
      match Token.kind token with
      | Semicolon when peek_token_kind parser1 = closing_token_kind ->
        make_list parser1 []
      | _ ->
        parse_terminated_list parser parse_switch_section closing_token_kind
    in
    match closing_token_kind with
    | Endswitch ->
      let (parser, endswitch_token_syntax) =
        require_token parser Endswitch (SyntaxError.error1059 Endswitch)
      in
      let (parser, semicolon) = require_semicolon parser in
      Make.alternate_switch_statement
        parser
        switch_keyword_token
        left_paren_token
        expr_node
        right_paren_token
        opening_token_syntax
        section_list
        endswitch_token_syntax
        semicolon
    | _ ->
      let (parser, right_brace_token) = require_right_brace parser in
      Make.switch_statement
        parser
        switch_keyword_token
        left_paren_token
        expr_node
        right_paren_token
        opening_token_syntax
        section_list
        right_brace_token

  and is_switch_fallthrough parser =
    peek_token_kind parser = Fallthrough &&
    peek_token_kind ~lookahead:1 parser = Semicolon

  and parse_possible_erroneous_fallthrough parser =
    if is_switch_fallthrough parser then
      let parser = with_error parser SyntaxError.error1055
        ~on_whole_token:true in
      let (parser, result) = parse_switch_fallthrough parser in
      (parser, result)
    else
      parse_expression_statement parser

  and parse_switch_fallthrough parser =
    (* We don't get here unless we have fallthrough ; *)
    let (parser, keyword) = assert_token parser Fallthrough in
    let (parser, semi) = assert_token parser Semicolon in
    Make.switch_fallthrough parser keyword semi

  and parse_switch_fallthrough_opt parser =
    if is_switch_fallthrough parser then
      parse_switch_fallthrough parser
    else
      (**
       * As long as we have FALLTHROUGH comments, insert a faux-statement as if
       * there was a fallthrough statement. For example, the code
       *
       * > case 22:
       * >   $x = 0;
       * >   // FALLTHROUGH because we want all the other functionality as well
       * > case 42:
       * >   foo($x);
       * >   break;
       *
       * Should be parsed as if it were
       *
       * > case 22:
       * >   $x = 0;
       * >   // FALLTHROUGH because we want all the other functionality as well
       * >   fallthrough;
       * > case 43:
       * >   foo($x);
       * >   break;
       *
       * But since we have no actual occurrence (i.e. no position, no string) of
       * that `fallthrough;` statement, we construct a `switch_fallthrough`, but
       * fill it with `missing`.
       *)
      let next = peek_token parser in
      let commented_fallthrough =
        List.exists
          (fun t -> Trivia.kind t = TriviaKind.FallThrough)
          (Token.leading next)
      in
      let (parser, missing) = Make.missing parser (pos parser) in
      if commented_fallthrough
      then
        let (parser, missing1) = Make.missing parser (pos parser) in
        Make.switch_fallthrough parser missing missing1
      else
        (parser, missing)

  and parse_switch_section parser =
    (* See parse_switch_statement for grammar *)
    let (parser, labels) =
      parse_list_until_none parser parse_switch_section_label
    in
    let parser =
      if SC.is_missing labels then with_error parser SyntaxError.error2008
      else parser
    in
    let (parser, statements) =
      parse_list_until_none parser parse_switch_section_statement
    in
    let (parser, fallthrough) = parse_switch_fallthrough_opt parser in
    Make.switch_section parser labels statements fallthrough

  and parse_switch_section_statement parser =
    if is_switch_fallthrough parser then (parser, None)
    else match peek_token_kind parser with
    | Default
    | Case
    | RightBrace
    | Endswitch
    | TokenKind.EndOfFile -> (parser, None)
    | _ ->
      let (parser, statement) = parse_statement parser in
      (parser, Some statement)

  and parse_switch_section_label parser =
    (* See the grammar under parse_switch_statement *)
    match peek_token_kind parser with
    | Case ->
      let (parser, label) = parse_case_label parser in
      (parser, Some label)
    | Default ->
      let (parser, label) = parse_default_label parser in
      (parser, Some label)
    | _ -> (parser, None)

  and parse_catch_clause_opt parser =
    (* SPEC
      catch  (  type-specification-opt variable-name  )  compound-statement
      catch  (  type-specification-opt name  )  compound-statement [experimental-mode]
    *)
    if peek_token_kind parser = Catch then
      let (parser, catch_token) = assert_token parser Catch in
      let (parser, left_paren) = require_left_paren parser in
      let (parser, catch_type) =
        match peek_token_kind parser with
        | TokenKind.Variable ->
          let parser = with_error parser SyntaxError.error1007 in
          Make.missing parser (pos parser)
        | _ -> parse_type_specifier parser
      in
      let (parser, catch_var) =
        if Env.is_experimental_mode (env parser)
          then require_name_or_variable parser
          else require_variable parser
        in
      let (parser, right_paren) = require_right_paren parser in
      let (parser, compound_stmt) = parse_compound_statement parser in
      let (parser, catch_clause) =
        Make.catch_clause
          parser
          catch_token
          left_paren
          catch_type
          catch_var
          right_paren
          compound_stmt in
      (parser, Some catch_clause)
    else
      (parser, None)

  and parse_finally_clause_opt parser =
    (* SPEC
    finally-clause:
      finally   compound-statement
    *)
    if peek_token_kind parser = Finally then
      let (parser, finally_token) = assert_token parser Finally in
      let (parser, compound_stmt) = parse_compound_statement parser in
      Make.finally_clause parser finally_token compound_stmt
    else
      Make.missing parser (pos parser)

  and parse_try_statement parser =
    (* SPEC:
    try-statement:
      try  compound-statement   catch-clauses
      try  compound-statement   finally-clause
      try  compound-statement   catch-clauses   finally-clause
    *)
    let (parser, try_keyword_token) = assert_token parser Try in
    let (parser, try_compound_stmt) = parse_compound_statement parser in
    let (parser, catch_clauses) =
      parse_list_until_none parser parse_catch_clause_opt
    in
    let (parser, finally_clause) = parse_finally_clause_opt parser in
    (* If the catch and finally are both missing then we give an error in
       a later pass. *)
    Make.try_statement
      parser
      try_keyword_token
      try_compound_stmt
      catch_clauses
      finally_clause

  and parse_break_statement parser =
    (* SPEC
    break-statement:
      break  ;

    However, PHP allows an optional expression; though Hack does not have
    this feature, we allow it at parse time and produce an error later.
    TODO: Implement that error. *)

    (* We detect if we are not inside a switch or loop in a later pass. *)
    let (parser, break_token) = assert_token parser Break in
    let (parser, level) =
      if peek_token_kind parser = Semicolon then
        Make.missing parser (pos parser)
      else parse_expression parser in
    let (parser, semi_token) = require_semicolon parser in
    Make.break_statement parser break_token level semi_token

  and parse_continue_statement parser =
    (* SPEC
    continue-statement:
      continue  ;

    However, PHP allows an optional expression; though Hack does not have
    this feature, we allow it at parse time and produce an error later.
    TODO: Implement that error. *)

    (* We detect if we are not inside a loop in a later pass. *)
    let (parser, continue_token) = assert_token parser Continue in
    let (parser, level) =
      if peek_token_kind parser = Semicolon then
        Make.missing parser (pos parser)
      else parse_expression parser in
    let (parser, semi_token) = require_semicolon parser in
    Make.continue_statement parser continue_token level semi_token

  and parse_return_statement parser =
    let (parser, return_token) = assert_token parser Return in
    let (parser1, semi_token) = next_token parser in
    if Token.kind semi_token = Semicolon then
      let (parser, missing) = Make.missing parser1 (pos parser) in
      let (parser, semi_token) = Make.token parser semi_token in
      Make.return_statement parser return_token missing semi_token
    else
      let (parser, expr) = parse_expression parser in
      let (parser, semi_token) = require_semicolon parser in
      Make.return_statement parser return_token expr semi_token

  and parse_goto_label parser =
    let parser, goto_label_name = next_token_as_name parser in
    let (parser, goto_label_name) = Make.token parser goto_label_name in
    let parser, colon = require_colon parser in
    Make.goto_label parser goto_label_name colon

  and parse_goto_statement parser =
    let parser, goto = assert_token parser Goto in
    let parser, goto_label_name = next_token_as_name parser in
    let (parser, goto_label_name) = Make.token parser goto_label_name in
    let parser, semicolon = require_semicolon parser in
    Make.goto_statement parser goto goto_label_name semicolon

  and parse_throw_statement parser =
    let (parser, throw_token) = assert_token parser Throw in
    let (parser, expr) = parse_expression parser in
    let (parser, semi_token) = require_semicolon parser in
    Make.throw_statement parser throw_token expr semi_token

  and parse_default_label parser =
    (*
    See comments under parse_switch_statement for the grammar.
    TODO: Update the spec.
    TODO: The spec is wrong; it implies that a statement must always follow
          the default:, but in fact
          switch($x) { default: }
          is legal. Fix the spec.
    TODO: PHP allows a default to end in a semi; Hack does not.  We allow a semi
          here; add an error in a later pass.
    *)
    let (parser, default_token) = assert_token parser Default in
    let (parser, colon_token) =
      let (parser1, token) = next_token parser in
      if (Token.kind token) = Semicolon then
        Make.token parser1 token
      else
        require_colon parser
    in
    Make.default_label parser default_token colon_token

  and parse_case_label parser =
    (* SPEC:
      See comments under parse_switch_statement for the grammar.
    TODO: The spec is wrong; it implies that a statement must always follow
          the case, but in fact
          switch($x) { case 10: }
          is legal. Fix the spec.
    TODO: PHP allows a case to end in a semi; Hack does not.  We allow a semi
          here; add an error in a later pass.
          *)

    let (parser, case_token) = assert_token parser Case in
    let (parser, expr) = parse_expression parser in
    let (parser, colon_token) =
      let (parser1, token) = next_token parser in
      if (Token.kind token) = Semicolon then
        Make.token parser1 token
      else
        require_colon parser
    in
    Make.case_label parser case_token expr colon_token

  and parse_global_statement_or_expression_statement parser =
    (* PHP has a statement of the form
      global comma-separated-variable-list ;
      This is not supported in Hack, but we parse it anyways so as to give
      a good error message. However we do not want to disallow legal statements
      like "global(123);" so we use a heuristic to see if this is a likely
      global statement. If not, we parse it as an expression statement.
      TODO: Add an error in a later pass if this statement is found in a
      Hack file.
    *)
    let (parser1, keyword) = assert_token parser Global in
    let is_global_statement =
      match peek_token_kind parser1 with
      | TokenKind.Variable | TokenKind.Dollar -> true
      | _ -> false
    in
    if is_global_statement then
      let parse_simple_variable parser =
        with_expr_parser parser ExpressionParser.parse_simple_variable
      in
      let (parser, variables) = parse_comma_list
        parser1 Semicolon SyntaxError.error1008 parse_simple_variable
      in
      let (parser, semicolon) = require_semicolon parser in
      Make.global_statement parser keyword variables semicolon
    else
      parse_expression_statement parser

  and parse_function_static_declaration_or_expression_statement parser =
    (* Determine if the current token is a late-bound static scope to be
     * resolved by the '::' operator. (E.g., "static::foo".)
     *)
    if Token.kind (peek_token ~lookahead:1 parser) == TokenKind.ColonColon then
      parse_expression_statement parser
    else
      parse_function_static_declaration parser

  and parse_function_static_declaration parser =
    (* SPEC

    function-static-declaration:
      static static-declarator-list  ;

    static-declarator-list:
      static-declarator
      static-declarator-list  ,  static-declarator

    *)
    let (parser, static) = assert_token parser Static in
    let (parser, decls) = parse_comma_list
      parser Semicolon SyntaxError.error1008 parse_static_declarator in
    let (parser, semicolon) = require_semicolon parser in
    Make.function_static_statement parser static decls semicolon

  and parse_static_declarator parser =
    (* SPEC
        static-declarator:
          variable-name  function-static-initializer-opt
    *)
    (* TODO: ERROR RECOVERY not very sophisticated here *)
    let (parser, variable_name) = require_variable parser in
    let (parser, init) = parse_static_initializer_opt parser in
    Make.static_declarator parser variable_name init

  and parse_static_initializer_opt parser =
    (* SPEC
      function-static-initializer:
        = const-expression
    *)
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Equal ->
      (* TODO: Detect if expression is not const *)
      let (parser, equal) = Make.token parser1 token in
      let (parser, value) = parse_expression parser in
      Make.simple_initializer parser equal value
    | _ -> Make.missing parser (pos parser)

  (* SPEC:
    TODO: update the spec to reflect that echo and print must be a statement
    echo-intrinsic:
      echo  expression
      echo  (  expression  )
      echo  expression-list-two-or-more

    expression-list-two-or-more:
      expression  ,  expression
      expression-list-two-or-more  ,  expression
  *)
  and parse_echo_statement parser =
    let parser, token = assert_token parser Echo in
    let parser, expression_list = parse_comma_list
      parser Semicolon SyntaxError.error1015 parse_expression
    in
    let parser, semicolon = require_semicolon parser in
    Make.echo_statement parser token expression_list semicolon

  and parse_expression_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon ->
      let (parser, missing) = Make.missing parser1 (pos parser) in
      let (parser, token) = Make.token parser token in
      Make.expression_statement parser missing token
    | _ ->
      let parser = Parser.expect_in_new_scope parser [ Semicolon ] in
      let (parser, expression) = parse_expression parser in
      let (parser, token) =
        let (parser, token) = require_semicolon_token parser in
        match token with
        | Some t ->
          if SC.is_halt_compiler_expression expression then
            let (parser, token) = rescan_halt_compiler parser t in
            Make.token parser token
          else
            Make.token parser t
        | None ->
          Make.missing parser (pos parser)
      in
      let parser = Parser.pop_scope parser [ Semicolon ] in
      Make.expression_statement parser expression token

  and parse_compound_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon -> Make.token parser1 token
    | _ ->
      let (parser, left_brace_token) = require_left_brace parser in
      let (parser, statement_list) =
        parse_terminated_list parser parse_statement RightBrace
      in
      let (parser, right_brace_token) = require_right_brace parser in
      Make.compound_statement parser left_brace_token statement_list
        right_brace_token

  and parse_alternate_loop_statement parser ~terminator =
    let (parser, colon_token) = assert_token parser Colon in
    let (parser, statement_list) =
      parse_terminated_list parser parse_statement terminator in
    let (parser, terminate_token) = require_token parser terminator
      (SyntaxError.error1059 terminator) in
    let (parser, semicolon_token) = require_semicolon parser in
    Make.alternate_loop_statement
      parser
      colon_token
      statement_list
      terminate_token
      semicolon_token

end
end (* WithSmartConstructors *)
end (* WithSyntax *)
