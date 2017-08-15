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

module WithExpressionAndDeclAndTypeParser
  (ExpressionParser : Full_fidelity_expression_parser_type.ExpressionParserType)
  (DeclParser : Full_fidelity_declaration_parser_type.DeclarationParserType)
  (TypeParser : Full_fidelity_type_parser_type.TypeParserType) :
  Full_fidelity_statement_parser_type.StatementParserType = struct

  include SimpleParser
  include Full_fidelity_parser_helpers.WithParser(SimpleParser)

  let rec parse_statement parser =
    match peek_token_kind parser with
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
      parse_markup_section parser ~is_leading_section:false
    | Semicolon -> parse_expression_statement parser
    (* ERROR RECOVERY: when encountering a token that's invalid now but the
     * context says is expected later, make the whole statement missing
     * and continue on, starting at the unexpected token. *)
    (* TODO T20390825: Make sure this this won't cause premature recovery. *)
    | kind when SimpleParser.expects parser kind ->
      (parser, make_missing ())
    | _ -> parse_expression_statement parser

  and parse_markup_section parser ~is_leading_section =
    let parser, prefix =
      (* for markup section at the beginning of the file
         treat ?> as a part of markup text *)
      if not is_leading_section
        && peek_token_kind parser = TokenKind.QuestionGreaterThan then
        let (parser, prefix) = next_token parser in
        parser, make_token prefix
      else
        parser, make_missing ()
    in
    let parser, markup, suffix_opt = scan_markup parser ~is_leading_section in
    let markup = make_token markup in
    let suffix, is_echo_tag =
      match suffix_opt with
      | Some (less_than_question, language_opt) ->
        let less_than_question_token = make_token less_than_question in
        (* if markup section ends with <?= tag
           then script section embedded between tags should be treated as if it
           will be an argument to 'echo'. Technically it should be restricted to
           expression but since it permits trailing semicolons we parse it as
           expression statement.
           TODO: consider making it even more loose and parse it as declaration
           for better error recovery in cases when user
           accidentally type '<?=' instead of '<?php' so declaration in script
           section won't throw parser off the rails. *)
        let language, is_echo_tag =
          match language_opt with
          | Some language ->
            make_token language, (Token.kind language = TokenKind.Equal)
          | None -> make_missing (), false
        in
        make_markup_suffix less_than_question_token language,
        is_echo_tag
      | None -> make_missing (), false
    in
    let parser, expression =
      if is_echo_tag then parse_statement parser else parser, make_missing()
    in
    let s = make_markup_section prefix markup suffix expression in
    parser, s

  and parse_php_function parser =
    use_decl_parser DeclParser.parse_function parser

  and parse_possible_php_function parser =
    (* ERROR RECOVERY: PHP supports nested named functions, but Hack does not.
    (Hack only supports anonymous nested functions as expressions.)

    If we have a statement beginning with function left-paren, then parse it
    as a statement expression beginning with an anonymous function; it will
    then have to end with a semicolon.

    If it starts with something else, parse it as a function.

    TODO: Give an error for nested nominal functions in a later pass.

    *)
    if peek_token_kind ~lookahead:1 parser = LeftParen then
      parse_expression_statement parser
    else
      parse_php_function parser

  and parse_php_class parser =
    (* PHP allows classes nested inside of functions, but hack does not *)
    (* TODO check for hack error: no classish declarations inside functions *)
    let f decl_parser =
      DeclParser.parse_classish_declaration decl_parser (make_missing ()) in
    use_decl_parser f parser

  and use_decl_parser
      (f : DeclParser.t -> DeclParser.t * Full_fidelity_minimal_syntax.t)
      parser =
    let decl_parser = DeclParser.make parser.lexer
      parser.errors parser.context in
    let decl_parser, node = f decl_parser in
    let lexer = DeclParser.lexer decl_parser in
    let errors = DeclParser.errors decl_parser in
    let parser = { parser with lexer; errors } in
    parser, node

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
    let parser, for_statement = parse_statement parser in
    let syntax = make_for_statement for_keyword_token for_left_paren
      for_initializer_expr for_first_semicolon for_control_expr
      for_second_semicolon for_end_of_loop_expr for_right_paren for_statement
    in
    (parser, syntax)

  and parse_foreach_statement parser =
    let parser, foreach_keyword_token = assert_token parser Foreach in
    let parser, foreach_left_paren = require_left_paren parser in
    let parser, foreach_collection_name = parse_expression parser in
    let parser, await_token = optional_token parser Await in
    let parser, as_token = require_as parser in
    (* let (parser1, token) = next_token parser in *)
    let (parser, after_as) = parse_expression parser in
    let parser = SimpleParser.expect_in_new_scope parser [ RightParen ] in
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
    let parser, right_paren_token = require_right_paren parser in
    let parser = SimpleParser.pop_scope parser [ RightParen ] in
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
    let (parser, do_while_keyword_token) = require_while parser in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, do_semicolon_token) = require_semicolon parser in
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
    TODO: Unset is case-insentive. Should non-lowercase be an error?
    *)
    let (parser, keyword) = assert_token parser Unset in
    let (parser, left_paren, variables, right_paren) =
      parse_parenthesized_comma_list_opt_allow_trailing
        parser parse_expression in
    let (parser, semi) = require_semicolon parser in
    let result = make_unset_statement
      keyword left_paren variables right_paren semi in
    (parser, result)

  and parse_if_statement parser =
    (* SPEC:
  if-statement:
    if   (   expression   )   statement   elseif-clauses-opt   else-clause-opt

  elseif-clauses:
    elseif-clause
    elseif-clauses   elseif-clause

  elseif-clause:
    elseif   (   expression   )   statement

  else-clause:
    else   statement

    *)

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
    let parse_elseif_opt parser_elseif =
      if peek_token_kind parser_elseif = Elseif then
        let (parser_elseif, elseif_token) = assert_token parser_elseif Elseif in
        let (parser_elseif, elseif_left_paren, elseif_condition_expr,
          elseif_right_paren, elseif_statement) =
          parse_if_body_helper parser_elseif in
        let elseif_syntax = make_elseif_clause elseif_token elseif_left_paren
          elseif_condition_expr elseif_right_paren elseif_statement in
        (parser_elseif, Some elseif_syntax)
      else
        (parser_elseif, None)
    in
    (* do not eat token and return Missing if first token is not Else *)
    let parse_else_opt parser_else =
      let (parser_else, else_token) = optional_token parser_else Else in
      match syntax else_token with
      | Missing -> (parser_else, else_token)
      | _ ->
        let (parser_else, else_consequence) = parse_statement parser_else in
        let else_syntax = make_else_clause else_token else_consequence in
        (parser_else, else_syntax)
    in
    let (parser, if_keyword_token) = assert_token parser If in
    let (parser, if_left_paren, if_expr, if_right_paren, if_consequence) =
      parse_if_body_helper parser in
    let (parser, elseif_syntax) =
      parse_list_until_none parser parse_elseif_opt in
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
    let (parser, left_brace_token) = require_left_brace parser in
    (* TODO: I'm not convinced that this always terminates in some cases.
    Check that. *)
    let (parser, section_list) =
      parse_terminated_list parser parse_switch_section RightBrace in
    let (parser, right_brace_token) = require_right_brace parser in
    let syntax = make_switch_statement switch_keyword_token left_paren_token
      expr_node right_paren_token left_brace_token section_list
      right_brace_token in
    (parser, syntax)

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
    let result = make_switch_fallthrough keyword semi in
    (parser, result)

  and parse_switch_fallthrough_opt parser =
    if is_switch_fallthrough parser then
      parse_switch_fallthrough parser
    else
      (parser, (make_missing()))

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
    let (parser, fallthrough) = parse_switch_fallthrough_opt parser in
    let result = make_switch_section labels statements fallthrough in
    (parser, result)

  and parse_switch_section_statement parser =
    if is_switch_fallthrough parser then (parser, None)
    else match peek_token_kind parser with
    | Default
    | Case
    | RightBrace
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
    *)
    if peek_token_kind parser = Catch then
      let (parser, catch_token) = assert_token parser Catch in
      let (parser, left_paren) = require_left_paren parser in
      let (parser, catch_type) =
        match peek_token_kind parser with
        | TokenKind.Variable ->
          let parser = with_error parser SyntaxError.error1007 in
          parser, make_missing ()
        | _ ->
          let type_parser =
            TypeParser.make parser.lexer parser.errors parser.context
          in
          let (type_parser, node) =
            TypeParser.parse_type_specifier type_parser
          in
          let lexer = TypeParser.lexer type_parser in
          let errors = TypeParser.errors type_parser in
          { parser with lexer; errors }, node
      in
      let (parser, catch_var) = require_variable parser in
      let (parser, right_paren) = require_right_paren parser in
      let (parser, compound_stmt) = parse_compound_statement parser in
      let catch_clause = make_catch_clause catch_token left_paren
        catch_type catch_var right_paren compound_stmt in
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
      let finally_clause = make_finally_clause finally_token compound_stmt in
      (parser, finally_clause)
    else
      (parser, (make_missing()))

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
      parse_list_until_none parser parse_catch_clause_opt in
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
    let (parser, semi_token) = require_semicolon parser in
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
    let (parser, semi_token) = require_semicolon parser in
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
      let (parser, semi_token) = require_semicolon parser in
      (parser, make_return_statement return_token expr semi_token)

  and parse_goto_label parser =
    let parser, goto_label_name = next_token_as_name parser in
    let goto_label_name = make_token goto_label_name in
    let parser, colon = assert_token parser Colon in
    parser, make_goto_label goto_label_name colon

  and parse_goto_statement parser =
    let parser, goto = assert_token parser Goto in
    let parser, goto_label_name = next_token_as_name parser in
    let goto_label_name = make_token goto_label_name in
    let parser, semicolon = assert_token parser Semicolon in
    parser, make_goto_statement goto goto_label_name semicolon

  and parse_throw_statement parser =
    let (parser, throw_token) = assert_token parser Throw in
    let (parser, expr) = parse_expression parser in
    let (parser, semi_token) = require_semicolon parser in
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
        require_colon parser
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
        require_colon parser
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
    let is_global_statement =
      match peek_token_kind parser1 with
      | TokenKind.Variable | TokenKind.Dollar -> true
      | _ -> false
    in
    if is_global_statement then
      let (parser, variables) = parse_comma_list
        parser1 Semicolon SyntaxError.error1008 parse_simple_variable in
      let (parser, semicolon) = require_semicolon parser in
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
    let (parser, semicolon) = require_semicolon parser in
    let result = make_function_static_statement static decls semicolon in
    (parser, result)

  and parse_static_declarator parser =
    (* SPEC
        static-declarator:
          variable-name  function-static-initializer-opt
    *)
    (* TODO: ERROR RECOVERY not very sophisticated here *)
    let (parser, variable_name) = require_variable parser in
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
    let parser, semicolon = require_semicolon parser in
    let syntax = make_echo_statement token expression_list semicolon in
    (parser, syntax)

  and parse_expression_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon ->
      (parser1, make_expression_statement (make_missing ()) (make_token token))
    | _ ->
      let parser = SimpleParser.expect_in_new_scope parser [ Semicolon ] in
      let (parser, expression) = parse_expression parser in
      let (parser, token) = require_semicolon parser in
      let parser = SimpleParser.pop_scope parser [ Semicolon ] in
      (parser, make_expression_statement expression token)

  and parse_compound_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon -> (parser1, make_token token)
    | _ ->
      let (parser, left_brace_token) = require_left_brace parser in
      let (parser, statement_list) =
        parse_terminated_list parser parse_statement RightBrace in
      let (parser, right_brace_token) = require_right_brace parser in
      let syntax = make_compound_statement
        left_brace_token statement_list right_brace_token in
      (parser, syntax)

  and parse_expression parser =
    with_expression_parser parser ExpressionParser.parse_expression

  and parse_simple_variable parser =
    with_expression_parser parser ExpressionParser.parse_simple_variable

  and with_expression_parser parser f =
    let expression_parser = ExpressionParser.make parser.lexer
      parser.errors parser.context in
    let (expression_parser, node) = f expression_parser in
    let lexer = ExpressionParser.lexer expression_parser in
    let errors = ExpressionParser.errors expression_parser in
    let parser = { parser with lexer; errors } in
    (parser, node)

end
