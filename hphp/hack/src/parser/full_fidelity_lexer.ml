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
module Lexer : sig
  type t
  val make : SourceText.t -> t
  val start : t -> int
  val source : t -> SourceText.t
  val errors : t -> SyntaxError.t list
  val offset : t -> int

  val with_error : t -> string -> t
  val with_offset : t -> int -> t
  val with_offset_errors : t -> int -> SyntaxError.t list -> t
  val start_new_lexeme : t -> t
  val advance : t -> int -> t
end = struct

  type t = {
    text : SourceText.t;
    start : int;  (* Both start and offset are absolute offsets in the text. *)
    offset : int;
    errors : SyntaxError.t list
  }

  let make text =
    { text; start = 0; offset = 0; errors = [] }

  let start  x = x.start
  let source x = x.text
  let errors x = x.errors
  let offset x = x.offset

  let with_error lexer message =
    let error = SyntaxError.make lexer.start lexer.offset message in
    { lexer with errors = error :: lexer.errors }

  let with_offset lexer offset = {lexer with offset = offset}

  let with_offset_errors lexer offset errors = {
    lexer with offset = offset; errors = errors
  }

  let start_new_lexeme lexer =
    { lexer with start = lexer.offset }

  let advance lexer index =
    { lexer with offset = lexer.offset + index }
end

module WithToken(Token: Lexable_token_sig.LexableToken_S) = struct

module Trivia = Token.Trivia

type lexer = Lexer.t
type t = lexer

let make = Lexer.make
let start = Lexer.start
let source = Lexer.source
let errors = Lexer.errors
let offset = Lexer.offset
let with_error = Lexer.with_error
let with_offset = Lexer.with_offset
let start_new_lexeme = Lexer.start_new_lexeme
let advance = Lexer.advance
let with_offset_errors = Lexer.with_offset_errors

let start_offset = start
let end_offset = offset

let invalid = '\000'

let empty = make SourceText.empty

(* Housekeeping *)

let peek_char lexer index =
  let i = offset lexer + index in
  if i >= SourceText.length (source lexer) || i < 0 then invalid
  else SourceText.get (source lexer) i

let peek_string lexer size =
  let r = (SourceText.length (source lexer)) - offset lexer in
  if r < 0 then ""
  else if r < size then SourceText.sub (source lexer) (offset lexer) r
  else SourceText.sub (source lexer) (offset lexer) size

let match_string lexer s =
  s = peek_string lexer (String.length s)

let make_error_with_location (l : lexer) (msg : string) =
  SyntaxError.make (start l) (offset l) msg

let width lexer =
  (offset lexer) - (start lexer)

let current_text lexer =
  SourceText.sub (source lexer) (start lexer) (width lexer)

let current_text_at lexer length relative_start =
  SourceText.sub (source lexer) ((start lexer) + relative_start) length

let at_end lexer =
  (offset lexer) >= SourceText.length (source lexer)

let at_end_index lexer index =
  index >= SourceText.length (source lexer)

let remaining lexer =
  let r = (SourceText.length (source lexer)) - offset lexer in
  if r < 0 then 0 else r

let text_len (l : lexer) =
  SourceText.length (source l)

let peek (l : lexer) i =
  SourceText.get (source l) i

let peek_def (l: lexer) i ~def =
  if i >= SourceText.length (source l) then
    def
  else
    SourceText.get (source l) i

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

(* create a new lexer where the offset is advanced as
 * long as the predicate is true *)
let skip_while (l : lexer) (p : char -> bool) =
  let n = SourceText.length (source l) in
  let rec aux i =
  if i < n && p (peek l i) then aux (i + 1) else i in
  with_offset l (aux (offset l))

let skip_whitespace (l : lexer) =
  let is_space ch = (ch = '\t' || ch = ' ') in
  skip_while l is_space

let skip_to_end_of_line (l : lexer) =
  let not_newline ch = (ch <> '\n') in
  skip_while l not_newline

let skip_to_end_of_line_or_end_tag (l : lexer) =
  let n = text_len l in
  let peek_def i = if i < n then peek l i else invalid in
  let should_stop i =
    (i >= n) || begin
      let ch = peek l i in
      (is_newline ch) || (ch = '?' && peek_def (succ i) = '>')
    end in
  let i = ref (offset l) in
  while (not (should_stop !i)) do incr i done;
  with_offset l !i

let skip_name_end (l : lexer) =
  skip_while l is_name_letter

