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
module SyntaxError = Full_fidelity_syntax_error
module Context = Full_fidelity_parser_context
module MinimalTrivia = Full_fidelity_minimal_trivia

open Full_fidelity_minimal_syntax

module type ParserType = sig
  module Lexer : Full_fidelity_lexer_sig.Lexer_S
  type t
  val errors : t -> SyntaxError.t list
  val with_errors : t -> SyntaxError.t list -> t
  val lexer : t -> Lexer.t
  val with_lexer : t -> Lexer.t -> t
  val expect : t -> TokenKind.t list -> t
  val carry_extra : t -> Token.t -> t
  val carrying_extra: t -> bool
  val flush_extra : t -> t * MinimalTrivia.t list
end

module WithParser(Parser : ParserType) = struct

  let next_token parser =
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token lexer in
    let parser = Parser.with_lexer parser lexer in
    (* ERROR RECOVERY: Check if the parser's carring ExtraTokenError trivia.
     * If so, clear it and add it to the leading trivia of the current token. *)
    let (parser, token) =
      if Parser.carrying_extra parser then
        let (parser1, trivia_list) = Parser.flush_extra parser in
        (parser1, Token.prepend_to_leading token trivia_list)
      else (parser, token) in
    (parser, token)

  let next_token_no_trailing parser =
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token_no_trailing lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let next_docstring_header parser =
    let lexer = Parser.lexer parser in
    let (lexer, token, name) = Parser.Lexer.next_docstring_header lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token, name)

  let next_token_in_string parser name =
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token_in_string lexer name in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let peek_token ?(lookahead=0) parser =
    let rec lex_ahead lexer n =
      let (next_lexer, token) = Parser.Lexer.next_token lexer in
      match n with
      | 0 -> token
      | _ -> lex_ahead next_lexer (n-1)
    in
      lex_ahead (Parser.lexer parser) lookahead

  let next_token_as_name parser =
    (* TODO: This isn't right.  Pass flags to the lexer. *)
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_token_as_name lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let peek_token_kind ?(lookahead=0) parser =
    Token.kind (peek_token ~lookahead parser)

  let skip_token parser =
    let (parser, _) = next_token parser in
    parser

  let scan_markup parser ~is_leading_section =
    let (lexer, markup, suffix) =
      Parser.Lexer.scan_markup (Parser.lexer parser) ~is_leading_section
    in
    Parser.with_lexer parser lexer, markup, suffix

  let with_error parser message =
    (* TODO: Should be able to express errors on whole syntax node. *)
    (* TODO: Is this even right? Won't this put the error on the trivia? *)
    let lexer = Parser.lexer parser in
    let start_offset = Parser.Lexer.start_offset lexer in
    let end_offset = Parser.Lexer.end_offset lexer in
    let error = SyntaxError.make start_offset end_offset message in
    let errors = Parser.errors parser in
    Parser.with_errors parser (error :: errors)

  let current_token_text parser =
    let token = peek_token parser in
    let token_width = Token.width token in
    let token_str = Parser.Lexer.current_text_at
      (Parser.lexer parser) token_width 0 in
    token_str

  let process_next_as_extra ?(generate_error=true) parser =
    let parser =
      if generate_error then
        let extra_str = current_token_text parser in
        with_error parser (SyntaxError.error1057 extra_str)
      else parser in
    let (parser, token) = next_token parser in
    let parser = Parser.carry_extra parser token in
    parser

  let require_token parser kind error =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, make_token token)
    else
      (* ERROR RECOVERY: Look at the next token after this. Is it the one we
       * require? If so, process the current token as extra and return the next
       * one. Otherwise, create a missing token for what we required,
       * and continue on from the current token (don't skip it). *)
      let next_kind = peek_token_kind ~lookahead:1 parser in
      if next_kind = kind then
        let parser1 = process_next_as_extra parser in
        let (parser, token) = next_token parser1 in
        (parser, make_token token)
      else
        (with_error parser error, (make_missing()))

  let require_required parser =
    require_token parser TokenKind.Required SyntaxError.error1051

  let require_name parser =
    require_token parser TokenKind.Name SyntaxError.error1004

  let require_name_allow_keywords parser =
    let (parser1, token) = next_token_as_name parser in
    if (Token.kind token) = TokenKind.Name then
      (parser1, make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (with_error parser SyntaxError.error1004, (make_missing()))

  let next_xhp_category_name parser =
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_xhp_category_name lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  (* We have a number of issues involving xhp class names, which begin with
     a colon and may contain internal colons and dashes.  These are some
     helper methods to deal with them. *)

  let is_next_name parser =
    Parser.Lexer.is_next_name (Parser.lexer parser)

  let next_xhp_name parser =
    assert(is_next_name parser);
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_xhp_name lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let is_next_xhp_class_name parser =
    Parser.Lexer.is_next_xhp_class_name (Parser.lexer parser)

  let next_xhp_class_name parser =
    assert(is_next_xhp_class_name parser);
    let lexer = Parser.lexer parser in
    let (lexer, token) = Parser.Lexer.next_xhp_class_name lexer in
    let parser = Parser.with_lexer parser lexer in
    (parser, token)

  let require_xhp_class_name parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      (parser, make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (* TODO: Different error? *)
      (with_error parser SyntaxError.error1004, (make_missing()))

  let require_xhp_name parser =
    if is_next_name parser then
      let (parser, token) = next_xhp_name parser in
      (parser, make_token token)
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (* TODO: Different error? *)
      (with_error parser SyntaxError.error1004, (make_missing()))

  let require_class_name parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      (parser, make_token token)
    else
      require_name_allow_keywords parser

  let next_xhp_class_name_or_other parser =
    if is_next_xhp_class_name parser then next_xhp_class_name parser
    else next_token parser

  let is_next_xhp_category_name parser =
    Parser.Lexer.is_next_xhp_category_name (Parser.lexer parser)

  let next_xhp_children_name_or_other parser =
    if is_next_xhp_category_name parser then
      next_xhp_category_name parser
    else
      next_xhp_class_name_or_other parser

  (* We accept either a Name or a QualifiedName token when looking for a
     qualified name. *)
  let require_qualified_name parser =
    (* TODO: What if the name is a keyword? *)
    let (parser1, name) = next_token parser in
    match Token.kind name with
    | TokenKind.QualifiedName
    | TokenKind.Name -> (parser1, make_token name)
    | _ ->
      (with_error parser SyntaxError.error1004, (make_missing()))

  let require_function parser =
    require_token parser TokenKind.Function SyntaxError.error1003

  let require_variable parser =
    require_token parser TokenKind.Variable SyntaxError.error1008

  let require_semicolon parser =
    require_token parser TokenKind.Semicolon SyntaxError.error1010

  let require_colon parser =
    require_token parser TokenKind.Colon SyntaxError.error1020

  let require_left_brace parser =
    require_token parser TokenKind.LeftBrace SyntaxError.error1034

  let require_right_brace parser =
    require_token parser TokenKind.RightBrace SyntaxError.error1006

  let require_left_paren parser =
    require_token parser TokenKind.LeftParen SyntaxError.error1019

  let require_right_paren parser =
    require_token parser TokenKind.RightParen SyntaxError.error1011

  let require_left_angle parser =
    require_token parser TokenKind.LessThan SyntaxError.error1021

  let require_right_angle parser =
    require_token parser TokenKind.GreaterThan SyntaxError.error1013

  let require_right_double_angle parser =
    require_token parser TokenKind.GreaterThanGreaterThan SyntaxError.error1029

  let require_left_bracket parser =
    require_token parser TokenKind.LeftBracket SyntaxError.error1026

  let require_right_bracket parser =
    require_token parser TokenKind.RightBracket SyntaxError.error1032

  let require_equal parser =
    require_token parser TokenKind.Equal SyntaxError.error1036

  let require_arrow parser =
    require_token parser TokenKind.EqualGreaterThan SyntaxError.error1028

  let require_lambda_arrow parser =
    require_token parser TokenKind.EqualEqualGreaterThan SyntaxError.error1046

  let require_as parser =
    require_token parser TokenKind.As SyntaxError.error1023

  let require_while parser =
    require_token parser TokenKind.While SyntaxError.error1018

  let require_comma parser =
    require_token parser TokenKind.Comma SyntaxError.error1054

  let require_coloncolon parser =
    require_token parser TokenKind.ColonColon SyntaxError.error1047

  let require_name_or_variable_or_error parser error =
    let (parser1, token) = next_token_as_name parser in
    match Token.kind token with
    | TokenKind.Name
    | TokenKind.Variable -> (parser1, make_token token)
    | _ ->
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (with_error parser error, (make_missing()))

  let require_name_or_variable parser =
    require_name_or_variable_or_error parser SyntaxError.error1050

  let require_xhp_class_name_or_name_or_variable parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      (parser, make_token token)
    else
      require_name_or_variable parser

  let require_xhp_class_name_or_qualified_name_or_variable parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      (parser, make_token token)
    else
      let (parser1, token) = next_token_as_name parser in
      match Token.kind token with
      | TokenKind.Name
      | TokenKind.QualifiedName
      | TokenKind.Variable -> (parser1, make_token token)
      | _ ->
        (* ERROR RECOVERY: Create a missing token for the expected token,
           and continue on from the current token. Don't skip it. *)
        (with_error parser SyntaxError.error1050, (make_missing()))

  let optional_token parser kind =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, make_token token)
    else
      (parser, make_missing())

  let assert_token parser kind =
    let (parser, token) = next_token parser in
    if (Token.kind token) <> kind then
      failwith (Printf.sprintf "Expected token %s but got %s\n"
        (TokenKind.to_string kind)
        (TokenKind.to_string (Token.kind token)));
    (parser, make_token token)

  type separated_list_kind =
    | NoTrailing
    | TrailingAllowed
    | ItemsOptional

  (* This helper method parses a list of the form

    open_token item separator_token item ... close_token

    * We assume that open_token has already been consumed.
    * We do not consume the close_token.
    * The given error will be produced if an expected item is missing.
    * The caller is responsible for producing an error if the close_token
      is missing.
    * We expect at least one item.
    * If the list of items is empty then a Missing node is returned.
    * If the list of items is a singleton then the item is returned.
    * Otherwise, a list of the form (item, separator) ... item is returned.
*)

  let parse_separated_list_predicate parser separator_kind list_kind
      close_predicate error parse_item =
    let rec aux parser acc =
      (* At this point we are expecting an item followed by a separator,
         a close, or, if trailing separators are allowed, both *)
      let (parser1, token) = next_token parser in
      let kind = Token.kind token in
      if close_predicate kind || kind = TokenKind.EndOfFile then
        (* ERROR RECOVERY: We expected an item but we found a close or
           the end of the file. Make the item and separator both
           "missing" and give an error.

           If items are optional and we found a close, the last item was
           omitted and there was no error. *)
        let parser = if kind = TokenKind.EndOfFile || list_kind <> ItemsOptional
          then with_error parser error
          else parser in
        let list_item = make_list_item (make_missing()) (make_missing()) in
        (parser, (list_item :: acc))
      else if kind = separator_kind then

        (* ERROR RECOVERY: We expected an item but we got a separator.
           Assume the item was missing, eat the separator, and move on.

           If items are optional, there was no error, so eat the separator and
           continue.

           TODO: This could be poor recovery. For example:

                function bar (Foo< , int blah)

          Plainly the type arg is missing, but the comma is not associated with
          the type argument list, it's associated with the formal
          parameter list.  *)

        let parser = if list_kind <> ItemsOptional
          then with_error parser1 error
          else parser1 in
        let item = make_missing() in
        let separator = make_token token in
        let list_item = make_list_item item separator  in
        aux parser (list_item :: acc)
      else

        (* We got neither a close nor a separator; hopefully we're going
           to parse an item followed by a close or separator. *)
        let (parser, item) = parse_item parser in
        let (parser1, token) = next_token parser in
        let kind = Token.kind token in
        if close_predicate kind then
          let separator = make_missing() in
          let list_item = make_list_item item separator in
          (parser, (list_item :: acc))
        else if kind = separator_kind then
          let separator = make_token token in
          let list_item = make_list_item item separator in
          let acc = list_item :: acc in
          let allow_trailing = list_kind <> NoTrailing in
          (* We got an item followed by a separator; what if the thing
             that comes next is a close? *)
          if allow_trailing && close_predicate (peek_token_kind parser1) then
            (parser1, acc)
          else
            aux parser1 acc
        else
          (* ERROR RECOVERY: We were expecting a close or separator, but
             got neither. Bail out. Caller will give an error. *)
          let separator = make_missing() in
          let list_item = make_list_item item separator in
          (parser, (list_item :: acc)) in
    let (parser, items) = aux parser [] in
    (parser, make_list (List.rev items))

  let parse_separated_list parser separator_kind list_kind
      close_kind error parse_item =
    parse_separated_list_predicate
      parser
      separator_kind
      list_kind
      ((=) close_kind)
      error
      parse_item

  let parse_separated_list_opt_predicate
      parser separator_kind allow_trailing close_predicate error parse_item =
    let token = peek_token parser in
    let kind = Token.kind token in
    if close_predicate kind then
      (parser, make_missing())
    else
      parse_separated_list_predicate
        parser separator_kind allow_trailing close_predicate error parse_item

  let parse_separated_list_opt
      parser separator_kind allow_trailing close_kind error parse_item =
    parse_separated_list_opt_predicate
      parser
      separator_kind
      allow_trailing
      ((=) close_kind)
      error
      parse_item

  let parse_comma_list parser =
    parse_separated_list parser TokenKind.Comma NoTrailing

  let parse_comma_list_allow_trailing parser =
    parse_separated_list parser TokenKind.Comma TrailingAllowed

  let parse_comma_list_opt parser =
    parse_separated_list_opt parser TokenKind.Comma NoTrailing

  let parse_comma_list_opt_allow_trailing_predicate parser =
    parse_separated_list_opt_predicate parser TokenKind.Comma TrailingAllowed

  let parse_comma_list_opt_allow_trailing parser =
    parse_separated_list_opt parser TokenKind.Comma TrailingAllowed

  let parse_comma_list_opt_items_opt parser =
    parse_separated_list_opt parser TokenKind.Comma ItemsOptional

  let parse_semi_list parser =
    parse_separated_list parser TokenKind.Semicolon NoTrailing

  let parse_semi_list_opt parser =
    parse_separated_list_opt parser TokenKind.Semicolon NoTrailing

  let parse_delimited_list
      parser left_kind left_error right_kind right_error parse_items =
    let (parser, left) = require_token parser left_kind left_error in
    let (parser, items) = parse_items parser in
    let (parser, right) = require_token parser right_kind right_error in
    (parser, left, items, right)

  let parse_parenthesized_list parser parse_items =
    parse_delimited_list parser TokenKind.LeftParen SyntaxError.error1019
      TokenKind.RightParen SyntaxError.error1011 parse_items

  let parse_parenthesized_comma_list parser parse_item =
    let parse_items parser =
      parse_comma_list
        parser TokenKind.RightParen SyntaxError.error1011 parse_item in
    parse_parenthesized_list parser parse_items

  let parse_parenthesized_comma_list_opt parser parse_item =
    let parse_items parser =
      parse_comma_list_opt
        parser TokenKind.RightParen SyntaxError.error1011 parse_item in
    parse_parenthesized_list parser parse_items

  let parse_parenthesized_comma_list_opt_allow_trailing parser parse_item =
    let parse_items parser =
      parse_comma_list_opt_allow_trailing
        parser TokenKind.RightParen SyntaxError.error1011 parse_item in
    parse_parenthesized_list parser parse_items

  let parse_parenthesized_comma_list_opt_items_opt parser parse_item =
    let parse_items parser =
      parse_comma_list_opt_items_opt
        parser TokenKind.RightParen SyntaxError.error1011 parse_item in
    parse_parenthesized_list parser parse_items

  let parse_braced_list parser parse_items =
    parse_delimited_list parser TokenKind.LeftBrace SyntaxError.error1034
      TokenKind.RightBrace SyntaxError.error1006 parse_items

  let parse_braced_comma_list_opt parser parse_item =
    let parse_items parser =
      parse_comma_list_opt
        parser TokenKind.RightBrace SyntaxError.error1006 parse_item in
    parse_braced_list parser parse_items

  let parse_braced_comma_list_opt_allow_trailing parser parse_item =
    let parse_items parser =
      parse_comma_list_opt_allow_trailing
        parser TokenKind.RightBrace SyntaxError.error1006 parse_item in
    parse_braced_list parser parse_items

  let parse_bracketted_list parser parse_items =
    parse_delimited_list parser TokenKind.LeftBracket SyntaxError.error1026
      TokenKind.RightBracket SyntaxError.error1031 parse_items

  let parse_bracketted_comma_list_opt_allow_trailing parser parse_item =
    let parse_items parser =
      parse_comma_list_opt_allow_trailing
        parser TokenKind.RightBracket SyntaxError.error1031 parse_item in
    parse_bracketted_list parser parse_items

  let parse_double_angled_list parser parse_items =
    parse_delimited_list parser TokenKind.LessThanLessThan SyntaxError.error1029
      TokenKind.GreaterThanGreaterThan SyntaxError.error1029 parse_items

  let parse_double_angled_comma_list_allow_trailing parser parse_item =
    let parse_items parser =
      parse_comma_list_allow_trailing parser
        TokenKind.GreaterThanGreaterThan SyntaxError.error1029 parse_item in
    parse_double_angled_list parser parse_items

  (* Parse with parse_item while a condition is met. *)
  let parse_list_while parser parse_item predicate =
    let rec aux parser acc =
      if peek_token_kind parser = TokenKind.EndOfFile ||
        not (predicate parser) then
        (parser, acc)
      else
        let (parser, result) = parse_item parser in
        (* ERROR RECOVERY: If the item is was parsed as 'missing', then it means
         * the parser bailed out of that scope. So, pass on whatever's been
         * accumulated so far, but with a 'Missing' SyntaxNode prepended. *)
        if is_missing result
        then (parser, result :: acc )
        else aux parser (result :: acc) in (* Or if nothing's wrong, recurse. *)
    let (parser, items) = aux parser [] in
    (parser, make_list (List.rev items))

  let parse_terminated_list parser parse_item terminator =
    let predicate parser = peek_token_kind parser != terminator in
    parse_list_while parser parse_item predicate

  let parse_list_until_none parser parse_item =
    let rec aux parser acc =
      let (parser, maybe_item) = parse_item parser in
      match maybe_item with
      | None -> (parser, acc)
      | Some item -> aux parser (item :: acc) in
    let (parser, items) = aux parser [] in
    (parser, make_list (List.rev items))

  (* Parse a comma-separated list of items until there is an item that
  does not end in a comma. *)
  let parse_list_until_no_comma parser parse_item =
    let rec aux parser acc =
      let (parser, item) = parse_item parser in
      match syntax item with
      | ListItem { list_item; list_separator } ->
        if is_missing list_separator then
          (parser, item :: acc)
        else
          aux parser (item :: acc)
      | _ ->
        (parser, item :: acc) in
    let (parser, items) = aux parser [] in
    (parser, make_list (List.rev items))

end
