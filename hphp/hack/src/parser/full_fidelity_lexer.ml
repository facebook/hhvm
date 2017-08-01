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

let peek_char lexer index =
  let i = lexer.offset + index in
  if i >= SourceText.length lexer.text || i < 0 then invalid
  else SourceText.get lexer.text i

let peek_string lexer size =
  let r = (SourceText.length lexer.text) - lexer.offset in
  if r < 0 then ""
  else if r < size then SourceText.sub lexer.text lexer.offset r
  else SourceText.sub lexer.text lexer.offset size

let match_string lexer s =
  s = peek_string lexer (String.length s)

let advance lexer index =
  { lexer with offset = lexer.offset + index }

let width lexer =
  lexer.offset - lexer.start

let current_text lexer =
  SourceText.sub lexer.text lexer.start (width lexer)

let current_text_at lexer length relative_start =
  SourceText.sub lexer.text (lexer.start + relative_start) length

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

let rec skip_to_end_of_line_or_end_tag lexer =
  let ch = peek_char lexer 0 in
  if is_newline ch then lexer
  else if ch = invalid && at_end lexer then lexer
  else if ch = '?' && peek_char lexer 1 = '>' then lexer
  else skip_to_end_of_line_or_end_tag (advance lexer 1)

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
  let lexer = scan_name_impl (advance lexer 1) in
  (lexer, TokenKind.Variable)

let rec scan_with_underscores accepted_char lexer =
  let ch = peek_char lexer 0 in
  if accepted_char ch then scan_with_underscores accepted_char (advance lexer 1)
  else if ch = '_' && accepted_char (peek_char lexer 1) then
    scan_with_underscores accepted_char (advance lexer 2)
  else lexer

let rec scan_decimal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_decimal_digit ch then scan_decimal_digits (advance lexer 1)
  else lexer

let scan_decimal_digits_with_underscores lexer =
  scan_with_underscores is_decimal_digit lexer

let rec scan_octal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_octal_digit ch then scan_octal_digits (advance lexer 1)
  else lexer

let scan_octal_digits_with_underscores lexer =
  scan_with_underscores is_octal_digit lexer

let scan_binary_digits_with_underscores lexer =
  scan_with_underscores is_binary_digit lexer

let rec scan_hexadecimal_digits lexer =
  let ch = peek_char lexer 0 in
  if is_hexadecimal_digit ch then scan_hexadecimal_digits (advance lexer 1)
  else lexer

let scan_hexadecimal_digits_with_underscores lexer =
  scan_with_underscores is_hexadecimal_digit lexer

let scan_hex_literal lexer =
  let ch = peek_char lexer 0 in
  if not (is_hexadecimal_digit ch) then
    let lexer = with_error lexer SyntaxError.error0001 in
    (lexer, TokenKind.HexadecimalLiteral)
  else
    (scan_hexadecimal_digits_with_underscores lexer, TokenKind.HexadecimalLiteral)

let scan_binary_literal lexer =
  let ch = peek_char lexer 0 in
  if not (is_binary_digit ch) then
    let lexer = with_error lexer SyntaxError.error0002 in
    (lexer, TokenKind.BinaryLiteral)
  else
    (scan_binary_digits_with_underscores lexer, TokenKind.BinaryLiteral)

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
        else
        (* This is irritating - we only want to allow underscores for integer literals.
         * Deferring the lexing with underscores here allows us to make sure we're not dealing
         * with floats. *)
        let lexer_oct_with_underscores = scan_octal_digits_with_underscores lexer in
        (lexer_oct_with_underscores, TokenKind.OctalLiteral)
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
  let lexer_no_underscores = scan_decimal_digits lexer in
  let lexer_with_underscores = scan_decimal_digits_with_underscores lexer in
  let ch = peek_char lexer_no_underscores 0 in
  match ch with
  | '.' -> (* 123. *) scan_after_decimal_point lexer_no_underscores
  | 'e' | 'E' -> (* 123e *) scan_exponent lexer_no_underscores
  | _ -> (* 123 *) (lexer_with_underscores, TokenKind.DecimalLiteral)


