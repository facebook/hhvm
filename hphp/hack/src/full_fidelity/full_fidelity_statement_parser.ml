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
module SimpleParser = Full_fidelity_simple_parser.WithLexer(Full_fidelity_lexer)

open TokenKind
open Full_fidelity_minimal_syntax

module WithExpressionAndTypeParser
  (ExpressionParser : Full_fidelity_expression_parser_type.ExpressionParserType)
  (TypeParser : Full_fidelity_type_parser_type.TypeParserType) :
  Full_fidelity_statement_parser_type.StatementParserType = struct

  include SimpleParser
  include Full_fidelity_parser_helpers.WithParser(SimpleParser)

  let rec parse_statement parser =
    match peek_token_kind parser with
    | For -> parse_for_statement parser
    | Foreach -> parse_foreach_statement parser
    | Do -> parse_do_statement parser
    | While -> parse_while_statement parser
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
    | _ -> parse_expression_statement parser

  (* Helper: parses ( expr ) *)
  and parse_paren_expr parser =
    let (parser, left_paren) = expect_left_paren parser in
    let (parser, expr_syntax) = parse_expression parser in
    let (parser, right_paren) = expect_right_paren parser in
    (parser, left_paren, expr_syntax, right_paren)

  (* List of expressions and commas. No trailing comma. *)
  and parse_for_expr_group parser is_last =
    let rec aux parser acc =
      let (parser, expr) = parse_expression parser in
      let acc = expr :: acc in
      let (parser1, token) = next_token parser in
      match (Token.kind token) with
      | Comma -> aux parser1 ((make_token token) :: acc)
      | RightParen when is_last -> (parser, acc)
      | Semicolon when not is_last -> (parser, acc)
      (* TODO a similar error is reported by caller, should we duplicate? *)
      | _ when is_last ->
        let parser = with_error parser SyntaxError.error1009 in
        (parser, acc)
      | _ ->
        let parser = with_error parser SyntaxError.error1024 in
        (parser, acc)
    in
    let (parser, expressions_and_commas) = aux parser [] in
    (parser, make_list (List.rev expressions_and_commas))

  and parse_for_expr parser =
    let token = peek_token parser in
    let parser, for_expr_group = match Token.kind token with
      | Semicolon -> parser, make_missing ()
      | _ -> parse_for_expr_group parser false
    in
    let parser, semicolon = expect_semicolon parser in
    parser, for_expr_group, semicolon

  and parse_last_for_expr parser =
    let token = peek_token parser in
    let parser, for_expr_group = match Token.kind token with
      | RightParen -> parser, make_missing ()
      | _ -> parse_for_expr_group parser true
    in
    (parser, for_expr_group)

  and parse_for_statement parser =
    let parser, for_keyword_token = assert_token parser For in
    let parser, for_left_paren = expect_left_paren parser in
    let parser, for_initializer_expr, for_first_semicolon =
      parse_for_expr parser in
    let parser, for_control_expr, for_second_semicolon =
      parse_for_expr parser in
    let parser, for_end_of_loop_expr = parse_last_for_expr parser in
    let parser, for_right_paren = expect_right_paren parser in
    let parser, for_statement = parse_statement parser in
    let syntax = make_for_statement for_keyword_token for_left_paren
      for_initializer_expr for_first_semicolon for_control_expr
      for_second_semicolon for_end_of_loop_expr for_right_paren for_statement
    in
    (parser, syntax)

  and parse_foreach_statement parser =
    let parser, foreach_keyword_token = assert_token parser Foreach in
    let parser, foreach_left_paren = expect_left_paren parser in
    let parser, foreach_collection_name = parse_expression parser in
    let parser, await_token = optional_token parser Await in
    let parser, as_token = expect_as parser in
    (* let (parser1, token) = next_token parser in *)
    let (parser, after_as) = parse_expression parser in
    (* let parser, expr = parse_expression parser in *)
    let (parser, foreach_key, foreach_arrow, foreach_value) =
    match Token.kind (peek_token parser) with
      | RightParen ->
        (parser, make_missing (), make_missing (), after_as)
      | EqualGreaterThan ->
        let parser, arrow = assert_token parser EqualGreaterThan in
        let parser, value = parse_expression parser in
        (parser, after_as, arrow, value)
      | _ ->
        (* TODO ERROR RECOVERY. Now assumed that the arrow is missing
         * and goes on to parse the next expression *)
        let parser, token = next_token parser in
        let parser, foreach_value = parse_expression parser in
        (parser, after_as, make_error (make_token token), foreach_value)
    in
    let parser, right_paren_token = expect_right_paren parser in
    let parser, foreach_statement = parse_statement parser in
    let syntax =
      make_foreach_statement foreach_keyword_token foreach_left_paren
      foreach_collection_name await_token as_token foreach_key foreach_arrow
      foreach_value right_paren_token foreach_statement in
    (parser, syntax)

  and parse_do_statement parser =
    let (parser, do_keyword_token) =
      assert_token parser Do in
    let (parser, statement_node) =
      parse_statement parser in
    let (parser, do_while_keyword_token) = expect_while parser in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, do_semicolon_token) = expect_semicolon parser in
    let syntax = make_do_statement do_keyword_token statement_node
      do_while_keyword_token left_paren_token expr_node right_paren_token
      do_semicolon_token in
    (parser, syntax)

  and parse_while_statement parser =
    let (parser, while_keyword_token) =
      assert_token parser While in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, statement_node) =
      parse_statement parser in
    let syntax = make_while_statement while_keyword_token left_paren_token
      expr_node right_paren_token statement_node in
    (parser, syntax)

  and parse_unset_statement parser =
    (*
    TODO: This is listed as unsupported in Hack in the spec; is that true?
    TODO: If it is formally supported in Hack then update the spec; if not
    TODO: then should we make it illegal in strict mode?
    TODO: Can the list be comma-terminated?
    TODO: Can the list be empty?
    TODO: The list has to be expressions which evaluate as variables;
          add an error checking pass.
    *)
    let (parser, keyword) = assert_token parser Unset in
    let (parser, left_paren, variables, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing
        parser parse_expression in
    let (parser, semi) = expect_semicolon parser in
    let result = make_unset_statement
      keyword left_paren variables right_paren semi in
    (parser, result)

  and parse_if_statement parser =
    (* parses the "( expr ) statement" segment of If, Elseif or Else clauses.
     * Return a tuple of 5 elements, the first one being the resultant parser
     *)
    let parse_if_body_helper parser_body =
      let (parser_body, left_paren_token, expr_node, right_paren_token) =
        parse_paren_expr parser_body in
      let (parser_body, statement_node) = parse_statement parser_body in
        (parser_body, left_paren_token, expr_node, right_paren_token,
        statement_node)
    in
    (* Parse a elseif clause. Do not eat token and return Missing if the
     * first keyword is not elseif
     *)
    let parse_elseif_opt parser_elseif =
      let (parser_elseif, elseif_token) = optional_token parser_elseif Elseif in
      match syntax elseif_token with
      | Missing -> (parser_elseif, elseif_token)  (* return original parser *)
      | _ ->
        let (parser_elseif, elseif_left_paren, elseif_condition_expr,
          elseif_right_paren, elseif_statement) =
          parse_if_body_helper parser_elseif in
        let elseif_syntax = make_elseif_clause elseif_token elseif_left_paren
          elseif_condition_expr elseif_right_paren elseif_statement in
        (parser_elseif, elseif_syntax)
    in
    (* do not eat token and return Missing if first token is not Else *)
    let parse_else_opt parser_else =
      let (parser_else, else_token) = optional_token parser_else Else in
      match syntax else_token with
      | Missing ->(parser_else, else_token)
      | _ ->
        let (parser_else, else_consequence) = parse_statement parser_else in
        let else_syntax = make_else_clause else_token else_consequence in
        (parser_else, else_syntax)
    in
    (* keep parsing until there is no else if clause
     * return the new parser and a syntax list of all else if statements
     * return Missing if there is no Elseif, with parser unchanged
     *)
    let parse_elseif_clauses parser_elseif =
      let rec parse_clauses_helper acc parser_elseif =
        let (parser_elseif, elseif_syntax) = parse_elseif_opt parser_elseif in
        match (syntax elseif_syntax, acc) with
        | Missing, [] -> (parser_elseif, elseif_syntax)
        | Missing, _ -> (parser_elseif, make_list (List.rev acc))
        | _, _ -> parse_clauses_helper (elseif_syntax :: acc) parser_elseif
      in
      parse_clauses_helper [] parser_elseif
    in
    let (parser, if_keyword_token) = assert_token parser If in
    let (parser, if_left_paren, if_expr, if_right_paren, if_consequence) =
      parse_if_body_helper parser in
    let (parser, elseif_syntax) = parse_elseif_clauses parser in
    let (parser, else_syntax) = parse_else_opt parser in
    let syntax = make_if_statement if_keyword_token if_left_paren if_expr
      if_right_paren if_consequence elseif_syntax else_syntax in
    (parser, syntax)
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
    let (parser, left_brace_token) = expect_left_brace parser in
    (* TODO: I'm not convinced that this always terminates in some cases.
    Check that. *)
    let (parser, section_list) =
      parse_terminated_list parser parse_switch_section RightBrace in
    let (parser, right_brace_token) = expect_right_brace parser in
    let syntax = make_switch_statement switch_keyword_token left_paren_token
      expr_node right_paren_token left_brace_token section_list
      right_brace_token in
    (parser, syntax)

  and parse_switch_section parser =
    (* See parse_switch_statement for grammar *)
    let (parser, labels) =
      parse_list_until_none parser parse_switch_section_label in
    let parser = if is_missing labels then
        with_error parser SyntaxError.error2008
      else
        parser in
    let (parser, statements) =
      parse_list_until_none parser parse_switch_section_statement in
    let result = make_switch_section labels statements in
    (parser, result)

  and parse_switch_section_statement parser =
    match peek_token_kind parser with
    | Default
    | Case
    | RightBrace
    | EndOfFile -> (parser, None)
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

  and parse_try_statement parser =
    let parse_catch_clause_opt parser_catch =
      let (parser_catch, catch_token) = optional_token parser_catch Catch in
      match syntax catch_token with
      | Missing -> (parser_catch, catch_token)
      | _ ->
      (* SPEC
        catch  (  type-specification variable-name  )  compound-statement
      *)
        let (parser_catch, left_paren) = expect_left_paren parser_catch in
        let (parser_catch, catch_type) = parse_type_specifier parser_catch in
        let (parser_catch, catch_var) = expect_variable parser_catch in
        let (parser_catch, right_paren) = expect_right_paren parser_catch in
        let (parser_catch, compound_stmt) =
          parse_compound_statement parser_catch in
        let catch_clause = make_catch_clause catch_token left_paren
          catch_type catch_var right_paren compound_stmt in
        (parser_catch, catch_clause)
    in
    let parse_finally_clause_opt parser_f =
      let (parser_f, finally_token) = optional_token parser_f Finally in
      match syntax finally_token with
      | Missing -> (parser_f, finally_token)
      | _ ->
        let (parser_f, compound_stmt) = parse_compound_statement parser_f in
        let finally_clause = make_finally_clause finally_token compound_stmt in
        (parser_f, finally_clause)
    in
    let parse_catch_clauses parser_catch =
      let rec aux acc parser_catch =
        let (parser_catch, catch_clause) =
          parse_catch_clause_opt parser_catch in
        match (syntax catch_clause, acc) with
        | Missing, [] -> (parser_catch, catch_clause)
        | Missing, _ -> (parser_catch, acc |> List.rev |> make_list)
        | _, _ -> aux (catch_clause :: acc) parser_catch
      in
      aux [] parser_catch
    in
    let (parser, try_keyword_token) = assert_token parser Try in
    let (parser, try_compound_stmt) = parse_compound_statement parser in
    let (parser, catch_clauses) = parse_catch_clauses parser in
    let (parser, finally_clause) = parse_finally_clause_opt parser in
    (* If the catch and finally are both missing then we give an error in
       a later pass. *)
    let syntax = make_try_statement try_keyword_token try_compound_stmt
      catch_clauses finally_clause in
    (parser, syntax)

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
      if peek_token_kind parser = Semicolon then (parser, (make_missing()))
      else parse_expression parser in
    let (parser, semi_token) = expect_semicolon parser in
    let result = make_break_statement break_token level semi_token in
    (parser, result)

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
      if peek_token_kind parser = Semicolon then (parser, (make_missing()))
      else parse_expression parser in
    let (parser, semi_token) = expect_semicolon parser in
    let result = make_continue_statement continue_token level semi_token in
    (parser, result)

  and parse_return_statement parser =
    let (parser, return_token) = assert_token parser Return in
    let (parser1, semi_token) = next_token parser in
    if Token.kind semi_token = Semicolon then
      (parser1, make_return_statement
        return_token (make_missing()) (make_token semi_token))
    else
      let (parser, expr) = parse_expression parser in
      let (parser, semi_token) = expect_semicolon parser in
      (parser, make_return_statement return_token expr semi_token)

  and parse_throw_statement parser =
    let (parser, throw_token) = assert_token parser Throw in
    let (parser, expr) = parse_expression parser in
    let (parser, semi_token) = expect_semicolon parser in
    (parser, make_throw_statement throw_token expr semi_token)

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
    let (parser, semi_token) = optional_token parser Semicolon in
    let (parser, colon_token) =
      if is_missing semi_token then
        expect_colon parser
      else
        (parser, semi_token) in
    let result = make_default_label default_token colon_token in
    (parser, result)

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
    let (parser, semi_token) = optional_token parser Semicolon in
    let (parser, colon_token) =
      if is_missing semi_token then
        expect_colon parser
      else
        (parser, semi_token) in
    let result = make_case_label case_token expr colon_token in
    (parser, result)

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
    if peek_token_kind parser1 = TokenKind.Variable then
      let (parser, variables) = parse_comma_list
        parser1 Semicolon SyntaxError.error1008 expect_variable in
      let (parser, semicolon) = expect_semicolon parser in
      let result = make_global_statement keyword variables semicolon in
      (parser, result)
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
    let (parser, semicolon) = expect_semicolon parser in
    let result = make_function_static_statement static decls semicolon in
    (parser, result)

  and parse_static_declarator parser =
    (* SPEC
        static-declarator:
          variable-name  function-static-initializer-opt
    *)
    (* TODO: ERROR RECOVERY not very sophisticated here *)
    let (parser, variable_name) = expect_variable parser in
    let (parser, init) = parse_static_initializer_opt parser in
    let result = make_static_declarator variable_name init in
    (parser, result)

  and parse_static_initializer_opt parser =
    (* SPEC
      function-static-initializer:
        = const-expression
    *)
    let (parser1, token) = next_token parser in
    match (Token.kind token) with
    | Equal ->
      (* TODO: Detect if expression is not const *)
      let equal = make_token token in
      let (parser, value) = parse_expression parser1 in
      (parser, make_simple_initializer equal value)
    | _ -> (parser, make_missing())

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
    let parser, semicolon = expect_semicolon parser in
    let syntax = make_echo_statement token expression_list semicolon in
    (parser, syntax)

  and parse_expression_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon ->
      (parser1, make_expression_statement (make_missing ()) (make_token token))
    | _ ->
      let (parser, expression) = parse_expression parser in
      let (parser, token) = expect_semicolon parser in
      (parser, make_expression_statement expression token)

  and parse_statement_list_opt parser =
     let rec aux parser acc =
       let token = peek_token parser in
       match (Token.kind token) with
       | RightBrace
       | EndOfFile -> (parser, acc)
       | _ ->
         let (parser, statement) = parse_statement parser in
         aux parser (statement :: acc) in
     let (parser, statements) = aux parser [] in
     let statements = List.rev statements in
     (parser, make_list statements)

  and parse_compound_statement parser =
    let (parser, left_brace_token) = expect_left_brace parser in
    let (parser, statement_list) = parse_statement_list_opt parser in
    let (parser, right_brace_token) = expect_right_brace parser in
    let syntax = make_compound_statement
      left_brace_token statement_list right_brace_token in
    (parser, syntax)

  and parse_expression parser =
    let expression_parser = ExpressionParser.make parser.lexer parser.errors in
    let (expression_parser, node) =
      ExpressionParser.parse_expression expression_parser in
    let lexer = ExpressionParser.lexer expression_parser in
    let errors = ExpressionParser.errors expression_parser in
    let parser = make lexer errors in
    (parser, node)

and parse_type_specifier parser =
  let type_parser = TypeParser.make parser.lexer parser.errors in
  let (type_parser, node) =
    TypeParser.parse_type_specifier type_parser in
  let lexer = TypeParser.lexer type_parser in
  let errors = TypeParser.errors type_parser in
  let parser = make lexer errors in
  (parser, node)

end
