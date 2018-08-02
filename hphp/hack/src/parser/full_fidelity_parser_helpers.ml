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
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module SyntaxError = Full_fidelity_syntax_error
module Trivia = Token.Trivia
module SourceText = Full_fidelity_source_text
module Env = Full_fidelity_parser_env
module type SC_S = SmartConstructors.SmartConstructors_S

open Syntax

module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S

module WithLexer(Lexer : Lexer_S) = struct
module type Parser_S = ParserSig.WithSyntax(Syntax).WithLexer(Lexer).Parser_S

module WithParser(Parser : Parser_S) = struct
  open Parser

  let make_list parser items =
    Make.list parser (pos parser) items

  module NextToken : sig
    val next_token : ?tokenizer:(Lexer.t ->Lexer.t * Syntax.Token.t) -> t -> t * Syntax.Token.t
    val fetch_token : t -> t * Parser.SC.r
  end = struct
    let next_token_impl ~tokenizer parser =
      let lexer = lexer parser in
      let (lexer, token) = tokenizer lexer in
      let parser = with_lexer parser lexer in
      (* ERROR RECOVERY: Check if the parser's carrying ExtraTokenError trivia.
       * If so, clear it and add it to the leading trivia of the current token. *)
       let (parser, token) =
         match skipped_tokens parser with
         | [] -> (parser, token)
         | skipped_tokens ->
           let trivialise_token acc t =
             (* Every bit of a skipped token must end up in `acc`, so push all the
             * token's trailing trivia, then push the "trivialised" token itself,
             * followed by the leading trivia. *)
             let prepend_onto elt_list elt = List.cons elt elt_list in
             let acc = List.fold_left prepend_onto acc (Token.trailing t) in
               let acc = Trivia.make_extra_token_error
                  (Lexer.source lexer) (Lexer.start_offset lexer) (Token.width t)
                  :: acc in
             List.fold_left prepend_onto acc (Token.leading t)
           in
           let leading =
             List.fold_left trivialise_token (Token.leading token) skipped_tokens
           in
           let token = Token.with_leading leading token in
           let parser = clear_skipped_tokens parser in
           (parser, token)
         in
      (parser, token)

    let magic_cache = Little_magic_cache.make ()
    let next_token ?(tokenizer=Lexer.next_token) = Little_magic_cache.memoize magic_cache (next_token_impl ~tokenizer)
    let fetch_token parser =
      let (parser, token) = next_token parser in
      Make.token parser token
  end

  include NextToken

  let next_token_no_trailing parser =
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_token_no_trailing lexer in
    let parser = with_lexer parser lexer in
    (parser, token)

  let next_docstring_header parser =
    let lexer = lexer parser in
    let (lexer, token, name) = Lexer.next_docstring_header lexer in
    let parser = with_lexer parser lexer in
    (parser, token, name)

  let next_token_in_string parser literal_kind =
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_token_in_string lexer literal_kind in
    let parser = with_lexer parser lexer in
    (parser, token)

  let peek_token ?(lookahead=0) parser =
    let rec lex_ahead lexer n =
      let (next_lexer, token) = Lexer.next_token lexer in
      match n with
      | 0 -> token
      | _ -> lex_ahead next_lexer (n-1)
    in
      lex_ahead (lexer parser) lookahead

  let next_token_as_name parser =
    (* TODO: This isn't right.  Pass flags to the lexer. *)
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_token_as_name lexer in
    let parser = with_lexer parser lexer in
    (parser, token)

  let peek_token_as_name ?(lookahead=0) parser =
    let rec lex_ahead lexer n =
      let (next_lexer, token) = Lexer.next_token_as_name lexer in
      match n with
      | 0 -> token
      | _ -> lex_ahead next_lexer (n-1)
    in
      lex_ahead (lexer parser) lookahead

  let peek_token_kind ?(lookahead=0) parser =
    Token.kind (peek_token ~lookahead parser)

  let scan_markup parser ~is_leading_section =
    let (lexer, markup, suffix) =
      Lexer.scan_markup (lexer parser) ~is_leading_section
    in
    with_lexer parser lexer, markup, suffix

  let rescan_halt_compiler parser right_brace =
    let lexer = lexer parser in
    let (lexer, right_brace) = Lexer.rescan_halt_compiler lexer right_brace in
    with_lexer parser lexer, right_brace

  let error_offsets ?(on_whole_token=false) parser =
    let lexer = lexer parser in
    if on_whole_token then
      let token = peek_token parser in
      let start_offset =
        (Lexer.end_offset lexer) + (Token.leading_width token) in
      let end_offset = start_offset + (Token.width token) in
      (start_offset, end_offset)
    else
      let start_offset = Lexer.start_offset lexer in
      let end_offset = Lexer.end_offset lexer in
      (start_offset, end_offset)

  (* This function reports an error starting at the current location of the
   * parser. Setting on_whole_token=false will report the error only on trivia,
   * which is useful in cases such as when "a semicolon is expected here" before
   * the current node. However, setting on_whole_token=true will report the error
   * only on the non-trivia text of the next token parsed, which is useful
   * in cases like "flagging an entire token as an extra". *)
  let with_error ?(on_whole_token=false) parser message =
    let (start_offset, end_offset) = error_offsets parser ~on_whole_token in
    let error = SyntaxError.make start_offset end_offset message in
    let errors = errors parser in
    with_errors parser (error :: errors)

  let current_token_text parser =
    Token.text @@ peek_token parser

  let skip_and_log_unexpected_token ?(generate_error=true) parser =
    let parser =
      if generate_error then
        let extra_str = current_token_text parser in
        with_error parser (SyntaxError.error1057 extra_str) ~on_whole_token:true
      else parser in
    let parser, token = next_token parser in
    let skipped_tokens = token :: skipped_tokens parser in
    with_skipped_tokens parser skipped_tokens

  (* Returns true if the strings underlying two tokens are of the same length
   * but with one character different. *)
  let one_character_different str1 str2 =
    if String.length str1 != String.length str2 then false
    else begin
      let rec off_by_one str1 str2 =
        let str_len = String.length str1 in (* both strings have same length *)
        if str_len = 0 then true
        else begin
          let rest_of_str1 = String.sub str1 1 (str_len - 1) in
          let rest_of_str2 = String.sub str2 1 (str_len - 1) in
          if Char.equal (String.get str1 0) (String.get str2 0)
          then off_by_one rest_of_str1 rest_of_str2
          (* Allow only one mistake *)
          else (String.compare rest_of_str1 rest_of_str2) = 0
        end
      in
      off_by_one str1 str2
    end

  (* Compare the text of the token we have in hand to the text of the
   * anticipated kind. Note: this automatically returns false for any
   * TokenKinds of length 1. *)
  let is_misspelled_kind kind token_str =
    let tokenkind_str = TokenKind.to_string kind in
    if String.length tokenkind_str <= 1 then false
    else
      one_character_different tokenkind_str token_str

  let is_misspelled_from kind_list token_str =
    List.exists (fun kind -> is_misspelled_kind kind token_str) kind_list

  (* If token_str is a misspelling (by our narrow definition of misspelling)
   * of a TokenKind from kind_list, return the TokenKind that token_str is a
   * misspelling of. Otherwise, return None. *)
  let suggested_kind_from kind_list token_str =
    Hh_core.List.find_map kind_list ~f:(fun kind ->
      if is_misspelled_kind kind token_str then Some kind else None)

  let skip_and_log_misspelled_token parser required_kind =
    let received_str = current_token_text parser in
    let required_str = TokenKind.to_string required_kind in
    let parser = with_error parser
      (SyntaxError.error1058 received_str required_str) ~on_whole_token:true in
    skip_and_log_unexpected_token ~generate_error:false parser

  let require_and_return_token parser kind error =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      (parser1, Some token)
    else begin
      (* ERROR RECOVERY: Look at the next token after this. Is it the one we
       * require? If so, process the current token as extra and return the next
       * one. Otherwise, create a missing token for what we required,
       * and continue on from the current token (don't skip it). *)
      let next_kind = peek_token_kind ~lookahead:1 parser in
      if next_kind = kind then
        let parser = skip_and_log_unexpected_token parser in
        let (parser, token) = next_token parser in
        (parser, Some token)
      else begin
        (* ERROR RECOVERY: We know we didn't encounter an extra token.
         * So, as a second line of defense, check if the current token
         * is a misspelling, by our existing narrow definition of misspelling. *)
        if is_misspelled_kind kind (current_token_text parser) then
          let parser = skip_and_log_misspelled_token parser kind in
          (parser, None)
        else
          let parser = with_error parser error in
          (parser, None)
      end
    end

  let require_token_one_of parser kinds error =
    let (parser1, token) = next_token parser in
    if List.mem (Token.kind token) kinds
    then Make.token parser1 token
    else begin
      (* ERROR RECOVERY: Look at the next token after this. Is it the one we
       * require? If so, process the current token as extra and return the next
       * one. Otherwise, create a missing token for what we required,
       * and continue on from the current token (don't skip it). *)
      let next_kind = peek_token_kind ~lookahead:1 parser in
      if List.mem next_kind kinds then
        let parser = skip_and_log_unexpected_token parser in
        let (parser, token) = next_token parser in
        Make.token parser token
      else begin
        (* ERROR RECOVERY: We know we didn't encounter an extra token.
         * So, as a second line of defense, check if the current token
         * is a misspelling, by our existing narrow definition of misspelling. *)
        let is_misspelling k =
          is_misspelled_kind k (current_token_text parser)
        in
        if List.exists is_misspelling kinds then
          let kind = List.(hd @@ filter is_misspelling kinds) in
          let parser = skip_and_log_misspelled_token parser kind in
          Make.missing parser (pos parser)
        else
          let parser = with_error parser error in
          Make.missing parser (pos parser)
      end
    end


  let require_token parser kind error =
    (* Must behave as `require_token_one_of parser [kind] error` *)
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      Make.token parser1 token
    else begin
      (* ERROR RECOVERY: Look at the next token after this. Is it the one we
       * require? If so, process the current token as extra and return the next
       * one. Otherwise, create a missing token for what we required,
       * and continue on from the current token (don't skip it). *)
      let next_kind = peek_token_kind ~lookahead:1 parser in
      if next_kind = kind then
        let parser = skip_and_log_unexpected_token parser in
        let (parser, token) = next_token parser in
        Make.token parser token
      else begin
        (* ERROR RECOVERY: We know we didn't encounter an extra token.
         * So, as a second line of defense, check if the current token
         * is a misspelling, by our existing narrow definition of misspelling. *)
        if is_misspelled_kind kind (current_token_text parser) then
          let parser = skip_and_log_misspelled_token parser kind in
          Make.missing parser (pos parser)
        else
          let parser = with_error parser error in
          Make.missing parser (pos parser)
      end
    end

  let require_required parser =
    require_token parser TokenKind.Required SyntaxError.error1051

  let require_name parser =
    require_token parser TokenKind.Name SyntaxError.error1004

  let require_name_allow_keywords parser =
    let (parser1, token) = next_token_as_name parser in
    if (Token.kind token) = TokenKind.Name then
      Make.token parser1 token
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      let parser = with_error parser SyntaxError.error1004 in
      Make.missing parser (pos parser)

  let require_name_allow_std_constants parser =
    let start_offset = Lexer.end_offset @@ lexer parser in
    let (parser1, token) = require_name_allow_keywords parser in
    let end_offset = Lexer.end_offset @@ lexer parser1 in
    let source = Lexer.source @@ lexer parser in
    let text = SourceText.sub source start_offset (end_offset - start_offset) in
    match String.lowercase_ascii text with
    | "true" | "false" | "null" -> (parser1, token)
    | _ -> require_name parser

  let next_xhp_category_name parser =
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_xhp_category_name lexer in
    let parser = with_lexer parser lexer in
    (parser, token)

  (* We have a number of issues involving xhp class names, which begin with
     a colon and may contain internal colons and dashes.  These are some
     helper methods to deal with them. *)

  let is_next_name parser =
    Lexer.is_next_name (lexer parser)

  let next_xhp_name parser =
    assert(is_next_name parser);
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_xhp_name lexer in
    let parser = with_lexer parser lexer in
    (parser, token)

  let is_next_xhp_class_name parser =
    Lexer.is_next_xhp_class_name (lexer parser)

  let next_xhp_class_name parser =
    assert(is_next_xhp_class_name parser);
    let lexer = lexer parser in
    let (lexer, token) = Lexer.next_xhp_class_name lexer in
    let parser = with_lexer parser lexer in
    (parser, token)

  let require_xhp_name parser =
    if is_next_name parser then
      let (parser, token) = next_xhp_name parser in
      Make.token parser token
    else
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      (* TODO: Different error? *)
      let parser = with_error parser SyntaxError.error1004 in
      Make.missing parser (pos parser)

  let is_next_xhp_category_name parser =
    Lexer.is_next_xhp_category_name (lexer parser)

  (* Also returns whether last token was '\' *)
  let rec scan_qualified_name_worker parser name_opt acc is_backslash =
    let parser1, token = next_token_as_name parser in
    match name_opt, Token.kind token, acc with
    | Some name, TokenKind.Backslash, _ ->
      (* found backslash, create item and recurse *)
      let (parser, token) = Make.token parser1 token in
      let (parser, part) = Make.list_item parser name token in
      (* TODO(T25649779) *)
      scan_qualified_name_worker parser None (part :: acc) true
    | None, TokenKind.Name, _ ->
      (* found a name, recurse to look for backslash *)
      let (parser, token) = Make.token parser1 token in
      scan_qualified_name_worker parser (Some token) acc false
    | Some _name, _, [] ->
      (* have not found anything - return [] to indicate failure *)
      parser, [], false
    | Some name, _, _ ->
      (* next token is not part of qualified name but we've consume some
         part of the input - create part for name with missing backslash
         and return accumulated result *)
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, part) = Make.list_item parser name missing in
      (* TODO(T25649779) *)
      parser, List.rev (part :: acc), false
    | _ ->
      (* next token is not part of qualified name - return accumulated result *)
      parser, List.rev acc, is_backslash

  let scan_remaining_qualified_name_extended parser name_token =
    let (parser, parts, is_backslash) =
      scan_qualified_name_worker parser (Some name_token) [] false
    in
    match parts with
    | [] ->
      (parser, name_token, is_backslash)
    | parts ->
      let (parser, list_node) = make_list parser parts in
      let (parser, name) = Make.qualified_name parser list_node in
      (parser, name, is_backslash)

  let scan_remaining_qualified_name parser name_token =
    let (parser, name, _) =
      scan_remaining_qualified_name_extended parser name_token
    in
    (parser, name)

  let scan_qualified_name_extended parser missing backslash =
    let (parser, head) = Make.list_item parser missing backslash in
    let (parser, parts, is_backslash) =
      scan_qualified_name_worker parser None [head] false
    in
    let (parser, list_node) = make_list parser parts in
    let (parser, name) = Make.qualified_name parser list_node in
    (parser, name, is_backslash)

  let scan_qualified_name parser missing backslash =
    let (parser, name, _) =
      scan_qualified_name_extended parser missing backslash
    in
    (parser, name)

  let scan_name_or_qualified_name parser =
    let parser1, token = next_token_as_name parser in
    match Token.kind token with
    | TokenKind.Name ->
      let (parser, token) = Make.token parser1 token in
      scan_remaining_qualified_name parser token
    | TokenKind.Backslash ->
      let (parser, missing) = Make.missing parser1 (pos parser1) in
      let (parser, token) = Make.token parser token in
      scan_qualified_name parser missing token
    | _ -> Make.missing parser (pos parser)

  let next_xhp_class_name_or_other_token parser =
    if is_next_xhp_class_name parser then next_xhp_class_name parser
    else next_token parser

  let next_xhp_class_name_or_other parser =
    let parser, token = next_xhp_class_name_or_other_token parser in
    match Token.kind token with
    | TokenKind.Name ->
      let (parser, name_token) = Make.token parser token in
      scan_remaining_qualified_name parser name_token
    | TokenKind.Backslash ->
      let (parser, missing) = Make.missing parser (pos parser) in
      let (parser, backslash) = Make.token parser token in
      scan_qualified_name parser missing backslash
    | _ -> Make.token parser token

  let next_xhp_children_name_or_other parser =
    if is_next_xhp_category_name parser then
      let parser, token = next_xhp_category_name parser in
      parser, token
    else
      next_xhp_class_name_or_other_token parser

  (* We accept either a Name or a QualifiedName token when looking for a
     qualified name. *)
  let require_qualified_name parser =
    let (parser1, name) = next_token_as_name parser in
    match Token.kind name with
    | TokenKind.Name ->
      let (parser, token) = Make.token parser1 name in
      scan_remaining_qualified_name parser token
    | TokenKind.Backslash ->
      let (parser, missing) = Make.missing parser1 (pos parser1) in
      let (parser, backslash) = Make.token parser name in
      scan_qualified_name parser missing backslash
    | _ ->
      let parser = with_error parser SyntaxError.error1004 in
      Make.missing parser (pos parser)

  (**
   * TODO: If using qualified names for class names is legal in some cases, then
   * we need to update the specification accordingly.
   *
   * TODO: if we need the use of qualified names to be an error in some cases,
   * we need to add error checking code in a later pass.
   *)
  let require_class_name parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      Make.token parser token
    else
      require_qualified_name parser

  let require_function parser =
    require_token parser TokenKind.Function SyntaxError.error1003

  let require_variable parser =
    require_token parser TokenKind.Variable SyntaxError.error1008

  let require_semicolon_token parser =
    (* TODO: Kill PHPism; no semicolon required right before ?> *)
    match peek_token_kind parser with
    | TokenKind.QuestionGreaterThan ->
      parser, None
    | _ -> require_and_return_token parser TokenKind.Semicolon SyntaxError.error1010

  let require_semicolon parser =
    (* TODO: Kill PHPism; no semicolon required right before ?> *)
    match peek_token_kind parser with
    | TokenKind.QuestionGreaterThan -> Make.missing parser (pos parser)
    | _ -> require_token parser TokenKind.Semicolon SyntaxError.error1010

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

  let require_comma parser =
    require_token parser TokenKind.Comma SyntaxError.error1054

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


  let require_coloncolon parser =
    require_token parser TokenKind.ColonColon SyntaxError.error1047

  let require_name_or_variable_or_error parser error =
    let (parser1, token) = next_token_as_name parser in
    match Token.kind token with
    | TokenKind.Name ->
      let (parser, token) = Make.token parser1 token in
      scan_remaining_qualified_name parser token
    | TokenKind.Variable -> Make.token parser1 token
    | _ ->
      (* ERROR RECOVERY: Create a missing token for the expected token,
         and continue on from the current token. Don't skip it. *)
      let parser = with_error parser error in
      Make.missing parser (pos parser)

  let require_name_or_variable parser =
    require_name_or_variable_or_error parser SyntaxError.error1050

  let require_xhp_class_name_or_name_or_variable parser =
    if is_next_xhp_class_name parser then
      let (parser, token) = next_xhp_class_name parser in
      Make.token parser token
    else
      require_name_or_variable parser

  let optional_token parser kind =
    let (parser1, token) = next_token parser in
    if (Token.kind token) = kind then
      Make.token parser1 token
    else
      Make.missing parser (pos parser)

  let assert_token ?tokenizer parser kind =
    let (parser, token) = next_token ?tokenizer parser in
    let lexer = lexer parser in
    let source = Lexer.source lexer in
    let file_path = SourceText.file_path source in
    if (Token.kind token) <> kind then
      failwith (Printf.sprintf "Expected token '%s' but got '%s'\n  in %s\n"
        (TokenKind.to_string kind)
        (TokenKind.to_string (Token.kind token))
        (Relative_path.to_absolute file_path));
    Make.token parser token

  let assert_xhp_body_token = assert_token ~tokenizer:Lexer.next_xhp_body_token

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
          else parser
        in
        let (parser, missing1) = Make.missing parser (pos parser) in
        let (parser, missing2) = Make.missing parser (pos parser) in
        let (parser, list_item) = Make.list_item parser missing1 missing2 in
        (* TODO(T25649779) *)
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
        let (parser, item) = Make.missing parser (pos parser) in
        let (parser, separator) = Make.token parser token in
        let (parser, list_item) = Make.list_item parser item separator in
        (* TODO(T25649779) *)
        aux parser (list_item :: acc)
      else

        (* We got neither a close nor a separator; hopefully we're going
           to parse an item followed by a close or separator. *)
        let (parser, item) = parse_item parser in
        let (parser1, token) = next_token parser in
        let kind = Token.kind token in
        if close_predicate kind then
          let (parser, missing) = Make.missing parser (pos parser) in
          let (parser, list_item) = Make.list_item parser item missing in
          (* TODO(T25649779) *)
          (parser, (list_item :: acc))
        else if kind = separator_kind then
          let (parser, separator) = Make.token parser1 token in
          let (parser, list_item) = Make.list_item parser item separator in
          (* TODO(T25649779) *)
          let acc = list_item :: acc in
          let allow_trailing = list_kind <> NoTrailing in
          (* We got an item followed by a separator; what if the thing
             that comes next is a close? *)
          if allow_trailing && close_predicate (peek_token_kind parser) then
            (parser, acc)
          else
            aux parser acc
        else
          (* ERROR RECOVERY: We were expecting a close or separator, but
             got neither. Bail out. Caller will give an error. *)
          let (parser, missing) = Make.missing parser (pos parser) in
          let (parser, list_item) = Make.list_item parser item missing in
          (* TODO(T25649779) *)
          (parser, (list_item :: acc)) in
    let (parser, items) = aux parser [] in
    let no_arg_is_missing = List.for_all (fun c -> not (SC.is_missing c)) items in
    let (parser, item_list) = make_list parser (List.rev items) in
    parser, item_list, no_arg_is_missing

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
      Make.missing parser (pos parser)
    else
      let (parser, items, _) =
      parse_separated_list_predicate
        parser
        separator_kind
        allow_trailing
        close_predicate
        error
        parse_item
      in
      (parser, items)

  let parse_separated_list_opt
      parser separator_kind allow_trailing close_kind error parse_item =
    parse_separated_list_opt_predicate
      parser
      separator_kind
      allow_trailing
      ((=) close_kind)
      error
      parse_item

  let parse_comma_list parser close_kind error parse_item =
    let (parser, items, _) =
      parse_separated_list
        parser
        TokenKind.Comma
        NoTrailing
        close_kind
        error
        parse_item
    in
    (parser, items)

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
      let (parser, items, _) =
        parse_comma_list_allow_trailing
          parser
          TokenKind.GreaterThanGreaterThan
          SyntaxError.error1029
          parse_item
      in
      (parser, items)
    in
    parse_double_angled_list parser parse_items

  (* Parse with parse_item while a condition is met. *)
  let parse_list_while parser (parse_item : Parser.t -> Parser.t * Parser.SC.r) predicate =
    let rec aux parser acc =
      if peek_token_kind parser = TokenKind.EndOfFile ||
        not (predicate parser)
      then
        (parser, acc)
      else
        let lexer_before = Parser.lexer parser in
        let (parser, result) = parse_item parser in
        (* ERROR RECOVERY: If the item is was parsed as 'missing', then it means
         * the parser bailed out of that scope. So, pass on whatever's been
         * accumulated so far, but with a 'Missing' SyntaxNode prepended. *)
        if SC.is_missing result
        then (parser, result :: acc)
        else let current_lexer = Parser.lexer parser in
        (* INFINITE LOOP PREVENTION: If parse_item does not actually make
         * progress, just bail
         *)
        if Lexer.start_offset lexer_before = Lexer.start_offset current_lexer &&
           Lexer.end_offset lexer_before = Lexer.end_offset current_lexer
        then (parser, result :: acc)
        else aux parser (result :: acc) (* Or if nothing's wrong, recurse. *)
    in
    let (parser, items) = aux parser [] in
    make_list parser (List.rev items)

  let parse_terminated_list parser parse_item terminator =
    let predicate parser = peek_token_kind parser != terminator in
    parse_list_while parser parse_item predicate

  let parse_alternate_if_block parser parse_item =
    let parser1, block = parse_list_while parser parse_item (fun parser ->
      match peek_token_kind parser with
      | TokenKind.Elseif | TokenKind.Else | TokenKind.Endif -> false
      | _ -> true) in
    if SC.is_missing block
    then
      let parser, empty = Make.missing parser (pos parser) in
      let parser, es = Make.expression_statement parser empty empty in
      make_list parser [es]
    else
      parser1, block

  let parse_list_until_none parser parse_item =
    let rec aux parser acc =
      let (parser, maybe_item) = parse_item parser in
      match maybe_item with
      | None -> (parser, acc)
      | Some item when peek_token_kind parser = TokenKind.EndOfFile ->
        parser, item :: acc
      (* exit if parser did not make any progress *)
      | Some item when SC.is_missing item ->
        parser, item :: acc
      | Some item -> aux parser (item :: acc)
    in
    let (parser, items) = aux parser [] in
    make_list parser (List.rev items)

end (* WithParser *)
end (* WithLexer *)
end (* WithSyntax *)