let scan_execution_string_literal lexer =
  (* TODO: PHP supports literals of the form `command` where the command
  is then executed as a shell command.  Hack does not support this.
  We should give an error if this feature is used in Hack, but we should lex
  it anyways to give a good error message.

  TODO: Can execution strings have embedded expressions like double-quoted
  strings?

  TODO: Are there any escape sequences in execution strings?

  TODO: Are there any illegal characters in execution strings?
  *)

  let rec aux lexer =
    let ch = peek_char lexer 0 in
    match ch with
    | '\000' ->
      if at_end lexer then
        let lexer = with_error lexer SyntaxError.error0012 in
        (lexer, TokenKind.ExecutionString)
      else
        let lexer = with_error lexer SyntaxError.error0006 in
        aux (advance lexer 1)
    | '`' -> (advance lexer 1, TokenKind.ExecutionString)
    | _ -> aux (advance lexer 1) in
  aux (advance lexer 1)

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
    (* TODO: Consider producing an error for a malformed hex escape *)
    (* let lexer = with_error lexer SyntaxError.error0005 in *)
    advance lexer 2
  else if not (is_hexadecimal_digit ch3) then
    (* let lexer = with_error lexer SyntaxError.error0005 in *)
    advance lexer 3
  else
    advance lexer 4

let scan_unicode_escape lexer =
  (* At present the lexer is pointing at \u *)
  let ch2 = peek_char lexer 2 in
  let ch3 = peek_char lexer 3 in
  match (ch2, ch3) with
  | ( '{', '$') ->
    (* We have a malformed unicode escape that contains a possible embedded
    expression. Eat the \u and keep on processing the embedded expression. *)
    (* TODO: Consider producing a warning for a malformed unicode escape. *)
    advance lexer 2
  | ('{', _) ->
    (* We have a possibly well-formed escape sequence, and at least we know
       that it is not an embedded expression. *)
    (* TODO: Consider producing an error if the digits are out of range
       of legal Unicode characters. *)
    (* TODO: Consider producing an error if there are no digits. *)
    (* Skip over the slash, u and brace, and start lexing the number. *)
    let lexer = advance lexer 3 in
    let lexer = scan_hexadecimal_digits lexer in
    let ch = peek_char lexer 0 in
    if ch != '}' then
      (* TODO: Consider producing a warning for a malformed unicode escape. *)
      lexer
    else
      advance lexer 1
  | _ ->
    (* We have a malformed unicode escape sequence. Bail out. *)
    (* TODO: Consider producing a warning for a malformed unicode escape. *)
    advance lexer 2

let rec skip_uninteresting_double_quote_string_characters lexer =
  let ch = peek_char lexer 0 in
  if is_name_nondigit ch then
    lexer
  else match ch with
  | '\000' | '"' | '\\' | '$' | '{' | '[' | ']' | '-'
  | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' -> lexer
  | _ -> skip_uninteresting_double_quote_string_characters (advance lexer 1)

let scan_integer_literal lexer =
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('0', 'x')
  | ('0', 'X') -> scan_hex_literal (advance lexer 2)
  | ('0', 'b')
  | ('0', 'B') -> scan_binary_literal (advance lexer 2)
  | ('0', _) -> (scan_octal_digits_with_underscores lexer, TokenKind.OctalLiteral)
  | _ -> (scan_decimal_digits_with_underscores lexer, TokenKind.DecimalLiteral)

