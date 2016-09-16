(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Trivia = Full_fidelity_minimal_trivia
module Token = Full_fidelity_minimal_token

type t = {
  text : SourceText.t;
  start : int;  (* Both start and offset are absolute offsets in the text. *)
  offset : int;
  errors : SyntaxError.t list
}

let invalid = '\000'

let make text =
  { text; start = 0; offset = 0; errors = [] }

let errors lexer =
  lexer.errors

let with_error lexer message =
  let error = SyntaxError.make lexer.start lexer.offset message in
  { lexer with errors = error :: lexer.errors }

(* Housekeeping *)

let start_new_lexeme lexer =
  { lexer with start = lexer.offset }

(* let restart_lexeme lexer =
  { lexer with offset = lexer.start } *)

let peek_char lexer index =
  let i = lexer.offset + index in
  if i >= SourceText.length lexer.text then invalid
  else SourceText.get lexer.text i

let peek_string lexer size =
  let r = (SourceText.length lexer.text) - lexer.offset in
  if r < 0 then ""
  else if r < size then SourceText.sub lexer.text lexer.offset r
  else SourceText.sub lexer.text lexer.offset size

let advance lexer index =
  { lexer with offset = lexer.offset + index }

let width lexer =
  lexer.offset - lexer.start

let current_text lexer =
  SourceText.sub lexer.text lexer.start (width lexer)

let at_end lexer =
  lexer.offset >= SourceText.length lexer.text

let remaining lexer =
  let r = (SourceText.length lexer.text) - lexer.offset in
  if r < 0 then 0 else r

let start_offset lexer =
  lexer.start

let end_offset lexer =
  lexer.offset

(* Character classification *)

let is_newline ch =
  ch = '\r' || ch = '\n'

let is_binary_digit ch =
  ch == '0' || ch == '1'

let is_octal_digit ch =
  '0' <= ch && ch <= '7'

let is_decimal_digit ch =
  '0' <= ch && ch <= '9'

let is_hexadecimal_digit ch =
  ('0' <= ch && ch <= '9') ||
  ('a' <= ch && ch <= 'f') ||
  ('A' <= ch && ch <= 'F')

let is_name_nondigit ch =
  ('a' <= ch && ch <= 'z') ||
  ('A' <= ch && ch <= 'Z') ||
  ('\x7f' <= ch && ch <= '\xff') ||
  ch = '_'

let is_name_letter ch =
  (is_name_nondigit ch) || (is_decimal_digit ch)

(* Lexing *)

let rec skip_whitespace lexer =
  let ch = peek_char lexer 0 in
  if ch = '\t' || ch = ' ' then
    skip_whitespace (advance lexer 1)
  else
    lexer

let rec skip_to_end_of_line lexer =
  let ch = peek_char lexer 0 in
  if is_newline ch then lexer
  else if ch = invalid && at_end lexer then lexer
  else skip_to_end_of_line (advance lexer 1)

let rec skip_name_end lexer =
  if is_name_letter (peek_char lexer 0) then
    skip_name_end (advance lexer 1)
  else
    lexer

let skip_end_of_line lexer =
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('\r', '\n') -> advance lexer 2
  | ('\r', _)
  | ('\n', _) -> advance lexer 1
  | _ -> lexer

(* A qualified name which ends with a backslash is a namespace prefix; this is
   only legal in a "group use" declaration.
   TODO: Consider detecting usages of namespace prefixes in places where names
   and qualified names are expected; give a more meaningful error. *)
let rec scan_qualified_name lexer =
  assert ((peek_char lexer 0) = '\\');
  let lexer = advance lexer 1 in
  if is_name_nondigit (peek_char lexer 0) then
    begin
      let (lexer, token) = scan_name_or_qualified_name lexer in
      if token = TokenKind.Name then
        (lexer, TokenKind.QualifiedName)
      else
        (lexer, token)
    end
  else
    (lexer, TokenKind.NamespacePrefix)
and scan_name_or_qualified_name lexer =
  if (peek_char lexer 0) = '\\' then
    scan_qualified_name lexer
  else
    let lexer = scan_name_impl lexer in
    if (peek_char lexer 0) = '\\' then
      scan_qualified_name lexer
    else
      (lexer, TokenKind.Name)
and scan_name lexer =
  let lexer = scan_name_impl lexer in
  if (peek_char lexer 0) = '\\' then
    (* ERROR RECOVERY: Assume that a qualfied name was meant. *)
    (* TODO: This is poor recovery for the case where we're scanning
       the end of a local variable. *)
    let lexer = with_error lexer SyntaxError.error0009 in
    scan_qualified_name lexer
  else
    (lexer, TokenKind.Name)
and scan_name_impl lexer =
  assert (is_name_nondigit (peek_char lexer 0));
  skip_name_end (advance lexer 1)

let scan_variable lexer =
  assert('$' = peek_char lexer 0);
  let (lexer, _) = scan_name (advance lexer 1) in
  (lexer, TokenKind.Variable)

let rec scan_decimal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_decimal_digit ch then scan_decimal_digits (advance lexer 1)
  else lexer

let rec scan_octal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_octal_digit ch then scan_octal_digits (advance lexer 1)
  else lexer

let rec scan_binary_digits lexer =
  let ch = peek_char lexer 0 in
  if is_binary_digit ch then scan_binary_digits (advance lexer 1)
  else lexer

let rec scan_hexadecimal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_hexadecimal_digit ch then scan_hexadecimal_digits (advance lexer 1)
  else lexer

let scan_hex_literal lexer =
  let ch = peek_char lexer 0 in
  if not (is_hexadecimal_digit ch) then
    let lexer = with_error lexer SyntaxError.error0001 in
    (lexer, TokenKind.HexadecimalLiteral)
  else
    (scan_hexadecimal_digits lexer, TokenKind.HexadecimalLiteral)

let scan_binary_literal lexer =
  let ch = peek_char lexer 0 in
  if not (is_binary_digit ch) then
    let lexer = with_error lexer SyntaxError.error0002 in
    (lexer, TokenKind.BinaryLiteral)
  else
    (scan_binary_digits lexer, TokenKind.BinaryLiteral)

let scan_exponent lexer =
  let lexer = advance lexer 1 in
  let ch = peek_char lexer 0 in
  let lexer = if ch = '+' || ch = '-' then (advance lexer 1) else lexer in
  let ch = peek_char lexer 0 in
  if not (is_decimal_digit ch) then
    let lexer = with_error lexer SyntaxError.error0003 in
    (lexer, TokenKind.FloatingLiteral)
  else
    (scan_decimal_digits lexer, TokenKind.FloatingLiteral)

let scan_after_decimal_point lexer =
  let lexer = advance lexer 1 in
  let lexer = scan_decimal_digits lexer in
  let ch = peek_char lexer 0 in
  if ch = 'e' || ch = 'E' then
    scan_exponent lexer
  else
    (lexer, TokenKind.FloatingLiteral)

let scan_octal_or_float lexer =
  (* We've scanned a leading zero. *)
  (* We have an irritating ambiguity here.  09 is not a legal octal or
   * floating literal, but 09e1 and 09.1 are. *)
  let lexer = advance lexer 1 in
  let ch = peek_char lexer 0 in
  match ch with
  | '.' -> (* 0. *) scan_after_decimal_point lexer
  | 'e' | 'E' -> (* 0e *) scan_exponent lexer
  | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ->
    (* 05 *)
    let lexer_oct = scan_octal_digits lexer in
    let lexer_dec = scan_decimal_digits lexer in
    if (width lexer_oct) = (width lexer_dec) then
      begin
        (* Only octal digits. Could be an octal literal, or could
           be a float. *)
        let ch = peek_char lexer_oct 0 in
        if ch = 'e' || ch = 'E' then scan_exponent lexer_oct
        else if ch = '.' then scan_after_decimal_point lexer_oct
        else (lexer_oct, TokenKind.OctalLiteral)
      end
    else
      begin
        (* We had decimal digits following a leading zero; this MUST
           be a float literal. *)
        let ch = peek_char lexer_dec 0 in
        if ch = 'e' || ch = 'E' then
          scan_exponent lexer_dec
        else if ch = '.' then
          scan_after_decimal_point lexer_dec
        else
          let lexer_dec = with_error lexer_dec SyntaxError.error0004 in
          (lexer_dec, TokenKind.DecimalLiteral)
      end
  | _ -> (* 0 *) (lexer, TokenKind.OctalLiteral)

let scan_decimal_or_float lexer =
  (* We've scanned a leading non-zero digit. *)
  let lexer = scan_decimal_digits lexer in
  let ch = peek_char lexer 0 in
  match ch with
  | '.' -> (* 123. *) scan_after_decimal_point lexer
  | 'e' | 'E' -> (* 123e *) scan_exponent lexer
  | _ -> (* 123 *) (lexer, TokenKind.DecimalLiteral)

let scan_single_quote_string_literal lexer =
  (* TODO: What about newlines embedded? *)
  (* SPEC:
  single-quoted-string-literal::
    b-opt  ' sq-char-sequence-opt  '

    TODO: What is this b-opt?  We don't lex an optional 'b' before a literal.

  sq-char-sequence::
    sq-char
    sq-char-sequence   sq-char

  sq-char::
    sq-escape-sequence
    \opt   any character except single-quote (') or backslash (\)

  sq-escape-sequence:: one of
    \'  \\

  *)

  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    match (ch0, ch1) with
    | ('\000', _) ->
      if at_end lexer then
        let lexer = with_error lexer SyntaxError.error0012 in
        (lexer, TokenKind.SingleQuotedStringLiteral)
      else
        let lexer = with_error lexer SyntaxError.error0006 in
        aux (advance lexer 1)
    | ('\\', _) -> aux (advance lexer 2)
      (* Note that an "invalid" escape sequence in a single-quoted string
      literal is not an error. It's just the \ character followed by the
      next character. So no matter what, we can simply skip two chars. *)
    | ('\'', _) -> (advance lexer 1, TokenKind.SingleQuotedStringLiteral)
    | _ -> aux (advance lexer 1) in
  aux (advance lexer 1)

let scan_hexadecimal_escape lexer =
  let ch2 = peek_char lexer 2 in
  let ch3 = peek_char lexer 3 in
  if not (is_hexadecimal_digit ch2) then
    let lexer = with_error lexer SyntaxError.error0005 in
    advance lexer 2
  else if not (is_hexadecimal_digit ch3) then
    let lexer = with_error lexer SyntaxError.error0005 in
    advance lexer 3
  else
    advance lexer 4

let scan_unicode_escape lexer =
  let ch2 = peek_char lexer 2 in
  if ch2 != '{' then
    let lexer = with_error lexer SyntaxError.error0005 in
    advance lexer 2
  else
    (* TODO: Verify that number is in range. *)
    (* TODO: Verify that there are any digits at all! *)
    (* Skip over the slash, u and brace, and start lexing the number. *)
    let lexer = advance lexer 3 in
    let lexer = scan_hexadecimal_digits lexer in
    let ch = peek_char lexer 0 in
    if ch != '}' then
      let lexer = with_error lexer SyntaxError.error0005 in
      lexer
    else
      advance lexer 1

let scan_double_quote_string_literal lexer =
  (* This lexer does not attempt to verify that embedded variables
     are well-formed. It just finds the boundaries of the string,
     and verifies that escape sequences are well-formed.

     TODO: What about newlines embedded?
     *)
  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    match (ch0, ch1) with
    | ('\000', _) ->
      if at_end lexer then
        let lexer = with_error lexer SyntaxError.error0012 in
        (lexer, TokenKind.DoubleQuotedStringLiteral)
      else
        let lexer = with_error lexer SyntaxError.error0006 in
        aux (advance lexer 1)
    | ('\\', '"')
    | ('\\', '$')
    | ('\\', 'e')
    | ('\\', 'f')
    | ('\\', 'n')
    | ('\\', 'r')
    | ('\\', 't')
    | ('\\', 'v') -> aux (advance lexer 2)
    | ('\\', '0')
    | ('\\', '1')
    | ('\\', '2')
    | ('\\', '3')
    | ('\\', '4')
    | ('\\', '5')
    | ('\\', '6')
    | ('\\', '7') -> aux (advance lexer 2)
      (* The next two chars might be octal; who cares? we're not decoding
         the escape sequence, we're just verifying that its correct. If
         the next two characters are octal digits they'll just get eaten *)
    | ('\\', 'x') -> aux (scan_hexadecimal_escape lexer)
      (* Here we need to actually scan them for errors. *)
    | ('\\', 'u') -> aux (scan_unicode_escape lexer)
    | ('\\', _) ->
      let lexer = with_error lexer SyntaxError.error0005 in
      aux (advance lexer 1)
    | ('"', _) -> (advance lexer 1, TokenKind.DoubleQuotedStringLiteral)
    | _ -> aux (advance lexer 1) in
  aux (advance lexer 1)


(*  A heredoc string literal has the form

<<< (optional whitespace) name (no whitespace) (newline)
any characters whatsoever including newlines (newline)
(no whitespace) name (no whitespace) (optional semi) (no whitespace) (newline)

The names must be identical.  The trailing semi and newline are NOT part of
the literal, but they must be present.

A nowdoc string literal has the same form except that the first name is
enclosed in single quotes.

*)

let scan_docstring_name_actual lexer =
  let ch = peek_char lexer 0 in
  if is_name_nondigit ch then
    let end_lexer = skip_name_end (advance lexer 1) in
    let name = SourceText.sub
      lexer.text lexer.offset (end_lexer.offset - lexer.offset) in
    (end_lexer, name)
  else
    let lexer = with_error lexer SyntaxError.error0008 in
    (lexer, "")

let scan_docstring_name lexer =
  let lexer = skip_whitespace lexer in
  let ch = peek_char lexer 0 in
  let kind =
    if ch = '\'' then TokenKind.NowdocStringLiteral
    else TokenKind.HeredocStringLiteral in
  let (lexer, name) =
    if ch = '\'' then
      let (lexer, name) = scan_docstring_name_actual (advance lexer 1) in
      if (peek_char lexer 0) = '\'' then
        (advance lexer 1, name)
      else
        (with_error lexer SyntaxError.error0010, name)
    else
      scan_docstring_name_actual lexer in
  (lexer, name, kind)

let scan_docstring_header lexer =
  let lexer = advance lexer 3 in
  let (lexer, name, kind) = scan_docstring_name lexer in
  let ch = peek_char lexer 0 in
  let lexer =
    if is_newline ch then lexer
    else with_error lexer SyntaxError.error0011 in
  let lexer = skip_to_end_of_line lexer in
  let lexer = skip_end_of_line lexer in
  (lexer, name, kind)

let scan_docstring_remainder name lexer =
  let len = String.length name in
  let rec aux lexer =
    let ch0 = peek_char lexer len in
    let ch1 = peek_char lexer (len + 1) in
    if ((is_newline ch0) || ch0 = ';' && (is_newline ch1)) &&
        (peek_string lexer len) = name then
      advance lexer len
    else
      let lexer = skip_to_end_of_line lexer in
      let ch = peek_char lexer 0 in
      if is_newline ch then
        aux (skip_end_of_line lexer)
      else
        (* If we got here then we ran off the end of the file without
        finding a newline. Just bail. *)
        with_error lexer SyntaxError.error0011 in
  aux lexer

let scan_docstring_literal lexer =
  let (lexer, name, kind) = scan_docstring_header lexer in
  (* We need at least one line *)
  let lexer = skip_to_end_of_line lexer in
  let ch = peek_char lexer 0 in
  let lexer =
    if is_newline ch then
      scan_docstring_remainder name (skip_end_of_line lexer)
    else
      (* If we got here then we ran off the end of the file without
      finding a newline. Just bail. *)
      with_error lexer SyntaxError.error0011 in
  (lexer, kind)

let scan_xhp_label lexer =
  (* An XHP label has the same grammar as a Hack name. *)
  let (lexer, _) = scan_name lexer in
  lexer

let rec scan_xhp_element_name lexer =
  (* An XHP element name is a sequence of one or more XHP labels each separated
  by a single : or -.  Note that it is possible for an XHP element name to be
  followed immediately by a : or - that is the next token, so if we find
  a : or - not followed by a label, we need to terminate the token. *)
  let lexer = scan_xhp_label lexer in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  if (ch0 = ':' || ch0 = '-') && (is_name_nondigit ch1) then
    scan_xhp_element_name (advance lexer 1)
  else
    (lexer, TokenKind.XHPElementName)

(* Is the next token we're going to lex a possible xhp class name? *)
let is_xhp_class_name lexer =
  (peek_char lexer 0 = ':') && (is_name_nondigit (peek_char lexer 1))

let is_next_name lexer =
  is_name_nondigit (peek_char lexer 0)

let scan_xhp_class_name lexer =
  (* An XHP class name is a colon followed by an xhp name. *)
  if is_xhp_class_name lexer then
    let (lexer, _) = scan_xhp_element_name (advance lexer 1) in
    (lexer, TokenKind.XHPClassName)
  else
    let lexer = with_error lexer SyntaxError.error0008 in
    (advance lexer 1, TokenKind.Error)

let scan_xhp_string_literal lexer =
  (* XHP string literals are just straight up "find the closing quote"
     strings.  TODO: What about newlines embedded? *)
  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    match (ch0, ch1) with
      | ('\000', _) ->
        if at_end lexer then
          let lexer = with_error lexer SyntaxError.error0012 in
          (lexer, TokenKind.XHPStringLiteral)
        else
          let lexer = with_error lexer SyntaxError.error0006 in
          aux (advance lexer 1)

      | ('"', _) -> (advance lexer 1, TokenKind.XHPStringLiteral)
      | _ -> aux (advance lexer 1) in
  aux (advance lexer 1)

(* Note that this does not scan an XHP body *)
let scan_xhp_token lexer =
  (* TODO: HHVM requires that there be no trivia between < and name in an
     opening tag, but does allow trivia between </ and name in a closing tag.
     Consider allowing trivia in an opening tag. *)
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('{', _) -> (advance lexer 1, TokenKind.LeftBrace)
  | ('}', _) -> (advance lexer 1, TokenKind.RightBrace)
  | ('=', _) -> (advance lexer 1, TokenKind.Equal)
  | ('<', '/') -> (advance lexer 2, TokenKind.LessThanSlash)
  | ('<', _) ->
    if is_name_nondigit ch1 then scan_xhp_element_name (advance lexer 1)
    else (advance lexer 1, TokenKind.LessThan)
  | ('"', _) -> scan_xhp_string_literal lexer
  | ('/', '>') -> (advance lexer 2, TokenKind.SlashGreaterThan)
  | ('>', _) -> (advance lexer 1, TokenKind.GreaterThan)
  | _ ->
    if ch0 = invalid && at_end lexer then
      (lexer, TokenKind.EndOfFile)
    else if is_name_nondigit ch0 then
      scan_xhp_element_name lexer
    else
      let lexer = with_error lexer SyntaxError.error0006 in
      (advance lexer 1, TokenKind.Error)

let scan_xhp_comment lexer =
  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    let ch2 = peek_char lexer 2 in
    match (ch0, ch1, ch2) with
    | ('\000', _, _) -> with_error lexer SyntaxError.error0014
    | ('-', '-', '>') -> (advance lexer 3)
    | _ -> aux (advance lexer 1) in
  aux (advance lexer 4)

let scan_xhp_body lexer =
  let rec aux lexer =
    let ch = peek_char lexer 0 in
    match ch with
      | '\000' ->
        if at_end lexer then
          let lexer = with_error lexer SyntaxError.error0013 in
          lexer
        else
          let lexer = with_error lexer SyntaxError.error0006 in
          aux (advance lexer 1)
      | '{'
      | '<' -> lexer
      | _ -> aux (advance lexer 1) in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  let ch2 = peek_char lexer 2 in
  let ch3 = peek_char lexer 3 in
  match (ch0, ch1, ch2, ch3) with
  | ('\000', _, _, _) when at_end lexer -> (lexer, TokenKind.EndOfFile)
  | ('{', _, _, _) -> (advance lexer 1, TokenKind.LeftBrace)
  | ('<', '!', '-', '-') -> (scan_xhp_comment lexer, TokenKind.XHPComment)
  | ('<', '/', _, _) -> (advance lexer 2, TokenKind.LessThanSlash)
  | ('<', _, _, _) ->
    if is_name_nondigit ch1 then scan_xhp_element_name (advance lexer 1)
    else (advance lexer 1, TokenKind.LessThan)
  | _ -> ((aux lexer), TokenKind.XHPBody)

let scan_token in_type lexer =
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  let ch2 = peek_char lexer 2 in
  match (ch0, ch1, ch2) with
  | ('[', _, _) -> (advance lexer 1, TokenKind.LeftBracket)
  | (']', _, _) -> (advance lexer 1, TokenKind.RightBracket)
  | ('(', _, _) -> (advance lexer 1, TokenKind.LeftParen)
  | (')', _, _) -> (advance lexer 1, TokenKind.RightParen)
  | ('{', _, _) -> (advance lexer 1, TokenKind.LeftBrace)
  | ('}', _, _) -> (advance lexer 1, TokenKind.RightBrace)
  | ('.', '=', _) -> (advance lexer 2, TokenKind.DotEqual)
  | ('.', '.', '.') -> (advance lexer 3, TokenKind.DotDotDot)
  | ('.', '0', _)
  | ('.', '1', _)
  | ('.', '2', _)
  | ('.', '3', _)
  | ('.', '4', _)
  | ('.', '5', _)
  | ('.', '6', _)
  | ('.', '7', _)
  | ('.', '8', _)
  | ('.', '9', _) -> scan_after_decimal_point lexer
  | ('.', _, _) -> (advance lexer 1, TokenKind.Dot)
  | ('-', '=', _) -> (advance lexer 2, TokenKind.MinusEqual)
  | ('-', '-', _) -> (advance lexer 2, TokenKind.MinusMinus)
  | ('-', '>', _) -> (advance lexer 2, TokenKind.MinusGreaterThan)
  | ('-', _, _) -> (advance lexer 1, TokenKind.Minus)
  | ('+', '=', _) -> (advance lexer 2, TokenKind.PlusEqual)
  | ('+', '+', _) -> (advance lexer 2, TokenKind.PlusPlus)
  | ('+', _, _) -> (advance lexer 1, TokenKind.Plus)
  | ('*', '=', _) -> (advance lexer 2, TokenKind.StarEqual)
  | ('*', '*', '=') -> (advance lexer 3, TokenKind.StarStarEqual)
  | ('*', '*', _) -> (advance lexer 2, TokenKind.StarStar)
  | ('*', _, _) -> (advance lexer 1, TokenKind.Star)
  | ('~', _, _) -> (advance lexer 1, TokenKind.Tilde)
  | ('!', '=', '=') -> (advance lexer 3, TokenKind.ExclamationEqualEqual)
  | ('!', '=', _) -> (advance lexer 2, TokenKind.ExclamationEqual)
  | ('!', _, _) -> (advance lexer 1, TokenKind.Exclamation)
  | ('$', '$', _) -> (advance lexer 2, TokenKind.DollarDollar)
  | ('$', _, _) ->
    if is_name_nondigit ch1 then scan_variable lexer
    else (advance lexer 1, TokenKind.Dollar)
  | ('/', '=', _) -> (advance lexer 2, TokenKind.SlashEqual)
  | ('/', _, _) -> (advance lexer 1, TokenKind.Slash)
  | ('%', '=', _) -> (advance lexer 2, TokenKind.PercentEqual)
  | ('%', _, _) -> (advance lexer 1, TokenKind.Percent)
  | ('<', '<', '<') -> scan_docstring_literal lexer
  | ('<', '<', '=') -> (advance lexer 3, TokenKind.LessThanLessThanEqual)
  | ('<', '=', _) -> (advance lexer 2, TokenKind.LessThanEqual)
  | ('<', '<', _) -> (advance lexer 2, TokenKind.LessThanLessThan)
  | ('<', _, _) -> (advance lexer 1, TokenKind.LessThan)
  | ('>', '>', '=') -> (advance lexer 3, TokenKind.GreaterThanGreaterThanEqual)
  | ('>', '>', _) ->
    (* If we are parsing a generic type argument list then we might be
       at the >> in List<List<int>>.  In that case we want to lex two
       >'s, not one >>. *)
    if in_type then
      (advance lexer 1, TokenKind.GreaterThan)
    else
      (advance lexer 2, TokenKind.GreaterThanGreaterThan)
  | ('>', '=', _) -> (advance lexer 2, TokenKind.GreaterThanEqual)
  | ('>', _, _) -> (advance lexer 1, TokenKind.GreaterThan)
  | ('=', '=', '=') -> (advance lexer 3, TokenKind.EqualEqualEqual)
  | ('=', '=', '>') -> (advance lexer 3, TokenKind.EqualEqualGreaterThan)
  | ('=', '=', _) -> (advance lexer 2, TokenKind.EqualEqual)
  | ('=', '>', _) -> (advance lexer 2, TokenKind.EqualGreaterThan)
  | ('=', _, _) -> (advance lexer 1, TokenKind.Equal)
  | ('^', '=', _) -> (advance lexer 2, TokenKind.CaratEqual)
  | ('^', _, _) -> (advance lexer 1, TokenKind.Carat)
  | ('|', '=', _) -> (advance lexer 2, TokenKind.BarEqual)
  | ('|', '>', _) -> (advance lexer 2, TokenKind.BarGreaterThan)
  | ('|', '|', _) -> (advance lexer 2, TokenKind.BarBar)
  | ('|', _, _) -> (advance lexer 1, TokenKind.Bar)
  | ('&', '=', _) -> (advance lexer 2, TokenKind.AmpersandEqual)
  | ('&', '&', _) -> (advance lexer 2, TokenKind.AmpersandAmpersand)
  | ('&', _, _) -> (advance lexer 1, TokenKind.Ampersand)
  | ('?', '-', '>') -> (advance lexer 3, TokenKind.QuestionMinusGreaterThan)
  | ('?', '?', _) -> (advance lexer 2, TokenKind.QuestionQuestion)
  | ('?', _, _) -> (advance lexer 1, TokenKind.Question)
  | (':', ':', _) -> (advance lexer 2, TokenKind.ColonColon)
  | (':', _, _) -> (advance lexer 1, TokenKind.Colon)
  | (';', _, _) -> (advance lexer 1, TokenKind.Semicolon)
  | (',', _, _) -> (advance lexer 1, TokenKind.Comma)
  | ('@', _, _) -> (advance lexer 1, TokenKind.At)
  | ('0', 'x', _)
  | ('0', 'X', _) -> scan_hex_literal (advance lexer 2)
  | ('0', 'b', _)
  | ('0', 'B', _) -> scan_binary_literal (advance lexer 2)
  | ('0', _, _) -> scan_octal_or_float lexer
  | ('1', _, _)
  | ('2', _, _)
  | ('3', _, _)
  | ('4', _, _)
  | ('5', _, _)
  | ('6', _, _)
  | ('7', _, _)
  | ('8', _, _)
  | ('9', _, _) -> scan_decimal_or_float lexer
  | ('\'', _, _) -> scan_single_quote_string_literal lexer
  | ('"', _, _) -> scan_double_quote_string_literal lexer
  | ('\\', _, _) -> scan_qualified_name lexer
  (* Names *)
  | _ ->
    if ch0 = invalid && at_end lexer then
      (lexer, TokenKind.EndOfFile)
    else if is_name_nondigit ch0 then
      scan_name_or_qualified_name lexer
    else
      let lexer = with_error lexer SyntaxError.error0006 in
      (advance lexer 1, TokenKind.Error)

(* Lexing trivia *)

(* SPEC:
 *
 * white-space-character::
 *     new-line
 *     Space character (U+0020)
 *     Horizontal-tab character (U+0009)
 *
 * single-line-comment::
 *    //   input-characters-opt
 *    #    input-characters-opt
 *
 * new-line::
 *   Carriage-return character (U+000D)
 *   Line-feed character (U+000A)
 *   Carriage-return character followed by line-feed character
 *)

let scan_end_of_line lexer =
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('\r', '\n') ->  (advance lexer 2, Trivia.make_eol 2)
  | ('\r', _)
  | ('\n', _) ->  (advance lexer 1, Trivia.make_eol 1)
  | _ -> failwith "scan_end_of_line called while not on end of line!"

let scan_whitespace lexer =
  let lexer = skip_whitespace lexer in
  (lexer, Trivia.make_whitespace (width lexer))

let scan_delimited_comment lexer =
  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    if ch0 = invalid && at_end lexer then
      with_error lexer SyntaxError.error0007
    else if ch0 = '*' && ch1 = '/' then
      advance lexer 2
    else aux (advance lexer 1) in
  let lexer = aux (advance lexer 2) in
  let c = Trivia.make_delimited_comment (width lexer) in
  (lexer, c)

let scan_trivia lexer =
  let lexer = start_new_lexeme lexer in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('#', _)
  | ('/', '/') ->
    let lexer = skip_to_end_of_line lexer in
    let c = Trivia.make_single_line_comment (width lexer) in
    (lexer, Some c)
  | ('/', '*') ->
    let (lexer, c) = scan_delimited_comment lexer in
    (lexer, Some c)
  | (' ', _)
  | ('\t', _) ->
    let (lexer, w) = scan_whitespace lexer in
    (lexer, Some w)
  | ('\r', _)
  | ('\n', _) ->
    let (lexer, e) = scan_end_of_line lexer in
    (lexer, Some e)
  | _ -> (* Not trivia *) (lexer, None)

let scan_leading_trivia lexer =
  let rec aux lexer acc =
    let (lexer, trivia) = scan_trivia lexer in
    match trivia with
    | None -> (lexer, acc)
    | Some t -> aux lexer (t :: acc) in
  let (lexer, trivia_list) = aux lexer [] in
  (lexer, List.rev trivia_list)

let scan_trailing_trivia lexer =
  let rec aux lexer acc =
    let (lexer, trivia) = scan_trivia lexer in
    match trivia with
    | None -> (lexer, acc)
    | Some t ->
      if t.Trivia.kind = TriviaKind.EndOfLine then (lexer, t :: acc)
      else aux lexer (t :: acc) in
  let (lexer, trivia_list) = aux lexer [] in
  (lexer, List.rev trivia_list)

let is_next_xhp_class_name lexer =
  let (lexer, _) = scan_leading_trivia lexer in
  is_xhp_class_name lexer

let as_keyword kind lexer =
  if kind = TokenKind.Name then
    let text = current_text lexer in
    match TokenKind.from_string text with
    | Some keyword -> keyword
    | _ -> TokenKind.Name
  else
    kind

(* scanner takes a lexer, returns a lexer and a kind *)
let scan_token_and_leading_trivia scanner as_name lexer  =
  (* Get past the leading trivia *)
  let (lexer, leading) = scan_leading_trivia lexer in
  (* Remember where we were when we started this token *)
  let lexer = start_new_lexeme lexer in
  let (lexer, kind) = scanner lexer in
  let kind = if as_name then kind else as_keyword kind lexer in
  let w = width lexer in
  (lexer, kind, w, leading)

(* scanner takes a lexer, returns a lexer and a kind *)
let scan_token_and_trivia scanner as_name lexer  =
  let (lexer, kind, w, leading) =
    scan_token_and_leading_trivia scanner as_name lexer in
  let (lexer, trailing) = scan_trailing_trivia lexer in
  (lexer, Token.make kind w leading trailing)

(* tokenizer takes a lexer, returns a lexer and a token *)
let scan_assert_progress tokenizer lexer  =
  let original_remaining = remaining lexer in
  let (lexer, token) = tokenizer lexer in
  let new_remaining = remaining lexer in
  if (new_remaining < original_remaining ||
    original_remaining = 0 &&
    new_remaining = 0 &&
    (Token.kind token) = TokenKind.EndOfFile) then
      (lexer, token)
  else begin
    let message = Printf.sprintf
      "failed to make progress at %d\n" lexer.offset in
    print_endline message;
    assert false
  end

let scan_next_token scanner as_name lexer =
  let tokenizer = scan_token_and_trivia scanner as_name in
  scan_assert_progress tokenizer lexer

(* Entrypoints *)
(* TODO: Instead of passing Boolean flags, create a flags enum? *)

let next_token lexer =
  scan_next_token (scan_token false) false lexer

let next_token_as_name lexer =
  scan_next_token (scan_token false) true lexer

let next_token_in_type lexer =
  scan_next_token (scan_token true) false lexer

let next_xhp_element_token lexer =
  (* XHP elements have whitespace, newlines and Hack comments. *)
  let tokenizer lexer =
    let (lexer, kind, w, leading) =
      scan_token_and_leading_trivia scan_xhp_token true lexer in
    (* We cannot scan trivia after a > or /> because the next thing
       might be part of an XHP body, and that's not trivia, it's body
       text.

       TODO: If we are at the outermost > or /> in an XHP expression then the
       trailing trivia *should* be associated with the token. *)
    match kind with
    | TokenKind.GreaterThan
    | TokenKind.SlashGreaterThan ->
      (lexer, Token.make kind w leading [])
    | _ ->
      let (lexer, trailing) = scan_trailing_trivia lexer in
      (lexer, Token.make kind w leading trailing) in
  let (lexer, token) = scan_assert_progress tokenizer lexer in
  let token_width = Token.width token in
  let trailing_width = Token.trailing_width token in
  let token_start_offset = lexer.offset - trailing_width - token_width in
  let token_text = SourceText.sub lexer.text token_start_offset token_width in
  (lexer, token, token_text)

let next_xhp_body_token lexer =
  (* XHP bodies do not have whitespace, newlines or Hack comments. *)
  let scanner lexer =
    let lexer = start_new_lexeme lexer in
    let (lexer, kind) = scan_xhp_body lexer in
    let w = width lexer in
    (lexer, Token.make kind w [] []) in
  scan_assert_progress scanner lexer

let next_xhp_class_name lexer =
  scan_token_and_trivia scan_xhp_class_name false lexer

let next_xhp_name lexer =
  scan_token_and_trivia scan_xhp_element_name false lexer

let is_next_xhp_category_name lexer =
  (* An XHP category is an xhp element name preceded by a %. *)
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  ch0 = '%' && is_name_nondigit ch1

let scan_xhp_category_name lexer =
  if is_next_xhp_category_name lexer then
    let (lexer, _) = scan_xhp_element_name (advance lexer 1) in
    (lexer, TokenKind.XHPCategoryName)
  else
    scan_token false lexer

let next_xhp_category_name lexer =
  scan_token_and_trivia scan_xhp_category_name false lexer
