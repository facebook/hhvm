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

module WithExpressionAndDeclParser
  (ExpressionParser : Full_fidelity_expression_parser_type.ExpressionParserType)
  (DeclParser : Full_fidelity_declaration_parser_type.DeclarationParserType) :
  Full_fidelity_statement_parser_type.StatementParserType = struct

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
    let (lexer, token) = Lexer.next_token parser.lexer in
    let parser = { parser with lexer } in
    (parser, token)

  (* let skip_token parser =
    let (lexer, _) = Lexer.next_token parser.lexer in
    let parser = { parser with lexer } in
    parser *)

  (* let next_token_as_name parser =
    let (lexer, token) = Lexer.next_token_as_name parser.lexer in
    let parser = { parser with lexer } in
    (parser, token) *)

  let peek_token parser =
    let (_, token) = Lexer.next_token parser.lexer in
    token

  let optional_token parser kind =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, make_token token)
    else
      (parser, make_missing())

  let expect_token parser kind error =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (with_error parser error, (make_missing()))

  let assert_token parser kind =
    let (parser, token) = next_token parser in
    assert ((Token.kind token) = kind);
    (parser, make_token token)


  let rec parse_statement parser =
    let token = peek_token parser in
    match (Token.kind token) with
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
    | Default -> parse_default_label_statement parser
    | Case -> parse_case_label_statement parser
    | LeftBrace -> parse_compound_statement parser
    | Static -> parse_function_static_declaration parser
    | _ -> parse_expression_statement parser

  (* Helper: parses ( expr ) *)
  and parse_paren_expr parser =
    let (parser, left_paren) =
      expect_token parser LeftParen SyntaxError.error1019 in
    let (parser, expr_syntax) = parse_expression parser in
    let (parser, right_paren) =
      expect_token parser RightParen SyntaxError.error1011 in
    (parser, left_paren, expr_syntax, right_paren)

  (* List of expressions and commas. No trailing comma. *)
  and parse_for_expr_group parser =
    let rec aux parser acc =
      let (parser, expr) = parse_expression parser in
      let acc = expr :: acc in
      let (parser1, token) = next_token parser in
      match (Token.kind token) with
      | Comma -> aux parser1 ((make_token token) :: acc )
      | RightParen -> (parser, acc)
      | _ ->
        let parser = with_error parser SyntaxError.error1009 in
        (parser, acc) in
    let (parser, expressions_and_commas) = aux parser [] in
    (parser, make_list (List.rev expressions_and_commas))

  and parse_for_expr parser =
    let token = peek_token parser in
    let parser, for_expr_group = match Token.kind token with
      | Semicolon -> parser, make_missing ()
      | _ -> parse_for_expr_group parser
    in
    let parser, semicolon =
      expect_token parser Semicolon SyntaxError.error1010 in
    parser, for_expr_group, semicolon

  and parse_last_for_expr parser =
    let token = peek_token parser in
    let parser, for_expr_group = match Token.kind token with
      | RightParen -> parser, make_missing ()
      | _ -> parse_for_expr_group parser
    in
    (parser, for_expr_group)

  and parse_for_statement parser =
    let parser, for_keyword_token =
      assert_token parser For in
    let parser, for_left_paren =
      expect_token parser LeftParen SyntaxError.error1019 in
    let parser, for_initializer_expr, for_first_semicolon =
      parse_for_expr parser in
    let parser, for_control_expr, for_second_semicolon =
      parse_for_expr parser in
    let parser, for_end_of_loop_expr =
      parse_last_for_expr parser in
    let parser, for_right_paren =
      expect_token parser RightParen SyntaxError.error1011 in
    let parser, for_statement =
      parse_statement parser in
    let syntax = make_for_statement for_keyword_token for_left_paren
      for_initializer_expr for_first_semicolon for_control_expr
      for_second_semicolon for_end_of_loop_expr for_right_paren for_statement
    in
    (parser, syntax)

  and parse_foreach_statement parser =
    let parser, foreach_keyword_token = assert_token parser Foreach in
    let parser, foreach_left_paren =
      expect_token parser LeftParen SyntaxError.error1019 in
    let parser, foreach_collection_name = parse_expression parser in
    let parser, await_token = optional_token parser Await in
    let parser, as_token = expect_token parser As SyntaxError.error1023 in
    let (parser1, token) = next_token parser in
    let (parser, after_as) =
      match Token.kind token with
      | List ->
        (* TODO need to handle list intrinsic. For now just create error *)
        let rec aux parser acc =
          let (parser1, token) = next_token parser in
          match Token.kind token with
          | RightParen -> (parser, make_list (List.rev acc))
          | _ ->
            let error = make_error [make_token token] in
            aux parser1 (error :: acc)
        in
        aux parser []
      | _ -> parse_expression parser
    in
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
        (parser, after_as, make_error [make_token token], foreach_value)
    in
    let parser, right_paren_token =
      expect_token parser RightParen SyntaxError.error1011 in
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
    let (parser, do_while_keyword_token) =
      expect_token parser While SyntaxError.error1018 in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, do_semicolon_token) =
      expect_token parser Semicolon SyntaxError.error1010 in
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
    let (parser, switch_keyword_token) =
      assert_token parser Switch in
    let (parser, left_paren_token, expr_node, right_paren_token) =
      parse_paren_expr parser in
    let (parser, statement_node) =
      parse_compound_statement parser in
    let syntax = make_switch_statement switch_keyword_token left_paren_token
      expr_node right_paren_token statement_node in
    (parser, syntax)

  and parse_try_statement parser =
    let parse_catch_clause_opt parser_catch =
      let (parser_catch, catch_token) = optional_token parser_catch Catch in
      match syntax catch_token with
      | Missing -> (parser_catch, catch_token)
      | _ ->
      (* catch  (  parameter-declaration-list  )  compound-statement *)
        let (parser_catch, left_paren) =
          expect_token parser_catch LeftParen SyntaxError.error1019 in
        let (parser_catch, param_decl) =
          parse_parameter_list_opt parser_catch in
        let (parser_catch, right_paren) =
          expect_token parser_catch RightParen SyntaxError.error1011 in
        let (parser_catch, compound_stmt) =
          parse_compound_statement parser_catch in
        let catch_clause = make_catch_clause catch_token left_paren param_decl
          right_paren compound_stmt in
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
    (* TODO ERROR RECOVERY: give an error for missing both catch and finally *)
    let syntax = make_try_statement try_keyword_token try_compound_stmt
      catch_clauses finally_clause in
    (parser, syntax)

  and parse_break_statement parser =
    let (parser, break_token) = assert_token parser Break in
    let (parser, semi_token) =
      expect_token parser Semicolon SyntaxError.error1010 in
    (parser, make_break_statement break_token semi_token)

  and parse_continue_statement parser =
    let (parser, continue_token) = assert_token parser Continue in
    let (parser, semi_token) =
      expect_token parser Semicolon SyntaxError.error1010 in
    (parser, make_continue_statement continue_token semi_token)

  and parse_return_statement parser =
    let (parser, return_token) = assert_token parser Return in
    let (parser1, semi_token) = next_token parser in
    if Token.kind semi_token = Semicolon then
      (parser1, make_return_statement
        return_token (make_missing()) (make_token semi_token))
    else
      let (parser, expr) = parse_expression parser in
      let (parser, semi_token) =
        expect_token parser Semicolon SyntaxError.error1010 in
      (parser, make_return_statement return_token expr semi_token)

  and parse_throw_statement parser =
    let (parser, throw_token) = assert_token parser Throw in
    let (parser, expr) = parse_expression parser in
    let (parser, semi_token) =
      expect_token parser Semicolon SyntaxError.error1010 in
    (parser, make_throw_statement throw_token expr semi_token)

  and parse_default_label_statement parser =
    (* TODO: Only valid inside switch *)
    let (parser, default_token) = assert_token parser Default in
    let (parser, colon_token) =
      expect_token parser Colon SyntaxError.error1020 in
    let (parser, stmt) = parse_statement parser in
    (parser, make_default_statement default_token colon_token stmt)

  and parse_case_label_statement parser =
    (* TODO: Only valid inside switch *)
    let (parser, case_token) = assert_token parser Case in
    let (parser, expr) = parse_expression parser in
    let (parser, colon_token) =
      expect_token parser Colon SyntaxError.error1020 in
    let (parser, stmt) = parse_statement parser in
    (parser, make_case_statement case_token expr colon_token stmt)

  and parse_function_static_declaration parser =
    (* TODO *)
    let (parser, token) = next_token parser in
    (parser, make_error [make_token token])

  and parse_expression_statement parser =
    let (parser1, token) = next_token parser in
    match Token.kind token with
    | Semicolon ->
      (parser1, make_expression_statement (make_missing ()) (make_token token))
    | _ ->
      let (parser, expression) = parse_expression parser in
      let (parser, token) =
        expect_token parser Semicolon SyntaxError.error1010 in
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
    let (parser, left_brace_token) =
      expect_token parser LeftBrace SyntaxError.error1005 in
    let (parser, statement_list) = parse_statement_list_opt parser in
    let (parser, right_brace_token) =
      expect_token parser RightBrace SyntaxError.error1006 in
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

and parse_parameter_list_opt parser =
  let declaration_parser = DeclParser.make parser.lexer parser.errors in
  let (declaration_parser, node) =
    DeclParser.parse_parameter_list_opt declaration_parser in
  let lexer = DeclParser.lexer declaration_parser in
  let errors = DeclParser.errors declaration_parser in
  let parser = make lexer errors in
  (parser, node)


end