let scan_double_quote_string_literal_from_start lexer =
  let rec aux lexer =
    (* If there's nothing interesting in this double-quoted string then
       we can just hand it back as-is. *)
    let lexer = skip_uninteresting_double_quote_string_characters lexer in
    match peek_char lexer 0 with
    | '\000' ->
      (* If the string is unterminated then give an error; if this is an
         embedded zero character then give an error and recurse; we might
         be able to make more progress. *)
      if at_end lexer then
        let lexer = with_error lexer SyntaxError.error0012 in
        (lexer, TokenKind.DoubleQuotedStringLiteral)
      else
        let lexer = with_error lexer SyntaxError.error0006 in
        aux (advance lexer 1)
    | '"' ->
      (* We made it to the end without finding a special character. *)
      (advance lexer 1, TokenKind.DoubleQuotedStringLiteral)
    | _ -> (* We've found a backslash, dollar or brace. *)
      (lexer, TokenKind.DoubleQuotedStringLiteralHead) in
  aux (advance lexer 1)

let is_heredoc_tail lexer name =
  (* A heredoc tail is the identifier immediately preceded by a newline
  and immediately followed by an optional semi and then a newline.

  Note that the newline and optional semi are not part of the literal;
  the literal's lexeme ends at the end of the name. Either there is
  no trivia and the next token is a semi-with-trailing-newline, or
  the trailing trivia is a newline.

  This odd rule is to ensure that both
  $x = <<<HERE
  something
  HERE;

  and

  $x = <<<HERE
  something
  HERE
  . "something else";

  are legal.
  *)

  if not (is_newline (peek_char lexer (-1))) then
    false
  else
    let len = String.length name in
    let ch0 = peek_char lexer len in
    let ch1 = peek_char lexer (len + 1) in
    ((is_newline ch0) || ch0 = ';' && (is_newline ch1)) &&
      (peek_string lexer len) = name

let scan_string_literal_in_progress lexer name =
  let is_heredoc = (String.length name) != 0 in
  let ch0 = peek_char lexer 0 in
  if is_name_nondigit ch0 then
    if is_heredoc && (is_heredoc_tail lexer name) then
      (scan_name_impl lexer, TokenKind.HeredocStringLiteralTail)
    else
      (scan_name_impl lexer, TokenKind.Name)
  else
    let ch1 = peek_char lexer 1 in
    match (ch0, ch1) with
    | ('\000', _) ->
      if at_end lexer then
        let lexer = with_error lexer SyntaxError.error0012 in
        let kind =
          if is_heredoc then TokenKind.HeredocStringLiteralTail
          else TokenKind.DoubleQuotedStringLiteralTail in
        (lexer, kind)
      else
        let lexer = with_error lexer SyntaxError.error0006 in
        let lexer = advance lexer 1 in
        let lexer = skip_uninteresting_double_quote_string_characters lexer in
        (lexer, TokenKind.StringLiteralBody)
    | ('"', _) ->
      let kind =
        if is_heredoc then TokenKind.StringLiteralBody
        else TokenKind.DoubleQuotedStringLiteralTail in
      (advance lexer 1, kind)
    | ('$', _) ->
      if is_name_nondigit ch1 then scan_variable lexer
      else (advance lexer 1, TokenKind.Dollar)
    | ('{', _) -> (advance lexer 1, TokenKind.LeftBrace)

    (* In these cases we just skip the escape sequence and
     keep on scanning for special characters. *)
    | ('\\', '\\')
    | ('\\', '"')
    | ('\\', '$')
    | ('\\', 'e')
    | ('\\', 'f')
    | ('\\', 'n')
    | ('\\', 'r')
    | ('\\', 't')
    | ('\\', 'v')
    (* Same in these cases; there might be more octal characters following but
       if there are, we'll just eat them as normal characters. *)
    | ('\\', '0')
    | ('\\', '1')
    | ('\\', '2')
    | ('\\', '3')
    | ('\\', '4')
    | ('\\', '5')
    | ('\\', '6')
    | ('\\', '7') ->
      let lexer = advance lexer 2 in
      let lexer = skip_uninteresting_double_quote_string_characters lexer in
      (lexer, TokenKind.StringLiteralBody)
    | ('\\', 'x') ->
      let lexer = scan_hexadecimal_escape lexer in
      let lexer = skip_uninteresting_double_quote_string_characters lexer in
      (lexer, TokenKind.StringLiteralBody)
    | ('\\', 'u') ->
      let lexer = scan_unicode_escape lexer in
      let lexer = skip_uninteresting_double_quote_string_characters lexer in
      (lexer, TokenKind.StringLiteralBody)
    | ('\\', '{') ->
      (* The rules for escaping open braces in Hack are bizarre. Suppose we have
      $x = 123;
      $y = 456;
      $z = "\{$x,$y\}";
      What is the value of $z?  Naively you would think that the backslash
      escapes the braces, and the variables are embedded, so {123,456}. But
      that's not what happens. Yes, the backslash makes the brace no longer
      the opening brace of an expression. But the backslash is still part
      of the string!  This is the string \{123,456\}.

      TODO: We might want to fix this because this is very strange. *)
      (* Eat the backslash and the brace. *)

      let lexer = advance lexer 2 in
      (lexer, TokenKind.StringLiteralBody)
    | ('\\', _) ->
      (* TODO: A backslash followed by something other than an escape sequence
         is legal in hack, and treated as though it was just the backslash
         and the character. However we might consider making this a warning.
         It is particularly egregious when we have something like:
         $x = "abcdef \
               ghi";
         The author of the code likely means the backslash to mean line
         continuation but in fact it just means to put a backslash and newline
         in the string.
         *)
         let lexer = advance lexer 1 in
         let lexer = skip_uninteresting_double_quote_string_characters lexer in
         (lexer, TokenKind.StringLiteralBody)
    | ('[', _) ->
      let lexer = advance lexer 1 in
      (lexer, TokenKind.LeftBracket)
    | (']', _) ->
      let lexer = advance lexer 1 in
      (lexer, TokenKind.RightBracket)
    | ('-', '>') ->
      let lexer = advance lexer 2 in
      (lexer, TokenKind.MinusGreaterThan)
    | ('0', _)
    | ('1', _)
    | ('2', _)
    | ('3', _)
    | ('4', _)
    | ('5', _)
    | ('6', _)
    | ('7', _)
    | ('8', _)
    | ('9', _) ->
      scan_integer_literal lexer
    | _ ->
      (* Nothing interesting here. Skip it and find the next
         interesting character. *)
      let lexer = advance lexer 1 in
      let lexer = skip_uninteresting_double_quote_string_characters lexer in
      (lexer, TokenKind.StringLiteralBody)

(*  A heredoc string literal has the form

header
optional body
trailer

The header is:

<<< (optional whitespace) name (no whitespace) (newline)

The optional body is:

any characters whatsoever including newlines (newline)

The trailer is:

(no whitespace) name (no whitespace) (optional semi) (no whitespace) (newline)

The names must be identical.  The trailing semi and newline must be present.

The body is any and all characters, up to the first line that exactly matches
the trailer.

The body may contain embedded expressions.

A nowdoc string literal has the same form except that the first name is
enclosed in single quotes, and it may not contain embedded expressions.

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
      (* Starting with PHP 5.3.0, the opening Heredoc identifier
         may optionally be enclosed in double quotes:*)
      let lexer = if ch = '"' then advance lexer 1 else lexer in
      let lexer, name = scan_docstring_name_actual lexer in
      let lexer =
        if ch = '"' && peek_char lexer 0 = '\"' then advance lexer 1 else lexer
      in
      lexer, name
  in
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
    let lexer = scan_docstring_remainder name lexer in
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

let scan_xhp_class_name lexer =
  (* An XHP class name is a colon followed by an xhp name. *)
  if is_xhp_class_name lexer then
    let (lexer, _) = scan_xhp_element_name (advance lexer 1) in
    (lexer, TokenKind.XHPClassName)
  else
    let lexer = with_error lexer SyntaxError.error0008 in
    (advance lexer 1, TokenKind.ErrorToken)

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
  | ('<', _) -> (advance lexer 1, TokenKind.LessThan)
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
      (advance lexer 1, TokenKind.ErrorToken)

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
  (* Naively you might think that an XHP body is just a bunch of characters,
     terminated by an embedded { } expression or a tag.  However, whitespace
     and newlines are relevant in XHP bodies because they are "soft".
     That is, any section of contiguous trivia has the same semantics as a
     single space or newline -- just as in HTML.

     Obviously this is of relevance to code formatters.

     Therefore we detect whitespace and newlines within XHP bodies and treat
     it as trivia surrounding the tokens within the body.

     TODO: Is this also true of whitespace within XHP comments? If so then
     we need to make XHP comments a sequence of tokens, rather than a
     single token as they are now.
  *)
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
      | '\t' | ' ' | '\r' | '\n' | '{' | '}' | '<' -> lexer
      | _ -> aux (advance lexer 1) in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  let ch2 = peek_char lexer 2 in
  let ch3 = peek_char lexer 3 in
  match (ch0, ch1, ch2, ch3) with
  | ('\000', _, _, _) when at_end lexer -> (lexer, TokenKind.EndOfFile)
  | ('{', _, _, _) -> (advance lexer 1, TokenKind.LeftBrace)
  | ('}', _, _, _) -> (advance lexer 1, TokenKind.RightBrace)
  | ('<', '!', '-', '-') -> (scan_xhp_comment lexer, TokenKind.XHPComment)
  | ('<', '/', _, _) -> (advance lexer 2, TokenKind.LessThanSlash)
  | ('<', _, _, _) -> (advance lexer 1, TokenKind.LessThan)
  | _ -> ((aux lexer), TokenKind.XHPBody)

let scan_dollar_token lexer =
  (*
    We have a problem here.  We wish to be able to lexically analyze both
    PHP and Hack, but the introduction of $$ to Hack makes them incompatible.
    "$$x" and "$$ $x" are legal in PHP, but illegal in Hack.
    The rule in PHP seems to be that $ is a prefix operator, it is a token,
    it can be followed by trivia, but the next token has to be another $
    operator, a variable $x, or a {.

    Here's a reasonable compromise.  (TODO: Review this decision.)

    $$x lexes as $ $x
    $$$x lexes as $ $ $x
    and so on.

    $$ followed by anything other than a name or a $ lexes as $$.

    This means that lexing a PHP program which contains "$$ $x" is different
    will fail at parse time, but I'm willing to live with that.

    This means that lexing a Hack program which contains
    "$x |> $$instanceof Foo" produces an error as well.

    If these decisions are unacceptable then we will need to make the lexer
    be aware of whether it is lexing PHP or Hack; thus far we have not had
    to make this distinction.

     *)
  (* We are already at $. *)
  let ch1 = peek_char lexer 1 in
  let ch2 = peek_char lexer 2 in
  match ch1 with
  | '$' ->
    if is_name_nondigit ch2 then (advance lexer 1, TokenKind.Dollar) (* $$x *)
    else if ch2 = '$' then (advance lexer 1, TokenKind.Dollar) (* $$$ *)
    else (advance lexer 2, TokenKind.DollarDollar) (* $$ *)
  | _ ->
    if is_name_nondigit ch1 then scan_variable lexer (* $x *)
    else (advance lexer 1, TokenKind.Dollar) (* $ *)

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
  | ('$', _, _) -> scan_dollar_token lexer
  | ('/', '=', _) -> (advance lexer 2, TokenKind.SlashEqual)
  | ('/', _, _) -> (advance lexer 1, TokenKind.Slash)
  | ('%', '=', _) -> (advance lexer 2, TokenKind.PercentEqual)
  | ('%', _, _) -> (advance lexer 1, TokenKind.Percent)
  | ('<', '<', '<') -> scan_docstring_literal lexer
  | ('<', '<', '=') -> (advance lexer 3, TokenKind.LessThanLessThanEqual)
  (* TODO: We lex and parse the spaceship operator.
     TODO: This is not in the spec at present.  We should either make it an
     TODO: error, or add it to the specification. *)
  | ('<', '=', '>') -> (advance lexer 3, TokenKind.LessThanEqualGreaterThan)
  | ('<', '>', _) -> (advance lexer 2, TokenKind.LessThanGreaterThan)
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
  | ('?', '>', _) -> (advance lexer 2, TokenKind.QuestionGreaterThan)
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
  | ('`', _, _) -> scan_execution_string_literal lexer
  | ('\'', _, _) -> scan_single_quote_string_literal lexer
  | ('"', _, _) -> scan_double_quote_string_literal_from_start lexer
  | ('\\', _, _) -> scan_qualified_name lexer
  (* Names *)
  | _ ->
    if ch0 = invalid && at_end lexer then
      (lexer, TokenKind.EndOfFile)
    else if is_name_nondigit ch0 then
      scan_name_or_qualified_name lexer
    else
      let lexer = with_error lexer SyntaxError.error0006 in
      (advance lexer 1, TokenKind.ErrorToken)

let scan_token_inside_type = scan_token true
let scan_token_outside_type = scan_token false

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

let scan_hash_comment lexer =
  let lexer = skip_to_end_of_line lexer in
  let c = Trivia.make_single_line_comment (width lexer) in
  (lexer, c)

let scan_single_line_comment lexer =
  (* A fallthrough comment is two slashes, any amount of whitespace,
    FALLTHROUGH, and the end of the line.
    An unsafe comment is two slashes, any amount of whitespace,
    UNSAFE, and then any characters may follow.
    TODO: Consider allowing trailing space for fallthrough.
    TODO: Consider allowing lowercase fallthrough.
  *)
  let lexer = advance lexer 2 in
  let lexer_ws = skip_whitespace lexer in
  let lexer = skip_to_end_of_line_or_end_tag lexer_ws in
  let w = width lexer in
  let remainder = lexer.offset - lexer_ws.offset in
  let c =
    if remainder = 11 && peek_string lexer_ws 11 = "FALLTHROUGH" then
      Trivia.make_fallthrough w
    else if remainder >= 6 && peek_string lexer_ws 6 = "UNSAFE" then
      Trivia.make_unsafe w
    else
      Trivia.make_single_line_comment w in
  (lexer, c)

let rec skip_to_end_of_delimited_comment lexer =
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  if ch0 = invalid && at_end lexer then
    with_error lexer SyntaxError.error0007
  else if ch0 = '*' && ch1 = '/' then
    advance lexer 2
  else skip_to_end_of_delimited_comment (advance lexer 1)

let scan_delimited_comment lexer =
  (* An unsafe expression comment is a delimited comment that begins with any
    whitespace, followed by UNSAFE_EXPR, followed by any text.

    The original lexer lexes a fixme / ignore error as:

    slash star [whitespace]* HH_FIXME [whitespace or newline]* leftbracket
    [whitespace or newline]* integer [any text]* star slash

    Notice that the original lexer oddly enough does not verify that there
    is a right bracket.

    For our purposes we will just check for HH_FIXME / HH_IGNORE_ERROR;
    a later pass can try to parse out the integer if there is one,
    give a warning if there is not, and so on. *)

  let lexer = advance lexer 2 in
  let lexer_ws = skip_whitespace lexer in
  let lexer = skip_to_end_of_delimited_comment lexer_ws in
  let w = width lexer in
  let c =
    if match_string lexer_ws "UNSAFE_EXPR" then
      Trivia.make_unsafe_expression w
    else if match_string lexer_ws "HH_FIXME" then
      Trivia.make_fix_me w
    else if match_string lexer_ws "HH_IGNORE_ERROR" then
      Trivia.make_ignore_error w
    else
      Trivia.make_delimited_comment w in
  (lexer, c)

let scan_php_trivia lexer =
  (* Hack does not support PHP style embedded markup:
  <?php
  if (x) {
  ?>
  <foo>bar</foo>
  <?php
  } else { ... }

However, ?> is never legal in Hack, so we can treat ?> ... any text ... <?php
as a comment, and then give an error saying that this feature is not supported
in Hack.

TODO: Give an error if this appears in a Hack program.
*)
  let lexer = start_new_lexeme lexer in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | ('#', _) ->
    let (lexer, c) = scan_hash_comment lexer in
    (lexer, Some c)
  | ('/', '/') ->
    let (lexer, c) = scan_single_line_comment lexer in
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

let scan_xhp_trivia lexer =
  (* TODO: Should XHP comments <!-- --> be their own thing, or a kind of
  trivia associated with a token? Right now they are the former. *)
  let lexer = start_new_lexeme lexer in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  match (ch0, ch1) with
  | (' ', _)
  | ('\t', _) ->
    let (lexer, w) = scan_whitespace lexer in
    (lexer, Some w)
  | ('\r', _)
  | ('\n', _) ->
    let (lexer, e) = scan_end_of_line lexer in
    (lexer, Some e)
  | _ -> (* Not trivia *) (lexer, None)

let scan_leading_trivia scanner lexer =
  let rec aux lexer acc =
    let (lexer, trivia) = scanner lexer in
    match trivia with
    | None -> (lexer, acc)
    | Some t -> aux lexer (t :: acc) in
  let (lexer, trivia_list) = aux lexer [] in
  (lexer, List.rev trivia_list)

let scan_leading_php_trivia lexer =
  scan_leading_trivia scan_php_trivia lexer

let scan_leading_xhp_trivia lexer =
  scan_leading_trivia scan_xhp_trivia lexer

let scan_trailing_trivia scanner lexer  =
  let rec aux lexer acc =
    let (lexer, trivia) = scanner lexer in
    match trivia with
    | None -> (lexer, acc)
    | Some t ->
      if t.Trivia.kind = TriviaKind.EndOfLine then (lexer, t :: acc)
      else aux lexer (t :: acc) in
  let (lexer, trivia_list) = aux lexer [] in
  (lexer, List.rev trivia_list)

let scan_trailing_php_trivia lexer =
  scan_trailing_trivia scan_php_trivia lexer

let scan_trailing_xhp_trivia lexer =
  scan_trailing_trivia scan_xhp_trivia lexer

let is_next_name lexer =
  let (lexer, _) = scan_leading_php_trivia lexer in
  is_name_nondigit (peek_char lexer 0)

let is_next_xhp_class_name lexer =
  let (lexer, _) = scan_leading_php_trivia lexer in
  is_xhp_class_name lexer

let as_case_insensitive_keyword text =
  (* Some keywords are case-insensitive in Hack or PHP. *)
  (* TODO: Consider making non-lowercase versions of these keywords errors
     in strict mode. *)
  (* TODO: Consider making these illegal, period, and code-modding away all
  non-lower versions in our codebase. *)
  let lower = String.lowercase text in
  match lower with
  | "eval" | "isset" | "unset" | "empty" | "const"
  | "and"  | "or"    | "xor"  | "as" | "print" | "throw"
  | "true" | "false" | "null" | "array" | "instanceof" -> lower
  | _ -> text

let as_keyword kind lexer =
  if kind = TokenKind.Name then
    let text = as_case_insensitive_keyword (current_text lexer) in
    match TokenKind.from_string text with
    | Some keyword -> keyword
    | _ -> TokenKind.Name
  else
    kind

(* scanner takes a lexer, returns a lexer and a kind *)
let scan_token_and_leading_trivia scanner as_name lexer  =
  (* Get past the leading trivia *)
  let (lexer, leading) = scan_leading_php_trivia lexer in
  (* Remember where we were when we started this token *)
  let lexer = start_new_lexeme lexer in
  let (lexer, kind) = scanner lexer in
  let kind = if as_name then kind else as_keyword kind lexer in
  let w = width lexer in
  (lexer, kind, w, leading)

let suppress_trailing_trivia kind =
  match kind with
  | TokenKind.DoubleQuotedStringLiteralHead -> true
  | _ -> false

(* scanner takes a lexer, returns a lexer and a kind *)
let scan_token_and_trivia scanner as_name lexer  =
  let (lexer, kind, w, leading) =
    scan_token_and_leading_trivia scanner as_name lexer in
  let (lexer, trailing) =
    if suppress_trailing_trivia kind then (lexer, [])
    else scan_trailing_php_trivia lexer in
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

let scan_next_token as_name scanner lexer =
  let tokenizer = scan_token_and_trivia scanner as_name in
  scan_assert_progress tokenizer lexer

let scan_next_token_as_name = scan_next_token true
let scan_next_token_as_keyword = scan_next_token false

(* Entrypoints *)
(* TODO: Instead of passing Boolean flags, create a flags enum? *)

let next_token lexer =
  scan_next_token_as_keyword scan_token_outside_type lexer

let next_token_no_trailing lexer =
  let tokenizer lexer =
    let (lexer, kind, w, leading) =
      scan_token_and_leading_trivia scan_token_outside_type false lexer in
    (lexer, Token.make kind w leading []) in
  scan_assert_progress tokenizer lexer

let next_token_in_string lexer name =
  let lexer = start_new_lexeme lexer in
  (* We're inside a string. Do not scan leading trivia. *)
  let (lexer, kind) = scan_string_literal_in_progress lexer name in
  let w = width lexer in
  (* Only scan trailing trivia if we've finished the string. *)
  let (lexer, trailing) =
    match kind with
    | TokenKind.DoubleQuotedStringLiteralTail
    | TokenKind.HeredocStringLiteralTail -> scan_trailing_php_trivia lexer
    | _ -> (lexer, []) in
  let token = Token.make kind w [] trailing in
  (lexer, token)

let next_docstring_header lexer =
  (* We're at the beginning of a heredoc string literal. Scan leading
     trivia but not trailing trivia. *)
  let (lexer, leading) = scan_leading_php_trivia lexer in
  let lexer = start_new_lexeme lexer in
  let (lexer, name, _) = scan_docstring_header lexer in
  let w = width lexer in
  let token = Token.make TokenKind.HeredocStringLiteralHead w leading [] in
  (lexer, token, name)

let next_token_as_name lexer =
  scan_next_token_as_name scan_token_outside_type lexer

let next_token_in_type lexer =
  scan_next_token_as_keyword scan_token_inside_type lexer

let next_xhp_element_token ~no_trailing lexer =
  (* XHP elements have whitespace, newlines and Hack comments. *)
  let tokenizer lexer =
    let (lexer, kind, w, leading) =
      scan_token_and_leading_trivia scan_xhp_token true lexer in
    (* We do not scan trivia after an XHPOpen's >. If that is the beginning of
       an XHP body then we want any whitespace or newlines to be leading trivia
       of the body token. *)
    match kind with
    | TokenKind.GreaterThan when no_trailing ->
      (lexer, Token.make kind w leading [])
    | _ ->
      let (lexer, trailing) = scan_trailing_php_trivia lexer in
      (lexer, Token.make kind w leading trailing) in
  let (lexer, token) = scan_assert_progress tokenizer lexer in
  let token_width = Token.width token in
  let trailing_width = Token.trailing_width token in
  let token_start_offset = lexer.offset - trailing_width - token_width in
  let token_text = SourceText.sub lexer.text token_start_offset token_width in
  (lexer, token, token_text)

let next_xhp_body_token lexer =
  let scanner lexer  =
    let (lexer, leading) = scan_leading_xhp_trivia lexer in
    let lexer = start_new_lexeme lexer in
    let (lexer, kind) = scan_xhp_body lexer in
    let w = width lexer in
    let (lexer, trailing) =
      (* Trivia (leading and trailing) is semantically
         significant for XHPBody tokens. When we find elements or
         braced expressions inside the body, the trivia should be
         seen as leading the next token, but we should certainly
         keep it trailing if this is an XHPBody token. *)
      if kind = TokenKind.XHPBody
      then scan_trailing_xhp_trivia lexer
      else (lexer, [])
    in
    (lexer, Token.make kind w leading trailing) in
  scan_assert_progress scanner lexer

let next_xhp_class_name lexer =
  scan_token_and_trivia scan_xhp_class_name false lexer

let next_xhp_name lexer =
  scan_token_and_trivia scan_xhp_element_name false lexer

let make_markup_token lexer =
  Token.make TokenKind.Markup (width lexer) [] []

let skip_to_end_of_markup lexer ~is_leading_section =
  let make_markup_and_suffix lexer =
    let markup_text = make_markup_token lexer in
    let less_than_question_token =
      Token.make TokenKind.LessThanQuestion 2 [] []
    in
    let make_long_tag lexer size =
      (* skip name*)
      let lexer = advance lexer size in
      (* single line comments that follow the language in leading markup_text
        determine the file check mode, read the trailing trivia and attach it
        to the language token *)
      let lexer, trailing =
        if is_leading_section then scan_trailing_php_trivia lexer
        else lexer, []
      in
      let name = Token.make TokenKind.Name size [] trailing in
      lexer, markup_text, Some (less_than_question_token, Some name)
    in
    (* skip <? *)
    let lexer = advance lexer 2 in
    let ch0 = peek_char lexer 0 in
    let ch1 = peek_char lexer 1 in
    let ch2 = peek_char lexer 2 in
    match ch0, ch1, ch2 with
    | ('H' | 'h'), ('H' | 'h'), _ -> make_long_tag lexer 2
    | ('P' | 'p'), ('H' | 'h'), ('P' | 'p') -> make_long_tag lexer 3
    | '=', _, _ ->
      begin
        (* skip = *)
        let lexer = advance lexer 1 in
        let equal = Token.make TokenKind.Equal 1 [] [] in
        lexer, markup_text, Some (less_than_question_token, Some equal)
      end
    | _ ->
      lexer, markup_text, Some (less_than_question_token, None)
  in
  let rec aux lexer =
    let ch0 = peek_char lexer 0 in
    if ch0 = invalid && at_end lexer then
      (* It's not an error to run off the end of one of these. *)
      lexer, (make_markup_token lexer), None
    else if ch0 = '<' && peek_char lexer 1 = '?' then
      (* Found a beginning tag that delimits markup from the script *)
      make_markup_and_suffix lexer
    else
      aux (advance lexer 1)
  in
  aux lexer

let scan_markup lexer ~is_leading_section =
  let lexer = start_new_lexeme lexer in
  skip_to_end_of_markup lexer ~is_leading_section

let is_next_xhp_category_name lexer =
  let (lexer, _) = scan_leading_php_trivia lexer in
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