let skip_end_of_line lexer =
  match peek_char lexer 0 with
  | '\n' -> advance lexer 1
  | '\r' ->
    if (peek_char lexer 1) = '\n' then advance lexer 2 else advance lexer 1
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

let scan_with_underscores (l : lexer) accepted_char =
  let n = text_len l in
  let peek_def i = if i < n then peek l i else invalid in
  let rec aux i =
    if i >= n then i
    else let ch = peek l i in
      if accepted_char ch then aux (succ i)
      else if ch = ' ' && accepted_char (peek_def (succ i)) then
        aux (2 + i)
      else i in
  with_offset l (aux (offset l))

let scan_decimal_digits (l : lexer) =
  skip_while l is_decimal_digit

let scan_decimal_digits_with_underscores lexer =
  scan_with_underscores lexer is_decimal_digit

let scan_octal_digits (l : lexer) =
  skip_while l is_octal_digit

let scan_octal_digits_with_underscores (l : lexer) =
  scan_with_underscores l is_octal_digit

let scan_binary_digits_with_underscores (l : lexer) =
  scan_with_underscores l is_binary_digit

let scan_hexadecimal_digits (l : lexer) =
  skip_while l is_hexadecimal_digit

let scan_hexadecimal_digits_with_underscores (l : lexer) =
  scan_with_underscores l is_hexadecimal_digit

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
  let ch = peek_char lexer 1 in
  let lexer = if ch = '+' || ch = '-' then (advance lexer 2)
    else (advance lexer 1) in
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
        (* This is irritating - we only want to allow underscores for integer
        literals. Deferring the lexing with underscores here allows us to
        make sure we're not dealing with floats. *)
        let lexer_oct_with_underscores =
          scan_octal_digits_with_underscores lexer in
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


let scan_execution_string_literal (l : lexer) =
  (* TODO: PHP supports literals of the form `command` where the command
  is then executed as a shell command.  Hack does not support this.
  We should give an error if this feature is used in Hack, but we should lex
  it anyways to give a good error message.

  TODO: Can execution strings have embedded expressions like double-quoted
  strings?

  TODO: Are there any escape sequences in execution strings?

  TODO: Are there any illegal characters in execution strings?
  *)

  let n = SourceText.length (source l) in
  let peek = SourceText.get (source l) in

  let has_error0012 = ref false in
  let has_error0006 = ref false in

  let rec last_pos i =
    if i >= n then
      (has_error0012 := true; i)
    else begin
      let ch = peek i in
      match ch with
      | '\000' -> (has_error0006  := true; last_pos (1 + i))
      | '`'    -> (1 + i)
      | _      -> (last_pos (1 + i))
    end in

  let new_offset = last_pos (1 + (offset l)) in

  let new_errors =
    let err msg = make_error_with_location l msg in
    match (!has_error0006, !has_error0012) with
    | (true, true) -> (err SyntaxError.error0006 :: err SyntaxError.error0012 :: (errors l))
    | (true, false) -> (err SyntaxError.error0006 :: (errors l))
    | (false, true) -> (err SyntaxError.error0012 :: (errors l))
    | (false, false) -> (errors l) in

  let res = with_offset_errors l new_offset new_errors in
  (res, TokenKind.ExecutionString)

let scan_single_quote_string_literal (l : lexer) =
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

  let n = SourceText.length (source l) in
  let peek = SourceText.get (source l) in

  let has_error0012 = ref false in
  let has_error0006 = ref false in

  let rec stepper i =
    if i >= n then
      (has_error0012 := true; n - 1)
    else begin
      let ch = peek i in
      match ch with
      | '\000' -> (has_error0006 := true; stepper (1+i))
      | '\\' -> stepper (2+i)
      | '\'' -> (1+i)
      | _ -> stepper (1+i)
    end in

  let new_offset = stepper (1 + (offset l)) in

  let new_errors =
    let err msg = make_error_with_location l msg in
    match (!has_error0006, !has_error0012) with
    | (true, true) -> (err SyntaxError.error0006 :: err SyntaxError.error0012 :: (errors l))
    | (true, false) -> (err SyntaxError.error0006 :: (errors l))
    | (false, true) -> (err SyntaxError.error0012 :: (errors l))
    | (false, false) -> (errors l) in

  let res = with_offset_errors l new_offset new_errors in
  (res, TokenKind.SingleQuotedStringLiteral)

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
  if (peek_char lexer 2) = '{' then
    if (peek_char lexer 3) = '$' then
      (* We have a malformed unicode escape that contains a possible embedded
      expression. Eat the \u and keep on processing the embedded expression. *)
      (* TODO: Consider producing a warning for a malformed unicode escape. *)
      advance lexer 2
    else
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
    else
      (* We have a malformed unicode escape sequence. Bail out. *)
      (* TODO: Consider producing a warning for a malformed unicode escape. *)
      advance lexer 2

let skip_uninteresting_double_quote_string_characters (l : lexer) =
  let is_uninteresting ch =
    match ch with
    | '\000' | '"' | '\\' | '$' | '{' | '[' | ']' | '-'
    | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' -> false
    | ch -> not (is_name_nondigit ch)  in
  skip_while l is_uninteresting

let scan_integer_literal lexer =
  if (peek_char lexer 0) = '0' then
    match peek_char lexer 1 with
    | 'x' | 'X' -> scan_hex_literal (advance lexer 2)
    | 'b' | 'B' -> scan_binary_literal (advance lexer 2)
    | _ -> (scan_octal_digits_with_underscores lexer, TokenKind.OctalLiteral)
  else
    (scan_decimal_digits_with_underscores lexer, TokenKind.DecimalLiteral)

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
    match ch0 with
    | '\000' ->
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
    | '"' ->
      let kind =
        if is_heredoc then TokenKind.StringLiteralBody
        else TokenKind.DoubleQuotedStringLiteralTail in
      (advance lexer 1, kind)
    | '$' ->
      if is_name_nondigit (peek_char lexer 1) then scan_variable lexer
      else (advance lexer 1, TokenKind.Dollar)
    | '{' -> (advance lexer 1, TokenKind.LeftBrace)
    | '\\' -> begin
      match peek_char lexer 1 with
      (* In these cases we just skip the escape sequence and
       keep on scanning for special characters. *)
      | '\\' | '"' | '$' | 'e' | 'f' | 'n' | 'r' | 't' | 'v'
      (* Same in these cases; there might be more octal characters following but
         if there are, we'll just eat them as normal characters. *)
      | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' ->
        let lexer = advance lexer 2 in
        let lexer = skip_uninteresting_double_quote_string_characters lexer in
        (lexer, TokenKind.StringLiteralBody)
      | 'x' ->
        let lexer = scan_hexadecimal_escape lexer in
        let lexer = skip_uninteresting_double_quote_string_characters lexer in
        (lexer, TokenKind.StringLiteralBody)
      | 'u' ->
        let lexer = scan_unicode_escape lexer in
        let lexer = skip_uninteresting_double_quote_string_characters lexer in
        (lexer, TokenKind.StringLiteralBody)
      | '{' ->
        (* The rules for escaping open braces in Hack are bizarre. Suppose we
           have
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
    | _ ->
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
      end
    | '[' ->
      let lexer = advance lexer 1 in
      (lexer, TokenKind.LeftBracket)
    | ']' ->
      let lexer = advance lexer 1 in
      (lexer, TokenKind.RightBracket)
    | '-' ->
      if (peek_char lexer 1) = '>' then
        let lexer = advance lexer 2 in
        (lexer, TokenKind.MinusGreaterThan)
      else
        (* Nothing interesting here. Skip it and find the next
           interesting character. *)
        let lexer = advance lexer 1 in
        let lexer = skip_uninteresting_double_quote_string_characters lexer in
        (lexer, TokenKind.StringLiteralBody)
    | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ->
      let (lexer1, _) as literal = scan_integer_literal lexer in
      if errors lexer == errors lexer1 then literal else
        (* If we failed to scan a literal, do not interpret the literal *)
        (with_offset lexer (offset lexer1), TokenKind.StringLiteralBody)
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
      (source lexer) (offset lexer) (offset end_lexer - offset lexer) in
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

let rec scan_xhp_element_name ?(attribute=false) lexer =
  (* An XHP element name is a sequence of one or more XHP labels each separated
  by a single : or -.  Note that it is possible for an XHP element name to be
  followed immediately by a : or - that is the next token, so if we find
  a : or - not followed by a label, we need to terminate the token. *)
  let lexer = scan_xhp_label lexer in
  let ch0 = peek_char lexer 0 in
  let ch1 = peek_char lexer 1 in
  if (not attribute && ch0 = ':' || ch0 = '-') && is_name_nondigit ch1 then
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
     strings.  Embedded newlines are legal. *)
  let rec aux lexer offset =
    match peek_char lexer offset with
      | '\000' ->
        let lexer = advance lexer offset in
        if at_end lexer then
          let lexer = with_error lexer SyntaxError.error0012 in
          (lexer, TokenKind.XHPStringLiteral)
        else
          let lexer = with_error lexer SyntaxError.error0006 in
          aux lexer 1
      | '"' -> (advance lexer (offset + 1), TokenKind.XHPStringLiteral)
      | _ -> aux lexer (offset + 1) in
  aux lexer 1

(* Note that this does not scan an XHP body *)
let scan_xhp_token ?(attribute=false) lexer =
  (* TODO: HHVM requires that there be no trivia between < and name in an
     opening tag, but does allow trivia between </ and name in a closing tag.
     Consider allowing trivia in an opening tag. *)
  let ch0 = peek_char lexer 0 in
  if ch0 = invalid && at_end lexer then
    (lexer, TokenKind.EndOfFile)
  else if is_name_nondigit ch0 then
    scan_xhp_element_name ~attribute lexer
  else match ch0 with
  | '{' -> (advance lexer 1, TokenKind.LeftBrace)
  | '}' -> (advance lexer 1, TokenKind.RightBrace)
  | '=' -> (advance lexer 1, TokenKind.Equal)
  | '<' ->
    if (peek_char lexer 1) = '/' then
      (advance lexer 2, TokenKind.LessThanSlash)
    else
      (advance lexer 1, TokenKind.LessThan)
  | '"' -> scan_xhp_string_literal lexer
  | '/' ->
    if (peek_char lexer 1) = '>' then
      (advance lexer 2, TokenKind.SlashGreaterThan)
    else
      let lexer = with_error lexer SyntaxError.error0006 in
      (advance lexer 1, TokenKind.ErrorToken)
  | '>' -> (advance lexer 1, TokenKind.GreaterThan)
  | _ ->
    let lexer = with_error lexer SyntaxError.error0006 in
    (advance lexer 1, TokenKind.ErrorToken)

let scan_xhp_comment lexer =
  let rec aux lexer offset =
    let ch0 = peek_char lexer offset in
    let ch1 = peek_char lexer (offset + 1) in
    let ch2 = peek_char lexer (offset + 2) in
    match (ch0, ch1, ch2) with
    | ('\000', _, _) -> with_error (advance lexer offset) SyntaxError.error0014
    | ('-', '-', '>') -> (advance lexer (offset + 3))
    | _ -> aux lexer (offset + 1) in
  aux lexer 4

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
  let rec aux lexer offset =
    let ch = peek_char lexer offset in
    match ch with
      | '\000' ->
        let lexer = advance lexer offset in
        if at_end lexer then
          let lexer = with_error lexer SyntaxError.error0013 in
          lexer
        else
          let lexer = with_error lexer SyntaxError.error0006 in
          aux lexer 1
      | '\t' | ' ' | '\r' | '\n' | '{' | '}' | '<' -> advance lexer offset
      | _ -> aux lexer (offset + 1) in
  let ch0 = peek_char lexer 0 in
  match ch0 with
  | '\000' when at_end lexer -> (lexer, TokenKind.EndOfFile)
  | '{' -> (advance lexer 1, TokenKind.LeftBrace)
  | '}' -> (advance lexer 1, TokenKind.RightBrace)
  | '<' -> begin
    let ch1 = peek_char lexer 1 in
    let ch2 = peek_char lexer 2 in
    let ch3 = peek_char lexer 3 in
    match (ch1, ch2, ch3) with
    | ('!', '-', '-') -> (scan_xhp_comment lexer, TokenKind.XHPComment)
    | ('/', _, _) -> (advance lexer 2, TokenKind.LessThanSlash)
    | _ -> (advance lexer 1, TokenKind.LessThan)
    end
  | _ -> ((aux lexer 0), TokenKind.XHPBody)

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
  match ch1 with
  | '$' ->
    let ch2 = peek_char lexer 2 in
    if ch2 = '$' || is_name_nondigit ch2 then
      (advance lexer 1, TokenKind.Dollar) (* $$x or $$$*)
    else
      (advance lexer 2, TokenKind.DollarDollar) (* $$ *)
  | _ ->
    if is_name_nondigit ch1 then scan_variable lexer (* $x *)
    else (advance lexer 1, TokenKind.Dollar) (* $ *)

let scan_token in_type lexer =
  let ch0 = peek_char lexer 0 in
  match ch0 with
  | '[' -> (advance lexer 1, TokenKind.LeftBracket)
  | ']' -> (advance lexer 1, TokenKind.RightBracket)
  | '(' -> (advance lexer 1, TokenKind.LeftParen)
  | ')' -> (advance lexer 1, TokenKind.RightParen)
  | '{' -> (advance lexer 1, TokenKind.LeftBrace)
  | '}' -> (advance lexer 1, TokenKind.RightBrace)
  | '.' -> begin
    match peek_char lexer 1 with
    | '=' -> (advance lexer 2, TokenKind.DotEqual)
    | '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ->
      scan_after_decimal_point lexer
    | '.' ->
      if (peek_char lexer 2) = '.' then (advance lexer 3, TokenKind.DotDotDot)
      else (advance lexer 1, TokenKind.Dot)
    | _ -> (advance lexer 1, TokenKind.Dot)
    end
  | '-' -> begin
    match peek_char lexer 1 with
    | '=' -> (advance lexer 2, TokenKind.MinusEqual)
    | '-' -> (advance lexer 2, TokenKind.MinusMinus)
    | '>' -> (advance lexer 2, TokenKind.MinusGreaterThan)
    | _ -> (advance lexer 1, TokenKind.Minus)
    end
  | '+' -> begin
    match peek_char lexer 1 with
    | '=' -> (advance lexer 2, TokenKind.PlusEqual)
    | '+' -> (advance lexer 2, TokenKind.PlusPlus)
    | _ -> (advance lexer 1, TokenKind.Plus)
    end
  | '*' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('=', _) -> (advance lexer 2, TokenKind.StarEqual)
    | ('*', '=') -> (advance lexer 3, TokenKind.StarStarEqual)
    | ('*', _) -> (advance lexer 2, TokenKind.StarStar)
    | _ -> (advance lexer 1, TokenKind.Star)
    end
  | '~' -> (advance lexer 1, TokenKind.Tilde)
  | '!' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('=', '=') -> (advance lexer 3, TokenKind.ExclamationEqualEqual)
    | ('=', _) -> (advance lexer 2, TokenKind.ExclamationEqual)
    | _ -> (advance lexer 1, TokenKind.Exclamation)
    end
  | '$' -> scan_dollar_token lexer
  | '/' ->
    if (peek_char lexer 1) = '=' then (advance lexer 2, TokenKind.SlashEqual)
    else (advance lexer 1, TokenKind.Slash)
  | '%' ->
    if (peek_char lexer 1) = '=' then (advance lexer 2, TokenKind.PercentEqual)
    else (advance lexer 1, TokenKind.Percent)
  | '<' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('<', '<') -> scan_docstring_literal lexer
    | ('<', '=') -> (advance lexer 3, TokenKind.LessThanLessThanEqual)
    (* TODO: We lex and parse the spaceship operator.
       TODO: This is not in the spec at present.  We should either make it an
       TODO: error, or add it to the specification. *)
    | ('=', '>') -> (advance lexer 3, TokenKind.LessThanEqualGreaterThan)
    | ('>', _) -> (advance lexer 2, TokenKind.LessThanGreaterThan)
    | ('=', _) -> (advance lexer 2, TokenKind.LessThanEqual)
    | ('<', _) -> (advance lexer 2, TokenKind.LessThanLessThan)
    | _ -> (advance lexer 1, TokenKind.LessThan)
    end
  | '>' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('>', '=') -> (advance lexer 3, TokenKind.GreaterThanGreaterThanEqual)
    | ('>', _) ->
    (* If we are parsing a generic type argument list then we might be
       at the >> in List<List<int>>.  In that case we want to lex two
       >'s, not one >>. *)
      if in_type then
        (advance lexer 1, TokenKind.GreaterThan)
      else
        (advance lexer 2, TokenKind.GreaterThanGreaterThan)
    | ('=', _) -> (advance lexer 2, TokenKind.GreaterThanEqual)
    | _ -> (advance lexer 1, TokenKind.GreaterThan)
    end
  | '=' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('=', '=') -> (advance lexer 3, TokenKind.EqualEqualEqual)
    | ('=', '>') -> (advance lexer 3, TokenKind.EqualEqualGreaterThan)
    | ('=', _) -> (advance lexer 2, TokenKind.EqualEqual)
    | ('>', _) -> (advance lexer 2, TokenKind.EqualGreaterThan)
    | _ -> (advance lexer 1, TokenKind.Equal)
    end
  | '^' ->
    if (peek_char lexer 1) = '=' then (advance lexer 2, TokenKind.CaratEqual)
    else (advance lexer 1, TokenKind.Carat)
  | '|' -> begin
    match peek_char lexer 1 with
    | '=' -> (advance lexer 2, TokenKind.BarEqual)
    | '>' -> (advance lexer 2, TokenKind.BarGreaterThan)
    | '|' -> (advance lexer 2, TokenKind.BarBar)
    | _ -> (advance lexer 1, TokenKind.Bar)
    end
  | '&' -> begin
    match peek_char lexer 1 with
    | '=' -> (advance lexer 2, TokenKind.AmpersandEqual)
    | '&' -> (advance lexer 2, TokenKind.AmpersandAmpersand)
    | _ -> (advance lexer 1, TokenKind.Ampersand)
    end
  | '?' -> begin
    match (peek_char lexer 1, peek_char lexer 2) with
    | ('-', '>') -> (advance lexer 3, TokenKind.QuestionMinusGreaterThan)
    | ('?', _) -> (advance lexer 2, TokenKind.QuestionQuestion)
    | ('>', _) -> (advance lexer 2, TokenKind.QuestionGreaterThan)
    | _ -> (advance lexer 1, TokenKind.Question)
    end
  | ':' ->
    if (peek_char lexer 1) = ':' then (advance lexer 2, TokenKind.ColonColon)
    else (advance lexer 1, TokenKind.Colon)
  | ';' -> (advance lexer 1, TokenKind.Semicolon)
  | ',' -> (advance lexer 1, TokenKind.Comma)
  | '@' -> (advance lexer 1, TokenKind.At)
  | '0' -> begin
    match peek_char lexer 1 with
    | 'x' | 'X' -> scan_hex_literal (advance lexer 2)
    | 'b' | 'B' -> scan_binary_literal (advance lexer 2)
    | _ -> scan_octal_or_float lexer
    end
  | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ->
    scan_decimal_or_float lexer
  | '`' -> scan_execution_string_literal lexer
  | '\'' -> scan_single_quote_string_literal lexer
  | '"' -> scan_double_quote_string_literal_from_start lexer
  | '\\' -> scan_qualified_name lexer
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
  match peek_char lexer 0 with
  | '\r' ->
    let w = if peek_char lexer 1 = '\n' then 2 else 1 in
    advance lexer w, Trivia.make_eol (source lexer) (start lexer) w
  | '\n' -> (advance lexer 1, Trivia.make_eol (source lexer) (start lexer) 1)
  | _ -> failwith "scan_end_of_line called while not on end of line!"

let scan_whitespace lexer =
  let lexer = skip_whitespace lexer in
  (lexer, Trivia.make_whitespace (source lexer) (start lexer) (width lexer))

let scan_hash_comment lexer =
  let lexer = skip_to_end_of_line lexer in
  let c = Trivia.make_single_line_comment
    (source lexer) (start lexer) (width lexer) in
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
  let remainder = offset lexer - offset lexer_ws in
  let c =
    if remainder = 11 && peek_string lexer_ws 11 = "FALLTHROUGH" then
      Trivia.make_fallthrough (source lexer) (start lexer) w
    else if remainder >= 6 && peek_string lexer_ws 6 = "UNSAFE" then
      Trivia.make_unsafe (source lexer) (start lexer) w
    else
      Trivia.make_single_line_comment (source lexer) (start lexer) w in
  (lexer, c)

let skip_to_end_of_delimited_comment lexer =
  let rec aux lexer offset =
    let ch0 = peek_char lexer offset in
    if ch0 = invalid then
      let lexer = advance lexer offset in
      if at_end lexer then
        with_error lexer SyntaxError.error0007
      else
        (* TODO: Do we want to give a warning for an embedded zero char
        inside a comment? *)
        aux lexer 1
    else if ch0 = '*' && (peek_char lexer (offset + 1)) = '/' then
      advance lexer (offset + 2)
    else aux lexer (offset + 1) in
  aux lexer 0

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
      Trivia.make_unsafe_expression (source lexer) (start lexer) w
    else if match_string lexer_ws "HH_FIXME" then
      Trivia.make_fix_me (source lexer) (start lexer) w
    else if match_string lexer_ws "HH_IGNORE_ERROR" then
      Trivia.make_ignore_error (source lexer) (start lexer) w
    else
      Trivia.make_delimited_comment (source lexer) (start lexer) w in
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
  match peek_char lexer 0 with
  | '#' ->
    let (lexer, c) = scan_hash_comment lexer in
    (lexer, Some c)
  | '/' -> begin
    match peek_char lexer 1 with
    | '/' ->
      let (lexer, c) = scan_single_line_comment lexer in
      (lexer, Some c)
    | '*' ->
      let (lexer, c) = scan_delimited_comment lexer in
      (lexer, Some c)
    | _ -> (lexer, None)
    end
  | ' ' | '\t' ->
    let (lexer, w) = scan_whitespace lexer in
    (lexer, Some w)
  | '\r' | '\n' ->
    let (lexer, e) = scan_end_of_line lexer in
    (lexer, Some e)
  | _ -> (* Not trivia *) (lexer, None)

let scan_xhp_trivia lexer =
  (* TODO: Should XHP comments <!-- --> be their own thing, or a kind of
  trivia associated with a token? Right now they are the former. *)
  let lexer = start_new_lexeme lexer in
  match peek_char lexer 0 with
  | ' ' | '\t' ->
    let (lexer, w) = scan_whitespace lexer in
    (lexer, Some w)
  | '\r' | '\n' ->
    let (lexer, e) = scan_end_of_line lexer in
    (lexer, Some e)
  | _ -> (* Not trivia *) (lexer, None)

let scan_xhp_colon_trivia (lexer : t) : t * Trivia.t =
  (* TODO(T21789285): Take this mess out *)
  let lexer = start_new_lexeme lexer in
  let rec aux lexer index ch0 =
    let ch1 = peek_def lexer (succ index) ~def:invalid in
    match ch0, ch1 with
    | '>', _
    | '/', '>'
    | '\n', _
    | '\r', '\n'
    | '}', _
      -> begin
        let lexer' = with_offset lexer index in
        lexer', Trivia.make_extra_token_error
          (source lexer) (start lexer) (width lexer')
      end
    | _ -> aux lexer (succ index) ch1
  in
  (* The only valid case to use this scanner is when a name was interrupted by
   * a colon *)
  let () = assert (peek_char lexer 0 = ':') in
  let lexer, trivia = aux lexer (offset lexer) ':' in
  with_error lexer SyntaxError.error1002, trivia

(*
We divide trivia into "leading" and "trailing" trivia of an associated
token. This means that we must find a dividing line between the trailing trivia
following one token and the leading trivia of the following token. Plainly
we need only find this line while scanning trailing trivia. The heuristics
we use are:
* The first newline trivia encountered is the last trailing trivia.
* The newline which follows a // or # comment is not part of the comment
  but does terminate the trailing trivia.
* A pragma to turn checks off (HH_FIXME, HH_IGNORE_ERROR and UNSAFE_EXPR) is
* always a leading trivia.
*)

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
    let (lexer1, trivia) = scanner lexer in
    match trivia with
    | None -> (lexer1, acc)
    | Some t -> begin
      match Trivia.kind t with
      | TriviaKind.EndOfLine -> (lexer1, t :: acc)
      | TriviaKind.FixMe
      | TriviaKind.IgnoreError
      | TriviaKind.UnsafeExpression
        -> (lexer, acc)
      | _ -> aux lexer1 (t :: acc)
      end in
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
  | "eval" | "isset" | "unset" | "empty" | "const" | "new"
  | "and"  | "or"    | "xor"  | "as" | "print" | "throw"
  | "true" | "false" | "null" | "array" | "instanceof"
  | "trait" | "class" | "interface" -> lower
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
  (lexer, Token.make kind (source lexer) (start lexer) w leading trailing)

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
    Printf.kprintf failwith
      "failed to make progress at %d\n" (offset lexer)
  end

let scan_next_token as_name scanner lexer =
  let tokenizer = scan_token_and_trivia scanner as_name in
  scan_assert_progress tokenizer lexer

let scan_next_token_as_name = scan_next_token true
let scan_next_token_as_keyword = scan_next_token false

(* Entrypoints *)
(* TODO: Instead of passing Boolean flags, create a flags enum? *)

(* This function is the inner loop of the parser, is pure, and
is frequently called twice in a row with the same lexer due to the
design of the parser. We get a big win by memoizing it. *)


let next_token = (* takes a lexer, returns a (lexer, token) *)
  let next_token_cache = Little_cache.make empty
    (empty, Token.make TokenKind.EndOfFile SourceText.empty 0 0 [] []) in
  Little_cache.memoize next_token_cache
    (scan_next_token_as_keyword scan_token_outside_type)

let next_token_no_trailing lexer =
  let tokenizer lexer =
    let (lexer, kind, w, leading) =
      scan_token_and_leading_trivia scan_token_outside_type false lexer in
    (lexer, Token.make kind (source lexer) (start lexer) w leading []) in
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
  let token = Token.make kind (source lexer) (start lexer) w [] trailing in
  (lexer, token)

let next_docstring_header lexer =
  (* We're at the beginning of a heredoc string literal. Scan leading
     trivia but not trailing trivia. *)
  let (lexer, leading) = scan_leading_php_trivia lexer in
  let lexer = start_new_lexeme lexer in
  let (lexer, name, _) = scan_docstring_header lexer in
  let w = width lexer in
  let token = Token.make TokenKind.HeredocStringLiteralHead
    (source lexer) (start lexer) w leading [] in
  (lexer, token, name)

let next_token_as_name lexer =
  scan_next_token_as_name scan_token_outside_type lexer

let next_token_in_type lexer =
  scan_next_token_as_keyword scan_token_inside_type lexer

let next_xhp_element_token ~no_trailing ~attribute lexer =
  (* XHP elements have whitespace, newlines and Hack comments. *)
  let tokenizer lexer =
    let (lexer, kind, w, leading) =
      scan_token_and_leading_trivia (scan_xhp_token ~attribute) true lexer in
    (* We do not scan trivia after an XHPOpen's >. If that is the beginning of
       an XHP body then we want any whitespace or newlines to be leading trivia
       of the body token. *)
    let (_, token1) = next_token lexer in
    match kind with
    | TokenKind.GreaterThan when no_trailing ->
      (lexer, Token.make kind (source lexer) (start lexer) w leading [])
    | TokenKind.XHPElementName
      when attribute && Token.kind token1 = TokenKind.Colon ->
      (* TODO(T21789285): Take this hack out when illtyped xhp is gone *)
      let (lexer, dropped) = scan_xhp_colon_trivia lexer in
      let (lexer, trailing) = scan_trailing_xhp_trivia lexer in
      (lexer, Token.make kind
        (source lexer) (start lexer) w leading (dropped :: trailing))
    | _ ->
      let (lexer, trailing) = scan_trailing_php_trivia lexer in
      (lexer, Token.make
          kind (source lexer) (start lexer) w leading trailing) in
  let (lexer, token) = scan_assert_progress tokenizer lexer in
  let token_width = Token.width token in
  let trailing_width = Token.trailing_width token in
  let token_start_offset = (offset lexer) - trailing_width - token_width in
  let token_text = SourceText.sub (source lexer) token_start_offset token_width in
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
    (lexer, Token.make kind (source lexer) (start lexer) w leading trailing) in
  scan_assert_progress scanner lexer

let next_xhp_class_name lexer =
  scan_token_and_trivia scan_xhp_class_name false lexer

let next_xhp_name lexer =
  scan_token_and_trivia scan_xhp_element_name false lexer

let make_markup_token lexer =
  Token.make TokenKind.Markup (source lexer) (start lexer) (width lexer) [] []

let skip_to_end_of_markup lexer ~is_leading_section =
  let make_markup_and_suffix lexer =
    let markup_text = make_markup_token lexer in
    let less_than_question_token =
      Token.make TokenKind.LessThanQuestion (source lexer) (start lexer) 2 [] []
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
      let name = Token.make TokenKind.Name
        (source lexer) (start lexer) size [] trailing in
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
        let equal = Token.make TokenKind.Equal
          (source lexer) (start lexer) 1 [] [] in
        lexer, markup_text, Some (less_than_question_token, Some equal)
      end
    | _ ->
      lexer, markup_text, Some (less_than_question_token, None)
  in
  let rec aux lexer index =
    (* It's not an error to run off the end of one of these. *)
    if at_end_index lexer index then
      let lexer' = with_offset lexer index in
      lexer', (make_markup_token lexer'), None
    else begin
      let ch = peek lexer index in
      if ch = '<' && peek_def lexer (succ index) ~def:'\x00' = '?' then
        (* Found a beginning tag that delimits markup from the script *)
        make_markup_and_suffix (with_offset lexer index)
      else
        aux lexer (succ index)
    end
  in
  aux lexer (offset lexer)

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

end (* WithToken *)
